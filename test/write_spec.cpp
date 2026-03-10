#include <groov/config.hpp>
#include <groov/path.hpp>
#include <groov/resolve.hpp>
#include <groov/value_path.hpp>
#include <groov/write_spec.hpp>

#include <async/concepts.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
struct bus {
    struct sender {
        using is_sender = void;
    };

    template <stdx::ct_string, auto>
    static auto read(auto...) -> async::sender auto {
        return sender{};
    }
    template <stdx::ct_string, auto...>
    static auto write(auto...) -> async::sender auto {
        return sender{};
    }
};

using F0 = groov::field<"field0", std::uint8_t, 0, 0>;
using F1 = groov::field<"field1", std::uint8_t, 4, 1>;
using F2 = groov::field<"field2", std::uint8_t, 7, 5>;

std::uint32_t data0{};
using R0 =
    groov::reg<"reg0", std::uint32_t, &data0, groov::w::replace, F0, F1, F2>;
std::uint32_t data1{};
using R1 =
    groov::reg<"reg1", std::uint32_t, &data1, groov::w::replace, F0, F1, F2>;

using G = groov::group<"group", bus, R0, R1>;
constexpr auto grp = G{};
} // namespace

TEST_CASE("flatten_paths (nothing to flatten)", "[write_spec]") {
    using namespace groov::literals;
    using namespace stdx::literals;

    auto p = "f"_f = 5;
    auto pp = groov::detail::flatten_paths(p);

    using R = decltype(pp);
    STATIC_CHECK(stdx::is_specialization_of_v<R, stdx::tuple>);
    STATIC_CHECK(stdx::tuple_size_v<R> == 1);

    using P = stdx::tuple_element_t<0, R>;
    STATIC_CHECK(std::is_same_v<typename P::value_t, int>);
    STATIC_CHECK(equivalent(pp[0_idx], "f"_f));
    CHECK(pp[0_idx].value == 5);
}

TEST_CASE("flatten_paths (single depth)", "[write_spec]") {
    using namespace groov::literals;
    using namespace stdx::literals;

    auto p = "r"_r("f"_f = 5);
    auto pp = groov::detail::flatten_paths(p);

    using R = decltype(pp);
    STATIC_CHECK(stdx::is_specialization_of_v<R, stdx::tuple>);
    STATIC_CHECK(stdx::tuple_size_v<R> == 1);

    using P = stdx::tuple_element_t<0, R>;
    STATIC_CHECK(std::is_same_v<typename P::value_t, int>);
    STATIC_CHECK(equivalent(pp[0_idx], "r.f"_f));
    CHECK(pp[0_idx].value == 5);
}

TEST_CASE("flatten_paths (multi depth)", "[write_spec]") {
    using namespace groov::literals;
    using namespace stdx::literals;

    auto p = "r"_r("f"_f("subf"_f = 5));
    auto pp = groov::detail::flatten_paths(p);

    using R = decltype(pp);
    STATIC_CHECK(stdx::is_specialization_of_v<R, stdx::tuple>);
    STATIC_CHECK(stdx::tuple_size_v<R> == 1);

    using P = stdx::tuple_element_t<0, R>;
    STATIC_CHECK(std::is_same_v<typename P::value_t, int>);
    STATIC_CHECK(equivalent(pp[0_idx], "r.f.subf"_f));
    CHECK(pp[0_idx].value == 5);
}

TEST_CASE("flatten_paths (multi value)", "[write_spec]") {
    using namespace groov::literals;
    using namespace stdx::literals;

    auto p = "r"_r("f0"_f = 5, "f1"_f = 6);
    auto pp = groov::detail::flatten_paths(p);

    using R = decltype(pp);
    STATIC_CHECK(stdx::is_specialization_of_v<R, stdx::tuple>);
    STATIC_CHECK(stdx::tuple_size_v<R> == 2);

    using P1 = stdx::tuple_element_t<0, R>;
    STATIC_CHECK(std::is_same_v<typename P1::value_t, int>);
    STATIC_CHECK(equivalent(pp[0_idx], "r.f0"_f));
    CHECK(pp[0_idx].value == 5);

    using P2 = stdx::tuple_element_t<1, R>;
    STATIC_CHECK(std::is_same_v<typename P2::value_t, int>);
    STATIC_CHECK(equivalent(pp[1_idx], "r.f1"_f));
    CHECK(pp[1_idx].value == 6);
}

TEST_CASE("flatten_paths (arbitrary)", "[write_spec]") {
    using namespace groov::literals;
    using namespace stdx::literals;

    auto p = "r"_r("f0"_f = 5, "f1"_f("subf0"_f = 6, "subf1"_f = 7));
    auto pp = groov::detail::flatten_paths(p);

    using R = decltype(pp);
    STATIC_CHECK(stdx::is_specialization_of_v<R, stdx::tuple>);
    STATIC_CHECK(stdx::tuple_size_v<R> == 3);

    using P1 = stdx::tuple_element_t<0, R>;
    STATIC_CHECK(std::is_same_v<typename P1::value_t, int>);
    STATIC_CHECK(equivalent(pp[0_idx], "r.f0"_f));
    CHECK(pp[0_idx].value == 5);

    using P2 = stdx::tuple_element_t<1, R>;
    STATIC_CHECK(std::is_same_v<typename P2::value_t, int>);
    STATIC_CHECK(equivalent(pp[1_idx], "r.f1.subf0"_f));
    CHECK(pp[1_idx].value == 6);

    using P3 = stdx::tuple_element_t<2, R>;
    STATIC_CHECK(std::is_same_v<typename P3::value_t, int>);
    STATIC_CHECK(equivalent(pp[2_idx], "r.f1.subf1"_f));
    CHECK(pp[2_idx].value == 7);
}

TEST_CASE("write_spec is made by combining group and paths", "[write_spec]") {
    using namespace groov::literals;
    auto spec = grp("reg0"_r = 5);
    STATIC_CHECK(
        stdx::is_specialization_of_v<decltype(spec), groov::write_spec>);
}

TEST_CASE("valid paths are captured", "[write_spec]") {
    using namespace groov::literals;
    auto p = "reg0"_r = 5;
    auto spec = grp(p);

    using P = typename decltype(spec)::paths_t;
    STATIC_CHECK(stdx::is_specialization_of_v<P, stdx::tuple>);
    STATIC_CHECK(stdx::tuple_size_v<P> == 1);
    STATIC_CHECK(equivalent(spec.paths[0_idx], "reg0"_f));
}

TEST_CASE("values are captured", "[write_spec]") {
    using namespace groov::literals;
    auto p = "reg0"_r = 5;
    auto spec = grp(p);

    using V = typename decltype(spec)::values_t;
    STATIC_CHECK(stdx::is_specialization_of_v<V, stdx::tuple>);
    STATIC_CHECK(stdx::tuple_size_v<V> == 1);
    CHECK(spec.values[0_idx] == 5);
}

TEST_CASE("multiple paths can be passed (variadic)", "[write_spec]") {
    using namespace groov::literals;
    auto p = "reg0"_r = 5;
    auto q = "reg1"_r = 6;
    auto spec = grp(p, q);

    using P = typename decltype(spec)::paths_t;
    STATIC_CHECK(stdx::is_specialization_of_v<P, stdx::tuple>);
    STATIC_CHECK(stdx::tuple_size_v<P> == 2);
    STATIC_CHECK(equivalent(spec.paths[0_idx], "reg0"_f));
    STATIC_CHECK(equivalent(spec.paths[1_idx], "reg1"_f));
}

TEST_CASE("multiple paths can be passed (tree)", "[write_spec]") {
    using namespace groov::literals;
    auto p = "field0"_f = 5;
    auto q = "field1"_f = 6;
    auto spec = grp("reg0"_r(p, q));

    using P = typename decltype(spec)::paths_t;
    STATIC_CHECK(stdx::is_specialization_of_v<P, stdx::tuple>);
    STATIC_CHECK(stdx::tuple_size_v<P> == 2);
    STATIC_CHECK(equivalent(spec.paths[0_idx], "reg0.field0"_f));
    STATIC_CHECK(equivalent(spec.paths[1_idx], "reg0.field1"_f));
}

TEST_CASE("multiple paths can be passed (multi-tree)", "[write_spec]") {
    using namespace groov::literals;
    auto p = "field0"_f = 5;
    auto q = "field1"_f = 6;
    auto spec = grp("reg0"_r(p, q), "reg1"_r(p, q));

    using P = typename decltype(spec)::paths_t;
    STATIC_CHECK(stdx::is_specialization_of_v<P, stdx::tuple>);
    STATIC_CHECK(stdx::tuple_size_v<P> == 4);
    STATIC_CHECK(equivalent(spec.paths[0_idx], "reg0.field0"_f));
    STATIC_CHECK(equivalent(spec.paths[1_idx], "reg0.field1"_f));
    STATIC_CHECK(equivalent(spec.paths[2_idx], "reg1.field0"_f));
    STATIC_CHECK(equivalent(spec.paths[3_idx], "reg1.field1"_f));
}

TEST_CASE("operator/ is overloaded to make write_spec", "[write_spec]") {
    using namespace groov::literals;
    auto p = "reg0"_r = 5;
    auto spec = grp / p;

    using P = typename decltype(spec)::paths_t;
    STATIC_CHECK(stdx::is_specialization_of_v<P, stdx::tuple>);
    STATIC_CHECK(stdx::tuple_size_v<P> == 1);
    STATIC_CHECK(equivalent(spec.paths[0_idx], "reg0"_f));
}

TEST_CASE("write spec can be indexed by path", "[write_spec]") {
    using namespace groov::literals;
    auto const spec = grp("reg0"_r = 5);
    CHECK(spec["reg0"_r] == 5);
}

TEST_CASE("write spec specified with whole reg can be indexed by field",
          "[write_spec]") {
    using namespace groov::literals;
    auto spec = grp("reg0"_r = 5);
    CHECK(spec["reg0.field0"_f] == 1);
}

TEST_CASE("write spec allows lookup by unambiguous path", "[write_spec]") {
    using namespace groov::literals;
    auto spec = (grp("reg0.field1"_r = 0b1010u));
    CHECK(spec["field1"_f] == 0b1010u);
}

TEST_CASE(
    "write spec specified with whole reg can be indexed by unambiguous field",
    "[write_spec]") {
    using namespace groov::literals;
    auto spec = grp("reg0"_r = 5);
    CHECK(spec["field0"_f] == 1);
}

// TEST_CASE("write spec with only one value implicitly converts",
//           "[write_spec]") {
//     using namespace groov::literals;
//     auto spec = grp("reg0"_r = 5);
//     CHECK(spec == 5);
// }

// TEST_CASE("write spec indexing supports assignment", "[write_spec]") {
//     using namespace groov::literals;
//     auto spec = grp("reg0"_r = 5);
//     spec["reg0"_r] = 4;
//     CHECK(spec["reg0"_r] == 4);
// }

// TEST_CASE("write spec indexing supports op-assignment", "[write_spec]") {
//     using namespace groov::literals;
//     auto spec = grp("reg0"_r = 5);
//     spec["reg0"_r] += 1;
//     spec["reg0"_r] -= 1;
//     spec["reg0"_r] *= 1;
//     spec["reg0"_r] /= 1;
//     spec["reg0"_r] %= 6;
//     CHECK(spec["reg0"_r] == 5);
//     spec["reg0"_r] &= 3;
//     CHECK(spec["reg0"_r] == 1);
//     spec["reg0"_r] |= 4;
//     CHECK(spec["reg0"_r] == 5);
//     spec["reg0"_r] ^= 4;
//     CHECK(spec["reg0"_r] == 1);
// }

// TEST_CASE("write spec indexing supports increment/decrement", "[write_spec]")
// {
//     using namespace groov::literals;
//     auto spec = grp("reg0"_r = 5);
//     ++spec["reg0"_r];
//     --spec["reg0"_r];
//     CHECK(spec["reg0"_r] == 5);

//     CHECK(spec["reg0"_r]++ == 5);
//     CHECK(spec["reg0"_r]-- == 6);
//     CHECK(spec["reg0"_r] == 5);
// }
