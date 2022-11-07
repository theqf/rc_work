//
// Created by kingx on 2022/4/2.
//
#include "pwm.h"
#include <pthread.h>
#include <string.h>

int64_t get_microsecond()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*1000000 + tv.tv_usec;
}

static void myGpioSleep(int seconds, int micros)
{
    struct timespec ts, rem;

    ts.tv_sec  = seconds;
    ts.tv_nsec = micros * 1000;

    while (clock_nanosleep(CLOCK_REALTIME, 0, &ts, &rem))
    {
        /* copy remaining time to ts */
        ts = rem;
    }
}
#define THOUSAND 1000
#define MILLION  1000000
#define BILLION  1000000000
#define PI_MAX_BUSY_DELAY 100

void pwm_run_50hz(PWM_MANAGE* manage)
{
#if 0
    int ret =0;
    pid_t pid =getpid();

    int curschdu = sched_getscheduler(pid);
    if(curschdu <0 )
    {
        printf( "getschedu err %s\n",strerror( errno));
    }
    printf("schedu befor %d\n",curschdu);
    struct sched_param s_parm;
    s_parm.sched_priority = sched_get_priority_max(SCHED_FIFO);
    printf("schedu max %d min %d\n",sched_get_priority_max(SCHED_FIFO),sched_get_priority_min(SCHED_FIFO));
    ret = sched_setscheduler(pid, SCHED_FIFO, &s_parm);
    if(ret <0)
    {
        printf( "setschedu err %s\n",strerror( errno));
    }
    curschdu = sched_getscheduler(pid);
    printf("schedu after %d\n",curschdu);
#endif
    int hz50_mic = 1000/50 * 1000;
    while (manage->running) {
        bool need_sleep = true;
        int64_t now = get_microsecond();
        for (int i = 0; i < manage->pwm_handle_size; i++) {
            if (!manage->pwm_handles[i].enable) {
                continue;
            }
            int64_t step = hz50_mic / manage->pwm_handles[i].arr;
            if (manage->pwm_handles[i].begin_microsecond == 0) {
                manage->pwm_handles[i].begin_microsecond = now;
            }
            manage->pwm_handles[i].index = ((now - manage->pwm_handles[i].begin_microsecond)/step) % manage->pwm_handles[i].arr;
            if (manage->pwm_handles[i].index + 500 > manage->pwm_handles[i].ccr) {
                need_sleep = false;
            }
            bool state = (manage->pwm_handles[i].index >= manage->pwm_handles[i].ccr);
            if (state != manage->pwm_handles[i].status || manage->pwm_handles[i].index == 0) {
                manage->pwm_handles[i].status = state;
                if (manage->pwm_handles[i].index >= manage->pwm_handles[i].ccr) {
                    manage->pwm_handles[i].close_func(manage->pwm_handles[i].pwm_handle);
                } else {
                    manage->pwm_handles[i].open_func(manage->pwm_handles[i].pwm_handle);
                }
            }
        }

        if (need_sleep) {
            int64_t last = get_microsecond();
            int64_t sleep = manage->min_sleep_us - (last - now);
            if (sleep > 0) {
                int out = 0;
                while((out = usleep(sleep)) != 0) {
                    sleep -= out;
                };
            }
        }
    }
}

void close_pwm_handle(PWM_MANAGE *manage, PWM_HANDLE *handle)
{
    for (int i = 0; i < manage->pwm_handle_size; i++) {
        if (&manage->pwm_handles[i] == handle) {
            manage->pwm_handles[i].enable = false;
            manage->has_handle_size--;
            break;
        }
    }
    if (manage->has_handle_size == 0) {
        manage->min_sleep_us = 50;
        return;
    }
    int64_t min_sleep = 50;
    for (int i = 0; i < manage->pwm_handle_size; i++) {
        if (!manage->pwm_handles[i].enable) {
            continue;
        }
        int hz50_mic = 1000/50 * 1000;
        int64_t step = hz50_mic / manage->pwm_handles[i].arr;
        std::min(step , min_sleep);
    }
    manage->min_sleep_us = min_sleep;
}

PWM_HANDLE* add_pwm_handle(PWM_MANAGE *manage, int arr, int crr, void* pwm_handle, std::function<void(void*)> close_func ,std::function<void(void*)> open_func)
{
    PWM_HANDLE* handle = nullptr;
    for (int i = 0; i < manage->pwm_handle_size; i++) {
        if (manage->pwm_handles[i].enable) {
            continue;
        }
        handle = &manage->pwm_handles[i];
        break;
    }
    if (!handle) {
        return nullptr;
    }
    handle->arr = arr;
    handle->ccr = crr;
    handle->pwm_handle = pwm_handle;
    handle->close_func = close_func;
    handle->open_func  = open_func;
    int hz50_mic = 1000/50 * 1000;
    int64_t step = hz50_mic / handle->arr;
    if (manage->min_sleep_us > step) {
        manage->min_sleep_us = step;
    }
    handle->index = 0;
    handle->begin_microsecond = 0;
    handle->enable = true;
    manage->has_handle_size++;
    return handle;
}

void change_crr(PWM_HANDLE* handle, int ccr)
{
    handle->ccr = ccr;
    handle->begin_microsecond = get_microsecond();
}
