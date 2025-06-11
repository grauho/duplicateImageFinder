#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

#include "portopt.h"

#include "thirdparty/stb_image.h"
#include "thirdparty/stb_image_resize.h"
#include "thirdparty/stb_image_write.h" /* UNUSED */

#define HASH_WIDTH  (8)
#define HASH_HEIGHT (8)
#define HASH_LENGTH (HASH_WIDTH * HASH_HEIGHT)

PORTOPT_BOOL verbose = PORTOPT_FALSE;

static int scaleImage(unsigned char * const src_data, 
	const size_t src_width, const size_t src_height, 
	unsigned char * const dst_data, const size_t dst_width, 
	const size_t dst_height)
{
	size_t i;

	if ((src_data == NULL) || (dst_data == NULL))
	{
		fprintf(stderr, "Bad arguments to scaleImage\n");

		return -1;
	}

	if (stbir_resize_uint8_generic(src_data, src_width, src_height, 0,
		dst_data, dst_width, dst_height, 0, 1, 
		STBIR_ALPHA_CHANNEL_NONE, 0, STBIR_EDGE_WRAP, 
		STBIR_FILTER_TRIANGLE, STBIR_COLORSPACE_LINEAR, NULL) == 0)
	{
		fprintf(stderr, "Failed to rescale image\n");

		goto BAIL_OUT;
	}

	/* Flatten down the grayscale even more to heighten the constrast */
	for (i = 0; i < HASH_LENGTH; i++)
	{
		dst_data[i] = (dst_data[i] * 64) / UCHAR_MAX;
	}

	free(src_data);

	return 0;

BAIL_OUT:

	if (src_data != NULL)
	{
		free(src_data);
	}

	return -1;

}

int readImageFile(const char * const in_path, const size_t dst_width,
	const size_t dst_height, unsigned char *output)
{
	int src_width;
	int src_height;
	int dummy;
	unsigned char *src_data = NULL;
	unsigned char *dst_data = output;

	if (dst_data == NULL)
	{
		fprintf(stderr, "Must supply valid ALLOCATED output buffer\n");

		return -1;
	}

	/* loads as grayscale */
	if ((src_data = stbi_load(in_path, &src_width, &src_height, &dummy, 1))
		== NULL)
	{
		fprintf(stderr, "Failed load: '%s'\n", in_path);
		free(src_data);

		return -1;
	}

	return scaleImage(src_data, src_width, src_height, dst_data, dst_width, 
		dst_height);
}

static unsigned char getGrayscaleMean(const unsigned char * const data,
	const unsigned int width, const unsigned int height)
{
	uint64_t mean = 0;
	size_t i;

	for (i = 0; i < HASH_LENGTH; i++)
	{
		mean += data[i];
	}

	return (unsigned char) (mean / HASH_LENGTH);
}

static uint64_t getFingerprint(const unsigned char * const data)
{
	unsigned char mean = getGrayscaleMean(data, HASH_WIDTH, HASH_HEIGHT);
	uint64_t fingerprint = 0;
	size_t i;
	size_t j = 0;

	for (i = 0; i < HASH_LENGTH; i++)
	{
		/* casting here is important as otherwise it tries to use an
		 * int for the mask */
		fingerprint |= (((uint64_t) ((data[i] > mean) ? 1 : 0)) << i);

		if (verbose)
		{
			if (data[i] > mean)
			{
				fputc('#', stdout);
			}
			else
			{
				fputc('.', stdout);
			}

			if ((j = ((j + 1) % 8)) == 0)
			{
				fputc('\n', stdout);
			}
		}
	}

	if (verbose)
	{
		fprintf(stdout, "%lu\n%lx\n", fingerprint, fingerprint);
		fputc('\n', stdout);
	}

	return fingerprint;
}

static unsigned char calculateHamming(const uint64_t foo, const uint64_t bar)
{
	const uint64_t diff = foo ^ bar;
	unsigned char score = 0;
	size_t i;

	for (i = 0; i < HASH_LENGTH; i++)
	{
		if (diff & (((uint64_t) 1) << i))
		{
			score++;
		}
	}

	return score;
}

static uint64_t fingerprintFile(const char * const path)
{
	unsigned char img_data[HASH_LENGTH];

	if (path == NULL)
	{
		return 0;
	}

	if (readImageFile(path, HASH_WIDTH, HASH_HEIGHT, img_data) != 0)
	{
		fprintf(stderr, "Failed to load image data\n");

		return 0;
	}

	return getFingerprint(img_data);
}

static void printHelp(void)
{
	fputs("Image Comparison Program\n\n", stdout);
	fputs("Usage:\n", stdout);
	fputs("\t./a.out [flags]... [images]...\n\n", stdout);
	fputs("Command Line Flags:\n", stdout);
	fputs("\t-t, --threshold <NUM> : Similarity limit, default 10\n", 
		stdout);
	fputs("\t-o, --output <PATH>   : Path to output file\n", stdout);
	fputs("\t-v, --verbose         : Enables extra information output\n",
		stdout);
	fputs("\t-h, --help            : Prints this message and exits\n",
		stdout);
	fputs("\n", stdout);
}

struct entry 
{
	uint64_t print;
	const char *path;
};

int main(int argc, char **argv)
{
	const struct portoptVerboseOpt opts[] =
	{
		{'t', "threshold", PORTOPT_TRUE},
		{'o', "output",    PORTOPT_TRUE},
		{'v', "verbose",   PORTOPT_FALSE},
		{'h', "help",      PORTOPT_FALSE}
	};
	const size_t num_opts = sizeof(opts) / sizeof(opts[0]);
	const size_t argl = (size_t) argc;
	size_t ind = 0;
	int flag;

	unsigned char similar_threshold = 10;
	FILE *output = NULL;

	struct entry *entry_arr = NULL;
	int ret = 0;
	size_t i;

	while ((flag = portoptVerbose(argl, argv, opts, num_opts, &ind)) != -1)
	{
		switch (flag)
		{
			case 't':
				similar_threshold = atol(
					portoptGetArg(argl, argv, &ind));

				break;
			case 'o':
				if ((output = fopen(portoptGetArg(argl, argv, 
					&ind), "wb")) == NULL)
				{
					fputs("Failed to open output file\n",
						stderr);

					goto CLEANUP;
				}

				break;
			case 'v':
				verbose = PORTOPT_TRUE;

				break;
			case 'h':
				printHelp();

				goto CLEANUP;
			case '?': /* fallthrough */
			default:
				break;
		}
	}

	ind += (ind == 0);

	if (argl - ind < 2)
	{
		printHelp();

		goto CLEANUP;
	}

	if ((entry_arr = malloc(sizeof(struct entry) * (argl - ind))) == NULL)
	{
		fprintf(stderr, "Allocation failure\n");
		ret = 1;

		goto CLEANUP;
	}

	/* Would probably be faster to load the entire thing and then do any
	 * comparisons required */
	for (i = 0; ind < argl; i++, ind++)
	{
		size_t j;

		entry_arr[i].print = fingerprintFile(argv[ind]);
		entry_arr[i].path  = argv[ind];

		for (j = 0; j < i; j++)
		{
			const unsigned char ham_score = calculateHamming(
				entry_arr[j].print, entry_arr[i].print);

			if (ham_score < similar_threshold)
			{
				if (verbose)
				{
					fprintf(stdout, "score: %d\n", 
						ham_score);
				}

				fprintf(stdout, "'%s' == '%s'\n",
					entry_arr[j].path,
					entry_arr[i].path);

				if (output != NULL)
				{
					fprintf(output, "%s %s\n",
						entry_arr[j].path,
						entry_arr[i].path);
				}
			}
		}
	}

CLEANUP:

	if (entry_arr != NULL)
	{
		free(entry_arr);
	}

	if (output != NULL)
	{
		fclose(output);
	}

	return ret;
}

