#pragma once
#include <vector>
#include <list>
#include <cassert>
#include <cstddef>


template<typename T>
using Vector = std::vector<T>;


template<typename T>
using List = std::list<T>;

#define ASSERT(...) assert(__VA_ARGS__)
#define UNREACHABLE() __builtin_unreachable(); abort()
