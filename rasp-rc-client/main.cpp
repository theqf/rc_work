#include <iostream>
#include <cstdio>
#include "RcClient.h"
#include <thread>
#include <unistd.h>
#include <getopt.h>
#include <sys/wait.h>
#include "ina226.h"
#include <wiringPi.h>
#include "pwm.h"
#include <bitset>

using namespace std;

string version = "0.1";

struct Command {
    char c;
    uint16_t proportion;
    uint16_t last_proportion;
    uint8_t is_efficient;
    int count;
};

uint8_t left_opened = 0;
uint8_t right_opened = 0;
uint8_t turn_opened = 0;

Command command_g = {'G',0,0,0,0};
Command command_t = {'R',0,0,0,0};
Command command_b = {'B',0,0,0,0};
Command command_n = {'N',0,0,0,0};
Command command_c = {'C',0,0,0,0};


const static int gpio_bin1 = 21;
const static int gpio_bin2 = 22;
const static int gpio_ain1 = 23;
const static int gpio_ain2 = 24;
const static int gpio_stby = 25;
const static int pwm_a = 27;
const static int pwm_b = 28;
const static int pwm_steering_gear = 29;

const static int pwm_camera = 26;

const static int e1a = 4;
const static int e1b = 5;

const static int base_range = 1000;

int turn_close = 20;

bool running = true;

PWM_HANDLE* pwm_left_wheel = nullptr;
PWM_HANDLE* pwm_right_wheel = nullptr;
PWM_HANDLE* pwm_turn_wheel = nullptr;
PWM_HANDLE* pwm_camera_handle = nullptr;

PWM_MANAGE pwm_manage;

void open_camera_pwm()
{
    pwm_camera_handle = add_pwm_handle(&pwm_manage, base_range, 0, nullptr, [&](void*){
        digitalWrite (pwm_camera,  LOW);
    } ,[&](void*) {
        digitalWrite (pwm_camera, HIGH);
    });
}

void set_camera_pwm(int rcc)
{
    change_crr(pwm_camera_handle, rcc);
}

/* USER CODE BEGIN 1 */
void USR_TIM_PWM_Open_Left_Wheel()
{
    if (left_opened) {
        return;
    }
    left_opened = 1;
    pwm_left_wheel = add_pwm_handle(&pwm_manage, base_range, 0, nullptr, [&](void*){
        digitalWrite (pwm_a,  LOW);
    } ,[&](void*) {
        digitalWrite (pwm_a, HIGH);
    });
}

void USR_TIM_PWM_Close_Left_Wheel()
{
    if (!left_opened) {
        return;
    }
    left_opened = 0;
    change_crr(pwm_left_wheel, 0);
}

//duty == 0~100
void USR_TIM_PWM_Set_Left_Compare(uint16_t duty)
{
    if (!left_opened) {
        return;
    }
    if(duty > 100) {
        duty = 100;
    }
    duty = duty * 10;
    change_crr(pwm_left_wheel, duty);
}

void USR_TIM_PWM_Open_Right_Wheel()
{
    if (right_opened) {
        return;
    }
    right_opened = 1;
    pwm_right_wheel = add_pwm_handle(&pwm_manage, base_range, 0, nullptr, [&](void*){
        digitalWrite (pwm_b,  LOW);
    } ,[&](void*) {
        digitalWrite (pwm_b, HIGH);
    });
}

void USR_TIM_PWM_Close_Right_Wheel()
{
    if (!right_opened) {
        return;
    }
    right_opened = 0;
    change_crr(pwm_right_wheel, 0);
}

//duty == 0~100
void USR_TIM_PWM_Set_Right_Compare(uint16_t duty)
{
    if (!right_opened) {
        return;
    }
    if(duty > 100) {
        duty = 100;
    }
    duty = duty * 10;
    change_crr(pwm_right_wheel, duty);
}


void USR_TIM_PWM_Open_All_Wheel()
{
    USR_TIM_PWM_Open_Left_Wheel();
    USR_TIM_PWM_Open_Right_Wheel();
}

void USR_TIM_PWM_Close_All_Wheel()
{
    USR_TIM_PWM_Close_Left_Wheel();
    USR_TIM_PWM_Close_Right_Wheel();
}

//duty == 0~100
void USR_TIM_PWM_Set_All_Compare(uint16_t duty)
{
    USR_TIM_PWM_Set_Left_Compare(duty);
    USR_TIM_PWM_Set_Right_Compare(duty);
}

void USR_TIM_PWM_OPEN_TURN()
{
    if (turn_opened) {
        return;
    }
    turn_opened = 1;
    pwm_turn_wheel = add_pwm_handle(&pwm_manage, base_range, 0, nullptr, [&](void*){
        digitalWrite (pwm_steering_gear,  LOW);
    } ,[&](void*) {
        digitalWrite (pwm_steering_gear, HIGH);
    });
}

void USR_TIM_PWM_CLOSE_TURN()
{
    if (!turn_opened) {
        return;
    }
    turn_opened = 0;
    change_crr(pwm_turn_wheel, 0);
}

void USR_TIM_PWM_Set_TURN_Compare(uint16_t duty)
{
    if (!turn_opened) {
        return;
    }
    // 50 ~ 160
    if (duty < 50) {
        duty = 50;
    }
    if (duty > 160) {
        duty = 160;
    }
    if (duty > 180) {
        duty = 180;
    }
    const float d = 100.0f/18.0f;
    // 25 ~ 125   180d
    int duty_1 = 250 + (float)duty * d;
    change_crr(pwm_turn_wheel, duty_1/10);
}

void USR_TIM_PWM_Set_TURN_Centre()
{
    if (turn_close > 0) {
        turn_close--;
        USR_TIM_PWM_Set_TURN_Compare(115);
    } else {
        change_crr(pwm_turn_wheel, 0);
    }
}

void wheel_back()
{
    digitalWrite (gpio_ain1,  LOW) ;
    digitalWrite (gpio_ain2,  HIGH) ;
    digitalWrite (gpio_bin1,  LOW) ;
    digitalWrite (gpio_bin2,  HIGH) ;
}

void wheel_go()
{
    digitalWrite (gpio_ain1,  HIGH) ;
    digitalWrite (gpio_ain2,  LOW) ;
    digitalWrite (gpio_bin1,  HIGH) ;
    digitalWrite (gpio_bin2,  LOW) ;
}

void osDelay(int ms)
{
    this_thread::sleep_for(chrono::milliseconds(ms));
}

void StartDoCommandTask()
{
    /* USER CODE BEGIN StartDefaultTask */
    while (running)
    {
        osDelay(5);
        if (command_g.is_efficient || command_b.is_efficient ) {
            digitalWrite (gpio_stby,  HIGH);
        } else {
            digitalWrite (gpio_stby,  LOW);
        }

        if (command_g.is_efficient) {
            wheel_go();
            USR_TIM_PWM_Set_All_Compare(command_g.proportion);
        }

        if (command_t.is_efficient) {
            turn_close = 20;
            USR_TIM_PWM_Set_TURN_Compare(command_t.proportion);
        }

        if (command_b.is_efficient) {
            wheel_back();
            USR_TIM_PWM_Set_All_Compare(command_b.proportion);
        }

        if (command_n.is_efficient) {
            USR_TIM_PWM_Set_TURN_Centre();
        }

        if (command_g.count < 200) command_g.count ++;
        if (command_t.count < 200) command_t.count ++;
        if (command_b.count < 200) command_b.count ++;
        if (command_n.count < 200) command_n.count ++;

        if (command_g.count > 100) {
            command_g.is_efficient = FALSE;
        }
        if (command_t.count > 100) {
            command_t.is_efficient = FALSE;
        }
        if (command_b.count > 100) {
            command_b.is_efficient = FALSE;
        }
        if (command_n.count > 100) {
            command_n.is_efficient = FALSE;
        }
    }
    /* USER CODE END StartDefaultTask */
}
void parse_command(uint8_t* data)
{
    switch((char)data[0]) {
        case 'G':
            command_g.proportion = data[1];
            command_g.is_efficient = TRUE;
            command_g.count = 0;
            command_b.is_efficient = FALSE;
            break;
        case 'T':
            command_t.proportion = data[1];
            command_t.is_efficient = TRUE;
            command_n.is_efficient = FALSE;
            command_t.count = 0;
            break;
        case 'B':
            command_b.proportion = data[1];
            command_b.is_efficient = TRUE;
            command_b.count = 0;
            command_g.is_efficient = FALSE;
            break;
        case 'N':
            command_n.proportion = data[1];
            command_n.is_efficient = TRUE;
            command_t.is_efficient = FALSE;
            command_n.count = 0;
            break;
        case 'C':
            set_camera_pwm(data[1]);
            break;
        default:
            break;
    }
}

int InitDaemon() {
    if (fork() != 0) {
        exit(0);
    }
    setsid();
    signal(SIGHUP, SIG_IGN);
    if (fork() != 0) {
        exit(0);
    }
    return 0;
}

static void signal_pipe_fun(int signal_type) {
}

int main(int argc, char *argv[]) {
    int c;  //输入标记
    int help_flag = 0;
    string server_ip = "10.0.0.4";
    int server_port = 9001;
    bool is_daemon = false;
    bool debug = false;
    uint32_t uid = 211;
    string port_dev = "/dev/ttyS0";
    int baud_base = 115200;
    bool test = false;

    string config_path;

    struct option longOpts[] =
            {
                    {"ip",                   required_argument, 0,        'i'},
                    {"port",                 required_argument, 0,        'p'},
                    {"daemon",               no_argument,       0,        'd'},
                    {"debug",                no_argument,       0,        'D'},
                    {"version",              no_argument,       0,        'v'},
                    {"config",               required_argument, 0,        'c'},
                    {"dev",                  required_argument, 0,        'e'},
                    {"baud_base",            required_argument, 0,        'b'},
                    {"uid",                  required_argument, 0,        'u'},
                    {"test",                 required_argument, 0,        't'},
                    {"help", 0,                                 &help_flag, 1},
                    {0,      0,                                 0,        0}
            };

    //有：表示有参数，两个：表示参数可选
    while ((c = getopt_long(argc, argv, "i:p:dDvc:e:b:u:t?", longOpts, NULL)) != EOF) {
        switch (c) {
            case 'i':
                server_ip = optarg;
                break;
            case 'p':
                server_port = atoi(optarg);
                break;
            case 'd':
                is_daemon = true;
                break;
            case 'D':
                debug = true;
                break;
            case 'v':
                printf("version : [%s]\n", version.c_str());
                exit(0);
            case 'c':
                config_path = optarg;
                break;
            case 'e':
                port_dev = optarg;
                break;
            case 'b':
                baud_base = atoi(optarg);
                break;
            case 'u':
                uid = atoi(optarg);
                break;
            case 't':
                test = true;
                break;
            case '?':
                exit(0);
            default:
                break;
        }
    }
    if (help_flag) {
        fprintf(stderr, "Usage: rc_server [OPTION]\n%s",
                "\t-i, --ip             ip\n "
                "\t-p, --port           port\n "
                "\t-d, --daemon         server run daemon\n"
                "\t-D, --debug          debug mode\n"
                "\t-c, --config         config path\n"
                "\t-e, --dev            /dev/ttyS0\n"
                "\t-b, --baud_base      115200\n"
                "\t-u, --uid            211\n"
                "\t-t, --test  \n"
                "\t-v, --version        version\n"
        );
        exit(0);
    }

    if (is_daemon) {
        InitDaemon();
    }
    signal(SIGPIPE, signal_pipe_fun);
    wiringPiSetup () ;

    pinMode(gpio_bin1, OUTPUT);
    pinMode(gpio_bin2, OUTPUT);
    pinMode(gpio_ain1, OUTPUT);
    pinMode(gpio_ain2, OUTPUT);
    pinMode(gpio_stby, OUTPUT);
    pinMode(pwm_a, OUTPUT);
    pinMode(pwm_b, OUTPUT);
    pinMode(pwm_steering_gear, OUTPUT);
    pinMode(pwm_camera, OUTPUT);


    pinMode(e1a, INPUT);
    pinMode(e1b, INPUT);

    USR_TIM_PWM_Open_All_Wheel();
    USR_TIM_PWM_OPEN_TURN();
    USR_TIM_PWM_Set_TURN_Centre();
    this_thread::sleep_for(chrono::milliseconds(300));
    change_crr(pwm_turn_wheel, 0);

    open_camera_pwm();


    RcClient rcClient(RcClient::rover);

    rcClient.set_server_ip_port(server_ip,server_port);
    rcClient.set_uid(uid);

    rcClient.set_recv_callback([&](uint8_t* data, int len) {
        if (len < 2) {
            return ;
        }
        uint8_t msg_type = data[0];
        data++;
        switch (msg_type) {
            case RcClient:: msg_type_rc_info:
            {
                for (int i = 0; i < len ; i += 2) {
                    if (debug) {
                        printf("DEBUG: %c %d\n",(char)(data+i)[0],(data+i)[1]);
                    }
                    parse_command(data + i);
                }
            }
                break;
            case RcClient::msg_type_start_camera:
            {
                //32bit (4byte)开关;    16bit * 20 （40 byte）
                uint32_t switch_bytes = *(uint32_t*)data;
                std:bitset<32> switch_bt(switch_bytes);
                // if switch_bt.to_string()[0] == '1';
            }
                break;
        };
    });

    thread ina226_thread = thread(ina226_thread_function);
    thread print_th = thread([&](){
        char buffer[1024];
        buffer[0] = RcClient::msg_type_feedback_battery_info;
        while (running) {
            osDelay(500);
            float voltage = 0, current = 0, power = 0, shunt = 0, energy = 0, price = 0;
            get_ina226_info(voltage , current , power , shunt , energy , price );
            sprintf(buffer + 1,"%.3f,%.3f,%.3f,%.3f,%.3f,%.2f",voltage,current,voltage*current,shunt,energy,price);
            int len = strlen(buffer + 1);
            if (debug) {
                printf("send msg : %.3f,%.3f,%.3f,%.3f,%.3f,%.2f\n",voltage,current,voltage*current,shunt,energy,price);
            }
            rcClient.send_data((uint8_t*)buffer, len + 1);
        }
    });
    std::thread th(std::bind(pwm_run_50hz,std::placeholders::_1),&pwm_manage);
    rcClient.thread_start();
    StartDoCommandTask();
    rcClient.join();
    USR_TIM_PWM_Close_All_Wheel();
    USR_TIM_PWM_CLOSE_TURN();
    return 0;
}
