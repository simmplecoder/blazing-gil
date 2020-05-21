#include <blaze/Blaze.h>
#include <boost/gil/algorithm.hpp>
#include <boost/gil/extension/io/png.hpp>
#include <boost/gil/image.hpp>
#include <boost/gil/image_view.hpp>
#include <boost/gil/typedefs.hpp>

#include <CLI/CLI.hpp>

#include <flash/core.hpp>
#include <flash/numeric.hpp>

#include <iostream>
#include <limits>

namespace gil = boost::gil;

int main(int argc, char* argv[])
{
    std::string input_file;
    std::string determinants_map_file;
    std::string traces_map_file;
    std::string output_file;

    std::int32_t dets_threshold;
    std::int32_t traces_threshold;

    CLI::App app{"Demonstration of Hessian affine region detector - finding corners"};
    app.add_option("i,--input", input_file, "PNG file with RGB colorspace")
        ->required()
        ->check(CLI::ExistingFile);
    app.add_option("dmf",
                   determinants_map_file,
                   "determinants map file will be a grayscale PNG image with "
                   "determinant of Hessian as pixels")
        ->required();
    app.add_option("tmf",
                   traces_map_file,
                   "traces map file will be a grayscale PNG image with trace "
                   "of Hessian as pixels")
        ->required();
    app.add_option("o,--output",
                   output_file,
                   "Output file that will contain green markers where Hessian "
                   "corner detector tested positive")
        ->required();
    app.add_option("dt", dets_threshold, "Threshold for determinants response")->required();
    app.add_option("tt", traces_threshold, "Threshold for traces response")->required();

    CLI11_PARSE(app, argc, argv);

    gil::rgb8_image_t input;
    gil::read_image(input_file, input, gil::png_tag{});

    gil::gray8_image_t gray(input.dimensions());
    gil::copy_and_convert_pixels(gil::view(input), gil::view(gray));

    auto image = flash::to_matrix(gil::view(gray));
    blaze::DynamicMatrix<std::int16_t> mat(image);
    auto hessian_result = flash::hessian(mat);
    auto thresholded_dets =
        blaze::evaluate(blaze::map(hessian_result.determinants, [dets_threshold](auto x) {
            return x < dets_threshold ? 0 : x;
        }));
    auto thresholded_traces =
        blaze::evaluate(blaze::map(hessian_result.traces, [traces_threshold](auto x) {
            return x < traces_threshold ? 0 : x;
        }));
    auto dets_nonmax_map = flash::nonmax_map(thresholded_dets, 3);
    auto trace_nonmax_map = flash::nonmax_map(thresholded_traces, 3);

    auto determinants_map =
        flash::to_gray8_image(flash::remap_to<unsigned char>(hessian_result.determinants));
    auto traces_map = flash::to_gray8_image(flash::remap_to<unsigned char>(hessian_result.traces));
    for (std::size_t i = 0; i < image.rows(); ++i) {
        for (std::size_t j = 0; j < image.columns(); ++j) {
            if (!dets_nonmax_map(i, j) || !trace_nonmax_map(i, j)) {
                gil::view(input)(j, i) = gil::rgb8_pixel_t(0, 255, 0);
            }
        }
    }

    std::cout << "Gradient range: " << blaze::max(hessian_result.determinants) << ' '
              << blaze::min(hessian_result.determinants) << '\n'
              << "Final gray image range: " << static_cast<int>(blaze::max(image)) << ' '
              << static_cast<int>(blaze::min(image)) << '\n';
    gil::write_view(determinants_map_file, gil::view(determinants_map), gil::png_tag{});
    gil::write_view(traces_map_file, gil::view(traces_map), gil::png_tag{});
    gil::write_view(output_file, gil::view(input), gil::png_tag{});
}
