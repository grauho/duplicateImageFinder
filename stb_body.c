#include <stdio.h>
#include <stdlib.h>

#ifndef DIF_USE_IMAGEMAGICK

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT(x)
#include "thirdparty/stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STBIR_ASSERT(x)
/* The only function of STBIR_DEBUG is to turn on the debug assertions, those
 * are aliased to whatever is defined for STBIR_ASSERT which is due to the 
 * above is nothing. */
#define STBIR_DEBUG 
#include "thirdparty/stb_image_resize.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBIW_ASSERT(x)
#include "thirdparty/stb_image_write.h"

#endif /* !DIF_USE_IMAGEMAGICK */
