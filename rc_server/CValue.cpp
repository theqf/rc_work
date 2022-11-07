//
// Created by develop on 18-1-9.
//

#include "CValue.h"
#include <cmath>
#include <iomanip>
#include <cstring>
#include <cfloat>

const CValue CValue::Null;

CValue::CValue()
        : _type(Type::NONE)
{
    memset(&_field, 0, sizeof(_field));
}

CValue::CValue(unsigned char v)
        : _type(Type::BYTE)
{
    _field.byteVal = v;
}

CValue::CValue(int v)
        : _type(Type::INTEGER)
{
    _field.intVal = v;
}

CValue::CValue(unsigned int v)
        : _type(Type::UNSIGNED)
{
    _field.unsignedVal = v;
}

CValue::CValue(float v)
        : _type(Type::FLOAT)
{
    _field.floatVal = v;
}

CValue::CValue(double v)
        : _type(Type::DOUBLE)
{
    _field.doubleVal = v;
}

CValue::CValue(bool v)
        : _type(Type::BOOLEAN)
{
    _field.boolVal = v;
}

CValue::CValue(const char* v)
        : _type(Type::STRING)
{
    _field.strVal = new (std::nothrow) std::string();
    if (v)
    {
        *_field.strVal = v;
    }
}

CValue::CValue(const std::string& v)
        : _type(Type::STRING)
{
    _field.strVal = new (std::nothrow) std::string();
    *_field.strVal = v;
}


CValue::CValue(const CValue& other)
        : _type(Type::NONE)
{
    *this = other;
}

CValue::CValue(CValue&& other)
        : _type(Type::NONE)
{
    *this = std::move(other);
}

CValue::~CValue()
{
    clear();
}

//#define MAX_ITOA_BUFFER_SIZE 256
//double atof(const char* str)
//{
//    char buf[MAX_ITOA_BUFFER_SIZE];
//    strncpy(buf, str, MAX_ITOA_BUFFER_SIZE);
//
//    // str_ip string, only remain 7 numbers after '.'
//    char* dot = strchr(buf, '.');
//    if (dot != nullptr && dot - buf + 8 <  MAX_ITOA_BUFFER_SIZE)
//    {
//        dot[8] = '\0';
//    }
//
//    return ::atof(buf);
//}

CValue& CValue::operator= (const CValue& other)
{
    if (this != &other) {
        reset(other._type);

        switch (other._type) {
            case Type::BYTE:
                _field.byteVal = other._field.byteVal;
                break;
            case Type::INTEGER:
                _field.intVal = other._field.intVal;
                break;
            case Type::UNSIGNED:
                _field.unsignedVal = other._field.unsignedVal;
                break;
            case Type::FLOAT:
                _field.floatVal = other._field.floatVal;
                break;
            case Type::DOUBLE:
                _field.doubleVal = other._field.doubleVal;
                break;
            case Type::BOOLEAN:
                _field.boolVal = other._field.boolVal;
                break;
            case Type::STRING:
                if (_field.strVal == nullptr)
                {
                    _field.strVal = new std::string();
                }
                *_field.strVal = *other._field.strVal;
                break;
            default:
                break;
        }
    }
    return *this;
}

CValue& CValue::operator= (CValue&& other)
{
    if (this != &other)
    {
        clear();
        switch (other._type)
        {
            case Type::BYTE:
                _field.byteVal = other._field.byteVal;
                break;
            case Type::INTEGER:
                _field.intVal = other._field.intVal;
                break;
            case Type::UNSIGNED:
                _field.unsignedVal = other._field.unsignedVal;
                break;
            case Type::FLOAT:
                _field.floatVal = other._field.floatVal;
                break;
            case Type::DOUBLE:
                _field.doubleVal = other._field.doubleVal;
                break;
            case Type::BOOLEAN:
                _field.boolVal = other._field.boolVal;
                break;
            case Type::STRING:
                _field.strVal = other._field.strVal;
                break;
            default:
                break;
        }
        _type = other._type;

        memset(&other._field, 0, sizeof(other._field));
        other._type = Type::NONE;
    }

    return *this;
}

CValue& CValue::operator= (unsigned char v)
{
    reset(Type::BYTE);
    _field.byteVal = v;
    return *this;
}

CValue& CValue::operator= (int v)
{
    reset(Type::INTEGER);
    _field.intVal = v;
    return *this;
}

CValue& CValue::operator= (unsigned int v)
{
    reset(Type::UNSIGNED);
    _field.unsignedVal = v;
    return *this;
}

CValue& CValue::operator= (float v)
{
    reset(Type::FLOAT);
    _field.floatVal = v;
    return *this;
}

CValue& CValue::operator= (double v)
{
    reset(Type::DOUBLE);
    _field.doubleVal = v;
    return *this;
}

CValue& CValue::operator= (bool v)
{
    reset(Type::BOOLEAN);
    _field.boolVal = v;
    return *this;
}

CValue& CValue::operator= (const char* v)
{
    reset(Type::STRING);
    *_field.strVal = v ? v : "";
    return *this;
}

CValue& CValue::operator= (const std::string& v)
{
    reset(Type::STRING);
    *_field.strVal = v;
    return *this;
}



bool CValue::operator!= (const CValue& v)
{
    return !(*this == v);
}
bool CValue::operator!= (const CValue& v) const
{
    return !(*this == v);
}

bool CValue::operator== (const CValue& v)
{
    const auto &t = *this;
    return t == v;
}
bool CValue::operator== (const CValue& v) const
{
    if (this == &v) return true;
    if (v._type != this->_type) return false;
    if (this->isNull()) return true;
    switch (_type)
    {
        case Type::BYTE:    return v._field.byteVal     == this->_field.byteVal;
        case Type::INTEGER: return v._field.intVal      == this->_field.intVal;
        case Type::UNSIGNED:return v._field.unsignedVal == this->_field.unsignedVal;
        case Type::BOOLEAN: return v._field.boolVal     == this->_field.boolVal;
        case Type::STRING:  return *v._field.strVal     == *this->_field.strVal;
        case Type::FLOAT:   return std::abs(v._field.floatVal  - this->_field.floatVal)  <= FLT_EPSILON;
        case Type::DOUBLE:  return std::abs(v._field.doubleVal - this->_field.doubleVal) <= DBL_EPSILON;
        default:
            break;
    };

    return false;
}

/// Convert CValue to a specified type
unsigned char CValue::asByte() const
{
    if (_type == Type::BYTE)
    {
        return _field.byteVal;
    }

    if (_type == Type::INTEGER)
    {
        return static_cast<unsigned char>(_field.intVal);
    }

    if (_type == Type::UNSIGNED)
    {
        return static_cast<unsigned char>(_field.unsignedVal);
    }

    if (_type == Type::STRING)
    {
        return static_cast<unsigned char>(atoi(_field.strVal->c_str()));
    }

    if (_type == Type::FLOAT)
    {
        return static_cast<unsigned char>(_field.floatVal);
    }

    if (_type == Type::DOUBLE)
    {
        return static_cast<unsigned char>(_field.doubleVal);
    }

    if (_type == Type::BOOLEAN)
    {
        return _field.boolVal ? 1 : 0;
    }

    return 0;
}

int CValue::asInt() const
{
    if (_type == Type::INTEGER)
    {
        return _field.intVal;
    }

    if (_type == Type::UNSIGNED)
    {
        return (int)_field.unsignedVal;
    }

    if (_type == Type::BYTE)
    {
        return _field.byteVal;
    }

    if (_type == Type::STRING)
    {
        return atoi(_field.strVal->c_str());
    }

    if (_type == Type::FLOAT)
    {
        return static_cast<int>(_field.floatVal);
    }

    if (_type == Type::DOUBLE)
    {
        return static_cast<int>(_field.doubleVal);
    }

    if (_type == Type::BOOLEAN)
    {
        return _field.boolVal ? 1 : 0;
    }

    return 0;
}


unsigned int CValue::asUnsignedInt() const
{
    if (_type == Type::UNSIGNED)
    {
        return _field.unsignedVal;
    }

    if (_type == Type::INTEGER)
    {
        return static_cast<unsigned int>(_field.intVal);
    }

    if (_type == Type::BYTE)
    {
        return static_cast<unsigned int>(_field.byteVal);
    }

    if (_type == Type::STRING)
    {
        // NOTE: strtoul is required (need to augment on unsupported platforms)
        return static_cast<unsigned int>(strtoul(_field.strVal->c_str(), nullptr, 10));
    }

    if (_type == Type::FLOAT)
    {
        return static_cast<unsigned int>(_field.floatVal);
    }

    if (_type == Type::DOUBLE)
    {
        return static_cast<unsigned int>(_field.doubleVal);
    }

    if (_type == Type::BOOLEAN)
    {
        return _field.boolVal ? 1u : 0u;
    }

    return 0u;
}

float CValue::asFloat() const
{
    if (_type == Type::FLOAT)
    {
        return _field.floatVal;
    }

    if (_type == Type::BYTE)
    {
        return static_cast<float>(_field.byteVal);
    }

    if (_type == Type::STRING)
    {
        return atof(_field.strVal->c_str());
    }

    if (_type == Type::INTEGER)
    {
        return static_cast<float>(_field.intVal);
    }

    if (_type == Type::UNSIGNED)
    {
        return static_cast<float>(_field.unsignedVal);
    }

    if (_type == Type::DOUBLE)
    {
        return static_cast<float>(_field.doubleVal);
    }

    if (_type == Type::BOOLEAN)
    {
        return _field.boolVal ? 1.0f : 0.0f;
    }

    return 0.0f;
}

double CValue::asDouble() const
{
    if (_type == Type::DOUBLE)
    {
        return _field.doubleVal;
    }

    if (_type == Type::BYTE)
    {
        return static_cast<double>(_field.byteVal);
    }

    if (_type == Type::STRING)
    {
        return static_cast<double>(atof(_field.strVal->c_str()));
    }

    if (_type == Type::INTEGER)
    {
        return static_cast<double>(_field.intVal);
    }

    if (_type == Type::UNSIGNED)
    {
        return static_cast<double>(_field.unsignedVal);
    }

    if (_type == Type::FLOAT)
    {
        return static_cast<double>(_field.floatVal);
    }

    if (_type == Type::BOOLEAN)
    {
        return _field.boolVal ? 1.0 : 0.0;
    }

    return 0.0;
}

bool CValue::asBool() const
{
    if (_type == Type::BOOLEAN)
    {
        return _field.boolVal;
    }

    if (_type == Type::BYTE)
    {
        return _field.byteVal == 0 ? false : true;
    }

    if (_type == Type::STRING)
    {
        return (*_field.strVal == "0" || *_field.strVal == "false") ? false : true;
    }

    if (_type == Type::INTEGER)
    {
        return _field.intVal == 0 ? false : true;
    }

    if (_type == Type::UNSIGNED)
    {
        return _field.unsignedVal == 0 ? false : true;
    }

    if (_type == Type::FLOAT)
    {
        return _field.floatVal == 0.0f ? false : true;
    }

    if (_type == Type::DOUBLE)
    {
        return _field.doubleVal == 0.0 ? false : true;
    }

    return false;
}

std::string CValue::asString() const
{
    if (_type == Type::STRING)
    {
        return *_field.strVal;
    }

    std::stringstream ret;

    switch (_type)
    {
        case Type::BYTE:
            ret << _field.byteVal;
            break;
        case Type::INTEGER:
            ret << _field.intVal;
            break;
        case Type::UNSIGNED:
            ret << _field.unsignedVal;
            break;
        case Type::FLOAT:
            ret << std::fixed << std::setprecision( 7 )<< _field.floatVal;
            break;
        case Type::DOUBLE:
            ret << std::fixed << std::setprecision( 16 ) << _field.doubleVal;
            break;
        case Type::BOOLEAN:
            ret << (_field.boolVal ? "true" : "false");
            break;
        default:
            break;
    }
    return ret.str();
}


static std::string getTabs(int depth)
{
    std::string tabWidth;

    for (int i = 0; i < depth; ++i)
    {
        tabWidth += "\t";
    }

    return tabWidth;
}

#define CC_SAFE_DELETE(p)            do { if(p) { delete (p); (p) = 0; } } while(0)

void CValue::clear()
{
    // Free memory the old value allocated
    switch (_type)
    {
        case Type::BYTE:
            _field.byteVal = 0;
            break;
        case Type::INTEGER:
            _field.intVal = 0;
            break;
        case Type::UNSIGNED:
            _field.unsignedVal = 0u;
            break;
        case Type::FLOAT:
            _field.floatVal = 0.0f;
            break;
        case Type::DOUBLE:
            _field.doubleVal = 0.0;
            break;
        case Type::BOOLEAN:
            _field.boolVal = false;
            break;
        case Type::STRING:
            CC_SAFE_DELETE(_field.strVal);
            break;
        default:
            break;
    }

    _type = Type::NONE;
}

void CValue::reset(Type type)
{
    if (_type == type)
        return;

    clear();

    // Allocate memory for the new value
    switch (type)
    {
        case Type::STRING:
            _field.strVal = new (std::nothrow) std::string();
            break;
        default:
            break;
    }

    _type = type;
}