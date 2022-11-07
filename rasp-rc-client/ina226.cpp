//
// Created by kingx on 2022/3/27.
//
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include "ina226.h"
#include <cstdio>
#include <cstdint>
#include <cmath>
#include <wiringPiI2C.h>

enum
{
    INA226_REG_CONFIGURATION = 0x00,
    INA226_REG_SHUNT_VOLTAGE = 0x01,
    INA226_REG_BUS_VOLTAGE   = 0x02,
    INA226_REG_POWER         = 0x03,
    INA226_REG_CURRENT       = 0x04,
    INA226_REG_CALIBRATION   = 0x05,
    INA226_REG_MASK_ENABLE   = 0x06,
    INA226_REG_ALERT_LIMIT   = 0x07,
    INA226_REG_MANUFACTURER  = 0xFE,
    INA226_REG_DIE_ID        = 0xFF,
};

#define INA226_RESET 0x8000
#define INA226_MASK_ENABLE_CVRF 0x0008

enum
{
    INA226_BIT_SHUNT= 0,
    INA226_BIT_BUS	= 1,
    INA226_BIT_MODE	= 2,
};

#define INA226_MODE_SHUNT 1
#define INA226_MODE_BUS 2
#define INA226_MODE_TRIGGERED 0
#define INA226_MODE_CONTINUOUS 4

enum
{
    INA226_MODE_OFF = 0,
    INA226_MODE_SHUNT_TRIGGERED = 1,
    INA226_MODE_BUS_TRIGGERED = 2,
    INA226_MODE_SHUNT_BUS_TRIGGERED = 3,
    INA226_MODE_OFF2 = 4,
    INA226_MODE_SHUNT_CONTINUOUS = 5,
    INA226_MODE_BUS_CONTINUOUS = 6,
    INA226_MODE_SHUNT_BUS_CONTINUOUS = 7,
};

enum
{
    INA226_TIME_01MS  = 0, /* 140us */
    INA226_TIME_02MS  = 1, /* 204us */
    INA226_TIME_03MS  = 2, /* 332us */
    INA226_TIME_05MS  = 3, /* 588us */
    INA226_TIME_1MS   = 4, /* 1.1ms */
    INA226_TIME_2MS   = 5, /* 2.115ms */
    INA226_TIME_4MS   = 6, /* 4.156ms */
    INA226_TIME_8MS   = 7, /* 8.244ms */
};

enum
{
    INA226_AVERAGES_1	= 0,
    INA226_AVERAGES_4	= 1,
    INA226_AVERAGES_16	= 2,
    INA226_AVERAGES_64	= 3,
    INA226_AVERAGES_128	= 4,
    INA226_AVERAGES_256	= 5,
    INA226_AVERAGES_512	= 6,
    INA226_AVERAGES_1024= 7,
};

const uint16_t averages[] = {1,4,16,64,128,256,512,1024};

// Conservative to be done for all averages
//const uint16_t wait[] = {142,206,336,595,1113,2142,4207,8346};

// Minimum wait for 1 average
const uint16_t wait[] = {0,0,0,0,500,1500,3550,7690};

// Time in us per iteration to calculate average for a given measure time
const uint16_t avgwaits[]={300,450,700,1200,1250,1300,1300,1320};

// 8ms
// 8340 21c on 1024, 9 on 512, 3 on 256, 1 on 128
// 7800 on 1,


#define INA226_ADDRESS 0x40

static int fd;
static uint64_t config;
static float current_lsb;

static bool running = true;

uint16_t read16(int fd, uint8_t ad){
    uint16_t result = wiringPiI2CReadReg16(fd,ad);
    // Chip uses different endian
    return  (result<<8) | (result>>8);
}

void write16(int fd, uint8_t ad, uint16_t value){
    // Chip uses different endian
    wiringPiI2CWriteReg16(fd,ad,(value<<8)|(value>>8));
}

// R of shunt resistor in ohm. Max current in Amp
void ina226_calibrate(float r_shunt, float max_current)
{
    current_lsb = max_current / (1 << 15);
    float calib = 0.00512 / (current_lsb * r_shunt);
    uint16_t calib_reg = (uint16_t) floorf(calib);
    current_lsb = 0.00512 / (r_shunt * calib_reg);

    //printf("LSB %f\n",current_lsb);
    //printf("Calib %f\n",calib);
    //printf("Calib R%#06x / %d\n",calib_reg,calib_reg);

    write16(fd,INA226_REG_CALIBRATION, calib_reg);
}

void ina226_configure(uint8_t bus, uint8_t shunt, uint8_t average, uint8_t mode)
{
    config = (average<<9) | (bus<<6) | (shunt<<3) | mode;
    write16(fd,INA226_REG_CONFIGURATION, config);
}

uint16_t ina226_conversion_ready()
{
    return read16(fd,INA226_REG_MASK_ENABLE) & INA226_MASK_ENABLE_CVRF;
}

void ina226_wait(){
    uint8_t average = (config>>9) & 7;
    uint8_t bus = (config>>6) & 7;
    uint8_t shunt = (config>>3) & 7;

    uint32_t total_wait = (wait[bus] + wait[shunt] + (average ? avgwaits[bus>shunt ? bus : shunt] : 0)) * averages[average];

    usleep(total_wait+1000);

    int count=0;
    while(!ina226_conversion_ready()){
        count++;
    }
    //printf("%d\n",count);
}

void ina226_read(float *voltage, float *current, float *power, float* shunt_voltage)
{
    if (voltage) {
        uint16_t voltage_reg = read16(fd,INA226_REG_BUS_VOLTAGE);
        *voltage = (float) voltage_reg * 1.25e-3;
    }

    if (current) {
        int16_t current_reg = (int16_t) read16(fd,INA226_REG_CURRENT);
        *current = (float) current_reg * 1000.0 * current_lsb;
    }

    if (power) {
        int16_t power_reg = (int16_t) read16(fd,INA226_REG_POWER);
        *power = (float) power_reg * 25000.0 * current_lsb;
    }

    if (shunt_voltage) {
        int16_t shunt_voltage_reg = (int16_t) read16(fd,INA226_REG_SHUNT_VOLTAGE);
        *shunt_voltage = (float) shunt_voltage_reg * 2.5e-3;
    }
}

inline void ina226_reset()
{
    write16(fd, INA226_REG_CONFIGURATION, config = INA226_RESET);
}

inline void ina226_disable()
{
    write16(fd, INA226_REG_CONFIGURATION, config = INA226_MODE_OFF);
}
static float voltage = 0, current = 0, power = 0, shunt = 0, energy = 0, price = 0;

void ina226_thread_function()
{
    const float kwh_price = 0.13;

    time_t rawtime;
    char buffer[80];
    running = true;

    fd = wiringPiI2CSetup(INA226_ADDRESS);
    if(fd < 0) {
        printf("Device not found");
        return ;
    }

    printf("Manufacturer 0x%X Chip 0x%X\n",read16(fd,INA226_REG_MANUFACTURER),read16(fd,INA226_REG_DIE_ID));

    // Shunt resistor (Ohm), Max Current (Amp)
    ina226_calibrate(0.002, 20.0);

    // Header
    //printf("date,time,timestamp,bus voltage(V),current (mA),power (mW),shunt voltage (mV),energy (kWh year)\n,price year");

    // BUS / SHUNT / Averages / Mode
    ina226_configure(INA226_TIME_8MS, INA226_TIME_8MS, INA226_AVERAGES_16, INA226_MODE_SHUNT_BUS_CONTINUOUS);

    while(running) {
        //ina226_configure(INA226_TIME_8MS, INA226_TIME_8MS, INA226_AVERAGES_16, INA226_MODE_SHUNT_BUS_TRIGGERED);
        //ina226_wait();

        // Read
        ina226_read(&voltage, &current, &power, &shunt);
        energy = voltage*current*24*365.25/1000000;
        price = energy * kwh_price;

        // Timestamp / Date
//        time(&rawtime);
//        struct tm *info = localtime( &rawtime );
//        strftime(buffer,80,"%Y-%m-%d,%H:%M:%S", info);
//
//        printf("%s,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.2f\n",buffer,(int)rawtime,voltage,current,voltage*current,shunt,energy,price);
//        fflush(NULL);

        usleep(100000);
    }
    ina226_disable();
    return;
}

void stop_ina226()
{
    running = false;
}

void get_ina226_info(float & v,float &  c, float & p, float & s,float &  e, float &  pri)
{
    v = abs(voltage);
    c = abs(current);
    p = abs(power);
    s = abs(shunt);
    e = abs(energy);
    pri =abs(price);
}
