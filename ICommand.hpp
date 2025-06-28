#pragma once

#ifndef ICOMMAND_H
#define ICOMMAND_H

#include <string>
#include "ST.hpp"

class ICommand
{
public:
    enum CommandType
    {
        PRINT,
        DECLARE,
        ADD,
        SUBTRACT,
        SLEEP,
        FOR
    };

    ICommand(int pid, CommandType command_type)
        : pid_(pid), command_type_(command_type)
    {
    }

    virtual void execute() = 0;
    virtual void setCore(int core) = 0;
    void setSubcommandLevel(int level) { sub_level_ = level; }
    int getSubcommandLevel() const { return sub_level_; }


protected:
    int pid_;
    CommandType command_type_;
    int sub_level_ = 0;

};

#endif