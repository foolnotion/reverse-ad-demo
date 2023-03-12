#ifndef REVERSE_AD_DEMO_TEST_NNLS_HPP
#define REVERSE_AD_DEMO_TEST_NNLS_HPP

#include <Eigen/Core>
#include <unsupported/Eigen/LevenbergMarquardt>

#include "ext/boost/ut.hpp"
#include "reverse-ad-demo/expr.hpp"

namespace reverse::test
{

struct thurber_functor
{
    // see https://www.itl.nist.gov/div898/strd/nls/data/thurber.shtml
    using Scalar = double;

    // required by Eigen::LevenbergMarquardt
    using JacobianType = Eigen::Matrix<Scalar, -1, -1>;
    using QRSolver = Eigen::ColPivHouseholderQR<JacobianType>;

    static constexpr std::array start1 {1000.0, 1000.0, 400.0, 40.0, 0.7, 0.3, 0.03};
    static constexpr std::array start2 {1300.0, 1500.0, 500.0, 75.0, 1.0, 0.4, 0.05};

    static constexpr std::array xval {-3.067, -2.981, -2.921, -2.912, -2.840, -2.797, -2.702, -2.699, -2.633, -2.481,
                                      -2.363, -2.322, -1.501, -1.460, -1.274, -1.212, -1.100, -1.046, -0.915, -0.714,
                                      -0.566, -0.545, -0.400, -0.309, -0.109, -0.103, 0.010,  0.119,  0.377,  0.790,
                                      0.963,  1.006,  1.115,  1.572,  1.841,  2.047,  2.200};

    static constexpr std::array yval {80.574,   84.248,   87.264,   87.195,   89.076,   89.608,   89.868,   90.101,
                                      92.405,   95.854,   100.696,  101.060,  401.672,  390.724,  567.534,  635.316,
                                      733.054,  759.087,  894.206,  990.785,  1090.109, 1080.914, 1122.643, 1178.351,
                                      1260.531, 1273.514, 1288.339, 1327.543, 1353.863, 1414.509, 1425.208, 1421.384,
                                      1442.962, 1464.350, 1468.705, 1447.894, 1457.628};

    [[nodiscard]] auto values() const -> int { return xval.size(); }  // NOLINT
    [[nodiscard]] auto inputs() const -> int { return start1.size(); }  // NOLINT

    auto operator()(Eigen::Matrix<Scalar, -1, 1> const& input, Eigen::Matrix<Scalar, -1, 1>& residual) const -> int
    {
        return (*this)(input, residual.data(), static_cast<Scalar*>(nullptr));
    }

    auto df(Eigen::Matrix<Scalar, -1, 1> const& input, Eigen::Matrix<Scalar, -1, -1>& jacobian) const -> int  // NOLINT
    {
        return (*this)(input, static_cast<Scalar*>(nullptr), jacobian.data());
    }

  private:
    auto operator()(auto const& input, auto* residual, auto* jacobian) const -> int  // NOLINT
    {
        reverse::Tape<Scalar> tape;
        std::vector<decltype(tape)::Variable> beta;
        for (auto v : input) { beta.push_back(tape.variable(v)); }

        for (auto i = 0; i < std::ssize(xval); ++i) {
            tape.nodes.resize(input.size());
            for (auto j = 0; j < input.size(); ++j) {
                beta[j].value = input[j];
            }

            auto x = xval.at(static_cast<std::size_t>(i));
            auto xx = x * x;
            auto xxx = x * x * x;

            auto f = (beta[0] + beta[1] * x + beta[2] * xx + beta[3] * xxx)
                / (1 + beta[4] * x + beta[5] * xx + beta[6] * xxx);  // NOLINT

            if (residual != nullptr) {
                residual[i] = f.value - yval.at(static_cast<std::size_t>(i));  // NOLINT
            }

            if (jacobian != nullptr) {
                auto g = f.gradient();
                for (auto const& b : beta) {
                    jacobian[values() * b.index + i] = g.wrt(b);  // NOLINT
                }
            }
        }
        return 0;
    }
};

class approximately_equal
{
    double eps_;

  public:
    explicit approximately_equal(double epsilon)
        : eps_(epsilon)
    {
    }

    auto operator()(Arithmetic auto a, Arithmetic auto b) { return std::abs(a - b) < eps_; }
};

boost::ut::suite const nonlinear_least_squares_test_suite = []
{
    using namespace boost::ut;  // NOLINT

    "thurber"_test = [&]
    {
        auto constexpr tol {1.E4 * std::numeric_limits<double>::epsilon()};
        auto constexpr max_fun_eval {50};

        // try first starting point
        auto s1 = thurber_functor::start1;
        Eigen::VectorXd x = Eigen::Map<decltype(x) const>(s1.data(), std::ssize(s1));

        thurber_functor cost_function;
        Eigen::LevenbergMarquardt<thurber_functor> lm(cost_function);
        Eigen::LevenbergMarquardtSpace::Status status {};
        lm.setMaxfev(max_fun_eval);
        lm.setFtol(tol);
        lm.setXtol(tol);
        status = lm.minimize(x);

        auto constexpr expected_norm {5.6427082397E+03};
        std::array constexpr expected_x {1.2881396800E+03,
                                         1.4910792535E+03,
                                         5.8323836877E+02,
                                         7.5416644291E+01,
                                         9.6629502864E-01,
                                         3.9797285797E-01,
                                         4.9727297349E-02};

        auto constexpr eps {1e-4};
        expect(approximately_equal {eps}(lm.fvec().squaredNorm(), expected_norm));
        for (auto i = 0; i < x.size(); ++i) {
            expect(approximately_equal {eps}(x[i], expected_x.at(static_cast<std::size_t>(i))));
        }

        // try second starting point
        auto s2 = thurber_functor::start2;
        x = Eigen::Map<decltype(x) const>(s2.data(), std::ssize(s2));
        lm.resetParameters();
        lm.setMaxfev(max_fun_eval);  // NOLINT
        lm.setFtol(tol);
        lm.setXtol(tol);
        status = lm.minimize(x);

        expect(approximately_equal {eps}(lm.fvec().squaredNorm(), expected_norm));
        for (auto i = 0; i < x.size(); ++i) {
            expect(approximately_equal {eps}(x[i], expected_x.at(static_cast<std::size_t>(i))));
        }
    };
};

}  // namespace reverse::test

#endif
