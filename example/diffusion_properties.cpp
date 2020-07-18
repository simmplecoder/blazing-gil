#include <blaze/math/typetraits/UnderlyingElement.h>
#include <blaze/util/algorithms/Max.h>
#include <boost/gil/algorithm.hpp>
#include <boost/gil/extension/io/png.hpp>
#include <boost/gil/image.hpp>
#include <boost/gil/image_view.hpp>
#include <boost/gil/typedefs.hpp>

#include <iomanip>
#include <iostream>
#include <limits>

#include <flash/core.hpp>
#include <flash/numeric.hpp>

#include <CLI/CLI.hpp>

namespace gil = boost::gil;

using element_type = blaze::StaticVector<gil::uint8_t, 1>;
using diffuse_type = blaze::StaticVector<double, 1>;
auto generate_image()
{
    blaze::DynamicMatrix<element_type> mat(32, 32, element_type(0));
    blaze::submatrix(mat, 8, 8, 16, 16) = element_type(255);

    return flash::from_matrix<gil::gray8_image_t>(mat);
}

void print_image(const blaze::DynamicMatrix<element_type>& mat)
{
    for (std::size_t i = 0; i < mat.rows(); ++i) {
        for (std::size_t j = 0; j < mat.columns(); ++j) {
            std::cout << static_cast<gil::uint16_t>(mat(j, i)[0]) << ' ';
        }
        std::cout << '\n';
    }
}

void print_image(const blaze::DynamicMatrix<diffuse_type>& mat)
{
    for (std::size_t i = 0; i < mat.rows(); ++i) {
        for (std::size_t j = 0; j < mat.columns(); ++j) {
            std::cout << mat(j, i)[0] << ' ';
        }
        std::cout << '\n';
    }
}

int main(int argc, char* argv[])
{
    std::cout << std::fixed << std::setprecision(1);
    CLI::App app{"Demonstration of anisotropic diffusion usage"};
    std::string input_file;
    std::string output_file;
    double kappa = 30;
    std::int64_t iteration_count = 10;

    app.add_option("o,--output", output_file, "PNG output file that will be grayscale")->required();
    app.add_option("k,--kappa",
                   kappa,
                   "Control how well edges are respected, smaller value = more respect",
                   true);
    app.add_option("it,--iteration", iteration_count, "How many diffusion iteration to do", true);

    CLI11_PARSE(app, argc, argv);
    gil::gray8_image_t input = generate_image();

    auto view = gil::view(input);
    auto mat = flash::to_matrix_channeled(view);
    print_image(mat);
    std::uint64_t sum_before = 0;
    for (std::size_t i = 0; i < mat.rows(); ++i) {
        for (std::size_t j = 0; j < mat.columns(); ++j) {
            sum_before += mat(i, j)[0];
        }
    }
    auto diffused = flash::anisotropic_diffusion(mat, kappa, iteration_count);
    print_image(diffused);
    std::uint64_t sum_after = 0;
    for (std::size_t i = 0; i < mat.rows(); ++i) {
        for (std::size_t j = 0; j < mat.columns(); ++j) {
            sum_after += mat(i, j)[0];
        }
    }
    auto image = flash::from_matrix<gil::gray8_image_t>(diffused);
    auto diffused_min = flash::channelwise_min(diffused);
    auto diffused_max = flash::channelwise_max(diffused);

    std::cout << "diffused min: " << diffused_min << '\n'
              << "diffused max: " << diffused_max << '\n';

    std::cout << "sum before: " << sum_before << '\n' << "sum after: " << sum_after << '\n';

    //    std::cout << "Gradient range: " << blaze::max(diffused) << ' ' << blaze::min(diffused) <<
    //    '\n'
    //              << "Final gray image range: " << static_cast<int>(blaze::max(image)) << ' '
    //              << static_cast<int>(blaze::min(image)) << '\n';
    gil::write_view(output_file, gil::view(image), gil::png_tag{});
}
