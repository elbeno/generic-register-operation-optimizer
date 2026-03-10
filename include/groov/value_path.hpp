#pragma once

#include <groov/attach_value.hpp>
#include <groov/path_fwd.hpp>
#include <groov/resolve.hpp>

#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>
#include <stdx/type_traits.hpp>

namespace groov {
namespace detail {
template <typename... Ts> constexpr auto value_path_build_helper(Ts &&...ts) {
    return value_path{std::forward<Ts>(ts)...};
}
} // namespace detail

template <pathlike Path, typename Value> struct value_path : Path {
    using path_t = Path;
    using value_t = Value;
    [[no_unique_address]] value_t value;

    template <pathlike P>
        requires(P::ct_usable and path_t::ct_usable)
    constexpr auto resolve(P const &p) const {
        if constexpr (P::size() > path_t::size()) {
            using leftover_t = resolution_t<P, path_t>;
            if constexpr (pathlike<leftover_t>) {
                auto const valid_children =
                    stdx::filter<detail::resolves_q<leftover_t>::template fn>(
                        value);
                if constexpr (valid_children.size() == 0) {
                    return mismatch_t{};
                } else if constexpr (valid_children.size() > 1) {
                    return ambiguous_t{};
                } else {
                    auto const child = get<0>(valid_children);
                    if constexpr (pathlike<decltype(child)>) {
                        return groov::resolve(child, groov::resolve(p, *this));
                    } else {
                        return too_long_t{};
                    }
                }
            } else {
                return groov::resolve(p, *this);
            }
        } else {
            using leftover_t = resolution_t<path_t, P>;
            if constexpr (pathlike<leftover_t>) {
                if constexpr (leftover_t::empty() and value_t::size() == 1) {
                    return get<0>(value);
                } else {
                    return detail::value_path_build_helper(
                        groov::resolve(static_cast<path_t const &>(*this), p),
                        value);
                }
            } else {
                return groov::resolve(static_cast<path_t const &>(*this), p);
            }
        }
    }

    template <pathlike P> constexpr auto operator[](P p) const {
        return checked_resolve(*this, p);
    }

    constexpr auto without_root() const {
        return detail::value_path_build_helper(Path::without_root(), value);
    }

    template <pathlike P> constexpr auto with_prepend(P &&p) const {
        return std::forward<P>(p) / *this;
    }

    constexpr auto untuple() && {
        return detail::value_path_build_helper(static_cast<Path &&>(*this),
                                               get<0>(std::move(value)));
    }
    constexpr auto untuple() const & {
        return detail::value_path_build_helper(static_cast<Path const &>(*this),
                                               get<0>(value));
    }

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
constexpr auto tag_invoke(attach_value_t, P &&p, Vs &&...vs) {
    return value_path<std::remove_cvref_t<P>,
                      stdx::tuple<std::remove_cvref_t<Vs>...>>{
        std::forward<P>(p), std::forward<Vs>(vs)...};
}
} // namespace groov
