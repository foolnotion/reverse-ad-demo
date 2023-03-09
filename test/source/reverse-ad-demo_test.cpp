#include <cmath>

#include "ext/boost/ut.hpp"
#include "reverse-ad-demo/expr.hpp"

auto main() -> int
{
    using namespace boost::ut;

    auto eq = [](auto a, auto b)
    {
        constexpr auto eps {1e-6};
        return std::abs(a - b) < eps;
    };

    "x * y + sin(x) | x=0.5, y=4.2"_test = [&]
    {
        reverse::Tape<double> tape;

        auto constexpr a {0.5};
        auto constexpr b {4.2};

        auto x = tape.variable(a);
        auto y = tape.variable(b);

        auto z = x * y + x.sin();

        auto v = x.value * y.value + std::sin(x.value);
        expect(eq(z.value, v));

        auto g = z.gradient();
        expect(eq(g.wrt(x), y.value + std::cos(x.value)));
        expect(eq(g.wrt(y), x.value));
    };

    "sin(x) + cos(y) | x=2, y=3"_test = [&]
    {
        auto constexpr a {2.0};
        auto constexpr b {3.0};

        reverse::Tape<double> tape;
        auto x = tape.variable(a);
        auto y = tape.variable(b);
        auto z = x.sin() + y.cos();
        auto g = z.gradient();

        expect(eq(z.value, std::sin(a) + std::cos(b)));
        expect(eq(g.wrt(x), std::cos(a)));
        expect(eq(g.wrt(y), -std::sin(b)));
    };

    "log(x) + xy - sin(y) | x=2, y=5"_test = [&]
    {
        auto constexpr a {2.0};
        auto constexpr b {5.0};

        reverse::Tape<double> tape;
        auto x = tape.variable(a);
        auto y = tape.variable(b);
        auto z = x.log() + x * y - y.sin();
        auto g = z.gradient();

        expect(eq(z.value, std::log(a) + a * b - std::sin(b)));
        expect(eq(g.wrt(x), b + 1 / a));
        expect(eq(g.wrt(y), a - std::cos(b)));
    };

    "1 + x | x=2"_test = [&]
    {
        auto constexpr a {2.0};
        reverse::Tape<double> tape;

        auto x = tape.variable(a);
        auto z = 1 + x;
        auto g = z.gradient();
        expect(eq(z.value, 1 + a));
        expect(eq(g.wrt(x), 1));
    };

    "sin(x+1) | x=2"_test = [&]
    {
        auto constexpr a {2.0};
        reverse::Tape<double> tape;

        auto x = tape.variable(a);
        auto z = (x + 1).sin();
        auto g = z.gradient();
        expect(eq(z.value, std::sin(a + 1)));
        expect(eq(g.wrt(x), std::cos(a + 1)));
    };

    "1/x + y/4 | x=2, y=2"_test = [&]
    {
        auto constexpr a {2.0};
        auto constexpr b {2.0};

        reverse::Tape<double> tape;
        auto x = tape.variable(a);
        auto y = tape.variable(b);
        auto z = 1 / x + y / 4;
        auto g = z.gradient();

        expect(eq(z.value, 1 / a + b / 4));
        expect(eq(g.wrt(x), -1 / (a * a)));
        expect(eq(g.wrt(y), 1.0 / 4));
    };

    return 0;
}
