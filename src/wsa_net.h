#ifndef __WSA_NET_H__
#define __WSA_NET_H__

typedef enum{
    MSG_SET_PROPERTY = 0,
    MSG_GET_PROPERTY,
    MSG_METHOD_CALL,
    MSG_HEART_BEATS,
    MSG_UNSUPPORTED
}NET_MSG_TYPE;

#define CMD_SET_PROPERTY "setProperty"
#define CMD_GET_PROPERTY "getProperty"
#define CMD_METHOD_CALL  "methodCall"
#define CMD_HEARTBEATS   "heartbeats"



#endif

