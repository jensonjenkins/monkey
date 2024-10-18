#pragma once

#include <string>
#include <iostream>
#include <type_traits>

namespace parser {

class trace {
public:
    trace(std::string name) : _name(name) {
        if(_enable_trace) {
            std::cout<<std::string(_indent_level * 4, ' ')<<"BEGIN "<<_name<<std::endl;
            ++_indent_level;
        }
    };
    ~trace() {
        if(_enable_trace) {
            --_indent_level;
            std::cout<<std::string(_indent_level * 4, ' ')<<"END "<<_name<<std::endl;
        }
    };
private:
    std::string     _name;
    static bool     _enable_trace;
    static size_t   _indent_level;
};

} // namespace parser
