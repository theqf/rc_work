//
// Created by ubuntu on 2022/2/18.
//

#ifndef RC_SERVER_RCCLIENT_H
#define RC_SERVER_RCCLIENT_H

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#else

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#endif

#include <unordered_map>
#include <vector>
#include <list>
#include <string>
#include <cstring>
#include <iomanip>
#include <thread>
#include <cstdint>
#include <functional>
#include <mutex>
#include <set>
#include <condition_variable>
#include <iostream>
#include <fstream>
#include <queue>
#include <cmath>
#include <atomic>
#include "uv.h"

using namespace std;

class RcClient {
public:
    enum CMD_LOGO {
        rover = 0x01,
        controller = 0x02
    };
private:
    // 4byte(uid)|1byte(rover,controller)|xxx

    struct SendBuffer{
        SendBuffer(){

        }
        SendBuffer(uint8_t* data, int len, uint32_t group_id,CMD_LOGO logo)
        {
            *(uint32_t*)src_data = htonl(group_id);
            src_data[4] = logo;
            memcpy(src_data + 5,data, len);
            data_len = len + 5;
        }
        uint8_t src_data[1600]{};
        int data_len = 0;
    };
private:
    string server_ip;
    uint16_t server_port = 0;
    uv_loop_t *loop = nullptr;
    uv_udp_t udp_watcher{};
    struct sockaddr_in dest_addr{};
    static const int maxUvBufSize = 1;
    uv_buf_t uv_buf[maxUvBufSize]{};
    uint8_t *udpRecvBuf = nullptr;
    const static int UDP_RECV_BUFFER_LEN = 2048;
    CMD_LOGO logo;
    list<SendBuffer> send_list;
    mutex send_list_mx;
    string local_ip = "0.0.0.0";
private:
    bool running = false;
    function<void(uint8_t*,int)> recv_callback = nullptr;
    thread client_thread;
    uv_timer_t evCheckTime{};
    uint32_t uid = 0;
    char heart_buffer[512] = {0};
    int heart_buffer_len = 6;
private:
    bool initUdp();
    void process();
private:
    static void alloc_cb_udp(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
    static void
    udp_cb(uv_udp_t *handle, ssize_t size, const uv_buf_t *buf, const struct sockaddr *addr, unsigned flags);
    static void timer_check_cb(uv_timer_t *handle);
public:
    explicit RcClient(CMD_LOGO mode);
    ~RcClient();
    void set_local_ip(string ip);
    void set_uid(uint32_t id);
    void set_server_ip_port(string ip, int port);
    bool thread_start();
    void stop();
    void set_recv_callback(function<void(uint8_t*,int)> f);
    void send_data(uint8_t *data, int len);
    void join();
public:
    const static  uint8_t msg_type_feedback_battery_info = 10;
    const static uint8_t msg_type_rc_info = 11;
    const static uint8_t msg_type_start_camera = 12;
    const static uint8_t msg_type_remote_control = 13;
};
int64_t currentTimeMillis();

#endif //RC_SERVER_RCCLIENT_H
