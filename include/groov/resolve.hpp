#pragma once

#include <groov/path_fwd.hpp>

#include <stdx/concepts.hpp>
#include <stdx/type_traits.hpp>

#include <cstdint>
#include <utility>

namespace groov {
struct invalid_t {};
struct too_long_t : invalid_t {};
struct mismatch_t : invalid_t {};
struct ambiguous_t : invalid_t {};

template <typename T>
concept valid_resolution = not(stdx::derived_from<T, invalid_t>);

enum struct resolution : std::uint8_t {
    OK,
    TOO_LONG,
    MISMATCH,
    AMBIGUOUS,
};

constexpr inline struct resolve_t {
    template <typename...> struct failure_t {};

    template <typename T, typename... Args>
        requires true
    constexpr auto operator()(T &&t, Args &&...args) const noexcept(
        noexcept(std::forward<T>(t).resolve(std::forward<Args>(args)...)))
        -> decltype(std::forward<T>(t).resolve(std::forward<Args>(args)...)) {
        return std::forward<T>(t).resolve(std::forward<Args>(args)...);
    }

    template <typename... Ts>
    constexpr auto operator()(Ts &&...) const -> failure_t<Ts...> {
        static_assert(stdx::always_false_v<Ts...>,
                      "No function call for resolve");
        return {};
    }
} resolve{};

template <typename... Ts>
using resolution_t = decltype(resolve(std::declval<Ts>()...));

template <typename... Args>
concept can_resolve = not(stdx::derived_from<resolution_t<Args...>, invalid_t>);

namespace detail {
template <typename... Args> struct resolves_q {
    template <typename T>
    using fn = std::bool_constant<can_resolve<T, Args...>>;
};
} // namespace detail

template <typename T, pathlike Path, typename... Args>
constexpr auto checked_resolve([[maybe_unused]] T const &t,
                               [[maybe_unused]] Path const &p,
                               Args const &...args) {
    using R = resolution_t<T, Path, Args...>;
    if constexpr (std::is_same_v<R, too_long_t>) {
        static_assert(
            stdx::always_false_v<T, Path, Args...>,
            "Attempting to access value with a path that is too long");
    } else if constexpr (std::is_same_v<R, mismatch_t>) {
        static_assert(stdx::always_false_v<T, Path, Args...>,
                      "Attempting to access value with a mismatched path");
    } else if constexpr (std::is_same_v<R, ambiguous_t>) {
        static_assert(stdx::always_false_v<T, Path, Args...>,
                      "Attempting to access value with an ambiguous path");
    } else {
        static_assert(not stdx::derived_from<R, invalid_t>,
                      "Attempting to access value with an invalid path");
        return t.resolve(p, args...);
    }
}

// template <typename... Args>
// constexpr static bool is_resolvable_v = can_resolve<Args...>;

// template <typename... Ts>
// using is_resolvable_t = std::bool_constant<is_resolvable_v<Ts...>>;

// template <typename... Args> struct resolves_q {
//     template <typename T> using fn = is_resolvable_t<T, Args...>;
// };

// template <typename T, typename... Args> struct resolve_result_q {
//     template <pathlike P> using fn = resolve_t<T, P, Args...>;
// };

// template <path_elemental P, path_elemental... Ps>
// constexpr auto root(path<P, Ps...> const &) -> path_elemental auto {
//     return P{};
// }

// constexpr inline auto root(path<> const &) -> path_elemental auto {
//     return ct_path_element<"">{};
// }

// template <pathlike P> constexpr auto without_root(P const &p) -> pathlike
// auto {
//     return p.without_root();
// }
} // namespace groov
