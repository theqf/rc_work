# rc_work
# rc car

## rasp-rc-client 树莓派客户端， rc_server 服务器  ，client-libs 可以基于这个开发客户端

```
RcClient rc_client(RcClient::controller);

int uid = 123;
rc_client.set_uid(uid);

rc_client.set_server_ip_port("10.0.0.1", 9001);

rc_client.set_recv_callback([&](uint8_t* data, int len){
        uint8_t msg_type = data[0];
        if (msg_type == RcClient::msg_type_feedback_battery_info) {
            std::string ss = (char*)(data + 1);
            std::vector<std::string> vv;
            SplitString(ss, vv, ",");
            if (vv.size() != 6) {
                return;
            }
            voltage = atof(vv[0].c_str());
            current = atof(vv[1].c_str());
            power = atof(vv[2].c_str());
            shunt = atof(vv[3].c_str());
            energy = atof(vv[4].c_str());
            price = atof(vv[5].c_str());
            //sscanf(reinterpret_cast<const char *>(data + 1), "%f,%f,%f,%f,%f,%f", voltage, current, power, shunt, energy, price);
            //LOGD("recv msg : %.3f,%.3f,%.3f,%.3f,%.3f,%.2f\n",voltage,current,voltage*current,shunt,energy,price);
            vol_update = true;
        }
    });

rc_client.thread_start();
```

// if send
```
	// L/R 50 - 160 左转右转 中间 115
    // G 0-100 前进
    // B 0-100 后退
    // N 0     回正
    auto vec = ctr_sprite_->getPosition();
    int lr = vec.x - beginPoint_.x;
    int gb = vec.y - beginPoint_.y;
    uint8_t send_buf[1024] = {0};
    send_buf[0] = RcClient::msg_type_rc_info;
    uint8_t *p_send = send_buf + 1;
    int send_len = 0;
    if (gb > 0) {
        // 前进
        p_send[send_len++] = 'G';
        p_send[send_len++] = gb * 2 * speed_rate_;
    } else if(gb < 0) {
        // 后退
        p_send[send_len++] = 'B';
        p_send[send_len++] = -gb * 2 * speed_rate_;
    }
    if (lr != 0) {
        // 右 115 - 160
        p_send[send_len++] = 'T';
        p_send[send_len++] = 115 + lr;
    } else {
        // 回正
        p_send[send_len++] = 'N';
        p_send[send_len++] = 0;
    }

    if (camera_send_count > 0) {
        camera_send_count--;
        if (camera_send_count == 0) {
            camera_send_zero_count = max_camera_send_count;
        }
        auto vec = camera_sprite_->getPosition();
        //80 - 220
        int lx = camera_scope.x - (vec.x - (cameraBeginPoint_.x - camera_scope.x/2));

        p_send[send_len++] = 'C';
        p_send[send_len++] =  lx;
    } else if (camera_send_zero_count > 0) {
        camera_send_zero_count --;
        p_send[send_len++] = 'C';
        p_send[send_len++] = 0;
    }

    rc_client.send_data(send_buf, send_len + 1);
```
// end
```
rc_client.stop();
```

// reconnect
```
rc_client.set_server_ip_port(ip, port);
rc_client.thread_start();
```