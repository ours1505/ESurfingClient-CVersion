# Windows & Linux 环境使用教程

**1. 从 [Release](https://github.com/BadGhost520/ESurfingClient-CVersion/releases/latest) 中下载相应的程序**

**2. 将程序放在自己想要的位置**

**3. 运行如下命令以启动程序**

## 单账号模式

### Linux (x86_64)
```bash
./ESurfingClient-x86_64-linux -u <用户名> -p <密码>
```

### Linux (aarch64/ARM64)
```bash
./ESurfingClient-aarch64-linux -u <用户名> -p <密码>
```

### Windows
```shell
.\ESurfingClient-x86_64-windows.exe -u <用户名> -p <密码>
```

## 多账号模式

使用配置文件启动多账号同时拨号:

### Linux
```bash
./ESurfingClient-x86_64-linux -f config.json
```

### Windows
```shell
.\ESurfingClient-x86_64-windows.exe -f config.json
```

配置文件格式 (config.json):
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

# 示例

### Linux
```bash
./ESurfingClient-x86_64-linux -u 233333333 -p A1234567
```

### Windows
```shell
.\ESurfingClient-x86_64-windows.exe -u 233333333 -p A1234567
```

## 绑定网卡接口

使用 `-e` 参数指定绑定的网卡接口名或IP地址:

### Linux
```bash
./ESurfingClient-x86_64-linux -u 233333333 -p A1234567 -eeth0
./ESurfingClient-x86_64-linux -u 233333333 -p A1234567 -e192.168.1.100
```

### Windows
```shell
.\ESurfingClient-x86_64-windows.exe -u 233333333 -p A1234567 -e192.168.1.100
```

> [!TIP]
> 目前有两个认证通道: pc 和 phone
> 
> **两者并没有什么太大的区别**
> 
> 如果需要切换认证通道，可以在命令最后添加 -cpc (PC 通道)或者 -cphone (Phone 通道)来切换
> 
> 默认为 Phone 通道

# 通道切换示例

### Linux
```bash
./ESurfingClient-x86_64-linux -u 233333333 -p A1234567 -cpc
./ESurfingClient-x86_64-linux -u 233333333 -p A1234567 -cphone
```

### Windows
```shell
.\ESurfingClient-x86_64-windows.exe -u 233333333 -p A1234567 -cpc
.\ESurfingClient-x86_64-windows.exe -u 233333333 -p A1234567 -cphone
```

> [!TIP]
> 调试模式参数为 `-d`
> 
> 小容量设备模式参数为 `-s`
> 
> 绑定网卡接口参数为 `-e<接口名或IP>`
> 
> 配置文件模式参数为 `-f <配置文件路径>`