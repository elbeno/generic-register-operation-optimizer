#pragma once

#include <groov/config.hpp>
#include <groov/make_spec.hpp>
#include <groov/path_fwd.hpp>
#include <groov/resolve.hpp>

#include <stdx/tuple.hpp>

namespace groov {
template <typename Group, typename... Ps> struct read_spec {
    using group_t = Group;
    using paths_t = stdx::tuple<Ps...>;
    [[no_unique_address]] paths_t paths;
};

template <typename G, pathlike... Ps>
    requires(... and not valued<Ps>)
constexpr auto tag_invoke(make_spec_t, G, Ps &&...ps) {
    detail::check_valid_config<G>(ps...);
    return read_spec<G, std::remove_cvref_t<Ps>...>{std::forward<Ps>(ps)...};
}
} // namespace groov
