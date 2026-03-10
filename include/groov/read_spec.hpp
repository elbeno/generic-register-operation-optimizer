#pragma once

#include <groov/config.hpp>
#include <groov/make_spec.hpp>
#include <groov/path_fwd.hpp>
#include <groov/resolve.hpp>

#include <stdx/tuple.hpp>

namespace groov {
template <typename Group, typename... Ps> struct read_spec : Group {
    using paths_t = stdx::tuple<Ps...>;
    [[no_unique_address]] paths_t paths;
};
template <typename... Ts>
read_spec(Ts...) -> read_spec<std::remove_cvref_t<Ts>...>;

template <typename G, pathlike... Ps>
    requires(... and not valued<Ps>)
constexpr auto tag_invoke(make_spec_t, G g, Ps &&...ps) {
    using T = stdx::tuple<std::remove_cvref_t<Ps>...>;
    detail::check_valid_config<G, T>();
    return read_spec{g, std::forward<Ps>(ps)...};
}
} // namespace groov
