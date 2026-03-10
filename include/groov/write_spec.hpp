#pragma once

#include <groov/boost_extra.hpp>
#include <groov/config.hpp>
#include <groov/make_spec.hpp>
#include <groov/read_spec.hpp>
#include <groov/resolve.hpp>

#include <stdx/compiler.hpp>
#include <stdx/tuple.hpp>
#include <stdx/tuple_algorithms.hpp>
#include <stdx/type_traits.hpp>

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/list.hpp>

#include <regex>

template <typename...> struct undef;

namespace groov {
namespace detail {
// struct no_extract_type {};

template <stdx::has_trait<std::is_enum> E>
constexpr static auto enable_value() {
    static_assert(stdx::always_false_v<E>,
                  "Enum doesn't contain an ENABLE value");
}

template <stdx::has_trait<std::is_enum> E>
    requires requires { E::ENABLE; }
constexpr static auto enable_value() {
    return E::ENABLE;
}

template <stdx::has_trait<std::is_enum> E>
constexpr static auto disable_value() {
    static_assert(stdx::always_false_v<E>,
                  "Enum doesn't contain an DISABLE value");
}

template <stdx::has_trait<std::is_enum> E>
    requires requires { E::DISABLE; }
constexpr static auto disable_value() {
    return E::DISABLE;
}

template <typename F, typename V> constexpr auto convert_value(V const &v) {
    using T = typename F::type_t;
    if constexpr (std::is_same_v<V, enable_t>) {
        static_assert(std::is_enum_v<T>,
                      "enable can only be used with enumeration fields that "
                      "contain an ENABLE value");
        return enable_value<T>();
    } else if constexpr (std::is_same_v<V, disable_t>) {
        static_assert(std::is_enum_v<T>,
                      "disable can only be used with enumeration fields that "
                      "contain a DISABLE value");
        return disable_value<T>();
    } else if constexpr (std::is_same_v<V, set_t>) {
        static_assert(set_write_function<typename F::write_fn_t>,
                      "set can only be used with fields that "
                      "have a set_spec in their write function");
        return F::write_fn_t::set_spec::template mask<F::field_mask>();
    } else if constexpr (std::is_same_v<V, clear_t>) {
        static_assert(clear_write_function<typename F::write_fn_t>,
                      "clear can only be used with fields that "
                      "have a clear_spec in their write function");
        return F::write_fn_t::clear_spec::template mask<F::field_mask>();
    } else {
        return static_cast<T>(v);
    }
}

// template <typename M1, typename M2>
// using mask_overlap =
//     std::integral_constant<std::remove_cvref_t<decltype(M1::value)>,
//                            M1::value & M2::value>;
// template <typename M> using nonzero_mask = std::bool_constant<M::value != 0>;

// // NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
// template <typename R, typename F> struct field_proxy {
//     using type_t = typename F::type_t;

//     constexpr explicit field_proxy(R &reg) : r{reg} {}
//     constexpr field_proxy(field_proxy &&) = delete;

//     // NOLINTNEXTLINE(google-explicit-constructor)
//     constexpr operator type_t() const { return F::extract(r.value); }

//     // NOLINTNEXTLINE(misc-unconventional-assign-operator)
//     constexpr auto operator=(type_t v) const && -> void {
//         F::insert(r.value, v);
//     }

//     // NOLINTNEXTLINE(misc-unconventional-assign-operator)
//     constexpr auto operator=(enable_t) const && -> void
//         requires stdx::has_trait<type_t, std::is_enum>
//     {
//         F::insert(r.value, enable_value<type_t>());
//     }

//     // NOLINTNEXTLINE(misc-unconventional-assign-operator)
//     constexpr auto operator=(disable_t) const && -> void
//         requires stdx::has_trait<type_t, std::is_enum>
//     {
//         F::insert(r.value, disable_value<type_t>());
//     }

//     // NOLINTNEXTLINE(misc-unconventional-assign-operator)
//     constexpr auto operator=(set_t) const && -> void
//         requires set_write_function<typename F::write_fn_t>
//     {
//         F::insert(r.value,
//                   F::write_fn_t::set_spec::template mask<F::field_mask>());
//     }

//     // NOLINTNEXTLINE(misc-unconventional-assign-operator)
//     constexpr auto operator=(clear_t) const && -> void
//         requires clear_write_function<typename F::write_fn_t>
//     {
//         F::insert(r.value,
//                   F::write_fn_t::clear_spec::template mask<F::field_mask>());
//     }

//     constexpr auto operator+=(type_t v) const && -> void {
//         F::insert(r.value, static_cast<type_t>(*this) + v);
//     }
//     constexpr auto operator-=(type_t v) const && -> void {
//         F::insert(r.value, static_cast<type_t>(*this) - v);
//     }
//     constexpr auto operator*=(type_t v) const && -> void {
//         F::insert(r.value, static_cast<type_t>(*this) * v);
//     }
//     constexpr auto operator/=(type_t v) const && -> void {
//         F::insert(r.value, static_cast<type_t>(*this) / v);
//     }
//     constexpr auto operator%=(type_t v) const && -> void {
//         F::insert(r.value, static_cast<type_t>(*this) % v);
//     }
//     constexpr auto operator|=(type_t v) const && -> void {
//         F::insert(r.value, static_cast<type_t>(*this) | v);
//     }
//     constexpr auto operator&=(type_t v) const && -> void {
//         F::insert(r.value, static_cast<type_t>(*this) & v);
//     }
//     constexpr auto operator^=(type_t v) const && -> void {
//         F::insert(r.value, static_cast<type_t>(*this) ^ v);
//     }

//     constexpr auto operator++() const && -> void {
//         auto v = static_cast<type_t>(*this);
//         F::insert(r.value, ++v);
//     }
//     constexpr auto operator--() const && -> void {
//         auto v = static_cast<type_t>(*this);
//         F::insert(r.value, --v);
//     }
//     constexpr auto operator++(int) const && -> type_t {
//         auto v = static_cast<type_t>(*this);
//         auto ret = v;
//         F::insert(r.value, ++v);
//         return ret;
//     }
//     constexpr auto operator--(int) const && -> type_t {
//         auto v = static_cast<type_t>(*this);
//         auto ret = v;
//         F::insert(r.value, --v);
//         return ret;
//     }

//     // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
//     R &r;
// };
} // namespace detail

template <typename Registers, pathlike... Ps> struct write_spec {
    using is_write_spec = void;
    using registers_t = Registers;
    using paths_t = stdx::tuple<Ps...>;
    using values_t = boost::mp11::mp_transform<register_type_t, Registers>;

    [[no_unique_address]] paths_t paths;
    [[no_unique_address]] values_t values{};

    //   private:
    //     using extract_type = stdx::conditional_t<
    //         boost::mp11::mp_size<paths_t>::value == 1,
    //         typename resolve_t<boost::mp11::mp_front<value_t>,
    //                            boost::mp11::mp_front<paths_t>>::type_t,
    //         detail::no_extract_type>;

    //     template <pathlike P> constexpr static auto find_index() ->
    //     std::size_t {
    //         using actual_field_masks_t =
    //             boost::mp11::mp_transform_q<detail::field_mask_for_reg_q<paths_t>,
    //                                         value_t>;
    //         using lookup_field_masks_t = boost::mp11::mp_transform_q<
    //             detail::field_mask_for_reg_q<boost::mp11::mp_list<P>>,
    //             value_t>;
    //         using masks_t = boost::mp11::mp_transform<
    //             detail::mask_overlap, actual_field_masks_t,
    //             lookup_field_masks_t>;
    //         using matches = boost::mp11::mp_copy_if<masks_t,
    //         detail::nonzero_mask>; if constexpr
    //         (boost::mp11::mp_empty<matches>::value) {
    //             static_assert(stdx::always_false_v<P>,
    //                           "Invalid path passed to write_spec[]");
    //         } else if constexpr (boost::mp11::mp_size<matches>::value > 1) {
    //             static_assert(stdx::always_false_v<P>,
    //                           "Ambiguous path passed to write_spec[]");
    //         } else {
    //             using index_t =
    //                 boost::mp11::mp_find_if<masks_t, detail::nonzero_mask>;
    //             return index_t::value;
    //         }
    //         return {};
    //     }

    //   public:
    //     template <pathlike P> constexpr auto operator[](P const &)
    //     LIFETIMEBOUND
    //     {
    //         constexpr auto idx = find_index<P>();
    //         auto &r = stdx::get<idx>(value);
    //         using R = decltype(r);
    //         return detail::field_proxy<R, resolve_t<R, P>>{r};
    //     }

    //     template <pathlike P> constexpr auto operator[](P const &) const {
    //         constexpr auto idx = find_index<P>();
    //         auto &r = stdx::get<idx>(value);
    //         using R = decltype(r);
    //         using F = resolve_t<R, P>;
    //         return F::extract(r.value);
    //     }

    //     template <std::size_t N>
    //     // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    //     constexpr auto operator[](char const (&)[N]) const {
    //         static_assert(stdx::always_false_v<write_spec>,
    //                       "Trying to index into a write_spec with a string "
    //                       "literal: did you forget to use the UDL?");
    //     }

    //     // NOLINTNEXTLINE(google-explicit-constructor)
    //     constexpr operator extract_type() const
    //         requires(not std::is_same_v<extract_type,
    //         detail::no_extract_type>)
    //     {
    //         using P = boost::mp11::mp_first<paths_t>;
    //         using R = boost::mp11::mp_first<value_t>;
    //         auto const &r = stdx::get<R>(value);
    //         using F = resolve_t<R, P>;
    //         return F::extract(r.value);
    //     }
};

namespace detail {
template <typename Group, valued_pathlike... VPs>
constexpr auto unique_registers_for(VPs const &...vps) {
    return stdx::transform(
        []<typename T>(T &&t) { return get<0>(std::forward<T>(t)); },
        stdx::gather(stdx::tuple{register_for_path<Group>(vps)...}));
}

template <typename R, valued_pathlike VP>
constexpr auto try_insert([[maybe_unused]] auto &dest,
                          [[maybe_unused]] VP const &vp) -> bool {
    if constexpr (can_resolve<R, VP>) {
        using F = resolution_t<R, VP>;
        F::insert(dest, convert_value<F>(vp.value));
        return true;
    }
    return false;
}

template <typename WS, valued_pathlike VP>
constexpr auto insert(WS &ws, VP const &vp) {
    auto inserted = false;
    stdx::enumerate(
        [&]<auto I>(auto &dest) {
            using R = stdx::tuple_element_t<I, typename WS::registers_t>;
            inserted = inserted or try_insert<R>(dest, vp);
        },
        ws.values);
}

template <typename Group, valued_pathlike... VPs>
constexpr auto to_write_spec(VPs const &...vps)
    -> write_spec<decltype(unique_registers_for<Group>(vps...)),
                  typename VPs::path_t...> {
    auto registers = stdx::transform(
        []<typename T>(T &&t) { return get<0>(std::forward<T>(t)); },
        stdx::gather(stdx::tuple{register_for_path<Group>(vps)...}));
    using Rs = decltype(registers);

    auto ws = write_spec<Rs, typename VPs::path_t...>{vps.as_path()...};
    (insert(ws, vps), ...);
    return ws;
}

template <valued_pathlike P> constexpr auto flatten_paths(P &&p) {
    using VP = std::remove_cvref_t<P>;
    using contained_value_t = stdx::tuple_element_t<0, typename VP::value_t>;
    using path_t = typename VP::path_t;

    if constexpr (valued_pathlike<contained_value_t>) {
        return stdx::transform(
            [&](auto const &vp) {
                return vp.with_prepend(static_cast<path_t const &>(p));
            },
            p.value.apply([]<typename... Ps>(Ps &&...ps) {
                return stdx::tuple_cat(flatten_paths(std::forward<Ps>(ps))...);
            }));
    } else {
        return stdx::tuple{std::forward<P>(p).untuple()};
    }
}
} // namespace detail

template <typename G, valued_pathlike... Ps>
constexpr auto tag_invoke(make_spec_t, G, Ps &&...ps) {
    detail::check_valid_config<G>(ps...);
    return stdx::tuple_cat(detail::flatten_paths(std::forward<Ps>(ps))...)
        .apply([]<typename... VPs>(VPs &&...vps) {
            return detail::to_write_spec<G>(std::forward<VPs>(vps)...);
        });
}
} // namespace groov
