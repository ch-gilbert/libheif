/*
  libheif example application "heif-scale".

  MIT License

  Copyright (c) 2017 struktur AG, Dirk Farin <farin@struktur.de>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <errno.h>
#include <string.h>
#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#else
#define STDOUT_FILENO 1
#endif

#include <libheif/heif_cxx.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <getopt.h>
#include <assert.h>
#include <math.h>

static struct option long_options[] = {
  {"width", required_argument,      0, 'W' },
  {"height",   required_argument,      0, 'H' },
  {0,         0,                 0,  0 }
};

void show_help(const char* argv0)
{
    fprintf(stderr," heif-scale  libheif version: %s\n",heif_get_version());
    fprintf(stderr,"------------------------------------\n");
    fprintf(stderr,"usage: heif-scale [options] image.heic output.heic\n");
    fprintf(stderr,"\n");
    fprintf(stderr,"options:\n");
    fprintf(stderr,"  -W, --width width  scale image width\n");
    fprintf(stderr,"  -H, --height height  scale image height\n");
    fprintf(stderr,"  -h, --help           show help\n");
}


int main(int argc, char** argv)
{
  int scale_width = 0;
  int scale_height = 0;

  while (true) {
    int option_index = 0;
    int c = getopt_long(argc, argv, "W:H:h", long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {
    case 'W':
      scale_width = atoi(optarg);
      break;
    case 'H':
      scale_height = atoi(optarg);
      break;
    case 'h':
      show_help(argv[0]);
      return 0;
    }
  }

  if (optind + 2 > argc) {
    // Need input and output filenames as additional arguments.
    show_help(argv[0]);
    return 0;
  }

  const char* input_filename(argv[optind++]);
  const char* output_filename(argv[optind++]);

  // ==============================================================================

  try {
    heif::Context ctx;
    ctx.read_from_file(input_filename);

    heif::ImageHandle handle = ctx.get_primary_image_handle();
    heif::Image img = handle.decode_image(heif_colorspace_undefined, heif_chroma_undefined);
    int input_width = handle.get_width();
    int input_height = handle.get_height();

    int thumbnail_width = input_width;
    int thumbnail_height = input_height;

    double width_ratio = 1.0 * scale_width / input_width;
    double height_ratio = 1.0 * scale_height / input_height;

    double ratio = fmin(width_ratio, height_ratio);
    if (ratio < 1.0) {
      thumbnail_width = (int) floor(input_width * ratio);
      thumbnail_height = (int) floor(input_height * ratio);
    }

    heif::Image scaled_img = img.scale_image(thumbnail_width, thumbnail_height);

    heif::Encoder encoder(heif_compression_HEVC);
    heif::Context output_ctx;
    output_ctx.encode_image(scaled_img, encoder);
    output_ctx.write_to_file(output_filename);
  }
  catch (heif::Error err) {
    std::cerr << err.get_message() << "\n";
  }

  return 0;
}
