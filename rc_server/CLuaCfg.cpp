/*
 * CLuaCfg.cpp
 *
 *  Created on:
 *      Author: xnj
 */

#include "CLuaCfg.h"
#include <iostream>
#include <stdio.h>
#include <vector>
#include <sstream>
#include "CFileUtil.h"

using std::string;

using namespace std;

CLuaCfg::CLuaCfg() {
    m_isReady = false;
    m_L = luaL_newstate();
}

CLuaCfg::CLuaCfg(const char *luaPath) {
    m_isReady = false;
    m_L = luaL_newstate();
    m_luaPath.append(luaPath);
    if (0 == luaL_dofile(m_L, m_luaPath.c_str()))
        m_isReady = true;
    else
        printf("配置文件无效或路径错误！[%s][%s]\n", luaPath, lua_tostring(m_L, -1));
}

CLuaCfg::~CLuaCfg() {
    lua_close(m_L);
}

bool CLuaCfg::setCfgPath(const char *luaPath) {
    if (m_isReady)
        lua_close(m_L);
    m_isReady = false;
    m_luaPath.append(luaPath);
    if (0 == luaL_dofile(m_L, m_luaPath.c_str())) {
        printf("luaL_dofile OK！[%s][%s]\n", luaPath, lua_tostring(m_L, -1));
        m_isReady = true;
    } else {
        printf("配置文件读取错误![%s][%s]\n", luaPath, lua_tostring(m_L, -1));
        return false;
    }
    return true;
}

bool CLuaCfg::ReadTableFromItem(const char *lpszTableName,
                                const char *lpszTableItem, std::string &lpszTableValue) {
    printf("[%s]|[%s]\n", lpszTableName, lpszTableItem);
    auto type = lua_getglobal(m_L, lpszTableName);
    if (LUA_TTABLE != type) {
        printf("[%s] not a table\n", lpszTableName);
        return false;
    }
    lua_pushstring(m_L, lpszTableItem);
    type = lua_gettable(m_L, -2);
    if (LUA_TSTRING != type) {
        lua_pop(m_L, 1);
        printf("[%s]|[%s] error type:[%d] \n", lpszTableName, lpszTableItem, type);
        return false;
    }
    lpszTableValue.clear();
    auto *out = lua_tostring(m_L, -1);
    lpszTableValue.append(out);
    lua_pop(m_L, 2);
    if (strcmp(lpszTableValue.c_str(), LUA_NIL) == 0) {
        printf("%s --> %s not found!\n", lpszTableName, lpszTableItem);
        return false;
    }
    printf("[%s]|[%s][%s]\n", lpszTableName, lpszTableItem, lpszTableValue.c_str());
    return true;
}

bool CLuaCfg::ReadTableArrayFromItem(const char *lpszArrayName, std::vector<std::string> *vecstr) {
    printf("[%s]\n", lpszArrayName);
    std::string t_val;
    lua_settop(m_L, 0);
    auto type = lua_getglobal(m_L, lpszArrayName);
    if (LUA_TTABLE != type) {
        printf("[%s] not a table\n", lpszArrayName);
        return false;
    }
    auto n = luaL_len(m_L, 1);   //luaL_len可以获得table的元素个数
    int i = 1;
    for (i = 1; i <= n; ++i) {
        lua_pushnumber(m_L, i);  //往栈里面压入i
        auto type = lua_gettable(m_L, -2);
        if (LUA_TSTRING != type) {
            lua_pop(m_L, 1);
            return false;
        }
        t_val = lua_tostring(m_L, -1);
        vecstr->push_back(t_val);
        lua_pop(m_L, 1);
    }
    return true;
}

bool CLuaCfg::ReadTableArrayFromItem(const char *lpszArrayName, std::vector<int> *vecint) {
    lua_settop(m_L, 0);
    auto type = lua_getglobal(m_L, lpszArrayName);
    if (LUA_TTABLE != type) {
        printf("[%s] not a table\n", lpszArrayName);
        return false;
    }
    auto n = luaL_len(m_L, 1);   //luaL_len可以获得table的元素个数
    int i = 1;
    int t_tmp = 0;
    for (i = 1; i <= n; ++i) {
        lua_pushnumber(m_L, i);  //往栈里面压入i
        auto type = lua_gettable(m_L, -2);
        if (LUA_TNUMBER != type) {
            lua_pop(m_L, 1);
            return false;
        }
        t_tmp = lua_tonumber(m_L, -1);
        vecint->push_back(t_tmp);
        lua_pop(m_L, 1);
    }
    return true;
}

bool CLuaCfg::ReadTableFromItem(const char *lpszTableName,
                                const char *lpszTableItem, int *lpszTableValue) {
    printf("[%s]|[%s]\n", lpszTableName, lpszTableItem);
    auto type = lua_getglobal(m_L, lpszTableName);
    if (LUA_TTABLE != type) {
        printf("[%s] not a table\n", lpszTableName);
        return false;
    }
    lua_pushstring(m_L, lpszTableItem);
    type = lua_gettable(m_L, -2);
    if (LUA_TNUMBER != type) {
        lua_pop(m_L, 1);
        printf("[%s]|[%s] error type:[%d] \n", lpszTableName, lpszTableItem, type);
        return false;
    }
    *lpszTableValue = lua_tointeger(m_L, -1);
    lua_pop(m_L, 2);
    printf("[%s]|[%s][%d]\n", lpszTableName, lpszTableItem, *lpszTableValue);
    return true;
}

bool CLuaCfg::ReadTableFromItem(const char *lpszTableName,
                                const char *lpszTableItem, uint32_t *lpszTableValue) {
    printf("[%s]|[%s]\n", lpszTableName, lpszTableItem);
    auto type = lua_getglobal(m_L, lpszTableName);
    if (LUA_TTABLE != type) {
        printf("[%s] not a table\n", lpszTableName);
        return false;
    }
    lua_pushstring(m_L, lpszTableItem);
    type = lua_gettable(m_L, -2);
    if (LUA_TNUMBER != type) {
        lua_pop(m_L, 1);
        printf("[%s]|[%s] error type:[%d] \n", lpszTableName, lpszTableItem, type);
        return false;
    }
    *lpszTableValue = lua_tointeger(m_L, -1);
    lua_pop(m_L, 2);
    printf("[%s]|[%s][%u]\n", lpszTableName, lpszTableItem, *lpszTableValue);
    return true;
}

bool CLuaCfg::ReadTableFromItem(const char *lpszTableName,
                                const char *lpszTableItem, float *lpszTableValue) {
    printf("[%s]|[%s]\n", lpszTableName, lpszTableItem);
    auto type = lua_getglobal(m_L, lpszTableName);
    if (LUA_TTABLE != type) {
        printf("[%s] not a table\n", lpszTableName);
        return false;
    }
    lua_pushstring(m_L, lpszTableItem);
    type = lua_gettable(m_L, -2);
    if (LUA_TNUMBER != type) {
        lua_pop(m_L, 1);
        printf("[%s]|[%s] error type:[%d] \n", lpszTableName, lpszTableItem, type);
        return false;
    }
    *lpszTableValue = lua_tonumber(m_L, -1);
    lua_pop(m_L, 2);
    printf("[%s]|[%s][%f]\n", lpszTableName, lpszTableItem, *lpszTableValue);
    return true;
}

bool CLuaCfg::ReadTableFromItem(const char *lpszTableName,
                                const char *lpszTableItem, bool *lpszTableValue) {
    printf("[%s]|[%s]\n", lpszTableName, lpszTableItem);
    auto type = lua_getglobal(m_L, lpszTableName);
    if (LUA_TTABLE != type) {
        printf("[%s] not a table\n", lpszTableName);
        return false;
    }
    lua_pushstring(m_L, lpszTableItem);
    type = lua_gettable(m_L, -2);
    if (LUA_TBOOLEAN != type) {
        lua_pop(m_L, 1);
        printf("[%s]|[%s] error type:[%d] \n", lpszTableName, lpszTableItem, type);
        return false;
    }
    *lpszTableValue = (lua_toboolean(m_L, -1) == 1);
    lua_pop(m_L, 2);
    printf("[%s]|[%s][%s]\n", lpszTableName, lpszTableItem, (*lpszTableValue) ? "true" : "false");
    return true;
}

bool CLuaCfg::STReadTableFromItem(lua_State *L, const char *lpszTableName,
                                  const char *lpszTableItem, std::string &lpszTableValue) {
    printf("[%s]|[%s]\n", lpszTableName, lpszTableItem);
    auto type = lua_getglobal(L, lpszTableName);
    if (LUA_TTABLE != type) {
        printf("[%s] not a table\n", lpszTableName);
        return false;
    }
    lua_pushstring(L, lpszTableItem);

    type = lua_gettable(L, -2);
    if (LUA_TSTRING != type) {
        lua_pop(L, 1);
        printf("[%s]|[%s] error type:[%d] \n", lpszTableName, lpszTableItem, type);
        return false;
    }

    lpszTableValue.clear();
    lpszTableValue.append(lua_tostring(L, -1));
    lua_pop(L, 2);
    if (strcmp(lpszTableValue.c_str(), LUA_NIL) == 0) {
        printf("%s --> %s not found!\n", lpszTableName, lpszTableItem);
        return false;
    }
    return true;
}

bool CLuaCfg::STReadTableFromItem(lua_State *L, const char *lpszTableName,
                                  const char *lpszTableItem, int *lpszTableValue) {
    printf("[%s]|[%s]\n", lpszTableName, lpszTableItem);
    auto type = lua_getglobal(L, lpszTableName);
    if (LUA_TTABLE != type) {
        printf("[%s] not a table\n", lpszTableName);
        return false;
    }
    lua_pushstring(L, lpszTableItem);

    type = lua_gettable(L, -2);
    if (LUA_TNUMBER != type) {
        lua_pop(L, 1);
        printf("[%s]|[%s] error type:[%d] \n", lpszTableName, lpszTableItem, type);
        return false;
    }

    *lpszTableValue = lua_tonumber(L, -1);
    lua_pop(L, 2);
    return true;
}

bool CLuaCfg::table2file(table_t &tb) {
    string str = CFileUtil::getAllTextFromFile(m_luaPath);
    int beg = 0, end = 0;

    do {
        beg = CLuaCfg::findBeginOfTable(str, 0, tb.mapName.c_str());
        if (beg < 0) {
            break;
        }
        end = CLuaCfg::findEndOfTable(str, beg);
        if (end < 0) {
            break;
        }
    } while (0);
    if (beg == -1 || end == -1) {
        stringstream strbuf;

        strbuf << str;
        strbuf << tb.mapName << "={\n";
        for (auto &v:tb.values) {
            if (v.second.getType() == CValue::Type::BOOLEAN) {
                strbuf << "\t" << v.first << " = " << (v.second.asString()) << ";\n";
            } else if (v.second.getType() == CValue::Type::STRING) {
                strbuf << "\t" << v.first << " = \"" << (v.second.asString()) << "\";\n";
            } else if (v.second.getType() == CValue::Type::DOUBLE) {
                strbuf << "\t" << v.first << " = " << (v.second.asDouble()) << ";\n";
            } else if (v.second.getType() == CValue::Type::FLOAT) {
                strbuf << "\t" << v.first << " = " << (v.second.asFloat()) << ";\n";
            } else if (v.second.getType() == CValue::Type::INTEGER) {
                strbuf << "\t" << v.first << " = " << (v.second.asInt()) << ";\n";
            }
        }
        strbuf << "}\n";

        string outName = m_luaPath + ".tmp";
        ofstream out(outName);
        if (!out) {
            cout << "outName:" << outName << " open error" << endl;
            return false;
        }
        out << strbuf.str();
        out.flush();
        out.close();
        CFileUtil::mvFile(outName, m_luaPath);
    } else {
        stringstream string_buffer;

        string_buffer << str.substr(0, beg);

        string_buffer << tb.mapName << "={\n";
        for (auto &v:tb.values) {
            if (v.second.getType() == CValue::Type::BOOLEAN) {
                string_buffer << "\t" << v.first << " = " << (v.second.asString()) << ";\n";
            } else if (v.second.getType() == CValue::Type::STRING) {
                string_buffer << "\t" << v.first << " = \"" << (v.second.asString()) << "\";\n";
            } else if (v.second.getType() == CValue::Type::DOUBLE) {
                string_buffer << "\t" << v.first << " = " << (v.second.asDouble()) << ";\n";
            } else if (v.second.getType() == CValue::Type::FLOAT) {
                string_buffer << "\t" << v.first << " = " << (v.second.asFloat()) << ";\n";
            } else if (v.second.getType() == CValue::Type::INTEGER) {
                string_buffer << "\t" << v.first << " = " << (v.second.asInt()) << ";\n";
            }
        }
        string_buffer << "}";
        if (end != (int)(str.length() - 1)) {
            string_buffer << str.substr(end + 1);
        }
        string outName = m_luaPath + ".tmp";
        ofstream out(outName);
        if (!out) {
            cout << "outName:" << outName << " open error" << endl;
            return false;
        }
        out << string_buffer.str();
        out.flush();
        out.close();
        CFileUtil::mvFile(outName, m_luaPath);
    }
    if (m_L) {
        lua_close(m_L);
    }
    m_L = luaL_newstate();
    if (0 == luaL_dofile(m_L, m_luaPath.c_str()))
        m_isReady = true;
    else
        printf("配置文件无效或路径错误！[%s][%s]\n", m_luaPath.c_str(), lua_tostring(m_L, -1));
    return true;
}


int CLuaCfg::findBeginOfTable(string &str, uint32_t begin, const char *tableName) {
    bool isInQuotationMarks = false;  //是否在引号中
    bool isAnnotationType1 = false;   // --
    bool isAnnotationType2 = false;   // --[[
    bool isSkip = false;              //下一个字符是否跳过
    int braceNum = 0;  //花括号数量

    auto found = str.find(tableName);
    if (found == std::string::npos) {
        return -1;
    }

    auto length = str.length();
    for (auto i = begin; i < length; i++) {
        if (isSkip) {
            isSkip = false;
        } else if (isAnnotationType1) {
            if ((str.at(i) == '\r' && str.at(i + 1) == '\n')) {
                i++;
                isAnnotationType1 = false;
            } else if (str.at(i) == '\n') {
                isAnnotationType1 = false;
            }
        } else if (isAnnotationType2) {
            if (str.at(i) == ']' && str.at(i + 1) == ']') {
                i++;
                isAnnotationType2 = false;
            }
        } else if (isInQuotationMarks) {
            if (str.at(i) == '"') {
                isInQuotationMarks = false;
            }
        } else {
            if (str.at(i) == '\\') {
                isSkip = true;
            } else if (str.at(i) == '"') {
                isInQuotationMarks = true;
            } else if (str.at(i) == '-' && str.at(i + 1) == '-') {
                if (length >= i + 3 && str.at(i + 2) == '[' && str.at(i + 3) == '[') {
                    i += 3;
                    isAnnotationType2 = true;
                } else {
                    i++;
                    isAnnotationType1 = true;
                }
            } else if (str.at(i) == '{') {
                braceNum++;
            } else if (str.at(i) == '}') {
                braceNum--;
            } else if (braceNum == 0) {
                if (found == i && (i == 0 || str.at(i - 1) == ' ' || str.at(i - 1) == '\r' || str.at(i - 1) == '\n')) {
                    return i;
                } else if (i > found) {
                    found = str.find(tableName, i);
                    if (found == std::string::npos) {
                        return -1;
                    } else if (i == found) {
                        return i;
                    }
                }
            }
        }
    }
    return -1;
}


int CLuaCfg::findEndOfTable(string &str, int begin) {
    bool isInQuotationMarks = false;  //是否在引号中
    bool isAnnotationType1 = false;   // --
    bool isAnnotationType2 = false;   // --[[
    bool isSkip = false;              //下一个字符是否跳过
    int braceNum = 0;  //花括号数量

    int length = (int)str.length();
    for (int i = begin; i < length; i++) {
        if (isSkip) {
            isSkip = false;
        } else if (isAnnotationType1) {
            if ((str.at(i) == '\r' && str.at(i + 1) == '\n') || str.at(i) == '\n') {
                isAnnotationType1 = false;
            }
        } else if (isAnnotationType2) {
            if (str.at(i) == ']' && str.at(i + 1) == ']') {
                isAnnotationType2 = false;
            }
        } else if (isInQuotationMarks) {
            if (str.at(i) == '"') {
                isInQuotationMarks = false;
            }
        } else {
            if (str.at(i) == '\\') {
                isSkip = true;
            } else if (str.at(i) == '"') {
                isInQuotationMarks = true;
            } else if (str.at(i) == '-' && str.at(i + 1) == '-') {
                if (length >= i + 3 && str.at(i + 2) == '[' && str.at(i + 3) == '[') {
                    isAnnotationType2 = true;
                } else {
                    isAnnotationType1 = true;
                }
            } else if (str.at(i) == '{') {
                braceNum++;
            } else if (str.at(i) == '}') {
                if (--braceNum == 0) {
                    return i;
                }
            }
        }
    }
    return -1;
}

bool CLuaCfg::traverse_table(const char *lpszTableName, table_t &tb) {
    auto type = lua_getglobal(m_L, lpszTableName);
    if (LUA_TTABLE != type) {
        printf("[%s] is not a table!\n", lpszTableName);
        return false;
    }
    tb.mapName = lpszTableName;
    lua_pushnil(m_L);
    while (lua_next(m_L, -2)) {
        /*此时lua栈状态
        ----------------------------------
        |  -1 value
        |  -2 key
        |  -3 table NUMBER_TABLE
        ----------------------------------
        */
        if (!lua_isstring(m_L, -2)) {
            lua_pop(m_L, 1);
            continue;
        }
        string key = lua_tostring(m_L, -2);

        if (lua_isnumber(m_L, -1)) {
            CValue val(lua_tonumber(m_L, -1));
            tb.values.insert(pair<string, CValue>(key, val));
        } else if (lua_isstring(m_L, -1)) {
            CValue val(lua_tostring(m_L, -1));
            tb.values.insert(pair<string, CValue>(key, val));
        } else if (lua_isboolean(m_L, -1)) {
            CValue val(lua_toboolean(m_L, -1) == 1);
            tb.values.insert(pair<string, CValue>(key, val));
        }

        /*此时lua栈状态
        ----------------------------------
        |  -1 value
        |  -2 key
        |  -3 table NUMBER_TABLE
        ----------------------------------
        */
        lua_pop(m_L, 1);
        /*此时lua栈状态
        ----------------------------------
        |  -1 key
        |  -2 table NUMBER_TABLE
        ----------------------------------
        */
    }
    lua_pop(m_L, 1);
    return true;
}

lua_State *CLuaCfg::getLuaState() {
    lua_State *L = luaL_newstate();
    printf("getLuaState : [%s]\n", m_luaPath.c_str());
    luaL_loadfile(L, m_luaPath.c_str());
    return L;
}

