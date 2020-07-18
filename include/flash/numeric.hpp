#ifndef BLAZING_GIL_NUMERIC_HPP
#define BLAZING_GIL_NUMERIC_HPP

#include <blaze/Blaze.h>
#include <blaze/math/expressions/DMatGenExpr.h>
#include <blaze/math/typetraits/IsVector.h>
#include <blaze/math/typetraits/UnderlyingElement.h>
#include <blaze/math/views/Submatrix.h>
#include <flash/convolution.hpp>

#include <spdlog/spdlog.h>

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <type_traits>

namespace flash
{
namespace detail
{
template <typename Expr>
auto compute_diffusivity(Expr& nabla, double kappa)
{
    return blaze::exp(-(nabla / kappa) % (nabla / kappa));
}
} // namespace detail

template <typename T>
blaze::DynamicMatrix<bool> nonmax_map(const blaze::DynamicMatrix<T>& input, std::size_t window_size,
                                      bool padding_value = false)
{
    const auto middle = window_size / 2 + 1;
    blaze::DynamicMatrix<bool> result(input.rows(), input.columns(), padding_value);
    // will substract window_size, thus until .rows()
    for (std::size_t i = window_size; i < input.rows(); ++i) {
        // will substract window_size, thus until .columns()
        for (std::size_t j = window_size; j < input.columns(); ++j) {
            auto submatrix =
                blaze::submatrix(input, i - window_size, j - window_size, window_size, window_size);
            auto max = blaze::max(submatrix);
            auto other_max_flag = false;
            for (std::size_t ii = 0; ii < window_size; ++ii) {
                for (std::size_t jj = 0; jj < window_size; ++jj) {
                    if (ii == middle && jj == middle) {
                        continue;
                    }
                    if (submatrix(ii, jj) == max) {
                        other_max_flag = true;
                        break;
                    }
                }
            }

            result(i, j) = other_max_flag && (max == submatrix(middle, middle));
        }
    }

    return result;
}

blaze::DynamicMatrix<std::int64_t> harris(const blaze::DynamicMatrix<std::int64_t>& image, double k)
{
    auto dx = flash::convolve(image, flash::sobel_x);
    auto dy = flash::convolve(image, flash::sobel_y);

    auto dx_2 = dx % dx;
    auto dy_2 = dy % dy;

    auto dxdy = flash::convolve(dx, flash::sobel_y);

    auto ktrace_2 = (dx_2 + dy_2) % (dx_2 + dy_2) * k;
    auto det = dx_2 % dy_2 - dxdy % dxdy;
    return det - ktrace_2;
}

struct hessian_result {
    blaze::DynamicMatrix<std::int32_t> determinants;
    blaze::DynamicMatrix<std::int32_t> traces;
};

hessian_result hessian(const blaze::DynamicMatrix<std::uint8_t>& input)
{
    blaze::DynamicMatrix<std::int32_t> extended = input;
    auto dx = flash::convolve(extended, flash::sobel_x);
    auto dy = flash::convolve(extended, flash::sobel_y);

    auto ddxx = flash::convolve(dx, flash::sobel_x);
    auto dxdy = flash::convolve(dx, flash::sobel_y);
    auto ddyy = flash::convolve(dy, flash::sobel_y);

    auto det = ddxx % ddyy - dxdy % dxdy;
    auto trace = ddxx + ddyy;
    return {det, trace};
}

std::vector<double> build_exponent_table(unsigned int channel_count, double sigma)
{
    const double sigma_squared = 1.0 / (sigma * sigma);
    std::vector<double> exponent_table(255 * channel_count);
    spdlog::info("sigma^2: {}, size: {}", sigma_squared, exponent_table.size());
    for (double i = 0; i < exponent_table.size(); ++i) {
        spdlog::info("exponent: {}", -(i * i) * sigma_squared);
        exponent_table[i] = std::exp(-(i * i) * sigma_squared);
    }

    return exponent_table;
}

template <typename MT, bool StorageOrder, bool OutputStorageOrder = StorageOrder>
auto anisotropic_diffusion(const blaze::DenseMatrix<MT, StorageOrder>& input, double delta_t,
                           double kappa, std::uint64_t iteration_count)
{
    using element_type = blaze::UnderlyingElement_t<MT>;
    using output_element_type =
        std::conditional_t<blaze::IsVector_v<element_type>,
                           blaze::StaticVector<double, element_type::size()>,
                           double>;
    auto exponent_table =
        build_exponent_table(element_type::size(), kappa * element_type::size() * 255.0);
    spdlog::info("{}", fmt::join(exponent_table, ", "));
    using compute_element_type = blaze::StaticVector<output_element_type, 8>;
    using output_matrix_type = blaze::DynamicMatrix<output_element_type, OutputStorageOrder>;

    const auto rows = (~input).rows();
    const auto columns = (~input).columns();
    output_matrix_type scratch(rows + 2, columns + 2, output_element_type(0));
    auto scratch_area = blaze::submatrix(scratch, 1, 1, rows, columns);
    scratch_area = input;

    output_matrix_type scratch2(rows + 2, columns + 2);
    auto scratch_area2 = blaze::submatrix(scratch2, 1, 1, rows, columns);
    for (std::uint64_t counter = 0; counter < iteration_count; ++counter) {
        // borders
        blaze::row(scratch, 0) = blaze::row(scratch, 1);
        blaze::row(scratch, rows + 1) = blaze::row(scratch, rows);
        blaze::column(scratch, 0) = blaze::column(scratch, 1);
        blaze::column(scratch, columns + 1) = blaze::column(scratch, columns);

        // corners
        scratch(0, 0) = scratch(1, 1);
        scratch(0, columns + 1) = scratch(0, columns);
        scratch(rows + 1, 0) = scratch(rows, 0);
        scratch(rows + 1, columns + 1) = scratch(rows, columns);

        scratch_area2 = blaze::generate(
            rows,
            columns,
            [&scratch, &exponent_table, kappa, delta_t](std::size_t absolute_i,
                                                        std::size_t absolute_j) {
                auto i = absolute_i + 1;
                auto j = absolute_j + 1;
                const auto& current = scratch(i, j);
                compute_element_type nabla{scratch(i - 1, j) - current,
                                           scratch(i + 1, j) - current,
                                           scratch(i, j - 1) - current,
                                           scratch(i, j + 1) - current};
                compute_element_type diffusivity =
                    blaze::map(nabla, [&exponent_table, kappa](auto value) {
                        value /= kappa;
                        return output_element_type(blaze::exp(-(value * value)));
                    });

                return blaze::evaluate(current + blaze::sum(nabla * diffusivity) * delta_t);
            });
        std::swap(scratch, scratch2);
    }

    return output_matrix_type(scratch_area);
}
} // namespace flash

#endif