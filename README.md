## 阿里云 IoT LinkEdge WebSocket设备接入 SDK 

### 介绍

本 SDK 根据阿里云IoT LinkEdge WebSocket接入协议，屏蔽了协议的封装和解析细节，并整合了一款广泛应用的websocket库，方便开发者快速接入.

### 通讯协议

* [通讯协议完整版](doc/linkedge_websocket_protocol.md)

### 系统依赖

* [libwebsockets](https://github.com/warmcat/libwebsockets)
* 任何一种TLS库, 默认是[openssl](https://github.com/openssl/openssl), 其他可以是mbedTLS, wolfSSL, BoringSSL等.
* pthread 库.

### 编译指南

* Linux 环境:
 - 必须先安装[scons](http://scons.org/pages/download.html),详细信息请参考[编译介绍](doc/how_to_build.md)
 - 执行编译命令: `scons` 
 - 在当前目录会生成libwsa.a 和 wsa_sample

### 运行方法

    `LD_LIBRARY_PATH=./board/centos_64/prebuilt/libwebsockets/lib/:./board/centos_64/prebuilt/openssl/lib/ ./wsa_sample`

