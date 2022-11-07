/*
 * UdpServerBase.h
 *
 *  Created on: 2017年3月14日
 *      Author: wwuq
 */

#ifndef UDP_SERVER_BASE_H_
#define UDP_SERVER_BASE_H_
#undef EV_READ
#undef EV_WRITE
#undef EV_TIMEOUT
#undef EV_SIGNAL

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <vector>
#include <string>
#include <functional>
#include <glog/logging.h>

extern "C" {
#include "ev.h"
};

#include <cstdint>
#include <thread>
#include <list>
#include <map>
#include <mutex>
#include <unordered_map>

namespace RC {
    using namespace std;
    // 4byte(uid)|1byte(rover,controller)|xxx
    enum CMD_LOGO {
        rover = 0x01,
        controller = 0x02
    };

    class UdpServerBase {
    private:
        struct ServerInfo {
            int fd = 0;
            string server_ip;     //绑定的ip
            int server_port = 0;      //绑定的port
            ev_io watcher;
            struct sockaddr udp_local_addr;    //本地绑定的ip
            socklen_t udp_local_addr_len;       //长度
            UdpServerBase *udp_server_base = nullptr;
            struct ev_loop *loop = nullptr;
            uint8_t *recv_buffer = nullptr;
            bool eveInit = false;

            ~ServerInfo() {
                if (loop && eveInit) {
                    ev_io_stop(loop, &watcher);
                }
                if (fd > 0) {
                    close(fd);
                }
                delete[] recv_buffer;
            }
        };

        struct NET {
            int fd = 0;
            struct sockaddr_in _addr{};
            socklen_t addrLen = sizeof(struct sockaddr_in);
        };

        struct GroupRoverController {
            int group_id = 0;
            NET rover;
            NET Controller;
            int64_t last_recv_time_ms = 0;
        };

        struct Command {
            uint32_t group_id{};
            CMD_LOGO logo;
            uint8_t* data = nullptr;
            int data_len = 0;
            bool parse(uint8_t* d, int len) {
                if (len <= 5 ){
                    return false;
                }
                group_id = ntohl(*(uint32_t*)d);
                if (d[4] != CMD_LOGO::rover && d[4] != CMD_LOGO::controller) {
                    return false;
                }
                logo = (CMD_LOGO)d[4];
                this->data = d + 5;
                this->data_len = len - 5;
                return true;
            }
        };

        map<uint32_t,GroupRoverController*> groups;
        const static int buffer_length = 1024 * 512;
        struct ev_loop* loop = nullptr;
        ev_timer ev500msTime;
        ev_timer ev1000msTime;
        thread server_thread;
        uint8_t* recv_buffer = nullptr;
        ServerInfo serverInfo;
    private:
        bool initUdpServer(ServerInfo *);
        static void setNonBlocking(int sock); //设置非阻塞
        static void timer_500ms_cb(EV_P_ ev_timer *w, int events);
        static void timer_1000ms_cb(EV_P_ ev_timer *w, int events);
        static void udp_cb(EV_P_ ev_io *w, int events);
        static void fatal_error(const char *msg) noexcept;
    private:
        void process();
        static bool send_data(uint8_t* data, int len, NET &net);
    public:
        UdpServerBase();
        virtual ~UdpServerBase();
        bool stopServer();
        bool startServer();
        bool setIpPort(string ip, int port);
        void join();
    };

} /* namespace DB */

#endif /* UDP_SERVER_BASE_H_ */
