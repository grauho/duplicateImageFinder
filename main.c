/* Duplicate Image Finder, A program to detect duplicate or similar image using
 * a hamming distance */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> /* uint64_t */
#include <limits.h>

#include "portopt.h"

#ifndef DIF_DISABLE_THREADING
#include "thirdparty/macroThreadPool.h"
#endif
#include "imageHandling.h"

#define DIF_WIDTH  (8)
#define DIF_HEIGHT (8)
#define DIF_LENGTH (DIF_WIDTH * DIF_HEIGHT)

struct entry 
{
	uint64_t print;
	const char *path;
	unsigned char density;
};

static void fingerprintFile(struct entry * const node);
static unsigned char calculateHamming(const uint64_t foo, const uint64_t bar);

int compareDensities(const void * const l_ptr, const void * const r_ptr)
{
	const struct entry * const left = (struct entry * const) l_ptr;
	const struct entry * const right = (struct entry * const) r_ptr;

	return (left->density - right->density);
}

/* Cast to character pointer to avoid arithmetic on void pointers */
#define ACCESS_VAR(arr, index, size) (((char *) (arr)) + ((index) * (size)))

/* returns len on failed to find */
static size_t leftBinSearch(void *arr, const size_t len, const void *target,
	const size_t size, int (*CompFunc)(const void *, const void *))
{
	size_t left = 0;
	size_t right = len;

	while (left < right)
	{
		size_t mid = ((left + right) >> 1);

		if (CompFunc(ACCESS_VAR(arr, mid, size), target) < 0)
		{
			left = mid + 1;
		}
		else
		{
			right = mid;
		}
	}

	return left;
}

/* The idea here is just to make it so that the comparisons don't always have
 * to start at the beginning of the entry array instead they can start at the
 * density - threshold point found via using a binary search. This is because
 * definitionally a fingerprint differing by more than threshold bit density
 * will not have a hamming distance in the allowable range. The open question
 * is does this optimization warrent the time spent sorting the array */

/* It's possible that the threads could insertion sort the array while loading
 * the entries but that would require mutex locking a shared destination 
 * array. Furthermore, this isn't currently the performance bottleneck. */
static void doComparison(struct entry * const src, const size_t len, 
	const unsigned char threshold, FILE *output)
{
	size_t i, j;

	qsort(src, len, sizeof(struct entry), compareDensities);

	for (i = 0; i < len; i++)
	{
		const unsigned char target_density 
			= (src[i].density < threshold) 
				? 0 : src[i].density - threshold;
		const struct entry dummy = {0, 0, target_density};
		const size_t fnd = leftBinSearch(src, len, &dummy, 
			sizeof(struct entry), compareDensities);

		for (j = fnd; j < i; j++)
		{
			const unsigned char score = calculateHamming(
				src[i].print, src[j].print);

			if ((score <= threshold) && (j != i))
			{
				fprintf(stdout, "\"%s\" \"%s\"\n",
					src[i].path, src[j].path);

				if (output != NULL)
				{
					fprintf(output, "\"%s\" \"%s\"\n",
						src[i].path, src[j].path);
				}
			}
		}
	}
}

#ifndef DIF_DISABLE_THREADING

static void threadFunction(struct entry *node);

MACRO_THREAD_POOL_COMPLETE(loader, struct entry *, threadFunction);

static void threadFunction(struct entry *node)
{
	if (node != NULL)
	{
		fingerprintFile(node);
	}
}

#endif /* !DIF_DISABLE_THREADING */

PORTOPT_BOOL verbose = PORTOPT_FALSE;

static unsigned char getGrayscaleMean(const unsigned char * const data,
	const unsigned int width, const unsigned int height)
{
	const size_t lim = width * height;
	uint64_t mean = 0;
	size_t i;

	for (i = 0; i < lim; i++)
	{
		mean += data[i];
	}

	return (unsigned char) (mean / lim);
}

static void getFingerprintWithDensity(const unsigned char * const data,
	struct entry * const out)
{
	unsigned char mean = getGrayscaleMean(data, DIF_WIDTH, DIF_HEIGHT);
	size_t i, j = 0;

	out->density = 0;
	out->print = 0;

	for (i = 0; i < DIF_LENGTH; i++)
	{
		/* casting here is important as otherwise it tries to use an
		 * int for the mask */
		if (data[i] > mean)
		{
			out->print |= (((uint64_t) 1) << i);
			out->density++;
		}

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
}

static unsigned char calculateHamming(const uint64_t foo, const uint64_t bar)
{
	const uint64_t diff = foo ^ bar;
	unsigned char score = 0;
	size_t i;

	for (i = 0; i < DIF_LENGTH; i++)
	{
		if (diff & (((uint64_t) 1) << i))
		{
			score++;
		}
	}

	return score;
}

static void fingerprintFile(struct entry * const node)
{
	unsigned char img_data[DIF_LENGTH];

	if (node == NULL) 
	{
		return;
	}

	node->print = 0;
	node->density = 0;

	if ((node->path == NULL)
	|| (readImageFile(node->path, DIF_WIDTH, DIF_HEIGHT, img_data) != 0))
	{
		return;
	}

	getFingerprintWithDensity(img_data, node);
}

static void printHelp(void)
{
	fputs("Image Comparison Program\n\n", stderr);
	fputs("Usage:\n", stderr);
	fputs("\t./difDemo [flags]... [images]...\n\n", stderr);
	fputs("Command Line Flags:\n", stderr);
	fputs("\t-t, --threshold <NUM> : Similarity limit, default 5\n", 
		stderr);
	fputs("\t-T, --threads   <NUM> : Thread max, if built, default 5\n", 
		stderr);
	fputs("\t-o, --output <PATH>   : Path to output file\n", stderr);
	fputs("\t-v, --verbose         : Enables extra information output\n",
		stderr);
	fputs("\t-h, --help            : Prints this message and exits\n",
		stderr);
	fputs("\n", stderr);
}

int main(int argc, char **argv)
{
	const struct portoptVerboseOpt opts[] =
	{
		{'t', "threshold", PORTOPT_TRUE},
		{'o', "output",    PORTOPT_TRUE},
		{'T', "threads",   PORTOPT_TRUE},
		{'v', "verbose",   PORTOPT_FALSE},
		{'h', "help",      PORTOPT_FALSE}
	};
	const size_t num_opts = sizeof(opts) / sizeof(opts[0]);
	const size_t argl = (size_t) argc;
	size_t ind = 0;
	int flag;

	unsigned char similar_threshold = 5;
	FILE *output = NULL;
#ifndef DIF_DISABLE_THREADING
	unsigned char num_threads = 5;
	struct loaderThreadPool *pool = NULL;
#endif /* !DIF_DISABLE_THREADING */

	struct entry *entry_arr = NULL;
	int ret = 0;
	size_t lim, i;

	initializeImageHandling(argv[0]);

	while ((flag = portoptVerbose(argl, argv, opts, num_opts, &ind)) != -1)
	{
		switch (flag)
		{
			case 't':
				similar_threshold = atol(
					portoptGetArg(argl, argv, &ind));

				break;
			case 'T':
#ifndef DIF_DISABLE_THREADING
				num_threads = atol(
					portoptGetArg(argl, argv, &ind));
#else
				fputs("Not built with threading support",
					stderr);
#endif /* DIF_DISABLE_THREADING */

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

#ifndef DIF_DISABLE_THREADING
	if ((pool = loaderNewThreadPool(num_threads, num_threads)) == NULL)
	{
		fputs("Failed to initialize thread pool\n", stderr);
		ret = 1;

		goto CLEANUP;
	}
#endif /* !DIF_DISABLE_THREADING */

	lim = argl - ind;

	if ((entry_arr = malloc(sizeof(struct entry) * lim)) == NULL)
	{
		fputs("Allocation failure\n", stderr);
		ret = 1;

		goto CLEANUP;
	}

#ifndef DIF_DISABLE_THREADING
	for (i = 0; (i < lim) && (ind < argl); i++, ind++)
	{
		entry_arr[i].path = argv[ind];
		loaderEnqueueJob(pool, &entry_arr[i]);
	}

	loaderWaitOnIdle(pool);
#else
	for (i = 0; (i < lim) && (ind < argl); i++, ind++)
	{
		entry_arr[i].path = argv[ind];
		fingerprintFile(&entry_arr[i]);
	}
#endif /* DIF_DISABLE_THREADING */

	fputs("loading complete\n", stderr);
	doComparison(entry_arr, lim, similar_threshold, output);

CLEANUP:

#ifndef DIF_DISABLE_THREADING
	loaderCleanupThreadPool(pool);
#endif /* !DIF_DISABLE_THREADING */

	cleanupImageHandling();

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

