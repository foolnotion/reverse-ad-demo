#ifndef REVERSE_AD_DEMO_TEST_CORRECTNESS_HPP
#define REVERSE_AD_DEMO_TEST_CORRECTNESS_HPP

#include <iostream>

#include "ext/boost/ut.hpp"

template<>
auto boost::ut::cfg<boost::ut::override> = ut::runner<ut::reporter<>> {};

#include "reverse-ad-demo/dual.hpp"
#include "reverse-ad-demo/expr.hpp"

namespace reverse::test
{

boost::ut::suite const correctness_test_suite = []() -> void
{
    using namespace boost::ut;  // NOLINT

    auto eq = [](auto a, auto b) -> auto
    {
        constexpr auto eps {1e-6};
        return std::abs(a - b) < eps;
    };

    using Tape = reverse::Tape<double>;
    using Dual = forward::dual;

    "x * y + sin(x) | x=0.5, y=4.2"_test = [&]() -> void
    {
        Tape tape;

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

        std::array duals {Dual {0.5}, Dual {4.2}};
        std::array gradient {0.0, 0.0};
        for (auto i = 0; i < duals.size(); ++i) {
            duals[i].b = 1;
            auto c = duals[0] * duals[1] + duals[0].sin();
            gradient[i] = c.b;
            duals[i].b = 0;
        }
        expect(eq(gradient[0], y.value + std::cos(x.value)));
        expect(eq(gradient[1], x.value));
    };

    "sin(x) + cos(y) | x=2, y=3"_test = [&]
    {
        auto constexpr a {2.0};
        auto constexpr b {3.0};

        Tape tape;
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

        Tape tape;
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
        Tape tape;

        auto x = tape.variable(a);
        auto z = 1 + x;
        auto g = z.gradient();
        expect(eq(z.value, 1 + a));
        expect(eq(g.wrt(x), 1));
    };

    "sin(x+1) | x=2"_test = [&]
    {
        auto constexpr a {2.0};
        Tape tape;

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

        Tape tape;
        auto x = tape.variable(a);
        auto y = tape.variable(b);
        auto z = 1 / x + y / 4;
        auto g = z.gradient();

        expect(eq(z.value, 1 / a + b / 4));
        expect(eq(g.wrt(x), -1 / (a * a)));
        expect(eq(g.wrt(y), 1.0 / 4));
    };

    "(x + y) / (z + w)|x=1,y=2,z=3,w=4"_test = [&]
    {
        constexpr std::array a {1.0, 2.0, 3.0, 4.0};
        Tape tape;
        auto x = tape.variable(a[0]);
        auto y = tape.variable(a[1]);
        auto z = tape.variable(a[2]);
        auto w = tape.variable(a[3]);

        auto f = (x + y) / (z + w);
        auto g = f.gradient();

        expect(eq(f.value, (a[0] + a[1]) / (a[2] + a[3])));
        expect(eq(g.wrt(x), 1 / (a[2] + a[3])));
        expect(eq(g.wrt(y), 1 / (a[2] + a[3])));
        expect(eq(g.wrt(z), -(a[0] + a[1]) / ((a[2] + a[3]) * (a[2] + a[3]))));
        expect(eq(g.wrt(w), -(a[0] + a[1]) / ((a[2] + a[3]) * (a[2] + a[3]))));
    };
};

}  // namespace reverse::test

#endif
