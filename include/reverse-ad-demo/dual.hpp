#ifndef REVERSE_AD_DEMO_DUAL_HPP
#define REVERSE_AD_DEMO_DUAL_HPP

#include <cmath>
#include <concepts>
#include <tuple>

namespace forward
{
struct dual
{
    double a {0};
    double b {0};

    auto operator*() const -> std::tuple<double, double> { return {a, b}; }

    friend auto operator+(dual const& lhs, dual const& rhs) -> dual
    {
        auto const [a, b] = *lhs;
        auto const [c, d] = *rhs;
        return {.a = a + c, .b = b + d};
    }
    friend auto operator+(std::floating_point auto x, dual const& a) { return dual {x} + a; }
    friend auto operator+(dual const& a, std::floating_point auto x) { return a + dual {x}; }

    auto operator+=(dual const& other) -> dual&
    {
        auto tmp = *this + other;
        std::swap(*this, tmp);
        return *this;
    }

    friend auto operator-(dual const& lhs, dual const& rhs) -> dual
    {
        auto const [a, b] = *lhs;
        auto const [c, d] = *rhs;
        return {.a = a - c, .b = b - d};
    }
    friend auto operator-(std::floating_point auto x, dual const& a) { return dual {x} - a; }
    friend auto operator-(dual const& a, std::floating_point auto x) { return a - dual {x}; }

    friend auto operator*(dual const& lhs, dual const& rhs) -> dual
    {
        auto const [a, b] = *lhs;
        auto const [c, d] = *rhs;
        return {.a = a * c, .b = a * d + b * c};
    }
    friend auto operator*(std::floating_point auto x, dual const& a) { return dual {x} * a; }
    friend auto operator*(dual const& a, std::floating_point auto x) { return a * dual {x}; }

    friend auto operator/(dual const& lhs, dual const& rhs) -> dual
    {
        auto const [a, b] = *lhs;
        auto const [c, d] = *rhs;
        return {.a = a / c, .b = (b * c - a * d) / (c * c)};
    }
    friend auto operator/(std::floating_point auto x, dual const& a) { return dual {x} / a; }
    friend auto operator/(dual const& a, std::floating_point auto x) { return a / dual {x}; }

    friend auto operator-(dual const& a) { return -1.0 * a; }

    auto sin() const -> dual
    {
        auto const [a, b] = this->operator*();
        return {.a = std::sin(a), .b = b * std::cos(a)};
    }

    auto cos() const -> dual
    {
        auto const [a, b] = this->operator*();
        return {.a = std::cos(a), .b = -b * std::sin(a)};
    }

    auto exp() const -> dual
    {
        auto const [a, b] = this->operator*();
        return {.a = std::exp(a), .b = b * std::exp(a)};
    }

    auto log() const -> dual
    {
        auto const [a, b] = this->operator*();
        return {.a = std::log(a), .b = b / a};
    }
};
}  // namespace forward

#endif
