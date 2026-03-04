#pragma once

#include <groov/attach_value.hpp>
#include <groov/path_fwd.hpp>
#include <groov/resolve.hpp>

#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>
#include <stdx/type_traits.hpp>

// #include <boost/mp11/algorithm.hpp>
// #include <boost/mp11/list.hpp>

// #include <iterator>

template <typename...> struct undef;

namespace groov {
namespace detail {
template <typename... Ts> constexpr auto value_path_build_helper(Ts &&...ts) {
    return value_path{std::forward<Ts>(ts)...};
}
} // namespace detail

namespace value_path_detail {
// template <typename... Args>
// concept can_resolve = not std::same_as<resolve_t<Args...>, mismatch_t> and
//                       not std::same_as<resolve_t<Args...>, ambiguous_t>;

template <typename... Args> struct resolves_q {
    template <typename T>
    using fn = std::bool_constant<can_resolve<T, Args...>>;
};
} // namespace value_path_detail

template <pathlike Path, typename Value> struct value_path : Path {
    using value_t = Value;
    [[no_unique_address]] value_t value;

    template <pathlike P>
        requires(P::ct_usable and Path::ct_usable)
    constexpr auto resolve(P const &p) const {
        if constexpr (P::size() > Path::size()) {
            using leftover_t = resolution_t<P, Path>;
            if constexpr (pathlike<leftover_t>) {
                auto const valid_children = stdx::filter<
                    value_path_detail::resolves_q<leftover_t>::template fn>(
                    value);
                if constexpr (valid_children.size() == 0) {
                    return mismatch_t{};
                } else if constexpr (valid_children.size() > 1) {
                    return ambiguous_t{};
                } else {
                    auto const child = get<0>(valid_children);
                    if constexpr (pathlike<decltype(child)>) {
                        return groov::resolve(child, groov::resolve(p, Path{}));
                    } else {
                        return too_long_t{};
                    }
                }
            } else {
                return groov::resolve(p, Path{});
            }
        } else {
            using leftover_t = resolution_t<Path, P>;
            if constexpr (pathlike<leftover_t>) {
                if constexpr (leftover_t::empty() and value_t::size() == 1) {
                    return get<0>(value);
                } else {
                    return detail::value_path_build_helper(
                        groov::resolve(Path{}, p), value);
                }
            } else {
                return groov::resolve(Path{}, p);
            }
        }
    }

    template <pathlike P> constexpr auto operator[](P p) const {
        return checked_resolve(*this, p);
    }

    constexpr auto without_root() const {
        return detail::value_path_build_helper(Path::without_root(), value);
    }

    // template <pathlike P> constexpr auto with_prepend() const {
    //     return value_path<decltype(P{} / Path{}), Value>{{}, value};
    // }

    // constexpr auto untuple() && {
    //     return value_path<Path, stdx::tuple_element_t<0, value_t>>{
    //         {}, get<0>(std::move(value))};
    // }
    // constexpr auto untuple() const & {
    //     return value_path<Path, stdx::tuple_element_t<0, value_t>>{
    //         {}, get<0>(value)};
    // }

  private:
    friend constexpr auto operator==(value_path const &, value_path const &)
        -> bool = default;

    template <pathlike P>
        requires(not valued<P>)
    friend constexpr auto operator/(P p, value_path const &v) {
        return detail::value_path_build_helper(p / Path{}, v.value);
    }
};

template <pathlike P, typename... Vs>
    requires(not valued<P>)
constexpr auto tag_invoke(attach_value_t, P const &p, Vs const &...vs) {
    return value_path<P, stdx::tuple<Vs...>>{p, {vs...}};
}
} // namespace groov
