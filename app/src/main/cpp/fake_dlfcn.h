//
// Created by lofei on 2019/1/24.
//

#ifndef __FAKE_DLFCN
#define __FAKE_DLFCN

#include <android/log.h>

struct ctx;


void *v_dlopen(const char *filename, int flags);
int v_dlclose(void *handle);
void *v_dlsym(void *handle, const char *symbol);


#endif
