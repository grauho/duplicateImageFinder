#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

#ifdef DIF_USE_IMAGEMAGICK
#include <MagickCore/MagickCore.h>
#else
#include "thirdparty/stb_image.h"
#include "thirdparty/stb_image_resize.h"
#include "thirdparty/stb_image_write.h" /* UNUSED */
#endif /* !DIF_USE_IMAGEMAGICK */

#define DIF_CHECKED_FUNC(ptr, func) \
do                                  \
{                                   \
	if ((ptr) != NULL)          \
	{                           \
		func((ptr));        \
	}                           \
} while (0)

#define DIF_CHECKED_FREE(ptr) DIF_CHECKED_FUNC((ptr), free)

void initializeImageHandling(const char * const program)
{
#ifdef DIF_USE_IMAGEMAGICK
	MagickCoreGenesis(program, MagickTrue);
#else
	(void) program;
#endif /* !DIF_USE_IMAGEMAGICK */
}

void cleanupImageHandling(void)
{
#ifdef DIF_USE_IMAGEMAGICK
	MagickCoreTerminus();
#endif /* DIF_USE_IMAGEMAGICK */
}

#ifdef DIF_USE_IMAGEMAGICK
static int magickLoadAndScaleImage(const char * const path, 
	const unsigned int width, const unsigned int height, 
	unsigned char * const out)
{
	CacheView *cache = NULL;
	ExceptionInfo *exception = NULL;
	ImageInfo *image_info = NULL;
	Image *src_image = NULL;
	Image *dst_image = NULL;
	const Quantum *pixels;
	int ret = 0;
	const size_t lim = width * height;
	size_t i;

	if ((path == NULL) || (out == NULL))
	{
		ret = -1;

		goto CLEANUP;
	}

	image_info = CloneImageInfo(NULL);
	exception = AcquireExceptionInfo();

	/* The handling on failure for all of these is the exact same */
	if (((src_image = ReadImages(image_info, path, exception)) == NULL)
	|| (TransformImageColorspace(src_image, GRAYColorspace, exception) 
		!= MagickTrue)
	|| (SetImageAlphaChannel(src_image, OffAlphaChannel, exception) 
		!= MagickTrue)
	|| ((dst_image = ResizeImage(src_image, width, height, TriangleFilter,
		exception)) == NULL)
	|| ((cache = AcquireVirtualCacheView(dst_image, exception)) == NULL)
	|| ((pixels = GetCacheViewVirtualPixels(cache, 0, 0, width, height,
		exception)) == NULL))
	{
		ret = -1;
		MagickError(exception->severity, exception->reason,
			exception->description);

		goto CLEANUP;
	}

	for (i = 0; i < lim; i++)
	{
		out[i] = (pixels[i] * UCHAR_MAX) / QuantumRange;
	}

CLEANUP:
	
	DIF_CHECKED_FUNC(cache, DestroyCacheView);
	DIF_CHECKED_FUNC(exception, DestroyExceptionInfo);
	DIF_CHECKED_FUNC(image_info, DestroyImageInfo);
	DIF_CHECKED_FUNC(src_image, DestroyImage);
	DIF_CHECKED_FUNC(dst_image, DestroyImage);

	return ret;
}

#else

static int scaleImage(unsigned char * const src_data, 
	const size_t src_width, const size_t src_height, 
	unsigned char * const dst_data, const size_t dst_width, 
	const size_t dst_height)
{
	if ((src_data == NULL) || (dst_data == NULL))
	{
		fputs("Bad arguments to scaleImage\n", stderr);

		return -1;
	}

	if (stbir_resize_uint8_generic(src_data, src_width, src_height, 0,
		dst_data, dst_width, dst_height, 0, 1, 
		STBIR_ALPHA_CHANNEL_NONE, 0, STBIR_EDGE_WRAP, 
		STBIR_FILTER_TRIANGLE, STBIR_COLORSPACE_LINEAR, NULL) == 0)
	{
		fputs("Failed to rescale image\n", stderr);

		goto BAIL_OUT;
	}

	free(src_data);

	return 0;

BAIL_OUT:

	DIF_CHECKED_FREE(src_data);

	return -1;

}

#endif /* !DIF_USE_IMAGEMAGICK */

int readImageFile(const char * const in_path, const size_t dst_width,
	const size_t dst_height, unsigned char *output)
{
#ifndef DIF_USE_IMAGEMAGICK
	int src_width;
	int src_height;
	int dummy;
	unsigned char *src_data = NULL;

	if ((output == NULL)
	|| (src_data = stbi_load(in_path, &src_width, &src_height, &dummy, 1))
		== NULL)
	{
		fprintf(stderr, "Failed load: '%s'\n", in_path);

		return -1;
	}

	return scaleImage(src_data, src_width, src_height, output, dst_width, 
		dst_height);
#else
	return magickLoadAndScaleImage(in_path, dst_width, dst_height, output);
#endif /* DIF_USE_IMAGEMAGICK */
}

