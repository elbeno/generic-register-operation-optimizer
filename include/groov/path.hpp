#pragma once

#include <groov/attach_value.hpp>
#include <groov/path_fwd.hpp>
#include <groov/resolve.hpp>

#include <stdx/compiler.hpp>
#include <stdx/ct_string.hpp>
#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>
#include <stdx/type_traits.hpp>
#include <stdx/udls.hpp>

#include <algorithm>
#include <type_traits>

namespace groov {
template <pathlike T, pathlike U>
constexpr auto equivalent(T const &t, U const &u) -> bool {
    if constexpr (T::size() != U::size()) {
        return false;
    } else {
        return stdx::all_of([](path_elemental auto const &x,
                               path_elemental auto const &y) { return x == y; },
                            t.elems, u.elems);
    }
}

namespace detail {
template <typename... Ts> constexpr auto path_build_helper(Ts &&...ts) {
    return path{std::forward<Ts>(ts)...};
}
} // namespace detail

template <path_elemental... Elems> struct path {
    constexpr static auto ct_usable = (... and Elems::ct_usable);

    template <typename... Vs> constexpr auto operator()(Vs const &...vs) const {
        return attach_value(*this, vs...);
    }

    template <typename T>
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    constexpr auto operator=(T const &value) const {
        return (*this)(value);
    }

    template <pathlike P>
        requires(P::ct_usable and ct_usable)
    constexpr auto resolve(P) const {
        if constexpr (size() >= P::size()) {
            if constexpr (equivalent(P{}, path{}.take<P::size()>())) {
                return drop<P::size()>();
            } else {
                return mismatch_t{};
            }
        } else {
            return too_long_t{};
        }
    }

    template <pathlike P>
        requires(not P::ct_usable or not ct_usable)
    constexpr auto resolve(P const &p) const -> resolution {
        if constexpr (size() >= P::size()) {
            return equivalent(p, take<P::size()>()) ? resolution::OK
                                                    : resolution::MISMATCH;
        } else {
            return resolution::TOO_LONG;
        }
    }

    constexpr auto root() const {
        static_assert(size() > 0, "Trying to call root() on an empty path");
        return get<0>(elems);
    }

    CONSTEVAL auto ct_prefix() const {
        constexpr auto n = [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            auto i = size();
            auto const f = [&]<std::size_t I>() {
                using E = stdx::nth_t<I, Elems...>;
                if constexpr (not E::ct_usable) {
                    i = I;
                    return true;
                }
                return false;
            };
            (f.template operator()<Is>(), ...);
            return i;
        }(std::make_index_sequence<size()>{});
        return take<n>();
    }
    constexpr auto without_root() const { return drop<1>(); }
    constexpr auto parent() const { return take<size() - 1>(); }

    template <std::size_t N>
    constexpr auto operator[](std::integral_constant<std::size_t, N>) const {
        return *this / ct_path_element<N>{};
    }

    constexpr auto operator[](std::size_t n) const {
        return *this / rt_path_element{n};
    }

    constexpr static auto size =
        std::integral_constant<std::size_t, sizeof...(Elems)>{};
    constexpr static auto empty = std::bool_constant<sizeof...(Elems) == 0>{};

    [[no_unique_address]] stdx::tuple<Elems...> elems;

  private:
    template <std::size_t N> constexpr auto take() const {
        constexpr auto n = std::min(N, size());
        return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            return detail::path_build_helper(stdx::get<Is>(elems)...);
        }(std::make_index_sequence<n>{});
    }

    template <std::size_t N> constexpr auto drop() const {
        constexpr auto n = std::min(N, size());
        return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            return detail::path_build_helper(stdx::get<Is + n>(elems)...);
        }(std::make_index_sequence<size() - n>{});
    }

    friend constexpr auto operator==(path const &, path const &)
        -> bool = default;

    template <stdx::same_as_unqualified<path> Self, pathlike P>
        requires(not valued<P>)
    friend constexpr auto operator/(Self &&self, P &&p) -> pathlike auto {
        return detail::path_build_helper(stdx::tuple_cat(
            std::forward<Self>(self).elems, std::forward<P>(p).elems));
    }

    template <stdx::same_as_unqualified<path> Self, path_elemental E>
    friend constexpr auto operator/(Self &&self, E &&e) -> pathlike auto {
        return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            return path<Elems..., std::remove_cvref_t<E>>{
                stdx::get<Is>(std::forward<Self>(self).elems)...,
                std::forward<E>(e)};
        }(std::make_index_sequence<size()>{});
    }

    template <path_elemental E, stdx::same_as_unqualified<path> Self>
    friend constexpr auto operator/(E &&e, Self &&self) -> pathlike auto {
        return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            return path<std::remove_cvref_t<E>, Elems...>{
                std::forward<E>(e),
                stdx::get<Is>(std::forward<Self>(self).elems)...};
        }(std::make_index_sequence<size()>{});
    }
};

template <path_elemental T, path_elemental U>
constexpr auto operator/(T &&t, U &&u) -> pathlike auto {
    return path<std::remove_cvref_t<T>, std::remove_cvref_t<U>>{
        std::forward<T>(t), std::forward<U>(u)};
}

template <typename... Ts> path(Ts...) -> path<std::remove_cvref_t<Ts>...>;
template <typename... Ts>
path(stdx::tuple<Ts...>) -> path<std::remove_cvref_t<Ts>...>;

namespace detail {
template <stdx::ct_string Name> CONSTEVAL auto split_index() {
    constexpr auto s = stdx::split<Name, '['>();
    if constexpr (s.second.empty()) {
        return std::pair{s.first, stdx::ct_string{""}};
    } else {
        constexpr auto i = stdx::split<s.second, ']'>();
        static_assert(i.second.empty(), "Malformed index in path");
        return std::pair{s.first, i.first};
    }
}

template <stdx::ct_string Idx, typename T> CONSTEVAL auto extract_index() {
    return [&]<T... Is>(std::integer_sequence<T, Is...>) {
        return stdx::parse_literal<T, Idx.value[Is]...>();
    }(std::make_integer_sequence<T, std::size(Idx)>{});
}
} // namespace detail

template <stdx::ct_string S, auto... CTElems>
CONSTEVAL auto make_path() -> pathlike auto {
    constexpr auto p = stdx::split<S, '.'>();

    if constexpr (p.first.empty()) {
        static_assert(p.second.empty(), "Malformed path");
        return path{};
    } else {
        auto const f = [&]<auto... Es>() {
            if constexpr (p.second.empty()) {
                return path{CTElems..., Es...};
            } else {
                return make_path<p.second, CTElems..., Es...>();
            }
        };

        constexpr auto i = detail::split_index<p.first>();
        if constexpr (i.second.empty()) {
            return f.template operator()<ct_path_element<p.first>{}>();
        } else {
            constexpr auto n = detail::extract_index<i.second, std::size_t>();
            return f.template
            operator()<ct_path_element<i.first>{}, ct_path_element<n>{}>();
        }
    }
}

namespace literals {
#if __clang__ && __clang_major__ <= 14
template <class T, T... chars>
CONSTEVAL_UDL auto operator""_g() -> pathlike auto {
    constexpr auto s = stdx::ct_string<sizeof...(chars) + 1U>{{chars..., 0}};
    return make_path<s>();
}

template <class T, T... chars>
CONSTEVAL_UDL auto operator""_r() -> pathlike auto {
    constexpr auto s = stdx::ct_string<sizeof...(chars) + 1U>{{chars..., 0}};
    return make_path<s>();
}

template <class T, T... chars>
CONSTEVAL_UDL auto operator""_f() -> pathlike auto {
    constexpr auto s = stdx::ct_string<sizeof...(chars) + 1U>{{chars..., 0}};
    return make_path<s>();
}
#else
template <stdx::ct_string S>
CONSTEVAL_UDL auto operator""_g() -> pathlike auto {
    return make_path<S>();
}

template <stdx::ct_string S>
CONSTEVAL_UDL auto operator""_r() -> pathlike auto {
    return make_path<S>();
}

template <stdx::ct_string S>
CONSTEVAL_UDL auto operator""_f() -> pathlike auto {
    return make_path<S>();
}
#endif

using stdx::literals::operator""_idx;

} // namespace literals
} // namespace groov
