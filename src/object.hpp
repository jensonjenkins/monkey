#pragma once

#include "ast.hpp"
#include <string>

namespace object {

using object_t = const char*;

constexpr object_t INTEGER_OBJ      = "INTEGER";
constexpr object_t BOOLEAN_OBJ      = "BOOLEAN";
constexpr object_t NULL_OBJ         = "NULL";
constexpr object_t RETURN_VALUE_OBJ = "RETURN_VALUE";
constexpr object_t ERROR_OBJ        = "ERROR";
constexpr object_t FUNCTION_OBJ     = "FUNCTION";
constexpr object_t STRING_OBJ       = "STRING";
constexpr object_t BUILTIN_OBJ      = "BUILTIN";
constexpr object_t ARRAY_OBJ        = "ARRAY";


struct object {
    const virtual std::string& inspect() const noexcept = 0;
    const virtual object_t& type() const noexcept = 0;
    virtual ~object() noexcept = default; 
};

using builtin_fn_t = std::function<object*(std::vector<object*>)>;


class scope {
public:
    scope() noexcept = default;
    
    /**
     * Initializes a new scope with the outer scope initialized to this outer scope
     * @param outer the outer scope
     */
    scope(scope* outer) noexcept : _outer(outer) {}

    object* get(std::string_view name) const noexcept {
        auto it = _store.find(name);
        if(it == _store.end() && _outer != nullptr) {
            return _outer->get(name);
        } else if (it == _store.end()) {
            return nullptr;
        }
        return it->second;
    }

    object* set(std::string_view name, object* val) noexcept {
        _store[name] = val;
        return val;
    }

    const size_t scope_size() const noexcept { 
        if(_outer != nullptr) {
            return _outer->scope_size() + _store.size(); 
        } else {
            return _store.size();
        }
    }

    const std::string list_scope() const noexcept {
        std::string buf;
        if(_outer != nullptr) {
            buf += _outer->list_scope();
        }
        for (auto item : _store) {
            buf += std::string(item.first) + ", ";
        }
        return buf;
    }

protected:
    scope*                                          _outer;
    std::unordered_map<std::string_view, object*>   _store;
};


class integer : public object {
public:
    integer() noexcept = default;
    integer(std::int64_t value) noexcept : _value(value), _to_string(std::to_string(value)) {}

    std::int64_t value() const noexcept { return _value; } 

    const std::string& inspect() const noexcept { return _to_string; }
    const object_t& type() const noexcept { return INTEGER_OBJ; }
private:
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
private:
    std::string     _to_string;
    bool            _value;
};


class null : public object {
public:
    null() noexcept = default;

    const std::string& inspect() const noexcept { return _to_string; }
    const object_t& type() const noexcept { return NULL_OBJ; }

private:
    std::string     _to_string = "null";
};


class return_value : public object {
public:
    return_value(std::unique_ptr<object> value) noexcept : _value(std::move(value)) {}
    
    object* value() const noexcept { return _value.get(); }

    const std::string& inspect() const noexcept { return _value->inspect(); }
    const object_t& type() const noexcept { return RETURN_VALUE_OBJ; }
private:
    std::unique_ptr<object> _value;
};


class error : public object {
public:
    error() noexcept = default;
    error(std::string&& msg) noexcept : _message(std::move(msg)) {}

    const std::string& inspect() const noexcept { return _message; }
    const object_t& type() const noexcept { return ERROR_OBJ; }

private:
    std::string _message;
};


class function : public object {
public:
    function(const std::vector<std::unique_ptr<ast::identifier>>& parameters, ast::block_statement* body, scope* scope) 
        noexcept : _parameters(parameters), _body(body), _scope(scope) { build_to_string(); }

    const std::vector<std::unique_ptr<ast::identifier>>& parameters() const noexcept { return _parameters; }
    ast::block_statement* body() const noexcept { return _body; }
    scope* get_scope() const noexcept { return _scope; }

    const std::string& inspect() const noexcept { return _to_string; }
    const object_t& type() const noexcept { return FUNCTION_OBJ; }
    
private:
    void build_to_string() noexcept {
        _to_string += "fn(";
        for(int i=0;i<_parameters.size();i++){
            _to_string += _parameters[i]->to_string() + ',';
        }
        _to_string += ")";
        _to_string += _body->to_string();
    }

    scope*                                                  _scope;
    std::string                                             _to_string;
    ast::block_statement*                                   _body;
    const std::vector<std::unique_ptr<ast::identifier>>&    _parameters;
};


class string : public object {
public:
    string(std::string value) noexcept : _value(value) {}

    const std::string& value() const noexcept { return _value; }

    const std::string& inspect() const noexcept { return _value; }
    const object_t& type() const noexcept { return STRING_OBJ; }
    
private:
    std::string _value;
};


class builtin : public object {
public:
    builtin() noexcept = default;
    builtin(builtin_fn_t fn) noexcept : _fn(fn) {};

    builtin_fn_t fn() noexcept { return _fn; }

    const std::string& inspect() const noexcept { return _inspect; }
    const object_t& type() const noexcept { return BUILTIN_OBJ; }
    
private:
    std::string     _inspect = "builtin function";
    builtin_fn_t    _fn;
};


class array : public object {
public:
    array(std::vector<object*> elements) noexcept {
        for(object* e : elements) {
            _elements.push_back(std::unique_ptr<object>(e));
        }
        build_to_string();
    };

    const std::vector<std::unique_ptr<object>>& elements() const noexcept { return _elements; }

    const object_t& type() const noexcept { return ARRAY_OBJ; }
    const std::string& inspect() const noexcept { return _to_string; }
    
private:
    void build_to_string() noexcept {
        _to_string += "[";
        for (int i = 0; i < _elements.size(); i++) {
            _to_string += _elements[i]->inspect();
            if (i != _elements.size() - 1) { _to_string += ", "; }
        }
        _to_string += "]";
    }

    std::string                             _to_string;
    std::vector<std::unique_ptr<object>>    _elements;
};

} // namespace object


