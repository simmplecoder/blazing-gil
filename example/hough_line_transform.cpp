#include <blaze/math/dense/DynamicMatrix.h>
#include <cmath>
#include <flash/core.hpp>
#include <flash/numeric.hpp>

#include <boost/gil/typedefs.hpp>
#include <limits>

namespace gil = boost::gil;

int main()
{
    const std::size_t size = 32;
    blaze::DynamicMatrix<gil::uint8_t> input(size, size);

    for (std::size_t i = 0; i < size; ++i) {
        input(size - i - 1, i) = 1;
    }
    for (std::ptrdiff_t i = size - 1; i >= 0; --i) {
        for (std::ptrdiff_t j = 0; j < static_cast<std::ptrdiff_t>(size); ++j) {
            fmt::print("{} ", input(i, j));
        }
        fmt::print("\n");
    }

    const double _45_degrees = flash::pi / 4.0;
    const double _1_degree = flash::pi / 180.0;
    const double _5_degrees = flash::pi / 36.0;
    const std::size_t theta_step_count = 10;

    const flash::hough_parameter theta{_45_degrees - _5_degrees, _1_degree, theta_step_count};

    const std::size_t approximate_r = 32 * std::cos(_45_degrees);
    // const std::size_t approximate_r = 0;
    const std::size_t r_step_count = 5;
    const flash::hough_parameter r{approximate_r - 2.0, 1.0, r_step_count};
    // const flash::hough_parameter r{0, 1.0, r_step_count};

    fmt::print("expected maximum at r={} and theta={}\n", approximate_r, _45_degrees);
    auto parameter_space = flash::hough_line_transform(input, r, theta);
    for (std::size_t r_index = 0; r_index < r.step_count; ++r_index) {
        for (std::size_t theta_index = 0; theta_index < theta.step_count; ++theta_index) {
            double current_r = r.start_point + r.step_size * r_index;
            double current_theta = theta.start_point + theta.step_size * theta_index;
            fmt::print("r={}, theta={}, accumulated value={}\n",
                       current_r,
                       current_theta,
                       parameter_space(r_index, theta_index));
        }
    }
}