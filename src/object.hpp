#pragma once

#include <string>

namespace object {

using object_t = const char*;

constexpr object_t INTEGER_OBJ      = "INTEGER";
constexpr object_t BOOLEAN_OBJ      = "BOOLEAN";
constexpr object_t NULL_OBJ         = "NULL";
constexpr object_t RETURN_VALUE_OBJ = "RETURN_VALUE";
constexpr object_t ERROR_OBJ        = "ERROR";

struct object {
    const virtual std::string& inspect() const noexcept = 0;
    const virtual object_t& type() const noexcept = 0;
    virtual ~object() noexcept = default; 
};

class integer : public object {
public:
    integer() noexcept = default;
    integer(std::int64_t value) noexcept : _value(value), _to_string(std::to_string(value)) {}

    std::int64_t value() const noexcept { return _value; } 

    const std::string& inspect() const noexcept { return _to_string; }
    const object_t& type() const noexcept { return INTEGER_OBJ; }
protected:
    std::string     _to_string;
    std::int64_t    _value;
};

class boolean : public object {
public:
    boolean() noexcept = default;
    boolean(bool value) noexcept : _value(value), _to_string(std::to_string(value)) {}

    bool value() const noexcept { return _value; } 

    const std::string& inspect() const noexcept { return _to_string; }
    const object_t& type() const noexcept { return BOOLEAN_OBJ; }
protected:
    std::string     _to_string;
    bool            _value;
};

class null : public object {
public:
    null() noexcept = default;

    const std::string& inspect() const noexcept { return _to_string; }
    const object_t& type() const noexcept { return NULL_OBJ; }

protected:
    std::string     _to_string = "null";
};

class return_value : public object {
public:
    return_value(std::unique_ptr<object> value) noexcept : _value(std::move(value)) {}
    
    object* value() const noexcept { return _value.get(); }

    const std::string& inspect() const noexcept { return _value->inspect(); }
    const object_t& type() const noexcept { return RETURN_VALUE_OBJ; }
protected:
    std::unique_ptr<object> _value;
};

class error : public object {
public:
    error() noexcept = default;
    error(std::string&& msg) noexcept : _message(std::move(msg)) {}

    const std::string& inspect() const noexcept { return _message; }
    const object_t& type() const noexcept { return ERROR_OBJ; }

protected:
    std::string _message;
};

} // namespace object


