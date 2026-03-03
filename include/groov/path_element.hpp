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

template <typename T>
rt_path_element(T) -> rt_path_element<std::remove_cvref_t<T>>;
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
    stdx::is_value_specialization_of_v<std::remove_cvref_t<T>,
                                       ct_path_element> or
    stdx::is_type_specialization_of_v<std::remove_cvref_t<T>, rt_path_element>;

template <stdx::ct_string S> constexpr auto make_path_element() {
    return ct_path_element<detail::path_element_helper{S}>{};
}
template <auto V>
    requires(
        not stdx::is_value_specialization_of_v<decltype(V), stdx::ct_string>)
constexpr auto make_path_element() {
    return ct_path_element<detail::path_element_helper{V}>{};
}
template <typename T> constexpr auto make_path_element(T &&t) {
    return rt_path_element{std::forward<T>(t)};
}

namespace literals {
#if __clang__ && __clang_major__ <= 14
template <class T, T... chars> CONSTEVAL_UDL auto operator""_elem() {
    constexpr auto s = stdx::ct_string<sizeof...(chars) + 1U>{{chars..., 0}};
    return make_path_element<s>();
}
#else
template <stdx::ct_string S> CONSTEVAL_UDL auto operator""_elem() {
    return make_path_element<S>();
}
#endif

template <char... Chars> CONSTEVAL_UDL auto operator""_elem() {
    return make_path_element<stdx::parse_literal<std::size_t, Chars...>()>();
}
} // namespace literals
} // namespace groov
