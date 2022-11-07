/*
 * CLuaCfg.h
 *
 *  Created on:
 *      Author: xnj
 */

#ifndef CLUACFG_H_
#define CLUACFG_H_

#include "luasrc/lua.hpp"
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <string>
#include "CValue.h"
#include <map>

#define LUA_NIL "(null)"

class CLuaCfg {
private:
    lua_State *m_L;
    std::string m_luaPath;
    bool m_isReady;

    static int findBeginOfTable(std::string &str,uint32_t begin,const char*tableName);

    static int findEndOfTable(std::string &str,int begin);

public:
    struct table_t {
        std::string mapName;
        std::map<std::string, CValue> values;
    };

    CLuaCfg();

    CLuaCfg(const char *luaPath);

    virtual ~CLuaCfg();

public:
    bool setCfgPath(const char *path);  //设置配置文件位置并打开配置文件

    bool ReadTableFromItem(const char *lpszTableName, const char *lpszTableItem,
                           std::string &lpszTableValue);

    bool ReadTableFromItem(const char *lpszTableName, const char *lpszTableItem,
                           int *lpszTableValue);

    bool ReadTableFromItem(const char *lpszTableName, const char *lpszTableItem,
                               uint32_t *lpszTableValue);

    bool ReadTableFromItem(const char *lpszTableName, const char *lpszTableItem,
                                   float *lpszTableValue);

    bool ReadTableFromItem(const char *lpszTableName, const char *lpszTableItem,
                               bool *lpszTableValue);

    bool ReadTableArrayFromItem(const char *lpszArrayName,
                                std::vector<std::string> *vecstr);

    bool ReadTableArrayFromItem(const char *lpszArrayName,
                                std::vector<int> *vecint);

    static bool STReadTableFromItem(lua_State *L, const char *lpszTableName,
                                    const char *lpszTableItem, std::string &lpszTableValue);

    static bool STReadTableFromItem(lua_State *L, const char *lpszTableName,
                                    const char *lpszTableItem, int *lpszTableValue);

    lua_State *getLuaState();

    bool traverse_table(const char *lpszTableName, table_t &);

    bool table2file(table_t &);

};

#endif /* CLUACFG_H_ */
