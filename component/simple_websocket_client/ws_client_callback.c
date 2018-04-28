#include "libwebsockets.h"
#include "ws_client.h"
#include "wsc_buffer_mgmt.h"

extern int g_timeout;
extern p_wsc_param_cb g_cbs;
extern struct lws *g_wsi;

int cb_pop_msg(char *buf, int buf_len, int type, void *usr)
{
    struct lws *wsi = usr;
    
    if(!usr)
        return -1;
    return lws_write(wsi, buf, buf_len, type);;
}

int callback_dumb_increment(struct lws *wsi, enum lws_callback_reasons reason,
			void *user, void *in, size_t len)
{
    switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            if(g_cbs && g_cbs->p_cb_establish)
                g_cbs->p_cb_establish(g_cbs->usr_cb_establish);
            lws_set_timer(wsi, g_timeout);
            lws_callback_on_writable(wsi);
            break;

        case LWS_CALLBACK_CLIENT_WRITEABLE:
            client_buf_mgmt_pop(cb_pop_msg, (void *)wsi);
            break;
        case LWS_CALLBACK_TIMER:
            printf("timeout....\n");
            if(g_cbs && g_cbs->p_cb_close)
                g_cbs->p_cb_close(g_cbs->usr_cb_close);
            break;
        case LWS_CALLBACK_CLIENT_RECEIVE:
            lws_set_timer(wsi, -1);
            if(g_cbs && g_cbs->p_cb_recv)
                g_cbs->p_cb_recv(in, len, g_cbs->usr_cb_recv);
            lws_callback_on_writable(wsi);
            break;
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            if(g_cbs && g_cbs->p_cb_close)
                g_cbs->p_cb_close(g_cbs->usr_cb_close);
            g_wsi = NULL;
            break;
        case LWS_CALLBACK_CLOSED:
            if(g_cbs && g_cbs->p_cb_close)
                g_cbs->p_cb_close(g_cbs->usr_cb_close);
            g_wsi = NULL; 
            buf_mgmt_client_clear_msg();
            return -1;
        default:
            break;
    }

	return 0;
}
