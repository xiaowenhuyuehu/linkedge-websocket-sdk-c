#include <stdio.h>
#include <string.h>
#include "ws_client.h"

void cb_recv(const char *msg, int len, void *usr)
{
    char buf[1024] = {0};
    static int index = 0; 
    printf("get msg from remote: %s\n", msg);
    if(index >= 100)
        return;

    snprintf(buf, sizeof(buf), "Hello LWS by client %d", ++index);
    wsc_add_msg(buf, strlen(buf) + 1, 0);
}

void cb_establish(void *user)
{
    char *str = "hello LWS by client";
    int ret = 0;
    ret = wsc_add_msg(str, strlen(str) + 1, 0);
    printf("wss add msg length: %d\n",ret);
}

void cb_close(void *user)
{
    printf("conn close: \n");
}

int main(int argc, char **argv)
{
    int ret = 0;
    wsc_param_conn param;
    wsc_param_cb   cbs;
    
    memset(&param, 0, sizeof(param));
    param.url = "ws://hotel.xiongan.iot.aliyun.com:7682";
    param.ca_path = NULL;
    param.cert_path = NULL;
    param.key_path = NULL;
    param.protocol = NULL;

    memset(&cbs, 0, sizeof(cbs));
    cbs.usr_cb_establish = NULL;
    cbs.p_cb_establish = cb_establish;
    cbs.usr_cb_close = NULL;
    cbs.p_cb_close = cb_close;
    cbs.usr_cb_recv = NULL;
    cbs.p_cb_recv = cb_recv;

    ret = wsc_init(&param, &cbs);
    printf("##### ret:  %d\n",ret);
    while(++ret <= 60){
        sleep(1);
    }
    printf("main runtime turn down:  %d\n",ret);
    ws_client_destroy();
    return 0;
}

