#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "log.h"
#include "wsa.h"
#include "util.h"

#define LOG_TAG "WSA_NETPACK"

#define FMT_REGISTER            "{\"method\":\"registerDevice\",\"msgId\":%d,\"deviceId\":\"%s\",\"productKey\":\"%s\",\"payload\":{\"profile\":%s}}"
#define FMT_ONLINE              "{\"method\":\"onlineDevice\",\"msgId\":%d,\"deviceId\":\"%s\",\"productKey\":\"%s\",\"payload\":{}}"
#define FMT_OFFLINE             "{\"method\":\"offlineDevice\",\"msgId\":%d,\"deviceId\":\"%s\",\"productKey\":\"%s\",\"payload\":{}}"
#define FMT_REPORT_PROPERTY     "{\"method\":\"reportProperty\",\"msgId\":%d,\"deviceId\":\"%s\",\"productKey\":\"%s\",\"payload\":{\"%s\":%s}}"
#define FMT_REPORT_PROPERTY_STRING  "{\"method\":\"reportProperty\",\"msgId\":%d,\"deviceId\":\"%s\",\"productKey\":\"%s\",\"payload\":{\"%s\":\"%s\"}}"
#define FMT_REPORT_EVENT        "{\"method\":\"reportEvent\",\"msgId\":%d,\"deviceId\":\"%s\",\"productKey\":\"%s\",\"payload\":{\"name\":\"%s\",\"params\":{%s}}}"

static unsigned int     g_msg_id = 0;
static pthread_mutex_t  g_msg_locker = PTHREAD_MUTEX_INITIALIZER;

#define _MALLOC_MEMORY(len) do \
{ \
    msg = malloc(len); \
    if(!msg){ \
        log_e(LOG_TAG, "failed to alloc memroy\n"); \
        return -1; \
    } \
    memset(msg, 0, len); \
}while(0)

static inline unsigned int _get_msg_id()
{
    int ret = 0;

    pthread_mutex_lock(&g_msg_locker);
    ret = ++g_msg_id;
    pthread_mutex_unlock(&g_msg_locker);
    
    return ret;
}

int wsa_net_pack_register(const char *dn, const char *pk, const char *md5, const char *profile)
{
    char *msg = NULL;
    int ret = 0;
    int len = 0;
    int msg_id = _get_msg_id();

    len = strlen(FMT_REGISTER) + strlen(pk) + strlen(dn) + strlen(profile) + get_integer_length(msg_id) + 1;
    _MALLOC_MEMORY(len);
   
    snprintf(msg, len, FMT_REGISTER, msg_id, dn, pk, profile);
    ret = wsc_add_msg(msg, len, 0);

    log_d(LOG_TAG, "register device: %s\n", msg);
    free(msg);
   
    return ret; 
}

int wsa_net_pack_online(const char *dn, const char *pk)
{
    char *msg = NULL;
    int ret = 0;
    int len = 0;
    int msg_id = _get_msg_id();


    len = strlen(FMT_ONLINE) + strlen(dn) + strlen(pk) + get_integer_length(msg_id) + 1;
    _MALLOC_MEMORY(len);
   
    snprintf(msg, len, FMT_ONLINE, msg_id, dn, pk);
    ret = wsc_add_msg(msg, len, 0);

    log_d(LOG_TAG, "online device: %s, ret: %d\n", msg, ret);
    free(msg);
   
    return ret; 
}

int wsa_net_pack_offline(const char *dn, const char *pk)
{
    char *msg = NULL;
    int ret = 0;
    int len = 0;
    int msg_id = _get_msg_id();

    len = strlen(FMT_OFFLINE) + strlen(dn) + strlen(pk) + get_integer_length(msg_id) + 1;
    _MALLOC_MEMORY(len);
   
    snprintf(msg, len, FMT_OFFLINE, msg_id, dn, pk);
    ret = wsc_add_msg(msg, len, 0);

    log_d(LOG_TAG, "offline device: %s\n", msg);
    free(msg);

    return ret; 
}

int wsa_net_pack_report_property(const char *dn, const char *pk, const char *k, const char *v, WSA_Data_Type type)
{
    char *msg = NULL;
    int ret = 0;
    int len = 0;
    const char *fmt = NULL;
    int msg_id = _get_msg_id();

    fmt = (type == TYPE_STRING) ? FMT_REPORT_PROPERTY_STRING : FMT_REPORT_PROPERTY;

    len = strlen(fmt) + strlen(dn) + strlen(pk) + get_integer_length(msg_id) + strlen(k) + strlen(v) + 1;
    _MALLOC_MEMORY(len);
   
    snprintf(msg, len, fmt, msg_id, dn, pk, k, v);
    ret = wsc_add_msg(msg, len, 0);

    log_d(LOG_TAG, "report property: %s\n", msg);
    free(msg);
    
    return ret; 
}

static inline const char *_get_event_output_data(const WSA_Data data[], int count)
{
    int i = 0;
    int len = 0;
    char *output = NULL;
    const char *fmt = NULL;

    for(i = 0; i < count; i++){
        len += strlen("\"\"") + strlen(data[i].name) + strlen(":") + strlen(data[i].value);
        
        if(data[i].type == TYPE_STRING)
            len += strlen("\"\"");
    }
    
    len += count;
    output = malloc(len); 
    if(!output)
        return NULL;

    memset(output, 0, len);
    for(i = 0; i < count; i++){
        if(data[i].type == TYPE_STRING) 
            fmt = "\"%s\":\"%s\"";
        else
            fmt = "\"%s\":%s";
        snprintf(output + strlen(output), 
                    len - strlen(output), fmt, data[i].name, data[i].value); 
    
        if(i != count - 1)
            strcat(output, ",");
    }

    return output;
}

int wsa_net_pack_report_event(const char *dn, const char *pk, const char *e, const WSA_Data data[], int count)
{
    char *msg = NULL;
    int ret = 0;
    int len = 0;
    const char *output = NULL;
    int msg_id = _get_msg_id();

    output = _get_event_output_data(data, count);
    if(!output)
        output = "";
    len = strlen(FMT_REPORT_EVENT) + strlen(dn) + strlen(pk) + get_integer_length(msg_id) + strlen(e) + strlen(output) + 1;
    _MALLOC_MEMORY(len);
   
    snprintf(msg, len, FMT_REPORT_EVENT, msg_id, dn, pk, e, output);
    ret = wsc_add_msg(msg, len, 0);

    log_d(LOG_TAG, "report event: %s\n", msg);
    free(msg);
   
    if(strcmp(output, "") != 0)
        free((void *)output);

    return ret; 
}
