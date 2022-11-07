/*
 * UdpServerBase.cpp
 *
 *  Created on: 2017年3月14日
 *      Author: wwuq
 */

#include "UdpServerBase.h"
#include <iostream>
#include <cassert>
#include <iomanip>
#include <utility>
#include "CTimeUtil.h"

namespace RC {
    using namespace std::chrono;

    UdpServerBase::UdpServerBase() {
        loop = ev_loop_new(EVFLAG_AUTO);
        assert(loop != nullptr);
        recv_buffer = new uint8_t[buffer_length];
    }

    UdpServerBase::~UdpServerBase() {
        if (server_thread.joinable()) {
            server_thread.join();
        }
        delete[] recv_buffer;
    }

    void UdpServerBase::setNonBlocking(int sock) {
        int opts;
        opts = fcntl(sock, F_GETFL);
        if (opts < 0) {
            return;
        }
        opts = opts | O_NONBLOCK;
        if (fcntl(sock, F_SETFL, opts) < 0) {
            return;
        }
    }

    bool UdpServerBase::stopServer() {
        if (server_thread.joinable()) {
            server_thread.join();
        }
        return true;
    }

    void UdpServerBase::udp_cb(EV_P_ ev_io *w, int events) {
        auto *pServerInfo = (ServerInfo *) w->data;
        UdpServerBase *pServerBase = pServerInfo->udp_server_base;

        if (events & EV_READ) {
            NET net;
            net.fd = w->fd;
            //开始读取数据
            ssize_t bufLen = buffer_length;
            memset(&net._addr, 0, sizeof(struct sockaddr_in));
            net._addr.sin_family = AF_INET;
            //不设置会导致获取的ip为 0.0.0.0
            ssize_t len = 0;
            len = recvfrom(w->fd, pServerBase->recv_buffer, bufLen, 0, (struct sockaddr *) &net._addr, &net.addrLen);
            if (len <= 0) {
                return;
            }
            Command command;
            if (!command.parse(pServerBase->recv_buffer, len)){
                LOG(INFO)<<"parse error";
                return;
            }
            auto itr = pServerBase->groups.find(command.group_id);
            if (itr == pServerBase->groups.end()) {
                auto* group = new GroupRoverController;
                group->group_id = command.group_id;
                if (command.logo == rover) {
                    group->rover = net;
                } else {
                    group->Controller = net;
                }
                group->last_recv_time_ms = CurrentSteadyTimeMillis();
                pServerBase->groups.insert({command.group_id, group});
                LOG(INFO)<<"add new group "<<command.group_id<<" "<<(command.logo==rover?"rover":"controller");
                return;
            }
            GroupRoverController* group = itr->second;
            bool is_heart_bit = (command.data_len == 1 && command.data[0] == '#');

            if (command.logo == rover) {
                group->rover = net;
                if (!is_heart_bit){
                    pServerBase->send_data(command.data, command.data_len, group->Controller);
                }
            } else {
                group->Controller = net;
                if (!is_heart_bit) {
                    pServerBase->send_data(command.data, command.data_len, group->rover);
                }
            }
            group->last_recv_time_ms = CurrentSteadyTimeMillis();
        }
    }

    bool UdpServerBase::initUdpServer(ServerInfo *s) {
        struct addrinfo hints{};
        struct addrinfo *res = nullptr;
        memset(&hints, 0, sizeof(hints));
        hints.ai_flags = AI_PASSIVE;
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;
        char sport[10] = {0};
        sprintf(sport, "%d", s->server_port);
        bool retFlag = true;
        int fd = -1;
        do {
            int err = getaddrinfo(s->server_ip.c_str(), sport, &hints, &res);
            if (err != 0) {
                LOG(ERROR) << "getaddrinfo:" << gai_strerror(err);
                retFlag = false;
                break;
            }
            if (res == nullptr) {
                LOG(ERROR) << "getaddrinfo: res is NULL";
                retFlag = false;
                break;
            }

            fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
            if (fd < 0) {
                LOG(ERROR) << "socket err errno:" << errno;
                retFlag = false;
                break;
            }
            int optrVal = 1024 * 512;
            uint32_t optrLen = sizeof(int);
            if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *) &optrVal, optrLen) < 0) {
                LOG(ERROR) << "Failed to set SO_RCVBUF ";
                retFlag = false;
                break;
            }
            if (getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &optrVal, (socklen_t *) &optrLen) == 0) {
                LOG(INFO) << "SO_RCVBUF is " << optrVal;
            }

            int optsVal = 1024 * 512;
            uint32_t optsLen = sizeof(int);
            if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char *) &optsVal, optsLen) < 0) {
                LOG(ERROR) << "Failed to set SO_RCVBUF ";
                retFlag = false;
                break;
            }

            if (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &optsVal, (socklen_t *) &optsLen) == 0) {
                LOG(INFO) << "SO_RCVBUF is " << optsVal;
            }

            int optval = 1;
            if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))
                < 0) {
                LOG(ERROR) << "Failed to set address SO_REUSEADDR.";
                retFlag = false;
                break;
            }

            if (bind(fd, res->ai_addr, res->ai_addrlen) < 0) {
                LOG(ERROR) << "bind err errno:" << errno;
                retFlag = false;
                break;
            }
            memcpy(&s->udp_local_addr, res->ai_addr, res->ai_addrlen);
            s->udp_local_addr_len = res->ai_addrlen;
        } while (0);
        if (!retFlag) {
            if (fd > 0) close(fd);
        } else {
            s->fd = fd;
        }
        if (res != nullptr) {
            freeaddrinfo(res);
        }
        setNonBlocking(s->fd);
        LOG(INFO) << "initUdpServer OK  IP:" << s->server_ip << " PORT:" << sport;
        //添加服务器事件
        s->watcher.data = s;
        s->udp_server_base = this;
        s->loop = loop;
        s->eveInit = true;
        ev_io_init(&s->watcher, udp_cb, s->fd, EV_READ);
        ev_io_start(loop, &s->watcher);
        return retFlag;
    }

    void UdpServerBase::fatal_error(const char *msg) noexcept  //错误处理函数
    {
        perror(msg);
    }

    bool UdpServerBase::startServer() {
        //设置错误处理函数
        ev_set_syserr_cb(fatal_error);

        if (!initUdpServer(&serverInfo)) {
            return false;
        }

        ev500msTime.data = this;
        ev_timer_init(&ev500msTime, timer_500ms_cb, 0, 0.5);
        ev_timer_start(loop, &ev500msTime);

        ev1000msTime.data = this;
        ev_timer_init(&ev1000msTime, timer_1000ms_cb, 0, 1);
        ev_timer_start(loop, &ev1000msTime);

        server_thread = thread([this] { process(); });
        LOG(INFO) << "start udp server OK";
        return true;
    }

    void UdpServerBase::process() {
        ev_loop(loop, 0);
        ev_loop_destroy(loop);
    }

    bool UdpServerBase::setIpPort(string ip, int port) {
        serverInfo.server_ip = std::move(ip);
        serverInfo.server_port = port;
        return true;
    }

    void UdpServerBase::timer_500ms_cb(EV_P_ ev_timer *w, int events) {
        //UdpServerBase *udp_server_base = (UdpServerBase *) w->data;
    }

    void UdpServerBase::timer_1000ms_cb(EV_P_ ev_timer *w, int events) {
        auto *udpServerBase = (UdpServerBase*)w->data;
        int64_t now = CurrentSteadyTimeMillis();
        for(auto itr = udpServerBase->groups.begin(); itr != udpServerBase->groups.end();){
            if (now - itr->second->last_recv_time_ms > 2000) {
                LOG(INFO)<<"clear group "<<itr->first;
                delete itr->second;
                itr = udpServerBase->groups.erase(itr);
                continue;
            }
            itr++;
        }
    }

    bool UdpServerBase::send_data(uint8_t* data, int len, NET &net) {
        if (net.fd <= 0) {
            return false;
        }
        ssize_t ret = sendto(net.fd, data, len, MSG_DONTWAIT | MSG_NOSIGNAL,(const struct sockaddr*)&net._addr,net.addrLen);
        if (ret <= 0) {
           return false;
        }
        return true;
    }

    void UdpServerBase::join()
    {
        if (server_thread.joinable()){
            server_thread.join();
        }
    }

} /* namespace DB */
