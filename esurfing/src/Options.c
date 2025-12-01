//
// Created by bad_g on 2025/9/27.
//
char* usr;
char* pwd;
char* chn;
char* bindInterface = NULL;      // 绑定的网卡接口名或IP (-e 参数)
char* configFile = NULL;         // 配置文件路径 (-f 参数)
int isDebug = 0;
int isSmallDevice = 0;
int isMultiMode = 0;             // 是否多账号模式