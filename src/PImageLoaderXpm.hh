//
// PImageLoaderXpm.hh for pekwm
// Copyright (C) 2005-2021 Claes Nästén <pekdon@gmail.com>
//
// This program is licensed under the GNU GPL.
// See the LICENSE file for more information.
//

#ifndef _PEKWM_PIMAGELOADERXPM_HH_
#define _PEKWM_PIMAGELOADERXPM_HH_

#include "config.h"

#ifdef PEKWM_HAVE_IMAGE_XPM

#include "pekwm.hh"
#include "Types.hh"

extern "C" {
#include <X11/xpm.h>
}

/**
 * Xpm Loader class.
 */
namespace PImageLoaderXpm
{
	const char *getExt(void);
	uchar* load(const std::string &file, size_t &width, size_t &height,
		    bool &use_alpha);
}

#endif // PEKWM_HAVE_IMAGE_XPM

#endif // _PEKWM_PIMAGELOADERXPM_HH_
