#pragma once
#include <vector>
#include <list>
#include <set>
#include <span>
#include <unordered_map>
#include <cassert>
#include <cstddef>

template<typename T>
using Vector = std::vector<T>;

template<typename T>
using List = std::list<T>;

template<typename T>
using Set = std::set<T>;

template<typename T>
using Span = std::span<T>;

template<typename K, typename T>
using UnorderedMap = std::unordered_map<K, T>;

#define ASSERT(...) assert(__VA_ARGS__)
#define UNREACHABLE()  abort();

template <typename T>
class IteratorBase {
public:
    IteratorBase(T *val) : val_(val) {}
    IteratorBase() : val_(nullptr) {}

    //bool operator!=(const IteratorBase &other)
    bool operator!=(std::nullptr_t end)
    {
        return val_ != nullptr;
    }
    auto end()
    {
        return nullptr;
    }
    auto operator*()
    {
        return val_;
    }
    auto operator->()
    {
        return val_;
    }
    operator T*()
    {
        return val_;
    }

protected:
    T *val_{};
};
