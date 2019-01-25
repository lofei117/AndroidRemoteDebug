#include <jni.h>
#include <string>
#include <android/log.h>
#include <dlfcn.h>
#include <sys/system_properties.h>
#include <android/api-level.h>

#include "fake_jdwp.h"

#include "fake_dlfcn.h"

#define TAG "native-lib" // 这个是自定义的LOG的标识
#define DEBUG 1

#if DEBUG
    #define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__) // 定义LOGD类型
    #define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__) // 定义LOGI类型
    #define LOGW(...) __android_log_print(ANDROID_LOG_WARN,TAG ,__VA_ARGS__) // 定义LOGW类型
    #define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__) // 定义LOGE类型
    #define LOGF(...) __android_log_print(ANDROID_LOG_FATAL,TAG ,__VA_ARGS__) // 定义LOGF类型
#else
    #define LOGD(...)
    #define LOGI(...)
    #define LOGW(...)
    #define LOGE(...)
    #define LOGF(...)
#endif

void reloadJdwpPreNougat(jboolean);
void reloadJdwpNougat(jboolean);

extern "C" JNIEXPORT jstring JNICALL
Java_info_lofei_app_testjni_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C" JNIEXPORT void JNICALL
Java_info_lofei_app_testjni_MainActivity_reloadJdwp(JNIEnv* env, jobject obj, jboolean open){
    LOGD("reload jdwp called to %d.", open);

    char osVersion[PROP_VALUE_MAX];
    __system_property_get("ro.build.version.sdk", osVersion);
    int version = atoi(osVersion);
    LOGD("os version: %d", version);

    if (version >= __ANDROID_API_N__) {
        reloadJdwpNougat(open);
    } else {
        reloadJdwpPreNougat(open);
    }

}

/**
 * Android 7.0及以上重新装载jdwp
 * @param open
 */
void reloadJdwpNougat(jboolean open) {
    // TODO 需要考虑不同架构路径，如32位和64位不一样
    void *handler = v_dlopen("/system/lib64/libart.so", RTLD_NOW);
    if(handler == NULL){
        const char* err = dlerror();
        LOGD("dlerror: %s", err);
    }
    //对于debuggable false的配置，重新设置为可调试
    void (*allowJdwp)(bool);
    allowJdwp = (void (*)(bool)) v_dlsym(handler, "_ZN3art3Dbg14SetJdwpAllowedEb");
    allowJdwp(true);


    void (*pfun)();
    //关闭之前启动的jdwp-thread
    pfun = (void (*)()) v_dlsym(handler, "_ZN3art3Dbg8StopJdwpEv");
    pfun();

    if (open == JNI_TRUE) {
        //重新配置gJdwpOptions
//        bool (*parseJdwpOptions)(const std::string&);
//        parseJdwpOptions = (bool (*)(const std::string&)) v_dlsym(handler,
//                                                                "_ZN3art11CmdlineTypeINS_4JDWP11JdwpOptionsEE15ParseJdwpOptionERKNSt3__112basic_stringIcNS4_11char_traitsIcEENS4_9allocatorIcEEEESC_PS2_");
//        std::string options = "transport=dt_socket,address=8000,server=y,suspend=n";
//        parseJdwpOptions(options);

        JdwpOptions jdwpOptions;
        jdwpOptions.server = true;
        jdwpOptions.suspend = false;
        jdwpOptions.port = 8000;
        jdwpOptions.host = "127.0.0.1";
        jdwpOptions.transport = JdwpTransportType::kJdwpTransportSocket;

        void (*configureJdwp)(const void*);
        configureJdwp = (void (*)(const void*)) v_dlsym (handler, "_ZN3art3Dbg13ConfigureJdwpERKNS_4JDWP11JdwpOptionsE");

        configureJdwp(&jdwpOptions);

        //重新startJdwp
        pfun = (void (*)()) v_dlsym(handler, "_ZN3art3Dbg9StartJdwpEv");
        pfun();
    }
    v_dlclose(handler);
}

/**
 * Android 7.0以前版本重新装载jdwp
 * @param open
 */
void reloadJdwpPreNougat(jboolean open) {
    void *handler = dlopen("/system/lib/libart.so", RTLD_NOW);
    if(handler == NULL){
        const char* err = dlerror();
        LOGD("dlerror: %s", err);
    }
    //对于debuggable false的配置，重新设置为可调试
    void (*allowJdwp)(bool);
    allowJdwp = (void (*)(bool)) dlsym(handler, "_ZN3art3Dbg14SetJdwpAllowedEb");
    allowJdwp(true);


    void (*pfun)();
    //关闭之前启动的jdwp-thread
    pfun = (void (*)()) dlsym(handler, "_ZN3art3Dbg8StopJdwpEv");
    pfun();

    if (open == JNI_TRUE) {
        //重新配置gJdwpOptions
        bool (*parseJdwpOptions)(const std::string&);
        parseJdwpOptions = (bool (*)(const std::string&)) dlsym(handler,
                                                                "_ZN3art3Dbg16ParseJdwpOptionsERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE");
        std::string options = "transport=dt_socket,address=8000,server=y,suspend=n";
        parseJdwpOptions(options);

        //重新startJdwp
        pfun = (void (*)()) dlsym(handler, "_ZN3art3Dbg9StartJdwpEv");
        pfun();
    }
    dlclose(handler);
}