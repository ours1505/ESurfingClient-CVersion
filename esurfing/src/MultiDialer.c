//
// MultiDialer.c - 多账号同时拨号实现
// Created by bad_g on 2025/12/02.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "headFiles/MultiDialer.h"
#include "headFiles/ConfigParser.h"
#include "headFiles/utils/Logger.h"
#include "headFiles/utils/PlatformUtils.h"

#ifdef _WIN32
    #include <windows.h>
    #include <process.h>
#else
    #include <pthread.h>
    #include <unistd.h>
#endif

// 全局拨号器实例数组
static DialerInstance* g_instances = NULL;
static int g_instanceCount = 0;
static int g_multiDialerRunning = 0;

#ifdef _WIN32
static HANDLE* g_threads = NULL;
#else
static pthread_t* g_threads = NULL;
#endif

// 前向声明线程函数中需要的函数
extern void initChannel(int choose);
extern void initConstants(void);
extern void cipherFactoryDestroy(void);
extern void sessionFree(void);

// 实例线程运行函数
static void instanceRun(DialerInstance* instance);
static void instanceAuthorization(DialerInstance* instance);
static void instanceHeartbeat(DialerInstance* instance);
static void instanceTerm(DialerInstance* instance);

// 初始化单个拨号器实例
static int initDialerInstance(DialerInstance* instance, const AccountConfig* config, int id)
{
    memset(instance, 0, sizeof(DialerInstance));
    
    instance->instanceId = id;
    instance->usr = strdup(config->username);
    instance->pwd = strdup(config->password);
    
    if (strlen(config->bind_interface) > 0)
    {
        instance->bindInterface = strdup(config->bind_interface);
    }
    
    if (strlen(config->channel) > 0)
    {
        instance->chn = strdup(config->channel);
    }
    else
    {
        instance->chn = strdup("phone");
    }
    
    instance->algoId = strdup("00000000-0000-0000-0000-000000000000");
    setClientId(&instance->clientId);
    instance->macAddress = randomMacAddress();
    
    instance->isRunning = 1;
    instance->isLogged = 0;
    instance->isInitialized = 0;
    instance->authTime = 0;
    instance->tick = 0;
    
    return 0;
}

// 清理单个拨号器实例
static void cleanupDialerInstance(DialerInstance* instance)
{
    if (!instance) return;
    
    if (instance->usr) { free(instance->usr); instance->usr = NULL; }
    if (instance->pwd) { free(instance->pwd); instance->pwd = NULL; }
    if (instance->chn) { free(instance->chn); instance->chn = NULL; }
    if (instance->bindInterface) { free(instance->bindInterface); instance->bindInterface = NULL; }
    
    if (instance->clientId) { free(instance->clientId); instance->clientId = NULL; }
    if (instance->algoId) { free(instance->algoId); instance->algoId = NULL; }
    if (instance->macAddress) { free(instance->macAddress); instance->macAddress = NULL; }
    if (instance->ticket) { free(instance->ticket); instance->ticket = NULL; }
    if (instance->userIp) { free(instance->userIp); instance->userIp = NULL; }
    if (instance->acIp) { free(instance->acIp); instance->acIp = NULL; }
    
    if (instance->schoolId) { free(instance->schoolId); instance->schoolId = NULL; }
    if (instance->domain) { free(instance->domain); instance->domain = NULL; }
    if (instance->area) { free(instance->area); instance->area = NULL; }
    if (instance->ticketUrl) { free(instance->ticketUrl); instance->ticketUrl = NULL; }
    if (instance->authUrl) { free(instance->authUrl); instance->authUrl = NULL; }
    
    if (instance->keepRetry) { free(instance->keepRetry); instance->keepRetry = NULL; }
    if (instance->keepUrl) { free(instance->keepUrl); instance->keepUrl = NULL; }
    if (instance->termUrl) { free(instance->termUrl); instance->termUrl = NULL; }
}

// 线程入口函数
#ifdef _WIN32
static unsigned __stdcall dialerThreadFunc(void* arg)
#else
static void* dialerThreadFunc(void* arg)
#endif
{
    DialerInstance* instance = (DialerInstance*)arg;
    
    LOG_INFO("[实例 %d] 启动拨号线程，用户: %s, 绑定接口: %s", 
             instance->instanceId, 
             instance->usr,
             instance->bindInterface ? instance->bindInterface : "默认");
    
    // 等待一下让主线程完成
    sleepMilliseconds(1000 + instance->instanceId * 500);
    
    while (instance->isRunning && g_multiDialerRunning)
    {
        // 检查是否需要重新认证
        if (currentTimeMillis() - instance->authTime >= 172200000 && instance->authTime != 0)
        {
            LOG_WARN("[实例 %d] 已登录 2870 分钟，正在重新进行认证", instance->instanceId);
            if (instance->isInitialized && instance->isLogged)
            {
                instanceTerm(instance);
            }
            instance->authTime = 0;
            instance->isInitialized = 0;
            sleepMilliseconds(5000);
        }
        
        instanceRun(instance);
    }
    
    LOG_INFO("[实例 %d] 拨号线程结束", instance->instanceId);
    
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

int initMultiDialer(const Config* config)
{
    if (!config || config->account_count <= 0)
    {
        LOG_ERROR("无效的配置");
        return -1;
    }
    
    g_instanceCount = config->account_count;
    g_instances = calloc(g_instanceCount, sizeof(DialerInstance));
    if (!g_instances)
    {
        LOG_ERROR("分配拨号器实例内存失败");
        return -1;
    }
    
    for (int i = 0; i < g_instanceCount; i++)
    {
        if (initDialerInstance(&g_instances[i], &config->accounts[i], i + 1) != 0)
        {
            LOG_ERROR("初始化拨号器实例 %d 失败", i + 1);
            cleanupMultiDialer();
            return -1;
        }
    }
    
    LOG_INFO("成功初始化 %d 个拨号器实例", g_instanceCount);
    return 0;
}

int startMultiDialer(void)
{
    if (!g_instances || g_instanceCount <= 0)
    {
        LOG_ERROR("拨号器未初始化");
        return -1;
    }
    
    g_multiDialerRunning = 1;
    
#ifdef _WIN32
    g_threads = calloc(g_instanceCount, sizeof(HANDLE));
    if (!g_threads)
    {
        LOG_ERROR("分配线程句柄内存失败");
        return -1;
    }
    
    for (int i = 0; i < g_instanceCount; i++)
    {
        g_threads[i] = (HANDLE)_beginthreadex(NULL, 0, dialerThreadFunc, &g_instances[i], 0, NULL);
        if (g_threads[i] == 0)
        {
            LOG_ERROR("创建线程 %d 失败", i + 1);
            return -1;
        }
    }
#else
    g_threads = calloc(g_instanceCount, sizeof(pthread_t));
    if (!g_threads)
    {
        LOG_ERROR("分配线程内存失败");
        return -1;
    }
    
    for (int i = 0; i < g_instanceCount; i++)
    {
        if (pthread_create(&g_threads[i], NULL, dialerThreadFunc, &g_instances[i]) != 0)
        {
            LOG_ERROR("创建线程 %d 失败", i + 1);
            return -1;
        }
    }
#endif
    
    LOG_INFO("已启动 %d 个拨号线程", g_instanceCount);
    return 0;
}

void stopMultiDialer(void)
{
    g_multiDialerRunning = 0;
    
    if (g_instances)
    {
        for (int i = 0; i < g_instanceCount; i++)
        {
            g_instances[i].isRunning = 0;
        }
    }
    
    // 等待所有线程结束
    if (g_threads)
    {
#ifdef _WIN32
        WaitForMultipleObjects(g_instanceCount, g_threads, TRUE, 30000);
        for (int i = 0; i < g_instanceCount; i++)
        {
            if (g_threads[i])
            {
                CloseHandle(g_threads[i]);
            }
        }
#else
        for (int i = 0; i < g_instanceCount; i++)
        {
            pthread_join(g_threads[i], NULL);
        }
#endif
        free(g_threads);
        g_threads = NULL;
    }
    
    LOG_INFO("所有拨号线程已停止");
}

void cleanupMultiDialer(void)
{
    stopMultiDialer();
    
    if (g_instances)
    {
        for (int i = 0; i < g_instanceCount; i++)
        {
            cleanupDialerInstance(&g_instances[i]);
        }
        free(g_instances);
        g_instances = NULL;
    }
    
    g_instanceCount = 0;
}

int getDialerInstanceCount(void)
{
    return g_instanceCount;
}

DialerInstance* getDialerInstance(int index)
{
    if (index < 0 || index >= g_instanceCount)
    {
        return NULL;
    }
    return &g_instances[index];
}

// ============ 实例级别的网络操作 ============

// 这些函数需要在实际使用时与主客户端逻辑集成
// 这里提供基本的框架

static void instanceRun(DialerInstance* instance)
{
    // 这里需要调用实例特定的网络状态检查和认证逻辑
    // 具体实现需要修改 NetworkStatus 和 Client 模块以支持绑定接口
    
    // 简化版本：直接使用全局函数但传入实例上下文
    // 实际生产中需要更复杂的重构
    
    sleepMilliseconds(1000);
}

static void instanceAuthorization(DialerInstance* instance)
{
    (void)instance;
    // TODO: 实现实例级别的认证
}

static void instanceHeartbeat(DialerInstance* instance)
{
    (void)instance;
    // TODO: 实现实例级别的心跳
}

static void instanceTerm(DialerInstance* instance)
{
    (void)instance;
    // TODO: 实现实例级别的登出
}
