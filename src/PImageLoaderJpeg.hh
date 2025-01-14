//
// PImageLoaderJpeg.hh for pekwm
// Copyright (C) 2005-2021 Claes Nästén <pekdon@gmail.com>
//
// This program is licensed under the GNU GPL.
// See the LICENSE file for more information.
//

#ifndef _PEKWM_PIMAGELOADERJPEG_HH_
#define _PEKWM_PIMAGELOADERJPEG_HH_

#include "config.h"

#ifdef PEKWM_HAVE_IMAGE_JPEG

#include "pekwm.hh"

#include <cstdio>

/**
 * Jpeg Loader class.
 */
namespace PImageLoaderJpeg
{
	const char *getExt(void);
	uchar* load(const std::string &file, size_t &width, size_t &height,
		    bool &use_alpha);
}

#endif // PEKWM_HAVE_IMAGE_JPEG

#endif // _PEKWM_PIMAGELOADERJPEG_HH_
