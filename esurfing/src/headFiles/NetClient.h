//
// Created by bad_g on 2025/9/23.
//

#ifndef ESURFINGCLIENT_NETCLIENT_H
#define ESURFINGCLIENT_NETCLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_HEADER_LENGTH 256
#define MAX_HEADERS_COUNT 20

typedef struct {
    char* memory;
    size_t size;
} HTTPResponse;

typedef enum {
    NET_RESULT_SUCCESS,
    NET_RESULT_ERROR
} NetResultType;

typedef struct {
    NetResultType type;
    char* data;
    size_t dataSize;
    char* errorMessage;
    int statusCode;
} NetResult;

typedef struct {
    char key[MAX_HEADER_LENGTH];
    char value[MAX_HEADER_LENGTH];
} HeaderPair;

typedef struct {
    HeaderPair headers[MAX_HEADERS_COUNT];
    int count;
} ExtraHeaders;

/**
 * 释放网络返回值函数
 * @param result 网络返回值
 */
void freeNetResult(NetResult* result);

size_t writeResponseCallback(const void *contents, size_t size, size_t nmemb, HTTPResponse *response);

/**
 * POST 函数
 * @param url 网址
 * @param data 数据
 * @return 网络返回值
 */
NetResult* simPost(const char* url, const char* data);

/**
 * POST 函数 (带接口绑定)
 * @param url 网址
 * @param data 数据
 * @param interface 绑定的网卡接口名或IP地址 (可为NULL)
 * @return 网络返回值
 */
NetResult* simPostWithInterface(const char* url, const char* data, const char* interface);

/**
 * POST 函数 (完整版)
 * @param url 网址
 * @param data 数据
 * @param extraHeaders 额外的HTTP头
 * @param interface 绑定的网卡接口名或IP地址 (可为NULL)
 * @return 网络返回值
 */
NetResult* postRequestWithInterface(const char* url, const char* data, ExtraHeaders* extraHeaders, const char* interface);

#ifdef __cplusplus
}
#endif

#endif //ESURFINGCLIENT_NETCLIENT_H