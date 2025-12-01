//
// ConfigParser.h - JSON配置文件解析
// Created by bad_g on 2025/12/02.
//

#ifndef ESURFINGCLIENT_CONFIGPARSER_H
#define ESURFINGCLIENT_CONFIGPARSER_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_ACCOUNTS 16
#define MAX_STRING_LEN 256

/**
 * 账号配置结构
 */
typedef struct {
    char username[MAX_STRING_LEN];
    char password[MAX_STRING_LEN];
    char bind_interface[MAX_STRING_LEN];  // 绑定的网卡接口名或IP
    char channel[MAX_STRING_LEN];          // 通道: pc, phone, android, linux 等
} AccountConfig;

/**
 * 配置结构
 */
typedef struct {
    AccountConfig accounts[MAX_ACCOUNTS];
    int account_count;
} Config;

/**
 * 解析配置文件
 * @param filename 配置文件路径
 * @param config 配置结构指针
 * @return 成功返回0，失败返回-1
 */
int parseConfigFile(const char* filename, Config* config);

/**
 * 释放配置结构
 * @param config 配置结构指针
 */
void freeConfig(Config* config);

/**
 * 打印配置信息（调试用）
 * @param config 配置结构指针
 */
void printConfig(const Config* config);

#ifdef __cplusplus
}
#endif

#endif //ESURFINGCLIENT_CONFIGPARSER_H
