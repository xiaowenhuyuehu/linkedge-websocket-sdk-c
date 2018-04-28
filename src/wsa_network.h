#ifndef __WSA_NETWORK_H__
#define __WSA_NETWORK_H__

#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif

int wsa_network_init(const char *url, const char *root, const char *pub, const char *key, int timeout);

int wsa_network_deinit();

#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
}
#endif

#endif

