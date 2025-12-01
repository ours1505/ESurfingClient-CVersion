//
// ConfigParser.c - JSON配置文件解析实现
// Created by bad_g on 2025/12/02.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "headFiles/ConfigParser.h"
#include "headFiles/utils/Logger.h"

// 跳过空白字符
static const char* skipWhitespace(const char* str)
{
    while (*str && isspace((unsigned char)*str))
    {
        str++;
    }
    return str;
}

// 解析JSON字符串值
static const char* parseString(const char* str, char* output, size_t maxLen)
{
    str = skipWhitespace(str);
    if (*str != '"')
    {
        return NULL;
    }
    str++; // 跳过开始的引号
    
    size_t i = 0;
    while (*str && *str != '"' && i < maxLen - 1)
    {
        if (*str == '\\' && *(str + 1))
        {
            str++; // 跳过转义字符
            switch (*str)
            {
                case 'n': output[i++] = '\n'; break;
                case 't': output[i++] = '\t'; break;
                case 'r': output[i++] = '\r'; break;
                case '"': output[i++] = '"'; break;
                case '\\': output[i++] = '\\'; break;
                default: output[i++] = *str; break;
            }
        }
        else
        {
            output[i++] = *str;
        }
        str++;
    }
    output[i] = '\0';
    
    if (*str == '"')
    {
        str++; // 跳过结束的引号
    }
    return str;
}

// 查找键名
static const char* findKey(const char* str, const char* key)
{
    char searchKey[MAX_STRING_LEN];
    snprintf(searchKey, sizeof(searchKey), "\"%s\"", key);
    
    const char* pos = strstr(str, searchKey);
    if (!pos)
    {
        return NULL;
    }
    
    pos += strlen(searchKey);
    pos = skipWhitespace(pos);
    
    if (*pos != ':')
    {
        return NULL;
    }
    pos++; // 跳过冒号
    
    return skipWhitespace(pos);
}

// 解析单个账号对象
static const char* parseAccountObject(const char* str, AccountConfig* account)
{
    str = skipWhitespace(str);
    if (*str != '{')
    {
        return NULL;
    }
    
    // 查找对象结束位置
    const char* objEnd = str + 1;
    int braceCount = 1;
    while (*objEnd && braceCount > 0)
    {
        if (*objEnd == '{') braceCount++;
        else if (*objEnd == '}') braceCount--;
        objEnd++;
    }
    
    // 在对象范围内查找各个字段
    const char* keyPos;
    
    // 解析 username
    keyPos = findKey(str, "username");
    if (keyPos && keyPos < objEnd)
    {
        parseString(keyPos, account->username, MAX_STRING_LEN);
    }
    
    // 解析 password
    keyPos = findKey(str, "password");
    if (keyPos && keyPos < objEnd)
    {
        parseString(keyPos, account->password, MAX_STRING_LEN);
    }
    
    // 解析 bind_interface
    keyPos = findKey(str, "bind_interface");
    if (keyPos && keyPos < objEnd)
    {
        parseString(keyPos, account->bind_interface, MAX_STRING_LEN);
    }
    
    // 解析 channel
    keyPos = findKey(str, "channel");
    if (keyPos && keyPos < objEnd)
    {
        parseString(keyPos, account->channel, MAX_STRING_LEN);
    }
    else
    {
        // 默认通道
        strcpy(account->channel, "phone");
    }
    
    return objEnd;
}

int parseConfigFile(const char* filename, Config* config)
{
    if (!filename || !config)
    {
        return -1;
    }
    
    // 初始化配置结构
    memset(config, 0, sizeof(Config));
    
    // 读取文件
    FILE* file = fopen(filename, "r");
    if (!file)
    {
        LOG_ERROR("无法打开配置文件: %s", filename);
        return -1;
    }
    
    // 获取文件大小
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (fileSize <= 0 || fileSize > 1024 * 1024)  // 限制1MB
    {
        LOG_ERROR("配置文件大小无效");
        fclose(file);
        return -1;
    }
    
    // 读取文件内容
    char* content = malloc(fileSize + 1);
    if (!content)
    {
        LOG_ERROR("内存分配失败");
        fclose(file);
        return -1;
    }
    
    size_t readSize = fread(content, 1, fileSize, file);
    content[readSize] = '\0';
    fclose(file);
    
    // 解析JSON数组
    const char* pos = skipWhitespace(content);
    
    if (*pos != '[')
    {
        LOG_ERROR("配置文件格式错误: 期望JSON数组");
        free(content);
        return -1;
    }
    pos++; // 跳过 '['
    
    while (*pos && config->account_count < MAX_ACCOUNTS)
    {
        pos = skipWhitespace(pos);
        
        if (*pos == ']')
        {
            break; // 数组结束
        }
        
        if (*pos == '{')
        {
            AccountConfig* account = &config->accounts[config->account_count];
            memset(account, 0, sizeof(AccountConfig));
            
            pos = parseAccountObject(pos, account);
            if (!pos)
            {
                LOG_ERROR("解析账号配置失败");
                free(content);
                return -1;
            }
            
            // 验证必填字段
            if (strlen(account->username) > 0 && strlen(account->password) > 0)
            {
                config->account_count++;
            }
            else
            {
                LOG_WARN("跳过无效账号配置 (缺少用户名或密码)");
            }
        }
        
        pos = skipWhitespace(pos);
        if (*pos == ',')
        {
            pos++; // 跳过逗号
        }
    }
    
    free(content);
    
    if (config->account_count == 0)
    {
        LOG_ERROR("配置文件中没有有效的账号");
        return -1;
    }
    
    LOG_INFO("成功加载 %d 个账号配置", config->account_count);
    return 0;
}

void freeConfig(Config* config)
{
    if (config)
    {
        memset(config, 0, sizeof(Config));
    }
}

void printConfig(const Config* config)
{
    if (!config)
    {
        return;
    }
    
    LOG_DEBUG("配置信息:");
    LOG_DEBUG("账号数量: %d", config->account_count);
    
    for (int i = 0; i < config->account_count; i++)
    {
        const AccountConfig* acc = &config->accounts[i];
        LOG_DEBUG("账号 %d:", i + 1);
        LOG_DEBUG("  用户名: %s", acc->username);
        LOG_DEBUG("  密码: %s", "********");
        LOG_DEBUG("  绑定接口: %s", strlen(acc->bind_interface) > 0 ? acc->bind_interface : "默认");
        LOG_DEBUG("  通道: %s", acc->channel);
    }
}
