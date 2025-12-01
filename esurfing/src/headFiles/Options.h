//
// Created by bad_g on 2025/9/27.
//

#ifndef ESURFINGCLIENT_OPTIONS_H
#define ESURFINGCLIENT_OPTIONS_H

extern char* usr;
extern char* pwd;
extern char* chn;
extern char* bindInterface;      // 绑定的网卡接口名或IP (-e 参数)
extern char* configFile;         // 配置文件路径 (-f 参数)
extern int isDebug;
extern int isSmallDevice;
extern int isMultiMode;          // 是否多账号模式

#endif //ESURFINGCLIENT_OPTIONS_H