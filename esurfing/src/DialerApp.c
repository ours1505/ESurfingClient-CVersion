#include <locale.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "headFiles/Client.h"
#include "headFiles/Constants.h"
#include "headFiles/Options.h"
#include "headFiles/Session.h"
#include "headFiles/States.h"
#include "headFiles/ConfigParser.h"
#include "headFiles/MultiDialer.h"
#include "headFiles/cipher/CipherInterface.h"
#include "headFiles/utils/Logger.h"
#include "headFiles/utils/PlatformUtils.h"
#include "headFiles/utils/Shutdown.h"

void printUsage(const char* programName)
{
    printf("ESurfing Client - 电信天翼校园网认证客户端\n\n");
    printf("用法:\n");
    printf("  单账号模式: %s -u <用户名> -p <密码> [-c<通道>] [-e<网卡接口>] [-d] [-s]\n", programName);
    printf("  多账号模式: %s -f <配置文件> [-d] [-s]\n\n", programName);
    printf("选项:\n");
    printf("  -u <用户名>      登录用户名\n");
    printf("  -p <密码>        登录密码\n");
    printf("  -c <通道>        通道类型 (pc/phone)，默认为 phone\n");
    printf("  -e <网卡接口>    绑定的网卡接口名或IP地址 (如 eth0 或 192.168.1.100)\n");
    printf("  -f <配置文件>    JSON配置文件路径，用于多账号同时拨号\n");
    printf("  -d               开启调试模式\n");
    printf("  -s               小容量设备模式\n");
    printf("  -h               显示此帮助信息\n\n");
    printf("配置文件格式 (JSON):\n");
    printf("[\n");
    printf("  {\n");
    printf("    \"username\": \"10001234\",\n");
    printf("    \"password\": \"12345678\",\n");
    printf("    \"bind_interface\": \"eth1\",\n");
    printf("    \"channel\": \"android\"\n");
    printf("  },\n");
    printf("  {\n");
    printf("    \"username\": \"10005678\",\n");
    printf("    \"password\": \"87654321\",\n");
    printf("    \"bind_interface\": \"eth2\",\n");
    printf("    \"channel\": \"linux\"\n");
    printf("  }\n");
    printf("]\n");
}

int main(const int argc, char* argv[]) {
    int opt;
    int username = 0;
    int password = 0;
    int channel = 0;

#ifdef _WIN32
    system("chcp 65001 > nul");
#endif

    while ((opt = getopt(argc, argv, "u:p:c::e::f:dsh")) != -1)
    {
        switch (opt)
        {
        case 'u':
            username = 1;
            usr = optarg;
            break;
        case 'p':
            password = 1;
            pwd = optarg;
            break;
        case 'c':
            channel = 1;
            chn = optarg;
            break;
        case 'e':
            bindInterface = optarg;
            break;
        case 'f':
            configFile = optarg;
            isMultiMode = 1;
            break;
        case 'd':
            isDebug = 1;
            break;
        case 's':
            isSmallDevice = 1;
            break;
        case 'h':
            printUsage(argv[0]);
            return 0;
        case '?':
            printf("参数错误: %c\n", optopt);
            printUsage(argv[0]);
            return 1;
        default:
            printf("未知错误\n");
        }
    }
    loggerInit();

    // 多账号配置文件模式
    if (isMultiMode && configFile)
    {
        LOG_INFO("使用配置文件模式: %s", configFile);
        
        Config config;
        if (parseConfigFile(configFile, &config) != 0)
        {
            LOG_FATAL("解析配置文件失败");
            shut(0);
            return 1;
        }
        
        if (isDebug)
        {
            printConfig(&config);
        }
        
        if (isSmallDevice)
        {
            LOG_DEBUG("小容量设备模式已开启");
        }
        
        LOG_INFO("程序启动中 (多账号模式，共 %d 个账号)", config.account_count);
        sleepMilliseconds(3000);
        
        initShutdown();
        
        if (initMultiDialer(&config) != 0)
        {
            LOG_FATAL("初始化多拨号器失败");
            freeConfig(&config);
            shut(0);
            return 1;
        }
        
        if (startMultiDialer() != 0)
        {
            LOG_FATAL("启动多拨号器失败");
            cleanupMultiDialer();
            freeConfig(&config);
            shut(0);
            return 1;
        }
        
        // 主线程等待，直到收到退出信号
        while (getDialerInstanceCount() > 0)
        {
            int allStopped = 1;
            for (int i = 0; i < getDialerInstanceCount(); i++)
            {
                DialerInstance* inst = getDialerInstance(i);
                if (inst && inst->isRunning)
                {
                    allStopped = 0;
                    break;
                }
            }
            if (allStopped) break;
            sleepMilliseconds(1000);
        }
        
        cleanupMultiDialer();
        freeConfig(&config);
    }
    // 单账号命令行模式
    else if (username && password)
    {
        LOG_DEBUG("用户名: %s", usr);
        LOG_DEBUG("密码: %s", pwd);
        LOG_DEBUG("通道: %s", chn ? chn : "默认(phone)");
        LOG_DEBUG("绑定接口: %s", bindInterface ? bindInterface : "默认");
        if (isSmallDevice)
        {
            LOG_DEBUG("小容量设备模式已开启");
        }
        if (channel)
        {
            if (strcmp(chn, "pc") == 0)
            {
                initChannel(1);
            }
            else if (strcmp(chn, "phone") == 0)
            {
                initChannel(2);
            }
            else
            {
                LOG_FATAL("通道参数错误");
                LOG_FATAL("请使用正确的参数运行程序");
                LOG_FATAL("格式: ESurfingClient -u <用户名> -p <密码> -c<通道>");
                shut(0);
                return 0;
            }
        }
        else
        {
            initChannel(2);
        }
        LOG_INFO("程序启动中");
        sleepMilliseconds(5000);
        isRunning = 1;
        initShutdown();
        initConstants();
        refreshStates();
        while (isRunning)
        {
            if (currentTimeMillis() - authTime >= 172200000 && authTime != 0)
            {
                LOG_DEBUG("当前时间戳: %lld", currentTimeMillis());
                LOG_WARN("已登录 2870 分钟(1 天 23 小时 50 分钟)，为避免被远程服务器踢下线，正在重新进行认证");
                if (isInitialized)
                {
                    if (isLogged)
                    {
                        term();
                    }
                    cipherFactoryDestroy();
                    sessionFree();
                }
                authTime = 0;
                sleepMilliseconds(5000);
                initConstants();
                refreshStates();
            }
            run();
        }
    }
    else
    {
        LOG_FATAL("请使用正确的参数运行程序");
        printUsage(argv[0]);
    }
    shut(0);
}