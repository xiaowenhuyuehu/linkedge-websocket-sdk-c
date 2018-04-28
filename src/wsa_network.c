#include <stdlib.h>
#include "ws_client.h"
#include "log.h"
#include "wsa_net_unpack.h"
#include "wsa_net.h"
#include "wsa.h"

static wsc_param_conn g_param_conn;
static wsc_param_cb   g_param_cbs;

#define CONN_TIMEOUT    10*60
#define CONN_PROTOCOL   "alibaba-iot-linkedge-protocol"

#define LOG_TAG "wsa_network"

static void cb_ws_recv(const char *msg, int len, void *user);
static void cb_ws_estab(void *user);
static void cb_ws_close(void *user);
static int g_is_init = 0;
static int g_conn_state = 0;

extern const WSA_Callback *g_wsa_cbs;
int wsa_network_init(const char *url, const char *root, const char *pub, const char *key, int timeout)
{
    int ret = 0;
    
    if(g_is_init == 1)
        return 0;

    memset(&g_param_conn, 0, sizeof(g_param_conn));

    g_conn_state = 0;
    g_param_conn.url = url;
    g_param_conn.ca_path = root;
    g_param_conn.cert_path = pub;
    g_param_conn.key_path = key;

    g_param_conn.timeout = timeout;
    g_param_conn.protocol = CONN_PROTOCOL;

    g_param_cbs.p_cb_establish = cb_ws_estab;
    g_param_cbs.p_cb_close = cb_ws_close;
    g_param_cbs.p_cb_recv = cb_ws_recv;
    
    ret = wsc_init(&g_param_conn, &g_param_cbs);

    if(ret != 0)
        return ret;
    
    while(--timeout){
        sleep(1);
        if(g_conn_state == 1)
            break;
        else if(g_conn_state == 2)
            return -2; 
        else
            log_i(LOG_TAG, "connecting to remote server\n");
    }   
    
    g_is_init = 1;

    return 0; 
}

static void cb_ws_recv(const char *msg, int len, void *user)
{
    NET_MSG_TYPE type = MSG_UNSUPPORTED;    
    char *cmd = NULL;
    int   id = 0;
    int   code = 0;
    char *dn = NULL;
    char *payload = NULL;
    const char *reply = NULL;
    char *json = NULL;
    
    if(!msg || len <= 0)
        return;

    json = strdup(msg);

    if(!json)
        return;
   
    log_d(LOG_TAG, "received msg: %s\n", json); 
    if(wsa_get_msg_header(json, &cmd, &id, &code, &dn, &payload) != 0){
        log_e(LOG_TAG, "msg is not supported yet.\n");
        free(json);
        return;
    }
    
    type = wsa_get_msg_type(cmd); 
    switch(type){
        case MSG_SET_PROPERTY:
            reply = wsa_net_unpack_set_properties(id, dn, payload);  
            break;
        
        case MSG_GET_PROPERTY:
            reply = wsa_net_unpack_get_properties(id, dn, payload);  
            break;
    
        case MSG_METHOD_CALL:
            reply = wsa_net_unpack_method_call(id, dn, payload);  
            break;
   
        case MSG_HEART_BEATS:
        case MSG_UNSUPPORTED:
        default:
            break;
    }
    
    if(reply){
        wsc_add_msg(reply, strlen(reply), 0);
        log_d(LOG_TAG, "send resp to remote server: %s\n", reply);
        free((void *)reply); 
    }

    free(cmd);
    free(dn);
    free(payload);
    free(json);
}

static void cb_ws_close(void *user)
{
    g_conn_state = 2;
    log_i(LOG_TAG, "connection closed.\n");

    if(g_wsa_cbs && g_wsa_cbs->cb_state_changed)
        g_wsa_cbs->cb_state_changed(WSA_CONNECTION_FAILED, g_wsa_cbs->usr_data_state_changed);
}

static void cb_ws_estab(void *user)
{
    g_conn_state = 1;
    log_i(LOG_TAG, "connection establish.\n");

    if(g_wsa_cbs && g_wsa_cbs->cb_state_changed)
        g_wsa_cbs->cb_state_changed(WSA_CONNECTION_SUCCESS, g_wsa_cbs->usr_data_state_changed);
}

int wsa_network_deinit()
{
    g_is_init = 0; 
    return ws_client_destroy(); 
}



