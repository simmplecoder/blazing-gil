#include <blaze/math/typetraits/UnderlyingElement.h>
#include <blaze/util/algorithms/Max.h>
#include <boost/gil/algorithm.hpp>
#include <boost/gil/extension/io/png.hpp>
#include <boost/gil/image.hpp>
#include <boost/gil/image_view.hpp>
#include <boost/gil/typedefs.hpp>

#include <spdlog/spdlog.h>

#include <CLI/CLI.hpp>

#include <iostream>
#include <limits>

#include <flash/core.hpp>
#include <flash/numeric.hpp>

namespace gil = boost::gil;

template <typename ImageType>
void run(const std::string& input_file, double delta_t, double kappa, std::uint64_t iteration_count,
         const std::string& output_file)
{

    ImageType input;
    gil::read_image(input_file, input, gil::png_tag{});

    auto view = gil::view(input);
    auto mat = flash::to_matrix_channeled(view);
    auto diffused = flash::anisotropic_diffusion(mat, delta_t, kappa, iteration_count);

    auto image = flash::from_matrix<ImageType>(diffused);
    auto diffused_min = flash::channelwise_min(diffused);
    auto diffused_max = flash::channelwise_max(diffused);

    std::cout << "diffused min: " << diffused_min << '\n'
              << "diffused max: " << diffused_max << '\n';

    //    std::cout << "Gradient range: " << blaze::max(diffused) << ' ' << blaze::min(diffused) <<
    //    '\n'
    //              << "Final gray image range: " << static_cast<int>(blaze::max(image)) << ' '
    //              << static_cast<int>(blaze::min(image)) << '\n';
    gil::write_view(output_file, gil::view(image), gil::png_tag{});
}

int main(int argc, char* argv[])
{
    CLI::App app{"Demonstration of anisotropic diffusion usage"};
    std::string input_file;
    std::string output_file;
    double kappa = 30;
    std::int64_t iteration_count = 10;
    bool gray_mode = false;

    app.add_flag("--graymode", gray_mode, "set if the input file is grayscale");
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

    const double delta_t = 1.0 / 4.0;
    if (!gray_mode)
        run<gil::rgb8_image_t>(input_file, delta_t, kappa, iteration_count, output_file);
    else
        run<gil::gray8_image_t>(input_file, delta_t, kappa, iteration_count, output_file);
}
