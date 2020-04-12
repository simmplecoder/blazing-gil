#pragma once

#include <blaze/Blaze.h>
#include <convolution.hpp>

#include <cstdint>
#include <iostream>
#include <algorithm>

// temporary fix for mapping of matrix with static vector as elements
// https://bitbucket.org/blaze-lib/blaze/issues/344/incorrect-behavior-of-element-type
namespace blaze {

template< typename OP >
struct MapTrait< DynamicMatrix<StaticVector<double,8>>, OP >
{
   using Type = DynamicMatrix<std::invoke_result_t<OP, StaticVector<double,8>>>;
};

} // namespace blaze

namespace flash{
namespace detail
{
    template <typename Expr>
    auto compute_diffusivity(Expr& nabla, double kappa)
    {
        return blaze::exp(-(nabla / kappa) % (nabla / kappa));
    }
}

template <typename T>
blaze::DynamicMatrix<bool> nonmax_map(const blaze::DynamicMatrix<T>& input, std::size_t window_size, bool padding_value = false)
{
    const auto middle = window_size / 2 + 1;
    blaze::DynamicMatrix<bool> result(input.rows(), input.columns(), padding_value);
    // will substract window_size, thus until .rows()
    for (std::size_t i = window_size; i < input.rows(); ++i)
    {
        // will substract window_size, thus until .columns()
        for (std::size_t j = window_size; j < input.columns(); ++j)
        {
            auto submatrix = blaze::submatrix(input, i - window_size, j - window_size, window_size, window_size);
            auto max = blaze::max(submatrix);
            auto other_max_flag = false;
            for (std::size_t ii = 0; ii < window_size; ++ii)
            {
                for (std::size_t jj = 0; jj < window_size; ++jj)
                {
                    if (ii == middle && jj == middle)
                    {
                        continue;
                    }
                    if (submatrix(ii, jj) == max)
                    {
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

struct hessian_result{
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


blaze::DynamicMatrix<double> anisotropic_diffusion(const blaze::DynamicMatrix<std::uint8_t>& input, double kappa, std::uint64_t iteration_count)
{
    using matrix_type = blaze::DynamicMatrix<double>;
    matrix_type output(input);
    std::size_t output_area_start[2] = {1, 1};
    std::size_t output_area_dims[2] = {output.rows() - 2, output.columns() - 2};
    auto region = [&output, output_area_start, output_area_dims](int i, int j) {
        return blaze::submatrix(output, output_area_start[0] + i, output_area_start[1] + j, output_area_dims[0], output_area_dims[1]);
    };
    auto output_area = region(0, 0);
    for (std::uint64_t i = 0; i < iteration_count; ++i)
    {
        auto nabla = blaze::evaluate(blaze::generate(output_area.rows(), output_area.columns(), [&output](std::size_t i, std::size_t j) 
        {
            auto real_i = i + 1;
            auto real_j = j + 1;
            return blaze::StaticVector<double, 8>{
                output(real_i - 1, real_j) - output(real_i, real_j),
                output(real_i + 1, real_j) - output(real_i, real_j),
                output(real_i, real_j + 1) - output(real_i, real_j),
                output(real_i, real_j - 1) - output(real_i, real_j),
                output(real_i - 1, real_j + 1) - output(real_i, real_j),
                output(real_i - 1, real_j - 1) - output(real_i, real_j),
                output(real_i + 1, real_j + 1) - output(real_i, real_j),
                output(real_i + 1, real_j - 1) - output(real_i, real_j)
            };
        }));
        auto c = blaze::map(nabla, 
            [kappa](auto element)
        {
            auto c_element = blaze::evaluate(element / kappa);
            auto half = 0.5;
            return blaze::StaticVector<double, 8> {
                std::exp(-c_element[0] * c_element[0]),
                std::exp(-c_element[1] * c_element[1]),
                std::exp(-c_element[2] * c_element[2]),
                std::exp(-c_element[3] * c_element[3]),
                std::exp(-c_element[4] * c_element[4]) * half,
                std::exp(-c_element[5] * c_element[5]) * half,
                std::exp(-c_element[6] * c_element[6]) * half,
                std::exp(-c_element[7] * c_element[7]) * half,
            };
        }
        );
        nabla %= c;
        auto sum = blaze::evaluate(blaze::map(nabla, [](auto element) {
            return blaze::sum(element) * 1.0 / 7;
        }));
        output_area += sum;
    }

    return output;
}
}
