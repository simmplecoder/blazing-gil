#include <blaze/math/typetraits/UnderlyingElement.h>
#include <blaze/util/algorithms/Max.h>
#include <boost/gil/algorithm.hpp>
#include <boost/gil/extension/io/png.hpp>
#include <boost/gil/image.hpp>
#include <boost/gil/image_view.hpp>
#include <boost/gil/typedefs.hpp>

#include <CLI/CLI.hpp>

#include <iostream>
#include <limits>

#include <flash/core.hpp>
#include <flash/numeric.hpp>

namespace gil = boost::gil;

int main(int argc, char* argv[])
{
    CLI::App app{"Demonstration of anisotropic diffusion usage"};
    std::string input_file;
    std::string output_file;
    double kappa = 30;
    std::int64_t iteration_count = 10;

    app.add_option("i,--input", input_file, "PNG input file with RGB colorspace")
        ->required()
        ->check(CLI::ExistingFile);
    app.add_option("o,--output", output_file, "PNG output file that will be grayscale")->required();
    app.add_option("k,--kappa",
                   kappa,
                   "Control how well edges are respected, smaller value = more respect",
                   true);
    app.add_option("it,--iteration", iteration_count, "How many diffusion iteration to do", true);

    CLI11_PARSE(app, argc, argv);

    gil::rgb8_image_t input;
    gil::read_image(input_file, input, gil::png_tag{});

    auto view = gil::view(input);
    auto mat = flash::to_matrix_channeled(view);
    auto diffused = flash::anisotropic_diffusion(mat, kappa, iteration_count);
    auto remapped = flash::remap_to_channeled<std::uint8_t>(diffused);

    auto image = flash::from_matrix<gil::rgb8_image_t>(remapped);
    auto diffused_min =
        blaze::UnderlyingElement_t<decltype(diffused)>(flash::channelwise_minimum(diffused));
    auto diffused_max =
        blaze::UnderlyingElement_t<decltype(diffused)>(flash::channelwise_maximum(diffused));

    std::cout << "diffused min: " << diffused_min << '\n'
              << "diffused max: " << diffused_max << '\n';

    //    std::cout << "Gradient range: " << blaze::max(diffused) << ' ' << blaze::min(diffused) <<
    //    '\n'
    //              << "Final gray image range: " << static_cast<int>(blaze::max(image)) << ' '
    //              << static_cast<int>(blaze::min(image)) << '\n';
    gil::write_view(output_file, gil::view(image), gil::png_tag{});
}
