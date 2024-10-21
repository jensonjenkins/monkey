#pragma once

#include <string>

namespace object {

using object_t = const char*;

constexpr object_t INTEGER_OBJ  = "INTEGER";
constexpr object_t BOOLEAN_OBJ  = "BOOLEAN";
constexpr object_t NULL_OBJ     = "NULL";

struct object {
    const virtual std::string inspect() const noexcept = 0;
    const virtual object_t& type() const noexcept = 0;
};

class integer : public object {
public:
    integer() noexcept = default;
    integer(std::int64_t value) noexcept : _value(value) {};

    std::int64_t value() const noexcept { return _value; } 

    const std::string inspect() const noexcept { return std::to_string(_value); }
    const object_t& type() const noexcept { return INTEGER_OBJ; }
protected:
    std::int64_t _value;
};

class boolean : public object {
public:
    boolean() noexcept = default;
    boolean(bool value) noexcept : _value(value) {};

    bool value() const noexcept { return _value; } 

    const std::string inspect() const noexcept { return std::to_string(_value); }
    const object_t& type() const noexcept { return BOOLEAN_OBJ; }
protected:
    bool _value;
};

class null : public object {
public:
    null() noexcept = default;

    const std::string inspect() const noexcept { return "null"; }
    const object_t& type() const noexcept { return NULL_OBJ; }
};

} // namespace object
