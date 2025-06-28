#ifndef ST_H
#define ST_H

#include <string>

class ST {
public:
    enum DataType {
        INT,
        FLOAT,
        STRING,
        UINT16
    };

    DataType type = DataType::INT;
    std::string value = "";

    ST() = default;

    ST(DataType type, const std::string& value)
        : type(type), value(value) {
    }
};

#endif