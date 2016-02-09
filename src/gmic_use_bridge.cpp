/*
 #
 #  File        : gmic_use_bridge.cpp
 #                ( C++ source file )
 #
 #  Description : Show how to call the C bridge to the G'MIC interpreter from a C source code.
 #
 #  Copyright   : Tobias Fleischer
 #                ( https://plus.google.com/u/0/b/117441237982283011318/+TobiasFleischer )
 #
 #  License     : CeCILL v2.0
 #                ( http://www.cecill.info/licences/Licence_CeCILL_V2-en.html )
 #
 #  This software is governed by the CeCILL  license under French law and
 #  abiding by the rules of distribution of free software.  You can  use,
 #  modify and/ or redistribute the software under the terms of the CeCILL
 #  license as circulated by CEA, CNRS and INRIA at the following URL
 #  "http://www.cecill.info".
 #
 #  As a counterpart to the access to the source code and  rights to copy,
 #  modify and redistribute granted by the license, users are provided only
 #  with a limited warranty  and the software's author,  the holder of the
 #  economic rights,  and the successive licensors  have only  limited
 #  liability.
 #
 #  In this respect, the user's attention is drawn to the risks associated
 #  with loading,  using,  modifying and/or developing or reproducing the
 #  software by the user in light of its specific status of free software,
 #  that may mean  that it is complicated to manipulate,  and  that  also
 #  therefore means  that it is reserved for developers  and  experienced
 #  professionals having in-depth computer knowledge. Users are therefore
 #  encouraged to load and test the software's suitability as regards their
 #  requirements in conditions enabling the security of their systems and/or
 #  data to be ensured and, more generally, to use and operate it in the
 #  same conditions as regards security.
 #
 #  The fact that you are presently reading this means that you have had
 #  knowledge of the CeCILL license and that you accept its terms.
 #
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gmic_bridge.h"
#include <math.h>

int main(int argc, char **argv) {
  gmic_bridge_image images[1];
  memset(&images, 0, sizeof(gmic_bridge_image));

  // we only use 1 input image
  unsigned int nofImages = 1;

  // set the name of the image (optional)
  strcpy(images[0].name, "test_input");

  // set the dimensions of the input image 0
  // (usually depth will be 1 and spectrum will be 4 for RGBA or 3 for RGB)
  images[0].width = 500;
  images[0].height = 500;
  images[0].spectrum = 4;
  images[0].depth = 1;

  // if "is_interleaved" is set to true, the input data buffer is supposed to consist
  // of interleaved color channels (RGBA RGBA RGBA RGBA ...)
  // if it is set to false, color channels are seen as separate planar buffers in memory
  // (RRRR... GGGG... BBBB... AAAA...), which is G'MIC's native format and there a bit faster
  images[0].is_interleaved = false;

  // if input is 32bpc float values, set this to E_FORMAT_FLOAT
  // if it is 8bpc char values, set this to E_FORMAT_BYTE
  images[0].format = E_FORMAT_FLOAT;

  // allocate memory for the input image
  float* inp = new float[images[0].width * images[0].height * images[0].spectrum * images[0].depth];
  // set pointer to this memory in the images structure
  images[0].data = inp;

  // now fill the input image
  // in this example, 3 vertical bars in red, green and blue are drawn
  float* ptr = inp;
  for (unsigned int c = 0; c < images[0].spectrum; ++c)
    for (unsigned int y = 0; y < images[0].height; ++y)
      for (unsigned int x = 0; x < images[0].width; ++x) {
        if (c == 0) {
          if (x <= images[0].width / 3)
            *(ptr++) = 1.f;
          else
            *(ptr++) = 0.f;
        } else if (c == 1) {
          if (x <= images[0].width * 2 / 3 && x > images[0].width / 3)
            *(ptr++) = 1.f;
          else
            *(ptr++) = 0.f;
        } else if (c == 2) {
          if (x <= images[0].width && x > images[0].width * 2 / 3)
            *(ptr++) = 1.f;
          else
            *(ptr++) = 0.f;
        } else {
          *(ptr++) = 1.f;
        }
      }

  // create options structure and initialize it
  gmic_bridge_options options;
  memset(&options, 0, sizeof(gmic_bridge_options));

  // if this is set to true, the G'MIC standard library won't be loaded
  // usually you want this library, so be sure to set it to false
  options.ignore_stdlib = false;

  // define abort and progress variables
  bool abort = false;
  float progress;
  options.p_is_abort = &abort;
  options.p_progress = &progress;

  // if this is set to true, the color channels of the output will be
  // interleaved, i.e. in the format RGBA RGBA RGBA... else they
  // will be in G'MIC's native non-interleaved/planar format
  // RRRR... GGGG... BBBB... AAAA...
  options.interleave_output = false;

  // if you want to prevent the source image buffer from being changed,
  // set this to true. If it is set to false, the input data
  // may be overwritten and set as the actual output data
  options.no_inplace_processing = true;

  // if the output should be 32bpc float values, set this to E_FORMAT_FLOAT
  // if it should be 8bpc char values, set this to E_FORMAT_BYTE
  options.output_format = E_FORMAT_FLOAT;

  // and here is the actual call to the G'MIC library!
  // in this example, it will get the input buffer we created, divide only the red channel by 2
  // and then display the result
  gmic_call("-v 0 -apply_channels \"-div 2\",rgba_r -mul 255 -polaroid 5,30 -rotate 20 -drop_shadow , -display", &nofImages, &images[0], &options);

  // we have to dispose output images we got back from the gmic_call that were
  // not created by this thread
  // therefore, for any image data we did not allocate ourselves, we have to call the
  // external delete function
  for (int i = 0; i < nofImages; i++) {
    if (images[i].data != inp) {
      gmic_delete_external((float*)images[i].data);
    }
  }

  // and finally we free the memory we allocated for our input image
  delete[] inp;

  return 0;
}
