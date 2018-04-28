#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "wsa.h"

#define SAMPLE_DEVICE_NAME "LedSampleDevice"
#define SAMPLE_PRODUCT_KEY "a1cB3Yjpr58"

static char g_profile[4*1024] = {0};
static WSA_register_param param_req;
extern const char *get_md5(const unsigned char *msg, int len, char *output, int output_len);

static char *_read_profile(const char *f)
{
    char *content = NULL;   
    FILE *fp = NULL; 
    if(!f)
        return NULL;
     
    fp = fopen(f, "r");
    if(!fp){
        printf("failed to open file: %s: %s\n", f, strerror(errno));
        exit(1); 
    }
   
    fread(g_profile, sizeof(g_profile), 1, fp);
    fclose(fp);
    return g_profile;
}

static int cb_get_property(const char *device_id, const char *property_name, WSA_Data *output, void *usr)
{
    if(!device_id || !property_name || !output)
        return -1;

    memset(output, 0, sizeof(WSA_Data)); 
    printf("get property from local : %s, property: %s\n", device_id, property_name);
    if(strcmp(property_name, "power") == 0){
        strncpy(output->name, "power", MAX_LENGTH_PARAM_NAME);
        strncpy(output->value, "1", MAX_LENGTH_PARAM_VALUE);
        output->type = TYPE_INT;
    } else if(strcmp(property_name, "bright") == 0){
        strncpy(output->name, "bright", MAX_LENGTH_PARAM_NAME);
        strncpy(output->value, "99", MAX_LENGTH_PARAM_VALUE);
        output->type = TYPE_STRING;
    } else {
        return -1; 
    }

    return 0;
}

static int cb_set_property(const char *device_id, const WSA_Data *property, void *usr)
{
    if(!device_id || !property)
        return -1;

    printf("set property: %s value: %s\n", property->name, property->value);
    return 0;
}

static int cb_method_call(const char *device_id, const char *method_name, const WSA_Data *data, int count, char *output, int output_len, void *usr)
{
    if(!device_id || !method_name || !data || !output || output_len <= 0)
        return -1;
    //call device service.
    strncpy(output, "{\"execute_status\": 111", output_len);
    printf("new method call: %s\n", method_name);
    return 0;
}

static void cb_state_changed(WSA_STATE state, void *usr)
{
    printf("connection state: %s\n", !state ? "connected" : "closed");
    
    if(state == WSA_CONNECTION_SUCCESS){
        //register and online again 
        WSA_register_online_device(&param_req);   
    }
}

int main(int argc, char **argv)
{
    conn_info conn;
    char md5sum[33] = {0};
    int timeout = 20*60;

    param_req.device_id = SAMPLE_DEVICE_NAME;
    param_req.product_key = SAMPLE_PRODUCT_KEY;
    param_req.profile = _read_profile("led.json");
    param_req.product_md5 = (const char *)get_md5((const unsigned char *)param_req.profile, strlen(param_req.profile), md5sum, sizeof(md5sum));
 
    conn.url = argc == 1 ? "ws://127.0.0.1:7682" : argv[1];
    conn.timeout = timeout;
    conn.ca_path = NULL;
    conn.cert_path = NULL;
    conn.key_path = NULL;

    WSA_set_log_level(0);
   
    WSA_Callback cb = { cb_get_property, NULL, 
                        cb_set_property, NULL,
                        cb_method_call, NULL,
                        cb_state_changed, NULL};
    //register callback
    WSA_register_callback(&cb);

    //init WSA
    WSA_init(&conn);
  
    WSA_Data property;
    strncpy(property.name, "bright", MAX_LENGTH_PARAM_NAME);
    strncpy(property.value, "11", MAX_LENGTH_PARAM_VALUE);
    property.type = TYPE_INT;
    //report property
    WSA_report_properties(SAMPLE_DEVICE_NAME, SAMPLE_PRODUCT_KEY, &property, 1);

    WSA_Data event[2];
    strncpy(event[0].name, "temprature", MAX_LENGTH_PARAM_NAME);
    strncpy(event[0].value, "40", MAX_LENGTH_PARAM_VALUE);
    event[0].type = TYPE_INT;
    strncpy(event[1].name, "temprature2", MAX_LENGTH_PARAM_NAME);
    strncpy(event[1].value, "20", MAX_LENGTH_PARAM_VALUE);
    event[1].type = TYPE_INT;
 
    //report event
    WSA_report_event(SAMPLE_DEVICE_NAME, SAMPLE_PRODUCT_KEY, "high_temprature", &event, 2);

    while(--timeout)
        sleep(1);

    //offline device
    WSA_offline_device(SAMPLE_DEVICE_NAME, SAMPLE_PRODUCT_KEY);
    
    WSA_destroy();
    return 0;
}

