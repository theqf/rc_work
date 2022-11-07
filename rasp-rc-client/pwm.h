//
// Created by kingx on 2022/4/2.
//

#ifndef RASP_RC_CLIENT_PWM_H
#define RASP_RC_CLIENT_PWM_H
#include <iostream>
#include <functional>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <sys/time.h>
#include <unistd.h>
#include <wiringPi.h>


struct PWM_HANDLE
{
    int arr = 1000;  // 总频率 max 20000 (1000/50 * 1000)
    int ccr = 10;  // 小于这个1，大于这个0
    int index = 0; // 当前
    bool enable = false;
    void* pwm_handle = nullptr;
    bool status = false;
    std::function<void(void*)> close_func = nullptr;
    std::function<void(void*)> open_func = nullptr;
    int64_t begin_microsecond = 0;
};

struct PWM_MANAGE
{
#define MAX_HANDLE_SIZE 20
    int pwm_handle_size = MAX_HANDLE_SIZE;
    PWM_HANDLE pwm_handles[MAX_HANDLE_SIZE];
    int min_sleep_us = 50;
    bool running = true;
    int has_handle_size = 0;
};


int64_t get_microsecond();
void pwm_run_50hz(PWM_MANAGE* manage);
void close_pwm_handle(PWM_MANAGE *manage, PWM_HANDLE *handle);
PWM_HANDLE* add_pwm_handle(PWM_MANAGE *manage, int arr, int crr, void* pwm_handle, std::function<void(void*)> close_func ,std::function<void(void*)> open_func);
void change_crr(PWM_HANDLE* handle, int ccr);

#endif //RASP_RC_CLIENT_PWM_H
