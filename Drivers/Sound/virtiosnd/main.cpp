/**
* BSD 2-Clause License
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

#include <aurora.h>
#include <Drivers/uart.h>
#include <aucon.h>
#include <stdint.h>
#include <pcie.h>
#include <Hal/AA64/aa64lowlevel.h>
#include <Mm/vmmngr.h>
#include <audrv.h>
#include <Drivers/virtio.h>
#include <Mm/pmmngr.h>
#include <Hal/AA64/gic.h>
#include <string.h>
#include <_null.h>

#define VIRTIO_F_VERSION_1 (1ull << 32)
#define VIRTIO_PCI_CAP_ID 0x09
#define VIRTIO_PCI_CAP_COMMON_CFG 1
#define VIRTIO_PCI_CAP_DEVICE_CFG 4
#define VIRTQ_DESC_F_NEXT  1
#define VIRTQ_DESC_F_WRITE 2

/** private definitions */
volatile uint8_t* notifyBase;
static uint32_t notifyOffMultiplier;
static int controlq_sz;
static int eventq_sz;
static int txq_sz;
static VirtioQueue* controlq;
static VirtioQueue* eventq;
static VirtioQueue* txq;
static void* command_phys;
static void* resp_phys;
static void* pcm_buffer;
static int n_streams;
static int controlq_lst_idx;
static int eventq_lst_idx;
static int txq_lst_idx;
static bool _response_ok;

struct virtio_snd_config {
	uint32_t jacks;
	uint32_t streams;
	uint32_t chmaps;
	uint32_t controls;
};

enum {
	/* jack control request types */
	VIRTIO_SND_R_JACK_INFO = 1,
	VIRTIO_SND_R_JACK_REMAP,

	/* pcm control request types */
	VIRTIO_SND_R_PCM_INFO = 0x0100,
	VIRTIO_SND_R_PCM_SET_PARAMS,
	VIRTIO_SND_R_PCM_PREPARE,
	VIRTIO_SND_R_PCM_RELEASE,
	VIRTIO_SND_R_PCM_START,
	VIRTIO_SND_R_PCM_STOP,

	/* channel map control request types */
	VIRTIO_SND_R_CHMAP_INFO = 0x0200,

	VIRTIO_SND_R_PCM_XFER = 0x1000,

	/* control element request types */
	VIRTIO_SND_R_CTL_INFO = 0x0300,
	VIRTIO_SND_R_CTL_ENUM_ITEMS,
	VIRTIO_SND_R_CTL_READ,
	VIRTIO_SND_R_CTL_WRITE,
	VIRTIO_SND_R_CTL_TLV_READ,
	VIRTIO_SND_R_CTL_TLV_WRITE,
	VIRTIO_SND_R_CTL_TLV_COMMAND,

	/* jack event types */
	VIRTIO_SND_EVT_JACK_CONNECTED = 0x1000,
	VIRTIO_SND_EVT_JACK_DISCONNECTED,

	/* PCM event types */
	VIRTIO_SND_EVT_PCM_PERIOD_ELAPSED = 0x1100,
	VIRTIO_SND_EVT_PCM_XRUN,

	/* control element event types */
	VIRTIO_SND_EVT_CTL_NOTIFY = 0x1200,

	/* common status codes */
	VIRTIO_SND_S_OK = 0x8000,
	VIRTIO_SND_S_BAD_MSG,
	VIRTIO_SND_S_NOT_SUPP,
	VIRTIO_SND_S_IO_ERR
};

/* supported PCM stream features */
enum {
	VIRTIO_SND_PCM_F_SHMEM_HOST = 0,
	VIRTIO_SND_PCM_F_SHMEM_GUEST,
	VIRTIO_SND_PCM_F_MSG_POLLING,
	VIRTIO_SND_PCM_F_EVT_SHMEM_PERIODS,
	VIRTIO_SND_PCM_F_EVT_XRUNS
};

/* supported PCM sample formats */
enum {
	/* analog formats (width/ physical width) */
	VIRTIO_SND_PCM_FMT_IMA_ADPCM = 0,  // 4 / 4 bits
	VIRTIO_SND_PCM_FMT_MU_LAW,
	VIRTIO_SND_PCM_FMT_A_LAW,
	VIRTIO_SND_PCM_FMT_S8,
	VIRTIO_SND_PCM_FMT_U8,
	VIRTIO_SND_PCM_FMT_S16,
	VIRTIO_SND_PCM_FMT_U16,
	VIRTIO_SND_PCM_FMT_S18_3,
	VIRTIO_SND_PCM_FMT_U18_3,
	VIRTIO_SND_PCM_FMT_S20_3,
	VIRTIO_SND_PCM_FMT_U20_3,
	VIRTIO_SND_PCM_FMT_S24_3,
	VIRTIO_SND_PCM_FMT_U24_3,
	VIRTIO_SND_PCM_FMT_S20,
	VIRTIO_SND_PCM_FMT_U20,
	VIRTIO_SND_PCM_FMT_S24,
	VIRTIO_SND_PCM_FMT_U24,
	VIRTIO_SND_PCM_FMT_S32,
	VIRTIO_SND_PCM_FMT_U32,
	VIRTIO_SND_PCM_FMT_FLOAT,
	VIRTIO_SND_PCM_FMT_FLOAT64,
	VIRTIO_SND_PCM_FMT_DSD_U8,
	VIRTIO_SND_PCM_FMT_DSD_U16,
	VIRTIO_SND_PCM_FMT_DSD_U32,
	VIRTIO_SND_PCM_FMT_IEC958_SUBFRAME,
};

/* supported PCM frame rates */
enum {
	VIRTIO_SND_PCM_RATE_5512 = 0,
	VIRTIO_SND_PCM_RATE_8000,
	VIRTIO_SND_PCM_RATE_11025,
	VIRTIO_SND_PCM_RATE_16000,
	VIRTIO_SND_PCM_RATE_22050,
	VIRTIO_SND_PCM_RATE_32000,
	VIRTIO_SND_PCM_RATE_44100,
	VIRTIO_SND_PCM_RATE_48000,
	VIRTIO_SND_PCM_RATE_64000,
	VIRTIO_SND_PCM_RATE_88200,
	VIRTIO_SND_PCM_RATE_96000,
	VIRTIO_SND_PCM_RATE_176400,
	VIRTIO_SND_PCM_RATE_192000,
	VIRTIO_SND_PCM_RATE_384000,
};

struct virtio_snd_hdr {
	uint32_t code;
};

struct virtio_snd_info {
	uint32_t hda_fn_nid;
};

typedef struct _virtio_snd_query_info_{
	virtio_snd_hdr hdr;
	uint32_t start_id;
	uint32_t count;
	uint32_t size;
}virtio_snd_query_info;

struct virtio_snd_event {
	struct virtio_snd_hdr;
	uint32_t data;
};

/* jack info response */
typedef struct {
	virtio_snd_hdr hdr;
	uint32_t hda_fn_nid;
	uint32_t hda_pin_default;
	uint32_t features;
	uint8_t conected;
	uint8_t padding[7];
}virtio_snd_jack_info;

struct virtio_snd_pcm_hdr {
	struct virtio_snd_hdr hdr;
	uint32_t stream_id;
};

struct virtio_snd_pcm_info {
	struct virtio_snd_info info_hdr;
	uint32_t features;  // 1 << VIRTIO_SND_PCM_F_XXX 
	uint64_t formats;   // 1 << VIRTIO_SND_PCM_FMT_XXX
	uint64_t rates;     // 1 << VIRTIO_SND_PCM_RATE_XXX
	uint8_t directions;
	uint8_t channels_min;
	uint8_t channels_max;
	uint8_t padding[5];
};

struct virtio_snd_pcm_set_params {
	struct virtio_snd_pcm_hdr hdr;
	uint32_t buffer_bytes;
	uint32_t period_bytes;
	uint32_t features; // 1 << VIRTIO_SND_PCM_F_XXX
	uint8_t channels;
	uint8_t format;
	uint8_t rate;
	uint8_t padding;
};

typedef struct _snd_xfer_ {
	//virtio_snd_hdr hdr;
	uint32_t stream_id;
}virtio_snd_pcm_xfer;

typedef struct {
	//virtio_snd_hdr hdr;
	uint32_t status;
	uint32_t latency_bytes;
}virtio_snd_pcm_status;

/*
* AuDriverUnload -- deattach the driver from
* aurora system
*/
AU_EXTERN AU_EXPORT int AuDriverUnload() {

	return 0;
}

/**
 * @brief virtio_snd_reset -- reset the net device
 * @param common -- Pointer to virtio common config desc
 */
static void virtio_snd_reset(VirtioCommonCfg* common) {
	common->DeviceStatus = 0;
	isb_flush();
	dsb_ish();
	UARTDebugOut("[virtio-snd]: virtio-snd device reset successfull \r\n");
}

/**
 * @brief virtio_snd_alloc_controlq -- allocate sound controlq
 * @param cfg -- pointer to virtio common config
 */
static void virtio_snd_alloc_controlq(VirtioCommonCfg* cfg) {
	cfg->QueueSelect = 0;
	isb_flush();
	dsb_ish();

	int queueSz = cfg->QueueSize;
	controlq_sz = queueSz;
	uint64_t queuePhys = (uint64_t)AuPmmngrAlloc();//AuPmmngrAllocBlocks(((sizeof(struct VirtioQueue) * queueSz)) / 0x1000);
	controlq = (struct VirtioQueue*)AuMapMMIO(queuePhys, 1);
#if DEBUG
	UARTDebugOut("[virtio-snd]: controlq size : %d \r\n", queueSz);
#endif
	cfg->QueueDesc = queuePhys;
	cfg->QueueAvail = (queuePhys)+OFFSETOF(struct VirtioQueue, available);
	cfg->QueueUsed = (queuePhys)+OFFSETOF(struct VirtioQueue, used);
	cfg->MSix = 0;
	cfg->QueueMSixVector = 0;
	cfg->QueueEnable = 1;
	isb_flush();
	dsb_ish();

	cfg->DeviceStatus = 4;
	isb_flush();
	dsb_ish();

#if DEBUG
	UARTDebugOut("[virtio-snd]: controlq initialized \r\n");
#endif
}

/**
 * @brief virtio_snd_alloc_eventq -- allocate sound eventq
 * @param cfg -- pointer to virtio common config
 */
static void virtio_snd_alloc_eventq(struct VirtioCommonCfg* cfg) {
	cfg->QueueSelect = 1;
	isb_flush();
	dsb_ish();

	int queueSz = cfg->QueueSize;
	eventq_sz = queueSz;
	uint64_t queuePhys = (uint64_t)AuPmmngrAlloc();//AuPmmngrAllocBlocks(((sizeof(struct VirtioQueue) * queueSz)) / 0x1000);
	eventq = (struct VirtioQueue*)AuMapMMIO(queuePhys, 1);

#if DEBUG
	UARTDebugOut("[virtio-snd]: eventq size : %d \r\n", queueSz);
#endif

	cfg->QueueDesc = queuePhys;
	cfg->QueueAvail = (queuePhys)+OFFSETOF(struct VirtioQueue, available);
	cfg->QueueUsed = (queuePhys)+OFFSETOF(struct VirtioQueue, used);
	cfg->MSix = 0;
	cfg->QueueMSixVector = 0;
	cfg->QueueEnable = 1;
	isb_flush();
	dsb_ish();

#if DEBUG
	UARTDebugOut("[virtio-snd]: eventq initialized \r\n");
#endif
}


/**
 * @brief virtio_snd_alloc_txq -- allocate sound txq
 * @param cfg -- pointer to virtio common config
 */
static void virtio_snd_alloc_txq(VirtioCommonCfg* cfg) {
	cfg->QueueSelect = 2;
	isb_flush();
	dsb_ish();

	int queueSz = cfg->QueueSize;
	txq_sz = queueSz;
	uint64_t queuePhys = (uint64_t)AuPmmngrAlloc();//AuPmmngrAllocBlocks(((sizeof(struct VirtioQueue) * queueSz)) / 0x1000);
	txq = (struct VirtioQueue*)AuMapMMIO(queuePhys, 1);
#if DEBUG
	UARTDebugOut("[virtio-snd]: txq size : %d \r\n", queueSz);
#endif
	cfg->QueueDesc = queuePhys;
	cfg->QueueAvail = (queuePhys)+OFFSETOF(struct VirtioQueue, available);
	cfg->QueueUsed = (queuePhys)+OFFSETOF(struct VirtioQueue, used);
	cfg->MSix = 0;
	cfg->QueueMSixVector = 0;
	cfg->QueueEnable = 1;
	isb_flush();
	dsb_ish();

	cfg->DeviceStatus = 4;
	isb_flush();
	dsb_ish();

#if DEBUG
	UARTDebugOut("[virtio-snd]: txq initialized \r\n");
#endif
}


/**
 * @brief virtio_snd_feature_negotiate -- driver feature negotiation
 * for virtio snd device
 * @param common -- Pointer to virtio common config descriptor
 */
static void virtio_snd_feature_negotiate(struct VirtioCommonCfg* common) {
	common->DevFeatureSelect = 0;
	isb_flush();
	dsb_ish();
	uint32_t features_lo = common->DevFeature;
	common->DevFeatureSelect = 1;
	isb_flush();
	dsb_ish();
	uint32_t feature_hi = common->DevFeature;
	uint64_t features = ((uint64_t)feature_hi << 32) | features_lo;

	if (!(features & VIRTIO_F_VERSION_1))
		AuTextOut("[virtio-snd]: warning: device is not modern VirtIO! \r\n");

	uint64_t guestfeatures = 0;
	guestfeatures |= VIRTIO_F_VERSION_1;
	guestfeatures &= features;

	common->GuestFeatureSelect = 0;
	common->GuestFeature = guestfeatures & UINT32_MAX;
	isb_flush();
	dsb_ish();

	common->GuestFeatureSelect = 1;
	common->GuestFeature = (guestfeatures >> 32) & UINT32_MAX;
	isb_flush();
	dsb_ish();
}

/**
 * @brief snd_notify_queue -- notify host that new command
 * is present
 * @param queueIdx -- queue number, zero for controlq and
 * one for cursorq
 */
void snd_notify_queue(VirtioCommonCfg* cfg, uint16_t queueIdx) {
	cfg->QueueSelect = queueIdx;
	isb_flush();
	dsb_ish();
	uint16_t notify_off = cfg->QueueNotifyOff;
	volatile uint16_t* notifyAddr = (volatile uint16_t*)((uint64_t)notifyBase + notify_off * notifyOffMultiplier);
	*notifyAddr = queueIdx;
	isb_flush();
	dsb_ish();
}


virtio_snd_pcm_info* snd_query_pcm_info(VirtioCommonCfg* cfg) {
	uint32_t num_streams = n_streams;
	int index = controlq->available.index % controlq_sz;

	virtio_snd_query_info* req = (virtio_snd_query_info*)command_phys;
	req->hdr.code = VIRTIO_SND_R_PCM_INFO;
	req->start_id = 0;
	req->count = num_streams;
	req->size = sizeof(virtio_snd_pcm_info);
	aa64_data_cache_clean_range(command_phys, sizeof(virtio_snd_query_info));

	controlq->buffers[index].Addr = (uint64_t)V2P((uint64_t)command_phys);
	controlq->buffers[index].Length = sizeof(virtio_snd_query_info);
	controlq->buffers[index].Flags = VIRTQ_DESC_F_NEXT;
	controlq->buffers[index].Next = (index + 1) % controlq_sz;


	controlq->buffers[(index + 1) % controlq_sz].Addr = (uint64_t)V2P((uint64_t)resp_phys);
	controlq->buffers[(index + 1) % controlq_sz].Length = sizeof(virtio_snd_hdr) +  sizeof(virtio_snd_pcm_info) *num_streams;
	controlq->buffers[(index + 1) % controlq_sz].Flags = VIRTQ_DESC_F_WRITE;

	controlq->available.ring[index % controlq_sz] = index;
	isb_flush();
	dsb_ish();

	controlq->available.index++;
	isb_flush();
	dsb_ish();

	snd_notify_queue(cfg, 0);

	int timeout = 5000000;
	while (timeout) {
		if (_response_ok == true) {
			_response_ok = false;
			return (virtio_snd_pcm_info*)((uint64_t)resp_phys + sizeof(virtio_snd_hdr));
			break;
		}
		timeout--;
	}

	return NULL;
}


static void snd_send_pcm(VirtioCommonCfg* cfg, void* pcm_data, uint32_t len) {
	int index = txq->available.index % txq_sz;

	virtio_snd_pcm_xfer* xfer = (virtio_snd_pcm_xfer*)command_phys;
	xfer->stream_id = 0; //0 for audio output
	aa64_data_cache_clean_range(command_phys, 0x1000);

#ifdef DEBUG
	UARTDebugOut("txq index : %d \r\n", index);
#endif
	txq->buffers[index].Addr = V2P((uint64_t)command_phys);
	txq->buffers[index].Length = sizeof(virtio_snd_pcm_xfer);
	txq->buffers[index].Flags = VIRTQ_DESC_F_NEXT;
	txq->buffers[index].Next = (index + 1) % txq_sz;

	txq->buffers[(index + 1) % txq_sz].Addr = V2P((uint64_t)pcm_data);
	txq->buffers[(index + 1) % txq_sz].Length = len;
	txq->buffers[(index + 1) % txq_sz].Flags = VIRTQ_DESC_F_NEXT;
	txq->buffers[(index + 1) % txq_sz].Next = (index + 2) % txq_sz;

	txq->buffers[(index + 2) % txq_sz].Addr = V2P((uint64_t)resp_phys);
	txq->buffers[(index + 2) % txq_sz].Length = sizeof(virtio_snd_pcm_status);
	txq->buffers[(index + 2) % txq_sz].Flags = VIRTQ_DESC_F_WRITE;

	uint16_t ringSlot = txq->available.index % txq_sz;
	txq->available.ring[ringSlot] = index;
	isb_flush();
	dsb_ish();
	
	txq->available.index++;
	//txq->available.index %= txq_sz;
	isb_flush();
	dsb_ish();

	snd_notify_queue(cfg, 2);


	while (txq->used.index == txq_lst_idx)
		dsb_ish();

	while (txq->used.index != txq_lst_idx) {
		uint16_t idx = txq_lst_idx & (txq_sz - 1);
		uint32_t desc_id = txq->used.ring[idx].index;
		txq_lst_idx++;
	}

	while (1) {
		if (_response_ok == true) {
			_response_ok = false;
			break;
		}
	}

	virtio_snd_pcm_status* stat = (virtio_snd_pcm_status*)resp_phys;
	if (stat->status != VIRTIO_SND_S_OK)
		UARTDebugOut("[virtio-snd]: output xfer returned : %x , latency bytes : %d \r\n", stat->status,
			stat->latency_bytes);
}

/**
 * snd_pcm_set_params -- set parameter for output stream
 * @param cfg -- Pointer to common config
 */
static int snd_pcm_set_params(VirtioCommonCfg* cfg) {
	int val = 1;

	int index = controlq->available.index % controlq_sz;

	virtio_snd_pcm_set_params* parm = (virtio_snd_pcm_set_params*)command_phys;
	virtio_snd_hdr* resp = (virtio_snd_hdr*)resp_phys;

	parm->hdr.hdr.code = VIRTIO_SND_R_PCM_SET_PARAMS;
	parm->hdr.stream_id = 0;
	parm->features = 0;
	parm->buffer_bytes = 8192;
	parm->period_bytes = 4096;
	parm->channels = 2;
	parm->format = VIRTIO_SND_PCM_FMT_S16;
	parm->rate = VIRTIO_SND_PCM_RATE_44100;
	aa64_data_cache_clean_range(command_phys, sizeof(virtio_snd_pcm_set_params));


	controlq->buffers[index].Addr = (uint64_t)V2P((uint64_t)command_phys);
	controlq->buffers[index].Length = sizeof(virtio_snd_pcm_set_params);
	controlq->buffers[index].Flags = VIRTQ_DESC_F_NEXT;
	controlq->buffers[index].Next = (index + 1) % controlq_sz;


	controlq->buffers[(index + 1) % controlq_sz].Addr = (uint64_t)V2P((uint64_t)resp_phys);
	controlq->buffers[(index + 1) % controlq_sz].Length = sizeof(virtio_snd_pcm_set_params);
	controlq->buffers[(index + 1) % controlq_sz].Flags = VIRTQ_DESC_F_WRITE;

	controlq->available.ring[index % controlq_sz] = index;
	isb_flush();
	dsb_ish();

	controlq->available.index++;
	isb_flush();
	dsb_ish();

	snd_notify_queue(cfg, 0);

	int timeout = 5000000;
	while (timeout) {
		if (_response_ok == true) {
			_response_ok = false;
			val = 0;
			break;
		}
		timeout--;
	}
	return val;
}


/**
 * snd_pcm_prepare_output -- prepare output stream
 * @param cfg -- Pointer to common config
 */
static int snd_pcm_prepare_output(VirtioCommonCfg* cfg) {
	int val = 1;
	int index = controlq->available.index % controlq_sz;

	virtio_snd_pcm_hdr* pcm = (virtio_snd_pcm_hdr*)command_phys;
	virtio_snd_hdr* resp = (virtio_snd_hdr*)resp_phys;

	pcm->hdr.code = VIRTIO_SND_R_PCM_PREPARE;
	pcm->stream_id = 0;

	controlq->buffers[index].Addr = (uint64_t)V2P((uint64_t)command_phys);
	controlq->buffers[index].Length = sizeof(virtio_snd_pcm_hdr);
	controlq->buffers[index].Flags = VIRTQ_DESC_F_NEXT;
	controlq->buffers[index].Next = (index + 1) % controlq_sz;


	controlq->buffers[(index + 1) % controlq_sz].Addr = (uint64_t)V2P((uint64_t)resp_phys);
	controlq->buffers[(index + 1) % controlq_sz].Length = sizeof(virtio_snd_hdr);
	controlq->buffers[(index + 1) % controlq_sz].Flags = VIRTQ_DESC_F_WRITE;

	controlq->available.ring[index % controlq_sz] = index;
	isb_flush();
	dsb_ish();

	controlq->available.index++;
	isb_flush();
	dsb_ish();

	snd_notify_queue(cfg, 0);

	int timeout = 5000000;
	while (timeout--) {
		if (_response_ok == true) {
			_response_ok = false;
			val = 0;
			break;
		}
	}

	return val;
}

/**
 * snd_pcm_output_start -- start output stream
 * @param cfg -- Pointer to common config
 */
static int snd_pcm_output_start(VirtioCommonCfg* cfg) {
	int val = 1;
	int index = controlq->available.index % controlq_sz;

	virtio_snd_pcm_hdr* pcm = (virtio_snd_pcm_hdr*)command_phys;
	virtio_snd_hdr* resp = (virtio_snd_hdr*)resp_phys;

	pcm->hdr.code = VIRTIO_SND_R_PCM_START;
	pcm->stream_id = 0;

	controlq->buffers[index].Addr = (uint64_t)V2P((uint64_t)command_phys);
	controlq->buffers[index].Length = sizeof(virtio_snd_pcm_hdr);
	controlq->buffers[index].Flags = VIRTQ_DESC_F_NEXT;
	controlq->buffers[index].Next = (index + 1) % controlq_sz;


	controlq->buffers[(index + 1) % controlq_sz].Addr = (uint64_t)V2P((uint64_t)resp_phys);
	controlq->buffers[(index + 1) % controlq_sz].Length = sizeof(virtio_snd_hdr);
	controlq->buffers[(index + 1) % controlq_sz].Flags = VIRTQ_DESC_F_WRITE;

	controlq->available.ring[index % controlq_sz] = index;
	isb_flush();
	dsb_ish();

	controlq->available.index++;
	isb_flush();
	dsb_ish();

	snd_notify_queue(cfg, 0);

	int timeout = 5000000;
	while (timeout--) {
		if (_response_ok == true) {
			_response_ok = false;
			val = 0;
			break;
		}
	}
	return val;
}

void virtio_snd_interrupt(int spi_id) {
	uint16_t them = controlq->used.index;

	for (; controlq_lst_idx < them; controlq_lst_idx++) {

		isb_flush();
		controlq->available.index++;
	}

	virtio_snd_hdr* hdr = (virtio_snd_hdr*)resp_phys;
	if (hdr->code == VIRTIO_SND_S_OK) {
		_response_ok = true;
	}
}

/*
* AuDriverMain -- Main entry for vmware svga driver
*/
AU_EXTERN AU_EXPORT int AuDriverMain(AuDriver* drv) {
	AuTextOut("[virtio-sound]: initializing virtio sound driver \r\n");
	AuTextOut("bus : %d, dev : %d, func : %d \r\n", drv->bus, drv->dev, drv->func);
	int bus = drv->bus;
	int dev = drv->dev;
	int func = drv->func;
	_response_ok = false;

	command_phys = (uint64_t*)P2V((uint64_t)AuPmmngrAlloc());
	resp_phys = (void*)((uint64_t)command_phys + 2048);
	memset(command_phys, 0, PAGE_SIZE);
	controlq_lst_idx = 0;
	eventq_lst_idx = 0;
	txq_lst_idx = 0;


	pcm_buffer = (void*)P2V((uint64_t)AuPmmngrAlloc());
	memset(pcm_buffer, 0, 0x1000);

	/** change the class/subclass value **/
	uint64_t device = AuPCIEScanClass(drv->classCode, drv->subClassCode, &bus, &dev, &func);

	uint16_t command = AuPCIERead(device, PCI_COMMAND, bus, dev, func);
	command |= 4;
	command |= 2;
	command |= 1;
	AuPCIEWrite(device, PCI_COMMAND, command, bus, dev, func);
	isb_flush();
	dsb_ish();

	uint64_t barLo = AuPCIERead(device, PCI_BAR4, bus, dev, func);
	uint64_t barHi = AuPCIERead(device, PCI_BAR5, bus, dev, func);
	uint64_t bar = ((uint64_t)barHi << 32) | (barLo & ~0xFULL);
	uint64_t finalAddr = (uint64_t)AuMapMMIO(bar, 1);

	uint8_t cap_ptr = AuPCIERead(device, PCI_CAPABILITIES_PTR, bus, dev, func);
	uint32_t devcfg_offset = 0;
	while (cap_ptr != 0) {
		volatile virtio_pci_cap* cap = (volatile virtio_pci_cap*)(device + cap_ptr);
		if (cap->cap_vndr == VIRTIO_PCI_CAP_ID) {
			if (cap->cfg_type == VIRTIO_PCI_CAP_DEVICE_CFG) {
				UARTDebugOut("[virtio-snd]: device configuration offset : %x \n", cap->offset);
				devcfg_offset = cap->offset;
				//break;
			}
			if (cap->cfg_type == 2) { //NOTIFY_CFG
				notifyBase = (volatile uint8_t*)AuMapMMIO(bar + cap->offset, 1);
				struct virtio_notifier_cap* notify = (struct virtio_notifier_cap*)cap;
				notifyOffMultiplier = notify->notifer_mult_base;
				UARTDebugOut("[virtio-snd]: notify base : %x , off : %x\n", notifyBase, cap->offset);
			}
		}
		cap_ptr = cap->cap_next;
	}

	VirtioCommonCfg* cfg = (VirtioCommonCfg*)finalAddr;
	AuTextOut("[virtio-snd]: num queues : %d \r\n", cfg->Queues);
	/** reset the device **/
	virtio_snd_reset(cfg);

	/* acknowledge */
	cfg->DeviceStatus |= 0x01;
	isb_flush();
	dsb_ish();

	/* driver status */
	cfg->DeviceStatus |= 0x02;
	isb_flush();
	dsb_ish();

	virtio_snd_feature_negotiate(cfg);

	int spi_id = AuGICAllocateSPI();
	UARTDebugOut("[virtio-snd]: spi id: %d \n", spi_id);
	if (AuPCIEAllocMSI(device, spi_id, bus, dev, func)) {
		UARTDebugOut("[virtio-snd]: msi/msi-x allocated \r\n");
	}
	GICEnableSPIIRQ(spi_id);
	//GICSetTargetCPU(spiID);
	isb_flush();
	dsb_ish();

	GICRegisterSPIHandler(&virtio_snd_interrupt, spi_id);

	/** allocate controlq **/
	virtio_snd_alloc_controlq(cfg);
	/** allocate eventq **/
	virtio_snd_alloc_eventq(cfg);
	/** allocate txq **/
	virtio_snd_alloc_txq(cfg);
	/** TODO: input rxq for input **/

	virtio_snd_config* snd = (virtio_snd_config*)(bar + devcfg_offset);
	if (!snd)
		return 1;

#if DEBUG
	UARTDebugOut("[virtio-snd]: num jacks : %d \r\n", snd->jacks);
	UARTDebugOut("[virtio-snd]: total chmap: %d \r\n", snd->chmaps);
	UARTDebugOut("[virtio-snd]: num controls : %d \r\n", snd->controls);
	UARTDebugOut("[virtio-snd]: streams : %d \r\n", snd->streams);
	UARTDebugOut("[virtio-snd]: pcm info size : %d \r\n", sizeof(virtio_snd_pcm_info));
#endif
	n_streams = snd->streams;

	enable_irqs();

	virtio_snd_pcm_info* pcm = snd_query_pcm_info(cfg);
	if (!pcm) {
		UARTDebugOut("[virtio-snd]: failed to query pcm info, returned NULL \r\n");
		return 1;
	}
#if DEBUG
	for (int i = 0; i < n_streams; i++) {
		virtio_snd_pcm_info* s = &pcm[i];
		UARTDebugOut("[virtio-snd]: pcm : %d status : %x\r\n",i, s->info_hdr.hda_fn_nid);
		UARTDebugOut("[virtio-snd]: pcm : %d direction : %d \r\n", i, s->directions);
		UARTDebugOut("[virtio-snd]: pcm : %d format : %x\r\n",i, s->formats);
		UARTDebugOut("[virtio-snd]: pcm : %d frame rate : %x \r\n",i, s->rates);
	}
#endif
	memset(command_phys, 0, 0x1000);

	/** set params for the output stream **/
	if (snd_pcm_set_params(cfg)) {
		UARTDebugOut("[virtio-snd]: failed to set output stream parameters \r\n");
		return 1;
	}


	memset(command_phys, 0, 0x1000);
	UARTDebugOut("[virtio-snd]: parameter set successfully for output stream \r\n");


	/** prepare the output stream too **/
	if (snd_pcm_prepare_output(cfg)) {
		UARTDebugOut("[virtio-snd]: failed to prepare output stream \r\n");
		return 1;
	}

	memset(command_phys, 0, 0x1000);

	/** start the output stream **/
	if (snd_pcm_output_start(cfg)) {
		UARTDebugOut("[virtio-snd]: failed to start output stream \r\n");
		return 1;
	}

	UARTDebugOut("[virtio-snd]: output stream started \r\n");
	memset(command_phys, 0, 0x1000);

	UARTDebugOut("[virtio-snd]: initialized successfully \r\n");
	mask_irqs();
	return 0;
}