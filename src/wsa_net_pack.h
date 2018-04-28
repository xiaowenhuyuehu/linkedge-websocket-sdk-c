#ifndef __WSA_NET_PACK_H__
#define __WSA_NET_PACK_H__


#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif

int wsa_net_pack_register(const char *dn, const char *pk, const char *md5, const char *profile);

int wsa_net_pack_online(const char *dn, const char *pk);

int wsa_net_pack_offline(const char *dn, const char *pk);

int wsa_net_pack_report_property(const char *dn, const char *pk, const char *k, const char *v, WSA_Data_Type type);

int wsa_net_pack_report_event(const char *dn, const char *pk, const char *e,const WSA_Data data[], int count);

#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
}
#endif


#endif

