#pragma once

#include "object.hpp"
#include <string_view>
#include <unordered_map>

namespace evaluator {

inline object::object* len_builtin_fn(std::vector<object::object*> args) noexcept {
    if (args.size() != 1) {
        return new object::error("wrong number of arguments. got=" +std::to_string(args.size()) + ", want=1" );
    }
    if (auto str = dynamic_cast<object::string*>(args[0])) {
        return new object::integer(str->value().size());
    }
    return new object::error("argument to len not supported, got " + std::string(args[0]->type()));
}

static std::unordered_map<std::string_view, object::builtin*> builtin_fn_map {
    {"len", new object::builtin(len_builtin_fn)}
};

static object::object* get_builtin(std::string_view builtin_fn_name) noexcept {
    auto it = builtin_fn_map.find(builtin_fn_name);
    if (it == builtin_fn_map.end()) {
        return nullptr;
    }
    return it->second;
}

}
