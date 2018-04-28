#include <signal.h>
#include "libwebsockets.h"
#include "ws_client.h"

#define DEFAULT_LWS_PROTOCOL "alibaba-iot-linkedge-protocol"

#define CIPHERLIST "ECDHE-ECDSA-AES256-GCM-SHA384:" \
			       "ECDHE-RSA-AES256-GCM-SHA384:" \
			       "DHE-RSA-AES256-GCM-SHA384:" \
			       "ECDHE-RSA-AES256-SHA384:" \
			       "HIGH:!aNULL:!eNULL:!EXPORT:" \
			       "!DES:!MD5:!PSK:!RC4:!HMAC_SHA1:" \
			       "!SHA1:!DHE-RSA-AES128-GCM-SHA256:" \
			       "!DHE-RSA-AES128-SHA256:" \
			       "!AES128-GCM-SHA256:" \
			       "!AES128-SHA256:" \
			       "!DHE-RSA-AES256-SHA256:" \
			       "!AES256-GCM-SHA384:" \
			       "!AES256-SHA256"

volatile int force_exit = 0;
struct lws_context *context = NULL;
struct lws *g_wsi = NULL;
int    g_timeout = 0;

extern int callback_dumb_increment(struct lws *wsi, enum lws_callback_reasons reason,
			void *user, void *in, size_t len);

static struct lws_protocols protocols[] = {
    { DEFAULT_LWS_PROTOCOL, callback_dumb_increment, 0, 4096, },
    { NULL, NULL, 0, 0 }
};

static const struct lws_extension exts[] = {
    { "permessage-deflate", lws_extension_callback_pm_deflate,
        "permessage-deflate; client_no_context_takeover" },
    { "deflate-frame", lws_extension_callback_pm_deflate, "deflate_frame" },
    { NULL, NULL, NULL }
};

void alog_print(int lvl, const char *content)
{
    printf("<LIBWEBSOCKETS>  %s",content);
}

void notify_network()
{
    lws_callback_on_writable(g_wsi);
}
void *thread_wsc_network(void *arg)
{
    struct lws_context_creation_info info;
    struct lws_client_connect_info i;    
    p_wsc_param_conn param = (p_wsc_param_conn)arg;
	char interface_name[128] = "";
	unsigned int ms, oldms = 0;
	int uid = -1, gid = -1;
	int use_ssl = 0;
	int opts = 0;
	int n = 0;
    int debug_level = 0x1;
	const char *prot, *p;

    if(!param)
        return NULL;

    memset(&info, 0, sizeof info);
	lws_set_log_level(debug_level, alog_print);
     
    memset(&i, 0, sizeof(i));
    char *url = strdup(param->url);
    if (lws_parse_uri(url, &prot, &i.address, &i.port, &p)){
        printf("url is not correct\n"); 
		exit(0);
    }
    if(strcmp(prot, "wss") == 0){
        use_ssl = LCCSCF_USE_SSL |
            LCCSCF_ALLOW_SELFSIGNED |
            LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK;
    }
	info.port = CONTEXT_PORT_NO_LISTEN;
	info.protocols = protocols;
    if (use_ssl) {
        info.client_ssl_cert_filepath = param->cert_path;
		info.client_ssl_private_key_filepath = param->key_path;
		if (param->ca_path[0])
			info.client_ssl_ca_filepath = param->ca_path;
    }
	info.gid = gid;
	info.uid = uid;
	info.extensions = exts;
    info.ws_ping_pong_interval = 0;
    
#if defined(LWS_OPENSSL_SUPPORT)
    info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
#endif
    context = lws_create_context(&info);
	if (context == NULL) {
		lwsl_err("libwebsocket init failed\n");
        free(url);
		return NULL;
	}
    
    i.context = context;
    i.ssl_connection = use_ssl;
    i.host = i.address;
    i.origin = i.address;
    i.path = "//"; 
    i.protocol = DEFAULT_LWS_PROTOCOL;
    i.ietf_version_or_minus_one = -1;
    n = 0;
    g_timeout = param->timeout;	
    while (n >= 0 && !force_exit) {
        if(!g_wsi){
            printf("connecting to server....\n");
            i.pwsi = &g_wsi;
            lws_client_connect_via_info(&i);
            if(g_wsi)
                lws_set_timer(g_wsi, param->timeout);
            printf("connecting to server done, %p.\n", g_wsi);
            sleep(1);
            continue; 
        }

		n = lws_service(context, 100);
    }
	lws_context_destroy(context);

	lwsl_notice("libwebsockets-test-client exited cleanly\n");

    free(url);
	closelog();
	return NULL;
}
