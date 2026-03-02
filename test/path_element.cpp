#include <groov/path_element.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("ct_path_element models concept", "[path_element]") {
    auto x = groov::ct_path_element<0>{};
    STATIC_CHECK(groov::path_elemental<decltype(x)>);
}

TEST_CASE("rt_path_element models concept", "[path_element]") {
    auto x = groov::rt_path_element{0};
    STATIC_CHECK(groov::path_elemental<decltype(x)>);
}

TEST_CASE("ct_path_element equality", "[path_element]") {
    auto x = groov::ct_path_element<0>{};
    auto y = groov::ct_path_element<0>{};
    STATIC_CHECK(x == y);
}

TEST_CASE("ct_path_element inequality (same type)", "[path_element]") {
    auto x = groov::ct_path_element<0>{};
    auto y = groov::ct_path_element<1>{};
    STATIC_CHECK(x != y);
}

TEST_CASE("ct_path_element inequality (different types)", "[path_element]") {
    auto x = groov::ct_path_element<0>{};
    auto y = groov::ct_path_element<"foo">{};
    STATIC_CHECK(x != y);
}

TEST_CASE("rt_path_element equality", "[path_element]") {
    auto x = groov::rt_path_element{0};
    auto y = groov::rt_path_element{0};
    CHECK(x == y);
}

TEST_CASE("rt_path_element inequality (same type)", "[path_element]") {
    auto x = groov::rt_path_element{0};
    auto y = groov::rt_path_element{1};
    CHECK(x != y);
}

TEST_CASE("rt_path_element inequality (different types)", "[path_element]") {
    auto x = groov::rt_path_element{0};
    auto y = groov::rt_path_element{"foo"};
    CHECK(x != y);
}

TEST_CASE("ct-rt path_element equality (same type)", "[path_element]") {
    auto x = groov::ct_path_element<"foo">{};
    auto y = groov::rt_path_element{"foo"};
    CHECK(x == y);
    CHECK(y == x);
}

TEST_CASE("ct-rt path_element inequality (same type)", "[path_element]") {
    auto x = groov::ct_path_element<"foo">{};
    auto y = groov::rt_path_element{"bar"};
    CHECK(x != y);
    CHECK(y != x);
}

TEST_CASE("ct-rt path_element inequality (different types)", "[path_element]") {
    auto x = groov::ct_path_element<"foo">{};
    auto y = groov::rt_path_element{0};
    STATIC_CHECK(x != y);
    STATIC_CHECK(y != x);
}
