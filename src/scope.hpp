#pragma once

#include "object.hpp"
#include <unordered_map>

namespace object {

class scope {
public:
    scope() noexcept = default;

    object* get(std::string_view name) const noexcept {
        auto it = _store.find(std::string(name));
        if(it == _store.end()) {
            return nullptr;
        }
        return it->second;
    }
    object* get(const std::string& name) const noexcept {
        auto it = _store.find(name);
        if(it == _store.end()) {
            return nullptr;
        }
        return it->second;
    }

    object* set(std::string_view name, object* val) noexcept {
        _store[std::string(name)] = val;
        return val;
    }
    object* set(const std::string& name, object* val) noexcept {
        _store[name] = val;
        return val;
    }

protected:
    std::unordered_map<std::string, object*> _store;
};


} // namespace object


