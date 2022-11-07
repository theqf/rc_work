//
// Created by kingx on 2022/3/27.
//

#ifndef RASP_RC_CLIENT_INA226_H
#define RASP_RC_CLIENT_INA226_H
#include <cstdint>

void stop_ina226();
void ina226_thread_function();
void get_ina226_info(float & v,float &  c, float & p, float & s,float &  e, float &  pri);
#endif //RASP_RC_CLIENT_INA226_H
