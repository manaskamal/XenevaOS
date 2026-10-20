#ifndef NS_YT_H
#define NS_YT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Start a watch-page playback attempt from the HTML already fetched.
 * Safe to call for other hosts: it returns immediately. */
void NsYtStart(const char* page_url, const char* html, size_t html_len);

/* Decode the next picture. Returns 1 when a new frame is ready. */
int NsYtTick(void);

/* Current BGRA frame, or NULL. */
const uint32_t* NsYtFrame(int* w, int* h);

const char* NsYtStatus(void);

#ifdef __cplusplus
}
#endif

#endif
