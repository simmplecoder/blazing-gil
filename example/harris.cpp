#include <blaze/Blaze.h>
#include <boost/gil/algorithm.hpp>
#include <boost/gil/extension/io/png.hpp>
#include <boost/gil/image.hpp>
#include <boost/gil/image_view.hpp>
#include <boost/gil/typedefs.hpp>

#include <CLI11.hpp>

#include <iostream>
#include <limits>

#include <flash/core.hpp>
#include <flash/numeric.hpp>

namespace gil = boost::gil;

int main(int argc, char* argv[])
{
    std::string input_file;
    std::string harris_response_file;
    std::string output_file;
    double k = 0.04;
    std::int64_t threshold;

    CLI::App app{
        "Demonstration of Harris affine region detector - finding corners"};
    app.add_option("i,--input", input_file, "PNG file with RGB colorspace")
        ->required()
        ->check(CLI::ExistingFile);
    app.add_option(
           "hrf",
           harris_response_file,
           "Grayscale image that will contain Harris response as pixels")
        ->required();
    app.add_option("o,--output",
                   output_file,
                   "Output file that will contain green markers where Harris "
                   "corner detector tested positive")
        ->required();
    app.add_option("t,--threshold",
                   threshold,
                   "Threshold value to test on during marking the input file "
                   "with green pixels")
        ->required();
    app.add_option("k,--discriminant",
                   k,
                   "Discriminator to prefer corners to edges",
                   true);

    CLI11_PARSE(app, argc, argv);

    gil::rgb8_image_t input;
    gil::read_image(input_file, input, gil::png_tag{});

    gil::gray8_image_t gray(input.dimensions());
    gil::copy_and_convert_pixels(gil::view(input), gil::view(gray));

    auto image = flash::to_matrix(gil::view(gray));
    blaze::DynamicMatrix<std::int16_t> mat(image);
    auto harris = flash::harris(mat, k);
    harris = blaze::map(harris, [](std::int64_t x) {
        if (x >= 0)
            return x;
        else
            return static_cast<std::int64_t>(0);
    });

    image = flash::remap_to<unsigned char>(harris);
    for (std::size_t i = 0; i < image.rows(); ++i) {
        for (std::size_t j = 0; j < image.columns(); ++j) {
            if (harris(i, j) >= threshold) {
                gil::view(input)(j, i) = gil::rgb8_pixel_t(0, 255, 0);
            }
        }
    }

    std::cout << "Gradient range: " << blaze::max(harris) << ' '
              << blaze::min(harris) << '\n'
              << "Final gray image range: "
              << static_cast<int>(blaze::max(image)) << ' '
              << static_cast<int>(blaze::min(image)) << '\n';
    gil::write_view(harris_response_file, gil::view(gray), gil::png_tag{});
    gil::write_view(output_file, gil::view(input), gil::png_tag{});
}
