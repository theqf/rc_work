/*
 * CFileUtil.h
 *
 *  Created on: 2015年4月14日
 *      Author: wq-test
 */

#ifndef C_FILE_UTIL_H_
#define C_FILE_UTIL_H_

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdio>
#include <streambuf>

using namespace std;

class CFileUtil
{
public:
    CFileUtil();
    virtual ~CFileUtil();
public:
    static string getFileNameWithOutPostfix(string &);
    static string getFilePath(const string &);
    static void getAllFile(const string &path, vector<string>& files,const char* matching = nullptr, bool with_path = false);
    static void mvFile(vector<string>& files,const string &newPath);
    static void mvFile(const string &oldPath,const string &newPath);
    static void rmFile(vector<string>& files);
    static int  lock_set(int fd,short type);
    static bool strpp(const char *str1,const char *str2);
    static string getFileName(string path);
    static int make_dirs (std::string const & file_path);
    static double getDiskUsePercent(std::string const &file_path);
    static double getDiskUsePercent(std::string const &file_path, unsigned long long &totalSize, unsigned long long &freeDisk);
    static string getAllTextFromFile(const string &filePath);
    static bool touchFile(const string &filePath);
};

#endif /* C_FILE_UTIL_H_ */
