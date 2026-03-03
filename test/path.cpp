#include <groov/path.hpp>

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

TEST_CASE("register literal", "[path]") {
    using namespace groov::literals;
    constexpr auto r = "reg"_r;
    STATIC_CHECK(r == groov::path{"reg"_elem});
    STATIC_CHECK(sizeof(r) == 1);
}

TEST_CASE("field literal", "[path]") {
    using namespace groov::literals;
    constexpr auto f = "field"_f;
    STATIC_CHECK(f == groov::path{"field"_elem});
    STATIC_CHECK(sizeof(f) == 1);
}

TEST_CASE("dot-separated literal", "[path]") {
    using namespace groov::literals;
    constexpr auto f = "a.b.c.d"_f;
    STATIC_CHECK(f == groov::path{"a"_elem, "b"_elem, "c"_elem, "d"_elem});
    STATIC_CHECK(sizeof(f) == 1);
}

TEST_CASE("empty literal", "[path]") {
    using namespace groov::literals;
    constexpr auto r = ""_r;
    STATIC_CHECK(r == groov::path{});
    STATIC_CHECK(sizeof(r) == 1);
}

TEST_CASE("make_path", "[path]") {
    using namespace groov::literals;
    constexpr auto f = groov::make_path<"a.b.c.d">();
    STATIC_CHECK(f == groov::path{"a"_elem, "b"_elem, "c"_elem, "d"_elem});
}

TEST_CASE("make_path (indexed)", "[path]") {
    using namespace groov::literals;
    constexpr auto f = groov::make_path<"a.b.c[1].d">();
    STATIC_CHECK(f ==
                 groov::path{"a"_elem, "b"_elem, "c"_elem, 1_elem, "d"_elem});
}

TEST_CASE("CT elem append with /", "[path]") {
    using namespace groov::literals;
    constexpr auto p = "reg"_r / "field"_elem;
    STATIC_CHECK(p == "reg.field"_f);
}

TEST_CASE("CT elem prepend with /", "[path]") {
    using namespace groov::literals;
    constexpr auto p = "reg"_elem / "field"_f;
    STATIC_CHECK(p == "reg.field"_f);
}

TEST_CASE("CT path made from elems with /", "[path]") {
    using namespace groov::literals;
    constexpr auto p = "reg"_elem / "field"_elem;
    STATIC_CHECK(p == "reg.field"_f);
}

TEST_CASE("CT path concatenation with /", "[path]") {
    using namespace groov::literals;
    constexpr auto p = "reg"_r / "field"_f;
    STATIC_CHECK(p == "reg.field"_f);
}

TEST_CASE("RT elem append with /", "[path]") {
    using namespace groov::literals;
    constexpr auto p = "reg"_r / groov::make_path_element("field");
    STATIC_CHECK(equivalent(p, "reg.field"_f));
}

TEST_CASE("RT elem prepend with /", "[path]") {
    using namespace groov::literals;
    constexpr auto p = groov::make_path_element("reg") / "field"_f;
    STATIC_CHECK(equivalent(p, "reg.field"_f));
}

TEST_CASE("RT path made from elems with /", "[path]") {
    using namespace groov::literals;
    constexpr auto p =
        groov::make_path_element("reg") / groov::make_path_element("field");
    STATIC_CHECK(equivalent(p, "reg.field"_f));
}

TEST_CASE("RT path concatenation with /", "[path]") {
    using namespace groov::literals;
    constexpr auto p = groov::path{groov::make_path_element("reg")} / "field"_f;
    STATIC_CHECK(equivalent(p, "reg.field"_f));
}

TEST_CASE("CT path indexing", "[path]") {
    using namespace groov::literals;
    constexpr auto p = "reg"_r[1_idx];
    STATIC_CHECK(p == "reg[1]"_f);
}

TEST_CASE("RT path indexing", "[path]") {
    using namespace groov::literals;
    constexpr auto n = 1;
    constexpr auto p = "reg"_r[n];
    STATIC_CHECK(equivalent(p, "reg[1]"_f));
}

// TEST_CASE("path can resolve itself", "[path]") {
//     using namespace groov::literals;
//     constexpr auto p = "a"_r;
//     STATIC_CHECK(groov::is_resolvable_v<decltype(p), decltype(p)>);
// }

// TEST_CASE("path resolves itself to empty path", "[path]") {
//     using namespace groov::literals;
//     constexpr auto p = "a"_r;
//     constexpr auto r = groov::resolve(p, p);
//     STATIC_CHECK(std::is_same_v<decltype(r), groov::path<> const>);
// }

// TEST_CASE("path resolves a shorter path", "[path]") {
//     using namespace groov::literals;
//     constexpr auto p = "a.b.c"_r;
//     constexpr auto r1 = groov::resolve(p, "a"_r);
//     STATIC_CHECK(r1 == "b.c"_r);
//     constexpr auto r2 = groov::resolve(p, "a.b"_r);
//     STATIC_CHECK(r2 == "c"_r);
// }

// TEST_CASE("path doesn't resolve a non-path", "[path]") {
//     using namespace groov::literals;
//     constexpr auto p = "a.b.c"_r;
//     STATIC_CHECK(not groov::can_resolve<decltype(p), int>);
// }

// TEST_CASE("mismatched path gives invalid resolution", "[path]") {
//     using namespace groov::literals;
//     constexpr auto p = "a.b.c"_r;
//     STATIC_CHECK(std::is_same_v<groov::mismatch_t,
//                                 decltype(groov::resolve(p, "invalid"_r))>);
// }

// TEST_CASE("too-long path gives invalid resolution", "[path]") {
//     using namespace groov::literals;
//     constexpr auto p = "a.b"_r;
//     STATIC_CHECK(std::is_same_v<groov::too_long_t,
//                                 decltype(groov::resolve(p, "a.b.c"_r))>);
// }

TEST_CASE("root of a path", "[path]") {
    using namespace groov::literals;
    constexpr auto p = "a.b.c"_r;
    STATIC_CHECK(p.root() == "a"_elem);
}

TEST_CASE("root of a one-layer path", "[path]") {
    using namespace groov::literals;
    constexpr auto p = "a"_r;
    STATIC_CHECK(p.root() == "a"_elem);
}

TEST_CASE("path without its root", "[path]") {
    using namespace groov::literals;
    constexpr auto p = "a.b.c"_r;
    STATIC_CHECK(p.without_root() == "b.c"_r);
}

TEST_CASE("one-layer path without its root", "[path]") {
    using namespace groov::literals;
    constexpr auto p = "a"_r;
    STATIC_CHECK(p.without_root() == ""_r);
}

TEST_CASE("empty path without its root", "[path]") {
    using namespace groov::literals;
    constexpr auto p = ""_r;
    STATIC_CHECK(p.without_root() == ""_r);
}

TEST_CASE("parent of a path", "[path]") {
    using namespace groov::literals;
    constexpr auto p = "a.b.c"_r;
    STATIC_CHECK(p.parent() == "a.b"_r);
}

TEST_CASE("parent of a one-layer path", "[path]") {
    using namespace groov::literals;
    constexpr auto p = "a"_r;
    STATIC_CHECK(p.parent() == ""_r);
}

TEST_CASE("parent of an empty path", "[path]") {
    using namespace groov::literals;
    constexpr auto p = ""_r;
    STATIC_CHECK(p.parent() == ""_r);
}

TEST_CASE("empty path predicate", "[path]") {
    using namespace groov::literals;
    using P = decltype(""_r);
    STATIC_CHECK(std::empty(P{}));
    using Q = decltype("hello"_r);
    STATIC_CHECK(not std::empty(Q{}));
}

TEST_CASE("path is pathlike", "[path]") {
    using namespace groov::literals;
    STATIC_CHECK(groov::pathlike<decltype("reg"_r / "field"_f)>);
    STATIC_CHECK(not groov::valued_pathlike<decltype("reg"_r / "field"_f)>);
}
