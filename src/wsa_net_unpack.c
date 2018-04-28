#include <stdio.h>
#include "log.h"
#include "json_parser.h"
#include "wsa.h"
#include "wsa_net.h"
#include "util.h"

#define LOG_TAG "WSA_NET_UNPACK"

#define FMT_RESP                "{\"msgId\":%d,\"code\":%d,\"payload\":{%s}}"

extern WSA_Callback *g_wsa_cbs;

NET_MSG_TYPE wsa_get_msg_type(const char *cmd)
{
    if(!cmd)
        return MSG_UNSUPPORTED;

    if(strcmp(cmd, CMD_SET_PROPERTY) == 0)
        return MSG_SET_PROPERTY;
    else if(strcmp(cmd, CMD_GET_PROPERTY) == 0)
        return MSG_GET_PROPERTY;
    else if(strcmp(cmd, CMD_METHOD_CALL) == 0)
        return MSG_METHOD_CALL;
    else if(strcmp(cmd, CMD_HEARTBEATS) == 0)
        return MSG_HEART_BEATS;
    else
        return MSG_UNSUPPORTED;
}

static inline const char *_gen_resp_msg(int id, int code, const char *msg)
{
    int len = 0;
    char *output = NULL;
    len = strlen(FMT_RESP) + get_integer_length(id)
                           + get_integer_length(code) + strlen(msg) + 1; 
    
    if(!msg)
        return NULL;

    output = malloc(len);
    if(!output){
        log_e(LOG_TAG, "failed to alloc memory to generate response msg\n"); 
        return NULL;
    }

    snprintf(output, len, FMT_RESP, id, code, msg);

    return output;
}

int wsa_get_msg_header(char *json, char **cmd, int *id, int *code, char **dn, char **payload)
{
    char *tmp = NULL;       
    char *tmp_payload = NULL;
    int  len_val = 0;
    int  type_val = 0; 

    if(!json || !dn || !payload)
        return -1;

    tmp = json_get_value_by_name(json, strlen(json), "msgId", &len_val, &type_val); 
    if(tmp && type_val == JNUMBER && len_val > 0)
        *id = atoi(tmp); 

    tmp = json_get_value_by_name(json, strlen(json), "code", &len_val, &type_val); 
    if(tmp && type_val == JNUMBER && len_val > 0)
        *code = atoi(tmp); 

    tmp = json_get_value_by_name(json, strlen(json), "method", &len_val, &type_val); 
    if(tmp && type_val == JSTRING && len_val > 0)
        *cmd = strndup(tmp, len_val); 

    tmp = json_get_value_by_name(json, strlen(json), "deviceId", &len_val, &type_val); 
    if(tmp && type_val == JSTRING && len_val > 0)
        *dn = strndup(tmp, len_val); 

    tmp_payload = json_get_value_by_name(json, strlen(json), "payload", &len_val, &type_val);
    if(tmp_payload && type_val == JOBJECT && len_val > 0)
        *payload = strndup(tmp_payload, len_val);

    return 0;
}

static inline WSA_Data_Type _json_type_mapping(int type)
{
    WSA_Data_Type ret;

    switch(type){
        case JSTRING:  ret = TYPE_STRING;  break;
        case JNUMBER:  ret = TYPE_INT;     break;
        case JBOOLEAN: ret = TYPE_BOOL;    break;
        default:       ret = TYPE_INT;     break;
    }    

    return ret;
}

static inline int _foreach_set_properties(char *k, int k_len, char *v, int v_len, int v_type, void *usr)
{
    char *dn = (char *)usr;
    WSA_Data data;

    if(!k || k_len <= 0 || !v || v_len <= 0)
        return -1;
    
    strncpy(data.name, k, k_len < MAX_LENGTH_PARAM_NAME ? k_len : MAX_LENGTH_PARAM_NAME);
    strncpy(data.value, v, v_len < MAX_LENGTH_PARAM_VALUE ? v_len : MAX_LENGTH_PARAM_VALUE);
    data.type = _json_type_mapping(v_type); 

    g_wsa_cbs->cb_set_property(dn, &data, g_wsa_cbs->usr_data_set_property);

    return 0;
}

#define KV_PROPERTY_NUMBER "\"%s\":%s"
#define KV_PROPERTY_STRING "\"%s\":\"%s\""

typedef struct{
    char *dn;
    char **msg;
}_get_property_param;

static inline int _foreach_get_properties(char *k, int k_len, char *v, int v_len, int v_type, void *usr)
{
    _get_property_param *param = (_get_property_param *)usr;
    char **msg = param->msg;
    int len = 0;
    WSA_Data data;
    char *property = NULL;
    char *fmt = NULL;
    int offset = 0;
    int ret = 0;

    if(!k || k_len <= 0)
        return -1;
  
    if(*msg != NULL){
        len = strlen(*msg);
        offset = len; 
    } 

    property = strndup(k, k_len);
    if(!property)
        return -2;

    ret = g_wsa_cbs->cb_get_property(param->dn, property, &data, g_wsa_cbs->usr_data_get_property);
    if(ret != 0){
        log_e(LOG_TAG, "failed to get property: %s\n", property);
        free(property);
        return -3;
    }     

    fmt = data.type == TYPE_STRING ? KV_PROPERTY_STRING : KV_PROPERTY_NUMBER;
    len += strlen(fmt) + strlen(data.name) + strlen(data.value);

    *msg = realloc(*msg, len);
    if(!*msg){
        log_e(LOG_TAG, "failed to alloc memroy\n");
        free(property);
        return -4;
    } 
    sprintf(*msg + offset, fmt, data.name, data.value);
    
    free(property);
    
    return 0;
}

 
const char *wsa_net_unpack_set_properties(int id, char *dn, char *payload)
{
    int code = 0;
    char *msg = NULL;
    const char *ret = NULL;
    _get_property_param param;

    if(!dn || !payload)    
        return _gen_resp_msg(id, code, "msg is invalid");

    code = json_parse_name_value(payload, strlen(payload), _foreach_set_properties, dn);

    usleep(500*1000);
    
    param.dn = dn;
    param.msg = &msg;
    code += json_parse_name_value(payload, strlen(payload), _foreach_get_properties, &param);

    if(!msg)
        msg = ""; 
    
    ret = _gen_resp_msg(id, code, msg);
    if(strcmp(msg, "") != 0)
        free(msg);
    return ret; 
}

const char *wsa_net_unpack_get_properties(int id, char *dn, char *payload)
{
    char *msg = NULL;
    char *list = NULL;
    char *tmp = NULL; 
    int v_len = 0;
    int v_type = 0;
    int code = 0;
    const char *ret = NULL;
    _get_property_param param;

    if(!dn || !payload)
        return _gen_resp_msg(id, code, "msg is invalid");
    
    tmp = json_get_value_by_name(payload, strlen(payload), "list", &v_len, &v_type);
    if(!tmp || v_type != JARRAY || v_len <= 0){
        log_e(LOG_TAG, "json format is wrong.\n"); 
        return _gen_resp_msg(id, code, "msg is invalid");
    }

    list = strndup(tmp, v_len);
    tmp = NULL; 
   
    char *pos = NULL;
    char *entry = NULL;

    v_len = 0;
    v_type = 0;
    param.msg = &msg;
    param.dn = dn;

    json_array_for_each_entry(list, pos, entry, v_len, v_type){
        if(v_type != JSTRING)
            continue;
        code += _foreach_get_properties(entry, v_len, NULL, 0, v_type, &param); 
    }
    if(!msg)
        msg = "";
    ret = _gen_resp_msg(id, code, msg);
    if(strcmp(msg, "") != 0)
        free(msg);
  
    free(list);

    return ret;
}

typedef struct{
    WSA_Data *data;
    int       cnt;
}_method_call_param;

static inline int _foreach_method_call(char *k, int k_len, char *v, int v_len, int v_type, void *usr)
{
    _method_call_param *param = (_method_call_param *)usr;
    WSA_Data *data = NULL;

    if(!k || k_len <= 0 || !v || v_len <= 0)
        return -1;
   
    data = realloc(param->data, (param->cnt+1)*sizeof(WSA_Data));
    if(!data){
        log_e(LOG_TAG, "failed to alloc more memory to store method call param.\n"); 
        return -1;
    } 
    strncpy(data[param->cnt].name, k, k_len < MAX_LENGTH_PARAM_NAME ? k_len : MAX_LENGTH_PARAM_NAME);
    strncpy(data[param->cnt].value, v, v_len < MAX_LENGTH_PARAM_VALUE ? v_len : MAX_LENGTH_PARAM_VALUE);
 
    data[param->cnt].type = _json_type_mapping(v_type); 
    param->cnt += 1;

    param->data = data;
    
    return 0;
}

const char *wsa_net_unpack_method_call(int id, char *dn, char *payload)
{
    char *e_name = NULL;
    char *output = NULL; 
    char *tmp = NULL;
    int len = 0;
    int type = 0;
    int i = 0;
    int code = 0;
    char *msg = NULL;
    const char *ret = NULL;
    char *method_ret = NULL;
    _method_call_param param;
    
    if(!dn || !payload)
        return _gen_resp_msg(id, code, "msg is invalid");
    
    tmp = json_get_value_by_name(payload, strlen(payload), "name", &len, &type); 
    if(!tmp || len <= 0 || type != JSTRING)
        return NULL;

    e_name = strndup(tmp, len);
    if(!e_name)
        return _gen_resp_msg(id, code, "msg is invalid");

    tmp = NULL;
    len = 0;
    type = 0;

    tmp = json_get_value_by_name(payload, strlen(payload), "params", &len, &type);
    if(!tmp || len <= 0 || type != JOBJECT){
        free(e_name);
        log_e(LOG_TAG, "json format is not correct: %s\n", payload);
        return _gen_resp_msg(id, code, "msg is invalid");
    }
        
    output = strndup(tmp, len);
    if(!output){
        free(e_name); 
        return _gen_resp_msg(id, code, "msg is invalid");
    } 

    memset(&param, 0, sizeof(param));
    param.data = NULL;
    json_parse_name_value(output, strlen(output), _foreach_method_call, &param);
  
    if(!param.data){
        free(e_name); 
        free(output);
        return _gen_resp_msg(id, code, "msg is invalid");
    }
    
    method_ret = malloc(MAX_LENGTH_PARAM_VALUE*2);
    if(!method_ret){
        free(e_name); 
        free(output);
        free(param.data);
        return _gen_resp_msg(id, code, "no more memory");
    }

    code = g_wsa_cbs->cb_method_call(dn, e_name, param.data, param.cnt, method_ret, MAX_LENGTH_PARAM_VALUE*2, g_wsa_cbs->usr_data_method_call);
    if(code != 0){
        code = 105;
        msg = strdup("");
    }else{
        if(tmp[0] == '{')
            msg = strndup(tmp+1, strlen(tmp) - 1);
        else
            msg = strdup(tmp);
    } 
    
    ret = _gen_resp_msg(id, code, msg);
   
    free(param.data);
    if(msg)
        free(msg);
    free(e_name);
    free(output);
    free(method_ret);

    return ret;
}

