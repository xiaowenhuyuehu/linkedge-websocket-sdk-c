#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif

typedef enum {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERR,
    LOG_LEVEL_NONE,
} LOG_LEVEL;

typedef const char cchar;

#define COL_DEF "\x1B[0m"   //white
#define COL_RED "\x1B[31m"  //red
#define COL_GRE "\x1B[32m"  //green
#define COL_BLU "\x1B[34m"  //blue
#define COL_YEL "\x1B[33m"  //yellow
#define COL_WHE "\x1B[37m"  //white
#define COL_CYN "\x1B[36m"
#define COL_MAG "\x1B[35m"

void set_log_level(int lvl);

void log_print(LOG_LEVEL lvl, cchar *color, cchar *t, cchar *f, cchar *func, int l, cchar *fmt, ...);

#define log_d(tag, fmt,args...) \
    log_print(LOG_LEVEL_DEBUG,COL_WHE,tag,__FILE__,__FUNCTION__,__LINE__,fmt,##args)
#define log_i(tag, fmt,args...) \
    log_print(LOG_LEVEL_INFO,COL_GRE,tag,__FILE__,__FUNCTION__,__LINE__,fmt,##args)
#define log_w(tag, fmt,args...) \
    log_print(LOG_LEVEL_WARN,COL_CYN,tag,__FILE__,__FUNCTION__,__LINE__,fmt,##args)
#define log_e(tag,fmt,args...) \
    log_print(LOG_LEVEL_ERR,COL_YEL,tag,__FILE__,__FUNCTION__,__LINE__,fmt,##args)
#define log_f(tag,fmt,args...) \
    log_print(LOG_LEVEL_FATAL,COL_RED,tag,__FILE__,__FUNCTION__,__LINE__,fmt,##args)

#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
}
#endif

#endif

