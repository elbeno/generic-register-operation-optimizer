#pragma once

#include <groov/path_element.hpp>

#include <concepts>
#include <type_traits>

namespace groov {
template <path_elemental... Elems> struct path;

template <typename T>
concept pathlike = requires(T const &t) {
    {
        []<path_elemental... Elems>(path<Elems...> const &) {}(t)
    } -> std::same_as<void>;
};

template <pathlike Path, typename Value> struct value_path;

template <typename T>
concept valued = requires { typename std::remove_cvref_t<T>::value_t; };

template <typename T>
concept valued_pathlike = pathlike<T> and valued<T>;
} // namespace groov
