#pragma once

#include <stdx/compiler.hpp>
#include <stdx/ct_string.hpp>
#include <stdx/type_traits.hpp>

#include <cstddef>
#include <string_view>
#include <type_traits>

namespace groov {
namespace detail {
template <typename T> struct path_element_helper {
    CONSTEVAL path_element_helper(auto t) : value(t) {}
    T value;
};

template <typename T> path_element_helper(T) -> path_element_helper<T>;
template <std::size_t N>
path_element_helper(char const (&)[N])
    -> path_element_helper<stdx::ct_string<N>>;

template <typename X, typename Y>
[[nodiscard]] CONSTEVAL auto operator==(path_element_helper<X> const &x,
                                        path_element_helper<Y> const &y)
    -> bool {
    if constexpr (std::is_same_v<X, Y>) {
        return x.value == y.value;
    } else {
        return false;
    }
}
} // namespace detail

template <detail::path_element_helper V> struct ct_path_element {};

template <typename T> struct rt_path_element {
    T value{};
};

template <typename T> rt_path_element(T) -> rt_path_element<T>;
template <std::size_t N>
rt_path_element(char const (&)[N]) -> rt_path_element<std::string_view>;

template <auto X, auto Y>
[[nodiscard]] CONSTEVAL auto operator==(ct_path_element<X>, ct_path_element<Y>)
    -> bool {
    if constexpr (requires { X == Y; }) {
        return X == Y;
    } else {
        return false;
    }
}

template <typename X, typename Y>
[[nodiscard]] constexpr auto operator==(rt_path_element<X> const &x,
                                        rt_path_element<Y> const &y) -> bool {
    if constexpr (requires { x.value == y.value; }) {
        return x.value == y.value;
    } else {
        return false;
    }
}

template <auto V, typename Y>
[[nodiscard]] constexpr auto operator==(ct_path_element<V> const,
                                        rt_path_element<Y> const &y) -> bool {
    if constexpr (requires { V.value == y.value; }) {
        return V.value == y.value;
    } else if constexpr (requires { Y{V.value} == y.value; }) {
        return Y{V.value} == y.value;
    } else {
        return false;
    }
}

template <typename T>
concept path_elemental =
    stdx::is_value_specialization_of_v<T, ct_path_element> or
    stdx::is_type_specialization_of_v<T, rt_path_element>;
} // namespace groov
