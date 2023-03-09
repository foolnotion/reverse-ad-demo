#ifndef REVERSE_AD_DEMO_EXPR_HPP
#define REVERSE_AD_DEMO_EXPR_HPP

#include <array>
#include <cassert>
#include <cmath>
#include <type_traits>
#include <vector>

namespace reverse
{

template<typename T>
concept Arithmetic = requires
{
    std::is_arithmetic_v<T>;
};

template<Arithmetic T>
struct Node
{
    std::array<T, 2> partials {T {0}, T {0}};  // partial derivative values
    std::array<std::size_t, 2> parents {0, 0};  // indices of parent nodes
};

// forward declaration
template<Arithmetic T>
struct Var;

template<Arithmetic T>
struct Tape
{
    using node_t = Node<T>;
    std::vector<node_t> nodes;

    auto push() -> std::size_t
    {
        auto idx = std::size(nodes);
        nodes.push_back({{T {0}, T {0}}, {idx, idx}});
        return idx;
    }

    auto push(auto i, auto p) -> std::size_t
    {
        auto idx = std::size(nodes);
        nodes.push_back({{T {p}, T {0}}, {i, idx}});
        return idx;
    }

    auto push(auto i0, auto p0, auto i1, auto p1) -> std::size_t
    {
        auto idx = std::size(nodes);
        nodes.push_back({{T {p0}, T {p1}}, {i0, i1}});
        return idx;
    }

    auto length() const -> std::size_t { return std::size(nodes); }

    auto variable(T value)
    {
        Var<T> v {*this, value, push()};
        return v;
    }

    auto clear() { nodes.clear(); }
};

template<Arithmetic T>
struct grad
{
    auto wrt(Var<T> const& v) { return values[v.index]; }
    std::vector<T> values;
};

template<Arithmetic T>
struct Var
{
    explicit Var(Tape<T>& t, T v = T {0}, std::size_t i = 0)
        : tape(t)
        , index(i)
        , value(v)
    {
    }

    auto gradient() const -> grad<T>
    {
        std::vector<T> grad(tape.length(), T {0.0});
        grad[index] = 1.0;

        for (auto i = tape.length() - 1; i < tape.length(); --i) {
            auto const& n = tape.nodes[i];
            auto d = grad[i];

            for (auto j = 0UL; j < std::size(n.parents); ++j) {
                grad[n.parents[j]] += n.partials[j] * d;
            }
        }

        return {grad};
    }

    friend auto operator+(Var const& a, Var const& b) -> Var
    {
        assert(&a.tape == &b.tape);
        return Var {a.tape, a.value + b.value, a.tape.push(a.index, T {1.0}, b.index, T {1.0})};
    }

    friend auto operator+(Arithmetic auto a, Var const& b) -> Var
    {
        return Var {b.tape, a + b.value, b.tape.push(0UL, T {0.0}, b.index, T {1.0})};
    }

    friend auto operator+(Var const& a, Arithmetic auto b) -> Var
    {
        return Var {a.tape, a.value + b, a.tape.push(a.index, T {1.0}, 0UL, T {0.0})};
    }

    friend auto operator-(Var const& a, Var const& b) -> Var
    {
        assert(&a.tape == &b.tape);
        return Var {a.tape, a.value - b.value, a.tape.push(a.index, T {1.0}, b.index, T {-1.0})};
    }

    friend auto operator-(Arithmetic auto a, Var const& b) -> Var
    {
        return Var {b.tape, a - b.value, b.tape.push(0UL, T {0.0}, b.index, T {-1.0})};
    }

    friend auto operator-(Var const& a, Arithmetic auto b) -> Var
    {
        return Var {a.tape, a.value - b, a.tape.push(a.index, T {1.0}, 0UL, T {0.0})};
    }

    friend auto operator*(Var const& a, Var const& b) -> Var
    {
        assert(&a.tape == &b.tape);
        return Var {a.tape, a.value * b.value, a.tape.push(a.index, b.value, b.index, a.value)};
    }

    friend auto operator*(Arithmetic auto a, Var const& b) -> Var
    {
        return Var {b.tape, a * b.value, b.tape.push(0UL, T {0.0}, b.index, a)};
    }

    friend auto operator*(Var const& a, Arithmetic auto b) -> Var
    {
        return Var {a.tape, a.value * b, a.tape.push(a.index, b, 0UL, T {0.0})};
    }

    friend auto operator/(Var const& a, Var const& b) -> Var
    {
        assert(&a.tape == &b.tape);
        return Var {
            a.tape, a.value / b.value, a.tape.push(a.index, 1 / b.value, b.index, -a.value / (b.value * b.value))};
    }

    friend auto operator/(Arithmetic auto a, Var const& b) -> Var
    {
        return Var {b.tape, a / b.value, b.tape.push(0UL, T {0.0}, b.index, -1 / (b.value * b.value))};
    }

    friend auto operator/(Var const& a, Arithmetic auto b) -> Var
    {
        return Var {a.tape, a.value / b, a.tape.push(a.index, 1.0 / b, 0UL, T {0.0})};
    }

    auto sin() const -> Var { return Var {tape, std::sin(value), tape.push(index, std::cos(value))}; }

    auto cos() const -> Var { return Var {tape, std::cos(value), tape.push(index, -std::sin(value))}; }

    auto exp() const -> Var { return Var {tape, std::exp(value), tape.push(index, std::exp(value))}; }

    auto log() const -> Var { return Var {tape, std::log(value), tape.push(index, 1 / value)}; }

    Tape<T>& tape;  // reference to the tape
    std::size_t index {};  // index of the current node
    T value {};  // associated value
};
}  // namespace reverse

#endif
