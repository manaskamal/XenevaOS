#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"
#include "svgdec.h"
#include <stdlib.h>

int ns_svg_decode(char* data, size_t len, unsigned** out_px, int* out_w, int* out_h) {
	NSVGimage* image;
	NSVGrasterizer* rast;
	unsigned char* rgba;
	unsigned* px;
	float scale;
	int w, h, i, n;
	*out_px = 0;
	if (!data || !len || len > 2u * 1024u * 1024u)
		return -1;
	data[len] = 0;
	image = nsvgParse(data, "px", 96.0f);
	if (!image || image->width < 1.0f || image->height < 1.0f) {
		nsvgDelete(image);
		return -1;
	}
	scale = 480.0f / image->width;
	if (scale > 1.0f)
		scale = 1.0f;
	w = (int)(image->width * scale);
	h = (int)(image->height * scale);
	if (w < 1)
		w = 1;
	if (h < 1)
		h = 1;
	if (h > 280) {
		scale = 280.0f / image->height;
		w = (int)(image->width * scale);
		h = 280;
		if (w < 1)
			w = 1;
	}
	rgba = (unsigned char*)malloc((size_t)w * (size_t)h * 4);
	rast = nsvgCreateRasterizer();
	if (!rgba || !rast) {
		free(rgba);
		nsvgDeleteRasterizer(rast);
		nsvgDelete(image);
		return -1;
	}
	nsvgRasterize(rast, image, 0, 0, scale, rgba, w, h, w * 4);
	n = w * h;
	px = (unsigned*)malloc((size_t)n * 4);
	if (!px) {
		free(rgba);
		nsvgDeleteRasterizer(rast);
		nsvgDelete(image);
		return -1;
	}
	for (i = 0; i < n; i++) {
		unsigned char* p = rgba + i * 4;
		px[i] = ((unsigned)p[3] << 24) | ((unsigned)p[0] << 16) | ((unsigned)p[1] << 8) | p[2];
	}
	free(rgba);
	nsvgDeleteRasterizer(rast);
	nsvgDelete(image);
	*out_px = px;
	*out_w = w;
	*out_h = h;
	return 0;
}
