//
// MultiDialer.h - 多账号同时拨号
// Created by bad_g on 2025/12/02.
//

#ifndef ESURFINGCLIENT_MULTIDIALER_H
#define ESURFINGCLIENT_MULTIDIALER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ConfigParser.h"

/**
 * 拨号器实例状态
 */
typedef struct {
    char* usr;                  // 用户名
    char* pwd;                  // 密码
    char* chn;                  // 通道
    char* bindInterface;        // 绑定的网卡接口/IP
    
    // 会话状态
    char* clientId;
    char* algoId;
    char* macAddress;
    char* ticket;
    char* userIp;
    char* acIp;
    
    char* schoolId;
    char* domain;
    char* area;
    char* ticketUrl;
    char* authUrl;
    
    char* keepRetry;
    char* keepUrl;
    char* termUrl;
    
    int isLogged;
    int isInitialized;
    int isRunning;
    
    long long authTime;
    long long tick;
    
    int instanceId;             // 实例ID
} DialerInstance;

/**
 * 初始化多拨号器
 * @param config 配置结构指针
 * @return 成功返回0，失败返回-1
 */
int initMultiDialer(const Config* config);

/**
 * 启动多拨号器
 * @return 成功返回0，失败返回-1
 */
int startMultiDialer(void);

/**
 * 停止多拨号器
 */
void stopMultiDialer(void);

/**
 * 清理多拨号器资源
 */
void cleanupMultiDialer(void);

/**
 * 获取拨号器实例数量
 * @return 实例数量
 */
int getDialerInstanceCount(void);

/**
 * 获取指定拨号器实例
 * @param index 实例索引
 * @return 实例指针，失败返回NULL
 */
DialerInstance* getDialerInstance(int index);

#ifdef __cplusplus
}
#endif

#endif //ESURFINGCLIENT_MULTIDIALER_H
