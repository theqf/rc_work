//
// Created by ubuntu on 2022/2/18.
//

#include "RcClient.h"

int64_t currentTimeMillis()
{
    using namespace std::chrono;
    return time_point_cast<milliseconds>(steady_clock::now()).time_since_epoch().count();
}

void RcClient::alloc_cb_udp(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    auto *rcClient = (RcClient *) handle->data;
    buf->base = (char *) rcClient->udpRecvBuf;
    buf->len = UDP_RECV_BUFFER_LEN;
}

RcClient::RcClient(CMD_LOGO mode)
{
    udpRecvBuf = new uint8_t[UDP_RECV_BUFFER_LEN];
    logo = mode;
    heart_buffer[4] = logo;
    heart_buffer[5] = '#';
    heart_buffer_len = 6;
}

RcClient::~RcClient()
{
    delete[] udpRecvBuf;
}

bool RcClient::initUdp() {
    struct sockaddr_in sin{};
    int ret = uv_ip4_addr(local_ip.c_str(), 0, &sin);
    if (ret != 0) {
        cout<<"uv_ip4_addr error "<<endl;
        return false;
    }
    do {
        udp_watcher.data = this;
        ret = uv_udp_init(loop, &udp_watcher);
        if (ret != 0){
            cout<<"uv_udp_init error "<<endl;
            break;
        }
        ret = uv_udp_bind(&udp_watcher, (const struct sockaddr *) &sin, 0);
        if (ret != 0){
            cout<<"uv_udp_bind error "<<endl;
            break;
        }
        ret = uv_ip4_addr(server_ip.c_str(), server_port, &dest_addr);
        if (ret != 0){
            cout<<"uv_ip4_addr error "<<endl;
            break;
        }
        int bufferLenValue = 1024 * 512;
        //设置接收缓冲区大小和发送缓冲区大小
        ret = uv_recv_buffer_size((uv_handle_t *) &udp_watcher, &bufferLenValue);
        ret = uv_send_buffer_size((uv_handle_t *) &udp_watcher, &bufferLenValue);
        //获取大小
        int recvBufferLenValue = 0, sendBufferLenValue = 0;
        ret = uv_recv_buffer_size((uv_handle_t *) &udp_watcher, &recvBufferLenValue);
        ret = uv_send_buffer_size((uv_handle_t *) &udp_watcher, &sendBufferLenValue);
#if UV_VERSION_MINOR >= 28
        ret = uv_udp_connect(&udp_watcher, (const struct sockaddr *) &dest_addr);
        if (ret != 0){
            cout<<"uv_udp_connect error "<<endl;
            break;
        }
#endif
        ret = uv_udp_recv_start(&udp_watcher, alloc_cb_udp, udp_cb);
        if (ret != 0){
            cout<<"uv_udp_recv_start error "<<endl;
            break;
        }
    } while (0);
    if (ret != 0) {
        uv_close(reinterpret_cast<uv_handle_t *>(&udp_watcher), [](uv_handle_t *handle) {
        });
    }
    return ret == 0;
}

void RcClient::udp_cb(uv_udp_t *handle,
                               ssize_t size,
                               const uv_buf_t *buf,
                               const struct sockaddr *addr,
                               unsigned flags) {
    if (size <= 0) return;
    auto *rcClient = (RcClient *) handle->data;

    auto *data = (uint8_t *) buf->base;
    int data_len = size;
    if (rcClient->recv_callback) {
        rcClient->recv_callback(data, data_len);
    }
}

void RcClient::timer_check_cb(uv_timer_t *handle)
{
    auto *rcClient = (RcClient *) handle->data;
    if (!rcClient->running) {
        uv_stop(rcClient->loop);
        return;
    }
    bool need_send_heart = true;
    {
        lock_guard<mutex> lk(rcClient->send_list_mx);
        while (!rcClient->send_list.empty()) {
            const SendBuffer& buffer = rcClient->send_list.front();
            uv_buf_t uvBuf;
            uvBuf.base = (char *)(buffer.src_data);
            uvBuf.len = buffer.data_len;
#if UV_VERSION_MINOR < 28
            uv_udp_try_send(&rcClient->udp_watcher,&uvBuf,1,(sockaddr*)&rcClient->dest_addr);
#else
            uv_udp_try_send(&rcClient->udp_watcher, &uvBuf, 1, nullptr);
#endif
            rcClient->send_list.pop_front();
            need_send_heart = false;
        }
    }
    if (!need_send_heart){
        return;
    }
    //magic_str
    uv_buf_t uvBuf;
    uvBuf.base = rcClient->heart_buffer;
    uvBuf.len = rcClient->heart_buffer_len;
#if UV_VERSION_MINOR < 28
    int ret = uv_udp_try_send(&rcClient->udp_watcher,&uvBuf,1,(sockaddr*)&rcClient->dest_addr);
#else
    int ret = uv_udp_try_send(&rcClient->udp_watcher, &uvBuf, 1, nullptr);
#endif
    if (ret <= 0 ) {
        //return !(errno == EAGAIN || errno == ENOBUFS || errno == EINTR || errno == EWOULDBLOCK);
    }

}


void RcClient::set_server_ip_port(string ip, int port)
{
    server_ip = ip;
    server_port = port;
}

bool RcClient::thread_start()
{
    if (server_ip.empty()) {
        return false;
    }
    running = true;
    loop = new uv_loop_t;
    uv_loop_init(loop);

    if (!initUdp()){
        cout<<"init udp error "<<endl;
        return false;
    }

    evCheckTime.data = this;
    uv_timer_init(loop, &evCheckTime);
    uv_timer_start(&evCheckTime, timer_check_cb, 0, 10);

    client_thread = thread(bind(&RcClient::process, this));
    return true;
}

void RcClient::process()
{
    uv_run(loop, UV_RUN_DEFAULT);
    uv_walk(loop, [](uv_handle_t *h, void *args) {
        uv_close(h, [](uv_handle_t *handle) {
            if (uv_handle_type::UV_UDP == uv_handle_get_type(handle) ||
                uv_handle_type::UV_TCP == uv_handle_get_type(handle)) {
            }
        });
    }, nullptr);
    uv_run(loop, UV_RUN_DEFAULT);
    delete loop;
    running = false;
}

void RcClient::stop()
{
    running = false;
    client_thread.join();
}

void RcClient::set_recv_callback(function<void(uint8_t*,int)> f)
{
    recv_callback = f;
}

void RcClient::set_local_ip(string ip)
{
    local_ip = ip;
};

void RcClient::set_uid(uint32_t id)
{
    uid = id;
    *(uint32_t*)heart_buffer = htonl(id);
}

void RcClient::send_data(uint8_t *data, int len)
{
    lock_guard<mutex> lk(send_list_mx);
    SendBuffer buffer(data,len,uid,logo);
    send_list.emplace_back(buffer);
}

void RcClient::join()
{
    if (client_thread.joinable()){
        client_thread.join();
    }
}

