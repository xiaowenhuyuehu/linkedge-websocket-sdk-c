#ifndef __WSA_NET_UNPACK_H__
#define __WSA_NET_UNPACK_H__

#include "wsa_net.h"

NET_MSG_TYPE wsa_get_msg_type(const char *cmd);

int wsa_get_msg_header(char *json, char **cmd, int *id, int *code, char **dn, char **payload);

const char *wsa_net_unpack_set_properties(int id, char *dn, char *payload);

const char *wsa_net_unpack_method_call(int id, char *dn, char *payload);

const char *wsa_net_unpack_get_properties(int id, char *dn, char *payload);

#endif

