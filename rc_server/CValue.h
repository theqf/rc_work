//
// Created by develop on 18-1-9.
//

#ifndef DBUDPPROXY_DBVALUE_H
#define DBUDPPROXY_DBVALUE_H

#include <string>
#include <iostream>

class CValue {
public:
    /** A predefined CValue that has not CValue. */
    static const CValue Null;

    /** Default constructor. */
    CValue();
    CValue(const CValue& other);
    CValue(CValue&& other);
    /** Create a CValue by an unsigned char CValue. */
    explicit CValue(unsigned char v);

    /** Create a CValue by an integer CValue. */
    explicit CValue(int v);

    /** Create a CValue by an unsigned CValue. */
    explicit CValue(unsigned int v);

    /** Create a CValue by a float CValue. */
    explicit CValue(float v);

    /** Create a CValue by a double CValue. */
    explicit CValue(double v);

    /** Create a CValue by a bool CValue. */
    explicit CValue(bool v);

    /** Create a CValue by a char pointer. It will copy the chars internally. */
    explicit CValue(const char* v);

    /** Create a CValue by a string. */
    explicit CValue(const std::string& v);

    /** Destructor. */
    ~CValue();
    /** Assignment operator, assign from CValue to CValue. */
    CValue& operator= (const CValue& other);
    /** Assignment operator, assign from CValue to CValue. It will use std::move internally. */
    CValue& operator= (CValue&& other);
    /** Assignment operator, assign from unsigned char to CValue. */
    CValue& operator= (unsigned char v);
    /** Assignment operator, assign from integer to CValue. */
    CValue& operator= (int v);
    /** Assignment operator, assign from integer to CValue. */
    CValue& operator= (unsigned int v);
    /** Assignment operator, assign from float to CValue. */
    CValue& operator= (float v);
    /** Assignment operator, assign from double to CValue. */
    CValue& operator= (double v);
    /** Assignment operator, assign from bool to CValue. */
    CValue& operator= (bool v);
    /** Assignment operator, assign from char* to CValue. */
    CValue& operator= (const char* v);
    /** Assignment operator, assign from string to CValue. */
    CValue& operator= (const std::string& v);

    /** != operator overloading */
    bool operator!= (const CValue& v);
    /** != operator overloading */
    bool operator!= (const CValue& v) const;
    /** == operator overloading */
    bool operator== (const CValue& v);
    /** == operator overloading */
    bool operator== (const CValue& v) const;

    /** Gets as a byte CValue. Will convert to unsigned char if possible, or will trigger assert error. */
    unsigned char asByte() const;
    /** Gets as an integer CValue. Will convert to integer if possible, or will trigger assert error. */
    int asInt() const;
    /** Gets as an unsigned CValue. Will convert to unsigned if possible, or will trigger assert error. */
    unsigned int asUnsignedInt() const;
    /** Gets as a float CValue. Will convert to float if possible, or will trigger assert error. */
    float asFloat() const;
    /** Gets as a double CValue. Will convert to double if possible, or will trigger assert error. */
    double asDouble() const;
    /** Gets as a bool CValue. Will convert to bool if possible, or will trigger assert error. */
    bool asBool() const;
    /** Gets as a string CValue. Will convert to string if possible, or will trigger assert error. */
    std::string asString() const;

    /**
     * Checks if the CValue is null.
     * @return True if the CValue is null, false if not.
     */
    bool isNull() const { return _type == Type::NONE; }

    /** CValue type wrapped by CValue. */
    enum class Type
    {
        /// no CValue is wrapped, an empty CValue
                NONE = 0,
        /// wrap byte
                BYTE,
        /// wrap integer
                INTEGER,
        /// wrap unsigned
                UNSIGNED,
        /// wrap float
                FLOAT,
        /// wrap double
                DOUBLE,
        /// wrap bool
                BOOLEAN,
        /// wrap string
                STRING
    };

    /** Gets the CValue type. */
    Type getType() const { return _type; }

    /** Gets the description of the class. */
    std::string getDescription() const;
private:
    void clear();
    void reset(Type type);

    union
    {
        unsigned char byteVal;
        int intVal;
        unsigned int unsignedVal;
        float floatVal;
        double doubleVal;
        bool boolVal;

        std::string* strVal;
    }_field;

    Type _type;
};


#endif //DBUDPPROXY_DBVALUE_H
