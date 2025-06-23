#ifndef DIF_IMAGE_HANDLING_H
#define DIF_IMAGE_HANDLING_H

void initializeImageHandling(const char * const program);
void cleanupImageHandling(void);
int readImageFile(const char * const in_path, const size_t dst_width,
	const size_t dst_height, unsigned char *output);

#endif /* DIF_IMAGE_HANDLING_H */

