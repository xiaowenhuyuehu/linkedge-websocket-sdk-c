#ifndef __UTIL_H__
#define __UTIL_H__

static inline int get_integer_length(unsigned int val)
{
    int ret = 0;
    
    do{
        ret++;
        val /= 10;
    }while(val > 0);

    return ret;
}


#endif

