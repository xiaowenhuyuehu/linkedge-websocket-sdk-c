#include <string.h>
#include <stdio.h>
#include "wsa.h"
#include "ws_client.h"
#include "wsa_net_pack.h"
#include "wsa_network.h"
#include "log.h"

const WSA_Callback *g_wsa_cbs = NULL;

#define WSA_VERSION "v0.1.2"
#define LOG_TAG "WSA"

int WSA_init(const conn_info *info)
{
    int ret = 0;
 
    log_i(LOG_TAG, "WSA version: %s, date: %s\n", WSA_VERSION, __DATE__);
    if(!info || !info->url)
        return -1;

    return wsa_network_init(info->url, info->ca_path, info->cert_path, info->key_path, info->timeout); 
}

int WSA_register_online_device(const WSA_register_param *param)
{
    int ret = 0;
    if(!param || !param->device_id || !param->product_key || !param->product_md5 || !param->profile)
        return -1;

    ret = wsa_net_pack_register(param->device_id, param->product_key, param->product_md5, param->profile);
    if(ret != 0)
        return ret;
    
    usleep(100);
    ret = wsa_net_pack_online(param->device_id, param->product_key);

    return ret;
}

int WSA_offline_device(const char *device_id, const char *product_key)
{
    if(!device_id || !product_key)
        return -1;

    return wsa_net_pack_offline(device_id, product_key);
}

int WSA_report_properties(const char *device_id, const char *product_key, const WSA_Data properties[], int count)
{
    int i = 0;
    int ret = 0;

    if(!device_id || !product_key || !properties || count <= 0)
        return -1;

    for(i = 0; i < count; i++){
        if(!properties[i].name || !properties[i].value)
            break;
        ret += wsa_net_pack_report_property(device_id, product_key, properties[i].name, properties[i].value, properties[i].type);    
    }

    return ret;
}

int WSA_report_event(const char *device_id, const char *product_key, const char *event_name, const WSA_Data data[], int count)
{
    if(!device_id || !product_key || !event_name || !data || count <= 0)
        return -1;

    return wsa_net_pack_report_event(device_id, product_key, event_name, data, count);
}

int WSA_register_callback(const WSA_Callback *cb)
{
    if(!cb)
        return -1;

    g_wsa_cbs = cb;
    
    return 0;
}

int WSA_destroy()
{
    return wsa_network_deinit();
}

void WSA_set_log_level(int lvl)
{
   set_log_level(lvl); 
}
