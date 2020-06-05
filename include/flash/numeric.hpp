#ifndef BLAZING_GIL_NUMERIC_HPP
#define BLAZING_GIL_NUMERIC_HPP

#include <blaze/Blaze.h>
#include <blaze/math/typetraits/IsVector.h>
#include <flash/convolution.hpp>

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

template <typename T>
struct identify_type;

template <typename MT, bool StorageOrder>
auto anisotropic_diffusion(const blaze::DenseMatrix<MT, StorageOrder>& input, double kappa,
                           std::uint64_t iteration_count)
{
    using element_type = blaze::UnderlyingElement_t<MT>;
    using output_element_type =
        std::conditional_t<blaze::IsVector_v<element_type>,
                           blaze::StaticVector<double, element_type::size()>,
                           double>;
    using matrix_type = blaze::DynamicMatrix<output_element_type, StorageOrder>;
    matrix_type output(input);
    std::size_t output_area_start[2] = {1, 1};
    std::size_t output_area_dims[2] = {output.rows() - 2, output.columns() - 2};
    auto region = [&output, output_area_start, output_area_dims](int i, int j) {
        return blaze::submatrix(output,
                                output_area_start[0] + i,
                                output_area_start[1] + j,
                                output_area_dims[0],
                                output_area_dims[1]);
    };
    auto output_area = region(0, 0);
    for (std::uint64_t i = 0; i < iteration_count; ++i) {
        auto nabla = blaze::evaluate(blaze::generate(
            output_area.rows(), output_area.columns(), [&output](std::size_t i, std::size_t j) {
                auto real_i = i + 1;
                auto real_j = j + 1;
                return blaze::StaticVector<output_element_type, 8>{
                    output(real_i - 1, real_j) - output(real_i, real_j),
                    output(real_i + 1, real_j) - output(real_i, real_j),
                    output(real_i, real_j + 1) - output(real_i, real_j),
                    output(real_i, real_j - 1) - output(real_i, real_j),
                    output(real_i - 1, real_j + 1) - output(real_i, real_j),
                    output(real_i - 1, real_j - 1) - output(real_i, real_j),
                    output(real_i + 1, real_j + 1) - output(real_i, real_j),
                    output(real_i + 1, real_j - 1) - output(real_i, real_j)};
            }));
        auto c = blaze::map(nabla, [kappa](const auto& element) {
            auto c_element = blaze::evaluate(element / kappa);
            auto half = 0.5;
            return blaze::StaticVector<output_element_type, 8>{
                blaze::exp(-c_element[0] * c_element[0]),
                blaze::exp(-c_element[1] * c_element[1]),
                blaze::exp(-c_element[2] * c_element[2]),
                blaze::exp(-c_element[3] * c_element[3]),
                blaze::exp(-c_element[4] * c_element[4]) * half,
                blaze::exp(-c_element[5] * c_element[5]) * half,
                blaze::exp(-c_element[6] * c_element[6]) * half,
                blaze::exp(-c_element[7] * c_element[7]) * half,
            };
        });
        nabla %= c;
        auto sum = blaze::evaluate(
            blaze::map(nabla, [](auto element) { return blaze::sum(element) * 1.0 / 7; }));
        output_area += sum;
    }

    return output;
}
} // namespace flash

#endif