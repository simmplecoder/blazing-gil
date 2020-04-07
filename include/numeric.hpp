#include <blaze/math/dense/DynamicMatrix.h>
#include <blaze/math/dense/StaticMatrix.h>
#include <blaze/math/expressions/DMatDMatAddExpr.h>
#include <blaze/math/expressions/DMatDMatSubExpr.h>
#include <blaze/math/expressions/DMatDMatSchurExpr.h>
#include <blaze/math/expressions/DMatExpExpr.h>
#include <blaze/math/expressions/DMatMapExpr.h>
#include <blaze/math/expressions/DMatScalarMultExpr.h>
#include <blaze/math/expressions/DMatScalarDivExpr.h>
#include <blaze/math/expressions/Matrix.h>
#include <blaze/math/views/Submatrix.h>
#include <convolution.hpp>

#include <cstdint>
#include <iostream>

namespace flash{
namespace detail
{
    template <typename Expr>
    auto compute_diffusivity(Expr& nabla, double kappa)
    {
        return blaze::exp(-(nabla / kappa) % (nabla / kappa));
    }
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
    std::cout << "entering first diffusion cycle\n";
    for (std::uint64_t i = 0; i < iteration_count; ++i)
    {
        auto nabla_north = region(-1, 0) - output_area;
        auto nabla_south = region(+1, 0) - output_area;
        auto nabla_east = region(0, +1) - output_area;
        auto nabla_west = region(0, -1) - output_area;
        auto nabla_ne = region(-1, +1) - output_area;
        auto nabla_nw = region(-1, -1) - output_area;
        auto nabla_se = region(+1, +1) - output_area;
        auto nabla_sw = region(+1, -1) - output_area;

        auto c_north = detail::compute_diffusivity(nabla_north, kappa);
        auto c_south = detail::compute_diffusivity(nabla_south, kappa);
        auto c_east = detail::compute_diffusivity(nabla_east, kappa);
        auto c_west = detail::compute_diffusivity(nabla_west, kappa);
        auto c_ne = detail::compute_diffusivity(nabla_ne, kappa);
        auto c_nw = detail::compute_diffusivity(nabla_nw, kappa);
        auto c_se = detail::compute_diffusivity(nabla_se, kappa);
        auto c_sw = detail::compute_diffusivity(nabla_sw, kappa);

        const auto half = 1 / 2.0;
        // blaze::DynamicMatrix<double> one_eighth_sum = c_north % nabla_north;  //c_north % nabla_north + ;
        // std::cout << "computed 1/8 sum\n" << one_eighth_sum.rows() << ' ' << one_eighth_sum.columns() << '\n';
        // blaze::DynamicMatrix<double> second_eighth_sum = c_south % nabla_south;
        // std::cout << "computed 2/8 sum\n";
        // auto quarter_sum = one_eighth_sum + second_eighth_sum;
        // std::cout << "computed quarter_sum\n";
        // blaze::DynamicMatrix<double> second_half_sum = ((c_ne % nabla_ne) * half) + ((c_nw % nabla_nw) * half)
        //     + ((c_se % nabla_se) * half) + ((c_sw % nabla_sw) * half);
        // std::cout << output_area_dims[0] << ' ' << output_area_dims[1] << ' ' << one_eighth_sum.rows() << ' ' << one_eighth_sum.columns() 
        //     << ' ' << second_eighth_sum.rows() << ' ' << second_eighth_sum.columns() 
        //     << ' ' << quarter_sum.rows() << ' ' << quarter_sum.columns() << ' ' << second_half_sum.rows() << ' ' << second_half_sum.columns() <<'\n';
        
        // std::cout << "computed second_half_sum\n";
        // output_area = (
        //     one_eighth_sum + second_half_sum
        // ) * (1 / 7.0);
        // std::cout << "completed diffusion cycle " << i + 1 << '\n';
        output_area = output_area + ((
            c_north % nabla_north + c_south % nabla_south
            + c_east % nabla_east + c_west % nabla_west
            + ((c_ne % nabla_ne) * half) + ((c_nw % nabla_nw) * half)
            + ((c_se % nabla_se) * half) + ((c_sw % nabla_sw) * half)
        ) * (1 / 7.0));
    }

    return output;
}
}
