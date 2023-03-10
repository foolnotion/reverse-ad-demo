#ifndef REVERSE_AD_DEMO_TEST_NNLS_HPP
#define REVERSE_AD_DEMO_TEST_NNLS_HPP

#include <Eigen/Core>
#include <unsupported/Eigen/LevenbergMarquardt>

#include "ext/boost/ut.hpp"
#include "reverse-ad-demo/expr.hpp"

struct thurber_cost_func
{
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
    [[nodiscard]] auto inputs() const -> int { return start1.size(); }  //

    auto operator()(Eigen::Matrix<Scalar, -1, 1> const& input, Eigen::Matrix<Scalar, -1, 1>& residual) const -> int
    {
        reverse::Tape<double> tape;
        std::vector<decltype(tape)::Variable> beta;

        for (auto i = 0; i < xval.size(); ++i) {
            tape.clear();
            beta.clear();
            std::transform(
                input.begin(), input.end(), std::back_inserter(beta), [&](auto v) { return tape.variable(v); });

            auto x = xval.at(i);
            auto xx = x * x;
            auto xxx = x * x * x;

            auto f = (beta[0] + beta[1] * x + beta[2] * xx + beta[3] * xxx)
                / (1 + beta[4] * x + beta[5] * xx + beta[6] * xxx);

            residual(i) = f.value - yval.at(i);
        }
        return 0;
    }

    auto df(Eigen::Matrix<Scalar, -1, 1> const& input, Eigen::Matrix<Scalar, -1, -1>& jacobian) const -> int  // NOLINT
    {
        assert(jacobian.rows() == values());
        assert(jacobian.cols() == inputs());

        reverse::Tape<double> tape;
        std::vector<decltype(tape)::Variable> beta;

        for (auto i = 0; i < xval.size(); ++i) {
            tape.clear();
            beta.clear();
            std::transform(
                input.begin(), input.end(), std::back_inserter(beta), [&](auto v) { return tape.variable(v); });

            auto x = xval.at(i);
            auto xx = x * x;
            auto xxx = x * x * x;

            auto f = (beta[0] + beta[1] * x + beta[2] * xx + beta[3] * xxx)
                / (1 + beta[4] * x + beta[5] * xx + beta[6] * xxx);

            auto g = f.gradient();

            for (auto const& b : beta) {
                jacobian(i, static_cast<Eigen::Index>(b.index)) = g.wrt(b);
            }
        }
        return 0;
    }
};

boost::ut::suite const nonlinear_least_squares_test_suite = []
{
    using namespace boost::ut;  // NOLINT
    using Tape = reverse::Tape<double>;

    "thurber"_test = [&]
    {
        thurber_cost_func cost_function;
        Eigen::LevenbergMarquardt<thurber_cost_func> lm(cost_function);
        lm.setMaxfev(50);  // NOLINT
        Eigen::VectorXd x0 =
            Eigen::Map<decltype(x0) const>(thurber_cost_func::start1.data(), thurber_cost_func::start1.size());
        Eigen::LevenbergMarquardtSpace::Status status = lm.minimizeInit(x0);

        std::cout << "initial cost: " << lm.fnorm() * lm.fnorm() << ", initial parameters: " << x0.transpose() << "\n";
        if (status != Eigen::LevenbergMarquardtSpace::ImproperInputParameters) {
            auto k {0};
            do {
                status = lm.minimizeOneStep(x0);
                std::cout << "iter " << (++k) << "\t\tnorm = " << lm.fnorm() * lm.fnorm() << "\n";
            } while (status == Eigen::LevenbergMarquardtSpace::Running);
        } else {
            throw std::runtime_error("improper input parameters\n");
        }
        std::cout << "final cost: " << lm.fnorm() * lm.fnorm() << ", final parameters: " << x0.transpose() << "\n";
    };
};

#endif
