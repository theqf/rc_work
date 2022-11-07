/*
 * CtimeUtil.h
 *
 *  Created on: 2015年4月23日
 *      Author: wq-test
 */

#ifndef TOOL_CTIME_UTIL_H_
#define TOOL_CTIME_UTIL_H_
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <string>
#include <chrono>

class CTimeUtil
{
public:
    CTimeUtil();
    virtual ~CTimeUtil();
    static void GetTimeByFormat(const char* SimpleDateFormat, char* date);
    static void GetTimeByFormat(const char* SimpleDateFormat, std::string& date);
    //YYYYMMDDHHMISS
    static time_t GetSec(const char* date);
    //YYYYMMDDHHMISS  20190810  10880  20190811
    static void AddSec(char* date, int sec);
    static void AddSec(std::string& date, int sec);
};

int64_t CurrentSteadyTimeMillis();

#endif /* TOOL_CTIME_UTIL_H_ */
