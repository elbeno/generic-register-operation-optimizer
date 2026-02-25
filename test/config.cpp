#include <groov/config.hpp>
#include <groov/path.hpp>
#include <groov/resolve.hpp>

#include <async/concepts.hpp>

#include <stdx/bit.hpp>
#include <stdx/udls.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <concepts>
#include <cstdint>
#include <type_traits>

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
} // namespace

TEST_CASE("fields inside a register", "[config]") {
    using F = groov::field<"field", std::uint32_t, 0, 0>;
    using R = groov::reg<"reg", std::uint32_t, 0, groov::w::replace, F>;
    using X = groov::get_child<R, "field">;
    STATIC_CHECK(std::is_same_v<F, X>);
}

TEST_CASE("registers in a group", "[config]") {
    using R = groov::reg<"reg", std::uint32_t, 0>;
    using G = groov::group<"group", bus, R>;
    using X = groov::get_child<G, "reg">;
    STATIC_CHECK(std::is_same_v<R, X>);
}

TEST_CASE("subfields inside a field", "[config]") {
    using SubF = groov::field<"subfield", std::uint32_t, 0, 0>;
    using F =
        groov::field<"field", std::uint32_t, 0, 0, groov::w::replace, SubF>;
    using X = groov::get_child<F, "subfield">;
    STATIC_CHECK(std::is_same_v<SubF, X>);
}

TEST_CASE("field can be extracted from register value", "[config]") {
    constexpr std::uint32_t value{0b11};
    using F = groov::field<"field", std::uint32_t, 0, 0>;
    STATIC_CHECK(F::extract(value) == 1);
}

TEST_CASE("field can resolve a path", "[config]") {
    using namespace groov::literals;
    using F = groov::field<"field", std::uint32_t, 0, 0>;
    constexpr auto r = groov::resolve(F{}, "field"_f);
    STATIC_CHECK(std::is_same_v<decltype(r), F const>);
}

TEST_CASE("field containing subfields can resolve a path", "[config]") {
    using namespace groov::literals;
    using SubF = groov::field<"subfield", std::uint32_t, 0, 0>;
    using F =
        groov::field<"field", std::uint32_t, 0, 0, groov::w::replace, SubF>;
    constexpr auto r = groov::resolve(F{}, "field.subfield"_f);
    STATIC_CHECK(std::is_same_v<decltype(r), SubF const>);
}

TEST_CASE("field can resolve an unambiguous subpath", "[config]") {
    using namespace groov::literals;
    using SubF = groov::field<"subfield", std::uint32_t, 0, 0>;
    using F =
        groov::field<"field", std::uint32_t, 0, 0, groov::w::replace, SubF>;
    constexpr auto r = groov::resolve(F{}, "subfield"_f);
    STATIC_CHECK(std::is_same_v<decltype(r), SubF const>);
}

TEST_CASE("register can resolve a path", "[config]") {
    using namespace groov::literals;
    using F = groov::field<"field", std::uint32_t, 0, 0>;
    using R = groov::reg<"reg", std::uint32_t, 0, groov::w::replace, F>;
    constexpr auto r = groov::resolve(R{}, "reg"_r);
    STATIC_CHECK(std::is_same_v<decltype(r), R const>);
}

TEST_CASE("register can resolve an unambiguous subpath", "[config]") {
    using namespace groov::literals;
    using F = groov::field<"field", std::uint32_t, 0, 0>;
    using R = groov::reg<"reg", std::uint32_t, 0, groov::w::replace, F>;
    constexpr auto r = groov::resolve(R{}, "field"_f);
    STATIC_CHECK(std::is_same_v<decltype(r), F const>);
}

TEST_CASE("register can resolve an unambiguous nested subpath", "[config]") {
    using namespace groov::literals;
    using SubF = groov::field<"subfield", std::uint32_t, 0, 0>;
    using F =
        groov::field<"field", std::uint32_t, 0, 0, groov::w::replace, SubF>;
    using R = groov::reg<"reg", std::uint32_t, 0, groov::w::replace, F>;
    constexpr auto r = groov::resolve(R{}, "subfield"_f);
    STATIC_CHECK(std::is_same_v<decltype(r), SubF const>);
}

TEST_CASE("invalid path gives invalid resolution", "[config]") {
    using namespace groov::literals;
    using F = groov::field<"field", std::uint32_t, 0, 0>;
    using R = groov::reg<"reg", std::uint32_t, 0, groov::w::replace, F>;
    STATIC_CHECK(std::is_same_v<groov::invalid_t,
                                decltype(groov::resolve(R{}, "invalid"_f))>);
}

TEST_CASE("ambiguous subpath gives ambiguous resolution", "[config]") {
    using namespace groov::literals;
    using SubF = groov::field<"subfield", std::uint32_t, 0, 0>;
    using F0 =
        groov::field<"field0", std::uint32_t, 0, 0, groov::w::replace, SubF>;
    using F1 =
        groov::field<"field1", std::uint32_t, 1, 1, groov::w::replace, SubF>;
    using R = groov::reg<"reg", std::uint32_t, 0, groov::w::replace, F0, F1>;
    STATIC_CHECK(std::is_same_v<groov::ambiguous_t,
                                decltype(groov::resolve(R{}, "subfield"_f))>);
}

TEST_CASE("group can resolve a path", "[config]") {
    using namespace groov::literals;
    using F = groov::field<"field", std::uint32_t, 0, 0>;
    using R = groov::reg<"reg", std::uint32_t, 0, groov::w::replace, F>;
    using G = groov::group<"group", bus, R>;
    constexpr auto r = groov::resolve(G{}, "reg.field"_f);
    STATIC_CHECK(std::is_same_v<decltype(r), F const>);
}

TEST_CASE("all fields inside a register with no fields", "[config]") {
    using namespace groov::literals;
    using R = groov::reg<"reg", std::uint32_t, 0>;
    STATIC_CHECK(
        std::is_same_v<groov::detail::all_fields_t<boost::mp11::mp_list<R>>,
                       boost::mp11::mp_list<R>>);
}

TEST_CASE("all fields inside a register with fields", "[config]") {
    using namespace groov::literals;
    using F0 = groov::field<"field0", std::uint32_t, 0, 0>;
    using F1 = groov::field<"field1", std::uint32_t, 1, 1>;
    using R = groov::reg<"reg", std::uint32_t, 0, groov::w::replace, F0, F1>;
    STATIC_CHECK(
        std::is_same_v<groov::detail::all_fields_t<boost::mp11::mp_list<R>>,
                       boost::mp11::mp_list<F0, F1>>);
}

TEST_CASE("all fields inside a register with fields and subfields",
          "[config]") {
    using namespace groov::literals;
    using SubF00 = groov::field<"subfield", std::uint32_t, 0, 0>;
    using SubF01 = groov::field<"subfield", std::uint32_t, 1, 1>;
    using F0 = groov::field<"field0", std::uint32_t, 1, 0, groov::w::replace,
                            SubF00, SubF01>;
    using F1 = groov::field<"field1", std::uint32_t, 2, 2>;
    using R = groov::reg<"reg", std::uint32_t, 0, groov::w::replace, F0, F1>;
    STATIC_CHECK(
        std::is_same_v<groov::detail::all_fields_t<boost::mp11::mp_list<R>>,
                       boost::mp11::mp_list<SubF00, SubF01, F1>>);
}

namespace {
struct be_bus {
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

    template <typename RegType>
    CONSTEVAL static auto transform_mask(RegType mask) -> RegType {
        using A = std::array<std::uint8_t, sizeof(RegType)>;
        auto arr = stdx::bit_cast<A>(mask);
        for (auto &i : arr) {
            i = i == 0 ? 0u : 0xffu;
        }
        return stdx::bit_cast<RegType>(arr);
    }
};
} // namespace

TEST_CASE("bus may support byte enables through transform_mask", "[config]") {
    STATIC_CHECK(groov::transform_mask<be_bus>(std::uint32_t{0b1u}) == 0xffu);
    STATIC_CHECK(std::is_same_v<decltype(groov::transform_mask<be_bus>(
                                    std::uint32_t{0b1u})),
                                std::uint32_t>);
}

TEST_CASE("bus without transform_mask returns all bits set", "[config]") {
    STATIC_CHECK(groov::transform_mask<bus>(std::uint32_t{0b1u}) ==
                 0xffff'ffffu);
    STATIC_CHECK(std::is_same_v<decltype(groov::transform_mask<bus>(
                                    std::uint32_t{0b1u})),
                                std::uint32_t>);
}

TEST_CASE(
    "indexed register resolves a non-indexed single-element path to itself",
    "[config]") {
    using namespace groov::literals;
    using F = groov::field<"field", std::uint32_t, 0, 0>;
    using R = groov::reg<"reg", std::uint32_t, 0, groov::w::replace, F>;
    using I = groov::indexed_reg<R, 1>;
    STATIC_CHECK(std::is_same_v<I, decltype(groov::resolve(I{}, "reg"_r))>);
}

TEST_CASE(
    "indexed register gives an invalid resolution for an invalid path (1)",
    "[config]") {
    using namespace groov::literals;
    using F = groov::field<"field", std::uint32_t, 0, 0>;
    using R = groov::reg<"reg", std::uint32_t, 0, groov::w::replace, F>;
    using I = groov::indexed_reg<R, 1>;
    STATIC_CHECK(std::is_same_v<groov::invalid_t,
                                decltype(groov::resolve(I{}, "invalid"_f))>);
}

TEST_CASE(
    "indexed register gives an invalid resolution for an invalid path (2)",
    "[config]") {
    using namespace groov::literals;
    using F = groov::field<"field", std::uint32_t, 0, 0>;
    using R = groov::reg<"reg", std::uint32_t, 0, groov::w::replace, F>;
    using I = groov::indexed_reg<R, 1>;
    STATIC_CHECK(std::is_same_v<groov::invalid_t,
                                decltype(groov::resolve(I{}, "reg.field"_f))>);
}

TEST_CASE(
    "indexed register gives an invalid resolution for an invalid path (3)",
    "[config]") {
    using namespace groov::literals;
    using F = groov::field<"field", std::uint32_t, 0, 0>;
    using R = groov::reg<"reg", std::uint32_t, 0, groov::w::replace, F>;
    using I = groov::indexed_reg<R, 1>;
    STATIC_CHECK(std::is_same_v<groov::invalid_t,
                                decltype(groov::resolve(I{}, "reg[1]"_f))>);
}

TEST_CASE("indexed register resolves an indexed path (no offset)", "[config]") {
    using namespace groov::literals;
    using F = groov::field<"field", std::uint32_t, 0, 0>;
    using R = groov::reg<"reg", std::uint32_t, 0, groov::w::replace, F>;
    using I = groov::indexed_reg<R, 1>;
    STATIC_CHECK(std::is_same_v<R, decltype(groov::resolve(I{}, "reg[0]"_f))>);
}

TEST_CASE("indexed register resolves an indexed path (with offset)",
          "[config]") {
    using namespace groov::literals;
    using F = groov::field<"field", std::uint32_t, 0, 0>;
    using R = groov::reg<"reg", std::uint32_t, 0, groov::w::replace, F>;
    using I = groov::indexed_reg<R, 2>;
    STATIC_CHECK(std::is_same_v<R::with_offset<1>,
                                decltype(groov::resolve(I{}, "reg[1]"_f))>);
}

TEST_CASE("indexed register resolves an indexed path (with offset) with field",
          "[config]") {
    using namespace groov::literals;
    using F = groov::field<"field", std::uint32_t, 0, 0>;
    using R = groov::reg<"reg", std::uint32_t, 0, groov::w::replace, F>;
    using I = groov::indexed_reg<R, 2>;
    STATIC_CHECK(
        std::is_same_v<F, decltype(groov::resolve(I{}, "reg[1].field"_f))>);
}

TEST_CASE("indexed register is runtime-indexable", "[config]") {
    using namespace groov::literals;
    using F = groov::field<"field", std::uint32_t, 0, 0>;
    using R = groov::reg<"reg", std::uint32_t, 0, groov::w::replace, F>;
    using I = groov::indexed_reg<R, 2>;
    constexpr auto r = I{}[1];
    STATIC_CHECK(
        std::is_same_v<groov::detail::rt_offset_reg<R> const, decltype(r)>);
    STATIC_CHECK(r.offset == 1);
}

TEST_CASE("indexed register is compile-time indexable", "[config]") {
    using namespace groov::literals;
    using namespace stdx::literals;
    using F = groov::field<"field", std::uint32_t, 0, 0>;
    using R = groov::reg<"reg", std::uint32_t, 0, groov::w::replace, F>;
    using I = groov::indexed_reg<R, 2>;
    constexpr auto r = I{}[1_c];
    STATIC_CHECK(std::is_same_v<R::with_offset<1> const, decltype(r)>);
    STATIC_CHECK(r.offset == 1);
}

TEST_CASE("indexed register in a group", "[config]") {
    using R = groov::reg<"reg", std::uint32_t, 0>;
    using I = groov::indexed_reg<R, 2>;
    using G = groov::group<"group", bus, I>;
    using X = groov::get_child<G, "reg[1]">;
    STATIC_CHECK(std::is_same_v<X, R::with_offset<1>>);
}

TEST_CASE("compile-time indexed register resolves a path", "[config]") {
    using namespace groov::literals;
    using namespace stdx::literals;
    using F = groov::field<"field", std::uint32_t, 0, 0>;
    using R = groov::reg<"reg", std::uint32_t, 0, groov::w::replace, F>;
    using I = groov::indexed_reg<R, 2>;
    constexpr auto r = I{}[1_c];
    STATIC_CHECK(std::is_same_v<F, decltype(groov::resolve(r, "reg.field"_f))>);
}

TEST_CASE("runtime-indexed register resolves a path", "[config]") {
    using namespace groov::literals;
    using F = groov::field<"field", std::uint32_t, 0, 0>;
    using R = groov::reg<"reg", std::uint32_t, 0, groov::w::replace, F>;
    using I = groov::indexed_reg<R, 2>;
    constexpr auto r = I{}[1];
    STATIC_CHECK(std::is_same_v<F, decltype(groov::resolve(r, "reg.field"_f))>);
}

TEST_CASE("indexed register resolved from group resolves a path with the "
          "correct index",
          "[config]") {
    using namespace groov::literals;
    using R = groov::reg<"reg", std::uint32_t, 0, groov::w::replace>;
    using I = groov::indexed_reg<R, 2>;
    using G = groov::group<"group", bus, I>;
    using X = groov::get_child<G, "reg[1]">;
    STATIC_CHECK(std::is_same_v<X, decltype(groov::resolve(X{}, "reg"_f))>);
    STATIC_CHECK(std::is_same_v<groov::invalid_t,
                                decltype(groov::resolve(X{}, "reg[0]"_f))>);
    STATIC_CHECK(std::is_same_v<X, decltype(groov::resolve(X{}, "reg[1]"_f))>);
}
