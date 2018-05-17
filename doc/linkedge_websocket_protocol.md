# 阿里云 IoT 边缘计算设备接入协议(WebSocket版)
# Alibaba Cloud IoT LinkEdge device access protocol (WebSocket version)
## 目录
* [简介](#简介)
* [消息体格式](#消息体格式)
* [设备注册](#设备注册)
* [设备上线](#设备上线)
* [设备下线](#设备下线)
* [上报属性](#上报属性)
* [上报事件](#上报事件)
* [设置属性](#设置属性)
* [获取属性](#获取属性)
* [方法操作](#方法操作)

## Contents
•	Summary
•	Message body format
•	Device register
•	Device online
•	Device offline
•	Report properties
•	Report Events
•	Set properties
•	Get properties
•	Call method


## 简介

本文档定义了一套基于JSON规范的设备接入阿里云IoT边缘计算(LinkEdge)的协议规范。设备端作为client, 边缘计算作为server, 基于本协议完成
设备注册，上线，上报属性，上报事件，获取属性，调用方法等业务需求。

## Brief introduction
This document defines a JSON protocol for device access to Alibaba Cloud IoT LinkEdge. Device end as client, LinkEdge as server. Based on this protocol it  complete the registry, online, report properties, report events, get properties, call methods and other business requirements of the devices.


## 消息体格式

- 请求命令

```ruby
method:     命令字，包括(registerDevice,onlineDevice,offlineDevice,reportProperty,reportEvent,setProperty,getProperty,callService)
msgId:      消息编号，唯一标识这条消息, 对端收到消息的反馈中携带相同id。
version:    协议版本号，用来标识本次通讯使用的协议版本号，当前版本为v0.1。
productKey: 产品序号，跟设备 profile 文件绑定，该序号由阿里云提供。
deviceId:   设备序号，指设备自身的唯一标识信息。
payload:    消息体, 需要符合json标准。
```
示例
```
{
    "method":"registerDevice",
    "msgId":  11,
	"version": "v0.1",
    "deviceId":"aaaaaa",
	"productKey":"bbbbbb",
    "payload":{ 
        "profile":{}
    }
}
```

- 应答

```ruby
msgId:消息编号，请求命令携带的编号一致
code：反馈处理结果，详见各个命令说明
payload：反馈payload
```

示例
```
{
    "msgId":  11,
    "code": 0,
    "payload":{}
}
```
## Message body format
•	Request
method:     command word，include (registerDevice,onlineDevice,offlineDevice,reportProperty,reportEvent,setProperty,getProperty,callService)
msgId:      Message number that uniquely identifies this message and the feedback message responded by LinkEdge carries the same id as the message.
version:    Protocol version number, which is used to identify the protocol version number used for the communication, the current version is v0.1.
productKey: Product serial number, bound to device profile file, this serial number is provided by Alibaba Cloud.
deviceId:   Device serial number refers to the unique identification information of the device itself.
payload:    Message body shall conform to JSON standard.
Example
{
    "method":"registerDevice",
    "msgId":  11,
	"version": "v0.1",
    "deviceId":"aaaaaa",
	"productKey":"bbbbbb",
    "payload":{ 
        "profile":{}
    }
}
•	Reply
msgId: Message number，the number request command is the same
code： Reply results, detailed in the instructions of each command
payload： Reply payload
Example
{
    "msgId":  11,
    "code": 0,
    "payload":{}
}

## 设备注册

- client->server
- 设备初次入网时需要向边缘网关注册设备信息

```
profile： 设备数据模型，包含：属性、服务、事件。
```

- 命令格式

```
{
    "method":"registerDevice",
    "msgId":  11,
	"version": "v0.1",
    "deviceId":"aaaaaa",
	"productKey":"bbbbbb",
    "payload":{ 
        "profile":{}
    }
}
```
- 返回结果

```
code：0注册成功，101注册失败
payload：空(null)
```
## Device register
•	client->server
•	The device which connect to LinkEdge first time need to register device information from LinkEdge
•	command format
{
    "method":"registerDevice",
    "msgId":  11,
	"version": "v0.1",
    "deviceId":"aaaaaa",
	"productKey":"bbbbbb",
    "payload":{ 
        "profile":{}
    }
}
•	return results
code：0 device register successfully, 101 Failure of device register
payload： null


## 设备上线

- client->server
- 设备注册成功之后，可以执行设备上线命令，设备上线成功之后，才能被操作
- 命令格式

```
{
    "method":"onlineDevice",
    "msgId":  11,
	"version": "v0.1",
    "deviceId":"aaaaaa",
	"productKey":"bbbbbb",
    "payload":{}
}
```

- 返回结果

```
code：0注册成功，102设备未注册，103设备上线失败
payload：空(null)
```
## Device online
•	client->server
•	After the device is registered successfully, you can execute the device online command, and the device cannot be operated until the device is online successfully
•	command format
{
    "method":"onlineDevice",
    "msgId":  11,
	"version": "v0.1",
    "deviceId":"aaaaaa",
	"productKey":"bbbbbb",
    "payload":{}
}
•	return results
code：0 device register successfully, 102 device not registered, 103 Failure of device online
payload： null

## 设备下线

- client->server
- 设备离线后，调用设备下线命令，通知边缘网关设备离线，设备重新上线后，无需再次注册，只需要上报设备上线即可
- 命令格式

```
{
    "method":"offlineDevice",
    "msgId":  11,
	"version": "v0.1",
    "deviceId":"aaaaaa",
	"productKey":"bbbbbb",
    "payload":{}
}
```

- 返回结果

```
code：0注册成功，102设备未注册，104设备下线失败，106设备未上线
payload：空(null)
```
## Device offline
•	client->server
•	After the device is offline, call the device offline command, notify the edge gateway device off the line, after the device is online again, it do not need to register again, just need to report the device to be online
•	command format
{
    "method":"offlineDevice",
    "msgId":  11,
	"version": "v0.1",
    "deviceId":"aaaaaa",
	"productKey":"bbbbbb",
    "payload":{}
}
•	return results
code： 0 device register successfully, 102 device not registered, 104 Failure of device offline, 106 device not online
payload：null


## 上报属性

- client->server
- 设备上线成功之后，上报全量设备，其他属性按照设备运行实际情况上报

- 命令格式

```
{
    "method":"reportProperty",
    "msgId":  11,
	"version": "v0.1",
    "deviceId":"aaaaaa",
	"productKey":"bbbbbb",
    "payload":{
        "temperature":25,
        "humidity":50
    }
}
```
## Report properties
•	client->server
•	After the device is online successfully, report total properties, other properties according to the actual operation of the device 
•	command format
{
    "method":"reportProperty",
    "msgId":  11,
	"version": "v0.1",
    "deviceId":"aaaaaa",
	"productKey":"bbbbbb",
    "payload":{
        "temperature":25,
        "humidity":50
    }
}
•	return results
code： 0 device register successfully, 102 device not registered, 106 device not online
payload：null

- 返回结果

```
code：0上报成功，102设备未注册，106设备未上线
payload：空(null)
```

## 上报事件

- client->server
- 事件触发时上报

```
name：事件名称
params：输出的状态
```

- 命令格式

```
{
    "method":"reportEvent",
    "msgId":  11,
	"version": "v0.1",
    "deviceId":"aaaaaa",
	"productKey":"bbbbbb",
    "payload":{
        "name":"alarm",
        "params":{
            "alarm":2
        }
    }
}
```

- 返回结果

```
code：0上报成功，102设备未注册，106设备未上线
payload：空(null)
```
## Report Events
•	client->server
•	Report when an event is triggered
name： event name
params： output state
•	command format
{
    "method":"reportEvent",
    "msgId":  11,
	"version": "v0.1",
    "deviceId":"aaaaaa",
	"productKey":"bbbbbb",
    "payload":{
        "name":"alarm",
        "params":{
            "alarm":2
        }
    }
}
•	return results
code： 0 device register successfully, 102 device not registered, 106 device not online
payload：null

## 设置属性

- server->client
- 边缘网关对设备属性设置

- 命令格式

```
{
    "method":"setProperty",
    "msgId":  11,
	"version": "v0.1",
    "deviceId":"aaaaaa",
	"productKey":"bbbbbb",
    "payload":{
        "temperature":25
    }
}
```

- 返回结果

```
code：0设置成功，105设备离线
payload：设置成功，反馈执行后属性值，如果与设置值完全一致，直接反馈设置命令中的payload
```
## Set properties
•	server->client
•	Edge gateways set device properties
•	command format
{
    "method":"setProperty",
    "msgId":  11,
	"version": "v0.1",
    "deviceId":"aaaaaa",
	"productKey":"bbbbbb",
    "payload":{
        "temperature":25
    }
}
•	return results
code：0 set successfully，105 device offline
payload： Set successfully, then feedback the property value that has been set, if it is exactly the same as the setting value, directly feedback the payload in the set command.

## 获取属性

- server->client
- 边缘网关获取设备属性

```
list：需要获取的属性列表
```

- 命令格式

```
{
    "method":"getProperty",
    "msgId":  11,
	"version": "v0.1",
    "deviceId":"aaaaaa",
	"productKey":"bbbbbb",
    "payload":{
        "list":["temperature","humidity"]
    }
}
```

- 返回结果

```
code：0获取成功，105设备离线
payload：反馈获取的属性状态，以key-value方式，如："payload":{"temperature":25,"humidity":50}
```

## Get properties
•	server->client
•	Edge gateways get device properties
list： List of properties to get
•	command format
{
    "method":"getProperty",
    "msgId":  11,
	"version": "v0.1",
    "deviceId":"aaaaaa",
	"productKey":"bbbbbb",
    "payload":{
        "list":["temperature","humidity"]
    }
}
•	return results
code：0 set successfully，105 device offline
payload： Feedback obtained property state in the way of key-value, e.g.
"payload":{"temperature":25,"humidity":50}


## 方法操作

- server->client
- 边缘网关操作设备方法

```
name：事件名称
params：输出的状态
```

- 命令格式

```
{
    "method":"callService",
    "msgId":  11,
	"version": "v0.1",
    "deviceId":"aaaaaa",
	"productKey":"bbbbbb",
    "payload":{
        "name":"control",
        "params":{
            "action":2
        }
    }
}
```

- 返回结果

```
code：0获取成功，105设备离线
payload：以profile定义中的output内容，反馈执行结果，没有output反馈空
```
## Call method
•	server->client
•	Edge gateway operate device method
name： event name 
params：output state
•	command format
{
    "method":"callService",
    "msgId":  11,
	"version": "v0.1",
    "deviceId":"aaaaaa",
	"productKey":"bbbbbb",
    "payload":{
        "name":"control",
        "params":{
            "action":2
        }
    }
}
•	return results
code：0 get successfully，105 device offline
payload：Feedback execution results with output content defined in profile, feedback empty if no output

## 时序图示例
## Sequential diagram example

![image](http://git.cn-hangzhou.oss-cdn.aliyun-inc.com/uploads/iot-gateway/gateway/26e509f1840389a7a96c55ab259635e9/image.png)

## 注意事项
## Attentions

* deviceId必须只能是由字母和数字组成，不能包含任何其他字符。
• DeviceId must consist only of letters and numbers and not contain any other characters.


