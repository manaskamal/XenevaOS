/**
* BSD 2-Clause License
* 
* @file alpha.cpp
*
* Copyright (c) 2022-2026, Manas Kamal Choudhury
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice, this
*    list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
**/

#include "deodxr.h"
#include "alpha.h"
#include <arm_neon.h>
#include <math.h>
#include "window.h"
#include <stdlib.h>

static const uint8_t alpha_shuffle[16] = {
	3,3, 3, 3,
	7,7,7,7,
	11,11,11,11,
	15,15,15,15
};

void __pixel_blend_neon(uint32_t* dst, const uint32_t* src, int width) {
	int x = 0;
	uint8x16_t shuf = vld1q_u8(alpha_shuffle);
	for (; x <= width - 4; x += 4) {
		/** Load 4 src and 4 dst pixels **/
		uint8x16_t s = vld1q_u8((uint8_t*)(src + x));
		uint8x16_t d = vld1q_u8((uint8_t*)(dst + x));

		uint8x16_t sa = vqtbl1q_u8(s, shuf);
		uint8x16_t inv = vsubq_u8(vdupq_n_u8(255), sa);

		uint16x8_t lo = vmlal_u8(vmull_u8(vget_low_u8(s), vget_low_u8(sa)),
			vget_low_u8(d), vget_low_u8(inv));
		uint16x8_t hi = vmlal_u8(vmull_u8(vget_high_u8(s), vget_high_u8(sa)),
			vget_high_u8(d), vget_high_u8(inv));

		uint8x16_t result = vcombine_u8(vshrn_n_u16(lo, 8), vshrn_n_u16(hi, 8));
		vst1q_u8((uint8_t*)(dst + x), result);

		uint32x4_t src4 = vld1q_u32(src + x);
		uint32x4_t alpha_mask = vshrq_n_u32(src4, 24);
		uint64_t all_opaque = vgetq_lane_u64(vreinterpretq_u64_u32(vceqq_u32(alpha_mask, vdupq_n_u32(255))), 0);

		if (all_opaque == 0xffffffffffffffff) {
			vst1q_u32(dst + x, src4);
			continue;
		}

		for (int i = x; i < x + 4 && i < width; i++) {
			uint32_t sp = src[i], dp = dst[i];
			uint8_t sa = sp >> 24;
			if (sa == 255) { dst[i] = sp; continue; }
			if (sa == 0) { continue; }
			uint32_t inv = 255 - sa;
			uint8_t r = ((uint32_t)((sp >> 16) & 0xFF) * sa + (uint32_t)((dp >> 16) & 0xFF) * inv) >> 8;
			uint8_t g = ((uint32_t)((sp >> 8) & 0xFF) * sa + (uint32_t)((dp >> 8) & 0xFF) * inv) >> 8;
			uint8_t b = ((uint32_t)((sp) & 0xFF) * sa + (uint32_t)((dp) & 0xFF) * inv) >> 8;
			dst[i] = (0xFF << 24) | (r << 16) | (g << 8) | b;
		}
	}

	// scaler tail
	for (; x < width; x++) {
		uint32_t sp = src[x], dp = dst[x];
		uint8_t sa = sp >> 24;
		if (sa == 255) { dst[x] = sp; }
		else if (sa > 0) {
			uint32_t inv = 255 - sa;
			uint8_t r = ((uint32_t)((sp >> 16) & 0xFF) * sa + (uint32_t)((dp >> 16) & 0xFF) * inv) >> 8;
			uint8_t g = ((uint32_t)((sp >> 8) & 0xFF) * sa + (uint32_t)((dp >> 8) & 0xFF) * inv) >> 8;
			uint8_t b = ((uint32_t)((sp) & 0xFF) * sa + (uint32_t)((dp) & 0xFF) * inv) >> 8;
			dst[x] = (0xFF << 24) | (r << 16) | (g << 8) | b;

		}
	}
}

#define GLASS_BLUR_RADIUS 6

static void __blur_pass_horizontal(uint32_t* tmp, const uint32_t* src, int src_w, int src_h, int rx,
	int ry, int rw, int r_h, int radius) {
	for (int row = 0; row < r_h; row++) {
		uint16_t sum_r = 0, sum_g = 0, sum_b = 0;
		int diameter = 2 * radius + 1;

		for (int k = -radius; k <= radius; k++) {
			int sx = rx + k;
			if (sx < 0) sx = 0;
			if (sx >= src_w) sx = src_w - 1;
			uint32_t px = src[(ry + row) * src_w + sx];
			sum_r += (px >> 16) & 0xFF;
			sum_g += (px >> 8) & 0xFF;
			sum_b += (px >> 0) & 0xFF;
		}
		for (int col = 0; col < rw; col++) {
			tmp[row * rw + col] = (0xFFu << 24) |
				((uint32_t)(sum_r / diameter) << 16)
				| ((uint32_t)(sum_g / diameter) << 8)
				| (uint32_t)(sum_b / diameter);

			int remove_x = rx + col - radius;
			int add_x = rx + col + radius + 1;
			if (remove_x < 0) remove_x = 0;
			if (remove_x >= src_w) remove_x = src_w - 1;
			if (add_x < 0) add_x = 0;
			if (add_x >= src_w) add_x = src_w - 1;
			uint32_t rem = src[(ry + row) * src_w + remove_x];
			uint32_t add = src[(ry + row) * src_w + add_x];
			sum_r += ((add >> 16) & 0xFF) - ((rem >> 16) & 0xFF);
			sum_g += ((add >> 8) & 0xFF) - ((rem >> 8) & 0xFF);
			sum_b += ((add >> 0) & 0xFF) - ((rem >> 0) & 0xFF);
		}
	}
}

static void blur_pass_vertical_neon(uint32_t* out, const uint32_t* tmp, int rw, int rh, int radius) {
	int diameter = 2 * radius + 1;

	for (int col = 0; col < rw; col++) {
		uint16_t sum_r = 0, sum_g = 0, sum_b = 0;

		for (int k = -radius; k <= radius; k++) {
			int sr = k;
			if (sr < 0) sr = 0;
			if (sr >= rh) sr = rh - 1;
			uint32_t px = tmp[sr * rw + col];
			sum_r += (px >> 16) & 0xFF;
			sum_g += (px >> 8) & 0xFF;
			sum_b += (px >> 0) & 0xFF;

		}

		for (int row = 0; row < rh; row++) {
			out[row * rw + col] = (0xFFu << 24) |
				((uint32_t)(sum_r / diameter) << 16)
				| ((uint32_t)(sum_g / diameter) << 8)
				| (uint32_t)(sum_b / diameter);

			int remove_y = row - radius;
			int add_y = row + radius + 1;
			if (remove_y < 0) remove_y = 0;
			if (remove_y >= rh) remove_y = rh - 1;
			if (add_y < 0) add_y = 0;
			if (add_y >= rh) add_y = rh - 1;

			uint32_t rem = tmp[remove_y * rw + col];
			uint32_t add = tmp[add_y * rw + col];
			sum_r += ((add >> 16) & 0xFF) - ((rem >> 16) & 0xFF);
			sum_g += ((add >> 8) & 0xFF) - ((rem >> 8) & 0xFF);
			sum_b += ((add >> 0) & 0xFF) - ((rem >> 0) & 0xFF);

		}
	}
}

void glass_precompute_blur(uint32_t* out_blur, uint32_t* tmp, const uint32_t* canvas, int canvas_w, int canvas_h,
	int rx, int ry, int rw, int rh, int radius) {
	__blur_pass_horizontal(tmp, canvas, canvas_w, canvas_h, rx, ry, rw, rh, radius);
	blur_pass_vertical_neon(out_blur, tmp, rw,rh, radius);
}

void _blend_scanline_glass_neon(uint32_t* canvas_row, const uint32_t* win_row, const uint32_t* blur_row, int width) {
	uint8x16_t shuf = vld1q_u8(alpha_shuffle);
	int x = 0;

	for (; x <= width - 4; x += 4) {
		uint8x16_t s = vld1q_u8((const uint8_t*)(win_row + x));
		uint8x16_t b = vld1q_u8((const uint8_t*)(blur_row + x));
		uint8x16_t d = vld1q_u8((const uint8_t*)(canvas_row + x));


		uint8x16_t sa = vqtbl1q_u8(s, shuf);
		uint8x16_t inv = vsubq_u8(vdupq_n_u8(255), sa);

		uint16x8_t lo = vmlal_u8(vmull_u8(vget_low_u8(s), vget_low_u8(sa)),
			vget_low_u8(b), vget_low_u8(inv));
		uint16x8_t hi = vmlal_u8(vmull_u8(vget_high_u8(s), vget_high_u8(sa)),
			vget_high_u8(b), vget_high_u8(inv));

		uint8x16_t blended = vcombine_u8(vshrn_n_u16(lo, 8), vshrn_n_u16(hi, 8));

		uint8x16_t zero = vdupq_n_u8(0);
		uint8x16_t is_zero = vceqq_u8(sa, zero);
		uint8x16_t is_nonzero = vmvnq_u8(is_zero);

		uint8x16_t result = vorrq_u8(vandq_u8(blended, is_nonzero), vandq_u8(d, is_zero));  // vcombine_u8(vshrn_n_u16(lo, 8), vshrn_n_u16(hi, 8));

		vst1q_u8((uint8_t*)(canvas_row + x), result);

	}

	/** scaler tail*/
	for (; x < width; x++) {
		uint32_t sp = win_row[x];
		uint32_t bp = blur_row[x];
		uint8_t sa = (uint8_t)(sp >> 24);

		if (sa == 255) {
			canvas_row[x] = sp;
		}
		else if (sa == 0) {
			//canvas_row[x] = bp;
		}
		else {
			uint32_t inv = 255 - sa;
			uint8_t a = (uint8_t)(((sp >> 24 & 0xFF) * sa + (bp >> 24 & 0xFF) * inv) >> 8);
			uint8_t r = (uint8_t)(((sp >> 16 & 0xFF) * sa + (bp >> 16 & 0xFF) * inv) >> 8);
			uint8_t g = (uint8_t)(((sp >> 8 & 0xFF) * sa + (bp >> 8 & 0xFF) * inv) >> 8);
			uint8_t b = (uint8_t)(((sp >> 0 & 0xFF) * sa + (bp >> 0 & 0xFF) * inv) >> 8);
			canvas_row[x] = ((uint32_t)a << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
		}
	}
}

void _shadow_blur_horizontal(uint32_t* dst, const uint32_t* src, int w, int h, int radius) {
	int diameter = 2 * radius + 1;

	for (int y = 0; y < h; y++) {
		uint32_t sum_a = 0;

		//prime sliding window
		for (int k = -radius; k <= radius; k++) {
			int sx = k;
			if (sx < 0) sx = 0;
			if (sx >= w) sx = w - 1;
			sum_a += (src[y * w + sx] >> 24) & 0xFF;
		}

		for (int x = 0; x < w; x++) {
			uint8_t a = (uint8_t)(sum_a / diameter);
			dst[y * w + x] = ((uint32_t)a << 24);
			int rem_x = x - radius;
			int add_x = x + radius + 1;
			if (rem_x < 0) rem_x = 0; if (rem_x >= w) rem_x = w - 1;
			if (add_x < 0) add_x = 0; if (add_x >= w) add_x = w - 1;

			sum_a += ((src[y * w + add_x] >> 24) & 0xFF) -
				((src[y * w + rem_x] >> 24) & 0xFF);

		}
	}
}

void _shadow_blur_vertical(uint32_t* dst, const uint32_t* src, int w, int h, int radius) {
	int diameter = 2 * radius + 1;

	for (int x = 0; x < w; x++) {
		uint32_t sum_a = 0;

		for (int k = -radius; k <= radius; k++) {
			int sy = k;
			if (sy < 0) sy = 0;
			if (sy >= h) sy = h - 1;
			sum_a += (src[sy * w + x] >> 24) & 0xFF;
		}

		for (int y = 0; y < h; y++) {
			uint8_t a = (uint8_t)(sum_a / diameter);
			dst[y * w + x] = ((uint32_t)a << 24);

			int rem_y = y - radius;
			int add_y = y + radius + 1;
			if (rem_y < 0) rem_y = 0; if (rem_y >= h) rem_y = h - 1;
			if (add_y < 0) add_y = 0; if (add_y >= h) add_y = h - 1;

			sum_a += ((src[add_y * w + x] >> 24) & 0xFF) -
				((src[rem_y * w + x] >> 24));
		}
	}
}

void _shadow_compose_neon(uint32_t* canv, int canvas_w, int canvas_h, const uint32_t* shadow_buf,
	int shadow_w, int shadow_h, int win_x, int win_y) {
	
	int draw_x = win_x - SHADOW_SIZE;
	int draw_y = win_y - SHADOW_SIZE;

	for (int j = 0; j < shadow_h; j++) {
		int cy = draw_y + j;
		if (cy < 0 || cy >= canvas_h) continue;

		int cx_start = draw_x;
		int sx_start = 0;
		int row_w = shadow_w;

		if (cx_start < 0) {
			sx_start -= cx_start;
			row_w += cx_start;
			cx_start = 0;
		}

		if (cx_start + row_w > canvas_w)
			row_w = canvas_w - cx_start;

		if (row_w <= 0) continue;

		uint32_t* dst = canv + cy * canvas_w + cx_start;
		const uint32_t* src = shadow_buf + j * shadow_w + sx_start;

		int x = 0;
		for (; x <= row_w - 4; x += 4) {
			uint32x4_t s4 = vld1q_u32(src + x);
			uint32x4_t d4 = vld1q_u32(dst + x);

			uint32x4_t sa4 = vshrq_n_u32(s4, 24);
			uint32x4_t inv4 = vsubq_u32(vdupq_n_u32(255), sa4);

			uint8x16_t db = vreinterpretq_u8_u32(d4);

			uint8x8_t inv_lo = vmovn_u16(vmovl_u32(vget_low_u32(inv4)));
			uint8x8_t inv_hi = vmovn_u16(vmovl_u32(vget_high_u32(inv4)));

			uint8x8_t inv_lo4 = vzip1_u8(inv_lo, inv_lo);

			for (int i = x; i < x + 4; i++) {
				uint32_t dp = dst[i];
				uint8_t sa = (uint8_t)(src[i] >> 24);
				if (sa == 0) continue;
				uint32_t inv = 255 - sa;
				uint8_t r = (uint8_t)(((dp >> 16 & 0xFF) * inv) >> 8);
				uint8_t g = (uint8_t)(((dp >> 8 & 0xFF) * inv) >> 8);
				uint8_t b = (uint8_t)(((dp >> 0 & 0xFF) * inv) >> 8);
				dst[i] = (0xFFu << 24) | ((uint32_t)r << 16) |
					((uint32_t)g << 8) | b;

			}
		}

		for (; x < row_w; x++) {
			uint32_t dp = dst[x];
			uint8_t sa = (uint8_t)(src[x] >> 24);
			if (sa == 0) continue;
			uint32_t inv = 255 - sa;
			uint8_t r = (uint8_t)(((dp >> 16 & 0xFF) * inv) >> 8);
			uint8_t g = (uint8_t)(((dp >> 8 & 0xFF) * inv) >> 8);
			uint8_t b = (uint8_t)(((dp >> 0 & 0xFF) * inv) >> 8);
			dst[x] = (0xFFu << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
		}
	}
}

#define CORNER_RADIUS 12

void _apply_rounded_corner(uint32_t* backbuff, int radius, int winw, int winh) {
	int w = winw;
	int h = winh;
	uint8_t* mask = (uint8_t*)malloc(radius * radius);

	for (int y = 0; y < radius; y++) {
		for (int x = 0; x < radius; x++) {
			float dx = (float)(radius - 1 - x);
			float dy = (float)(radius - 1 - y);
			float dist = sqrtf(dx * dx + dy * dy) - (float)(radius - 1);

			float a = 1.0f - fmaxf(0.0f, fminf(1.0f, dist + 0.5f));
			mask[y * radius + x] = (uint8_t)(a * 255.0f);
		}
	}

	for (int y = 0; y < radius; y++) {
		uint32_t* top_row = backbuff + y * w;
		uint32_t* bot_row = backbuff + (h - 1 - y) * w;

		for (int x = 0; x < radius; x++) {
			uint8_t ma = mask[y * radius + x];

			//top left
			uint32_t* p = &top_row[x];
			if (ma == 0)
				*p = 0x00000000;
			else if (ma == 255)
				*p = (*p & 0x00FFFFFF) | 0xFF000000;
			else
				*p = (*p & 0x00FFFFFF) | ((uint32_t)ma << 24);

			//top right
			p = &top_row[w - 1 - x];
			if (ma == 0) *p = 0x00000000;
			else if (ma == 255) *p = (*p & 0x00FFFFFF) | 0xFF000000;
			*p = (*p & 0x00FFFFFF) | ((uint32_t)ma << 24);

			//bottom left
			p = &bot_row[x];
			if (ma == 0)
				*p = 0x00000000;
			else if (ma == 255) *p = (*p & 0x00FFFFFF) | 0xFF000000;
			else *p = (*p & 0x00FFFFFF) | ((uint32_t)ma << 24);

			//bottom right
			p = &bot_row[w - 1 - x];
			if (ma == 0) *p = 0x00000000;
			else if (ma == 255) *p = (*p & 0x00ffffff) | 0xff000000;
			else *p = (*p & 0x00ffffff) | ((uint32_t)ma << 24);
		}
	}

	free(mask);
}