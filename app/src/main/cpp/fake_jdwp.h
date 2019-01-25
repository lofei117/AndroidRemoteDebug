//
// Created by lofei on 2019/1/24.
//

#ifndef TESTJNI_FAKE_JDWP_H
#define TESTJNI_FAKE_JDWP_H

#endif //TESTJNI_FAKE_JDWP_H


enum JdwpTransportType {
    kJdwpTransportUnknown = 0,
    kJdwpTransportSocket,       // transport=dt_socket
    kJdwpTransportAndroidAdb,   // transport=dt_android_adb
};

struct JdwpOptions {
    JdwpTransportType transport = kJdwpTransportUnknown;
    bool server = false;
    bool suspend = false;
    std::string host = "";
    uint16_t port = static_cast<uint16_t>(-1);
};
