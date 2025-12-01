# ESurfingClient-C

**根据 Rsplwe 大蛇的 Kotlin 源码编写的 C 语言版本的 `广东` 天翼校园认证客户端** :+1:

**优点是程序文件超级小(所有版本均是总共占用不超过 3MB 的储存空间😋)**

## 主要特性

- 🚀 支持多账号同时拨号
- 🔗 支持绑定指定网卡接口
- 📁 支持配置文件读取
- 💾 极小的程序体积

# 目前支持的环境

|系统|架构|
|----|----|
|Windows|x86_64|
|Linux|x86_64|
|Linux|aarch64 (ARM64)|

# 使用教程

[**Windows & Linux 环境**](Windows&Linux.md)

# 命令行参数

```
用法:
  单账号模式: ESurfingClient -u <用户名> -p <密码> [-c<通道>] [-e<网卡接口>] [-d] [-s]
  多账号模式: ESurfingClient -f <配置文件> [-d] [-s]

选项:
  -u <用户名>      登录用户名
  -p <密码>        登录密码
  -c <通道>        通道类型 (pc/phone)，默认为 phone
  -e <网卡接口>    绑定的网卡接口名或IP地址 (如 eth0 或 192.168.1.100)
  -f <配置文件>    JSON配置文件路径，用于多账号同时拨号
  -d               开启调试模式
  -s               小容量设备模式
  -h               显示帮助信息
```

# 配置文件格式

多账号模式使用 JSON 格式的配置文件:

```json
[
  {
    "username": "10001234",
    "password": "12345678",
    "bind_interface": "eth1",
    "channel": "android"
  },
  {
    "username": "10005678",
    "password": "87654321",
    "bind_interface": "eth2",
    "channel": "linux"
  }
]
```

配置项说明:
- `username`: 登录用户名 (必填)
- `password`: 登录密码 (必填)
- `bind_interface`: 绑定的网卡接口名或IP地址 (可选)
- `channel`: 通道类型，可选值: `pc`, `phone`, `android`, `linux` (可选，默认为 phone)

# 关于日志系统

### 在 Windows 系统中

- 程序运行后，会在程序的运行目录下新建 logs 文件夹

- 程序运行时，logs 目录下会生成实时更新的 run.log 日志文件

- 程序退出时，run.log 日志文件会被重命名为<当前时间>.log

### 在 Linux 类系统中

- 程序运行后，会新建 /var/log/esurfing/logs 目录

- 程序运行时，logs 目录下会生成实时更新的 run.log 日志文件

- 程序退出时，run.log 日志文件会被重命名为<当前时间>.log

- 同时满足 debug 模式开启和 smallDevice 模式关闭的条件的时候程序会将日志存储到 /usr/esurfing 目录中

# [更新日志](UpdateLogs.md)

> [!WARNING]
> 不要让我发现有人拿去做路由器贩卖
