/*
 * CtimeUtil.cpp
 *
 *  Created on: 2015年4月23日
 *      Author: wq-test
 */

#include "CTimeUtil.h"

CTimeUtil::CTimeUtil()
{
    // TODO Auto-generated constructor stub

}

CTimeUtil::~CTimeUtil()
{
    // TODO Auto-generated destructor stub
}
//YYYYMMDDHHMISS
time_t CTimeUtil::GetSec(const char* date)
{
    struct tm tm1;
    sscanf(date,"%4d%2d%2d%2d%2d%2d",&tm1.tm_year,&tm1.tm_mon,&tm1.tm_mday,&tm1.tm_hour,&tm1.tm_min,&tm1.tm_sec);
    tm1.tm_year -= 1900;
    tm1.tm_mon -= 1;
    return mktime(&tm1);
}
//YYYYMMDDHHMISS
void CTimeUtil::AddSec(char* date, int sec)
{
	struct tm tm1;
    struct tm p;
    sscanf(date,"%4d%2d%2d%2d%2d%2d",&tm1.tm_year,&tm1.tm_mon,&tm1.tm_mday,&tm1.tm_hour,&tm1.tm_min,&tm1.tm_sec);
    //夏令时不要
    tm1.tm_isdst = -1;
    time_t ti =  mktime(&tm1);
    ti+=sec;
    localtime_r(&ti,&p);
    p.tm_mon == 0?(p.tm_mon = 12,p.tm_year--):1;
    snprintf(date,15,"%d%02d%02d%02d%02d%02d",p.tm_year,p.tm_mon,p.tm_mday,p.tm_hour,p.tm_min,p.tm_sec);
}

//YYYYMMDDHHMISS
void CTimeUtil::AddSec(std::string& date, int sec)
{
    char tmpdate[64] = {0};
    struct tm tm1;
    struct tm p;
    sscanf(date.c_str(),"%4d%2d%2d%2d%2d%2d",&tm1.tm_year,&tm1.tm_mon,&tm1.tm_mday,&tm1.tm_hour,&tm1.tm_min,&tm1.tm_sec);
    //夏令时不要
    tm1.tm_isdst = -1;
    time_t ti =  mktime(&tm1);
    ti+=sec;
    localtime_r(&ti,&p);
    p.tm_mon == 0?(p.tm_mon = 12,p.tm_year--):1;
    snprintf(tmpdate,15,"%d%02d%02d%02d%02d%02d",p.tm_year,p.tm_mon,p.tm_mday,p.tm_hour,p.tm_min,p.tm_sec);
    date = tmpdate;
}

void CTimeUtil::GetTimeByFormat(const char* SimpleDateFormat, std::string& date){
    char strbuf[256]={0};
    char dateTmp[15] = {0};
    int strlens=0;
    int i = 0;
    time_t t;
    struct tm p;
    t = time(NULL);
    localtime_r(&t,&p);
    strcpy(strbuf,SimpleDateFormat);
    snprintf(dateTmp,15,"%d%02d%02d%02d%02d%02d",p.tm_year+1900,p.tm_mon+1,p.tm_mday,p.tm_hour,p.tm_min,p.tm_sec);
    dateTmp[15-1] ='\0';
    strlens = strlen(strbuf);
    for(i=0; i<strlens;)
    {
        if(i+3<strlens&&strncasecmp(strbuf+i,"YYYY",4)==0)
        {
            strbuf[i]=  dateTmp[0];
            strbuf[i+1]=  dateTmp[1];
            strbuf[i+2]=  dateTmp[2];
            strbuf[i+3]=  dateTmp[3];
            i+=3;
            continue;
        }
        if(i+1<strlens&&strncasecmp(strbuf+i,"MM",2)==0)
        {
            strbuf[i]=  dateTmp[4];
            strbuf[i+1]=  dateTmp[5];
            i+=1;
            continue;
        }
        if(i+1<strlens&&strncasecmp(strbuf+i,"DD",2)==0)
        {
            strbuf[i]=  dateTmp[6];
            strbuf[i+1]=  dateTmp[7];
            i+=1;
            continue;
        }
        if(i+1<strlens&&strncasecmp(strbuf+i,"HH",2)==0)
        {
            strbuf[i]=  dateTmp[8];
            strbuf[i+1]=  dateTmp[9];
            i+=1;
            continue;
        }
        if(i+1<strlens&&strncasecmp(strbuf+i,"MI",2)==0)
        {
            strbuf[i]=  dateTmp[10];
            strbuf[i+1]=  dateTmp[11];
            i+=1;
            continue;
        }
        if(i+1<strlens&&strncasecmp(strbuf+i,"SS",2)==0)
        {
            strbuf[i]=  dateTmp[12];
            strbuf[i+1]=  dateTmp[13];
            i+=1;
            continue;
        }
        i++;
    }
    date = strbuf;
}
void CTimeUtil::GetTimeByFormat(const char* SimpleDateFormat, char* date)
{
    char strbuf[256] = {0};
    char dateTmp[15] = {0};
    int strlens=0;
    int i = 0;
    time_t t;
    struct tm p;
    t = time(NULL);
    localtime_r(&t,&p);
    strcpy(strbuf,SimpleDateFormat);
    snprintf(dateTmp,15,"%d%02d%02d%02d%02d%02d",p.tm_year+1900,p.tm_mon+1,p.tm_mday,p.tm_hour,p.tm_min,p.tm_sec);
    dateTmp[15-1] ='\0';
    strlens = strlen(strbuf);
    for(i=0; i<strlens;)
    {
        if(i+3<strlens&&strncasecmp(strbuf+i,"YYYY",4)==0)
        {
            strbuf[i]=  dateTmp[0];
            strbuf[i+1]=  dateTmp[1];
            strbuf[i+2]=  dateTmp[2];
            strbuf[i+3]=  dateTmp[3];
            i+=3;
            continue;
        }
        if(i+1<strlens&&strncasecmp(strbuf+i,"MM",2)==0)
        {
            strbuf[i]=  dateTmp[4];
            strbuf[i+1]=  dateTmp[5];
            i+=1;
            continue;
        }
        if(i+1<strlens&&strncasecmp(strbuf+i,"DD",2)==0)
        {
            strbuf[i]=  dateTmp[6];
            strbuf[i+1]=  dateTmp[7];
            i+=1;
            continue;
        }
        if(i+1<strlens&&strncasecmp(strbuf+i,"HH",2)==0)
        {
            strbuf[i]=  dateTmp[8];
            strbuf[i+1]=  dateTmp[9];
            i+=1;
            continue;
        }
        if(i+1<strlens&&strncasecmp(strbuf+i,"MI",2)==0)
        {
            strbuf[i]=  dateTmp[10];
            strbuf[i+1]=  dateTmp[11];
            i+=1;
            continue;
        }
        if(i+1<strlens&&strncasecmp(strbuf+i,"SS",2)==0)
        {
            strbuf[i]=  dateTmp[12];
            strbuf[i+1]=  dateTmp[13];
            i+=1;
            continue;
        }
        i++;
    }
    strcpy(date,strbuf);
}

int64_t CurrentSteadyTimeMillis()
{
    using namespace std::chrono;
    return time_point_cast<milliseconds>(steady_clock::now()).time_since_epoch().count();
}

