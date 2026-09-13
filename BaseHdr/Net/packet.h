/**
 * Thin packet descriptor for local-delivery / loopback origin tracking.
 * Full iptable Phase-0 AuPacket (hooks, L2/L3 offsets, rt*) is out of scope.
 */

#ifndef __AU_PACKET_H__
#define __AU_PACKET_H__

#include <stdint.h>
#include <Fs/vfs.h>

enum {
	AU_PKT_ORIGIN_WIRE = 0,
	AU_PKT_ORIGIN_LOCAL = 1,
	AU_PKT_ORIGIN_STACK = 2
};

typedef struct _au_packet_ {
	void* data;       /* L3 start (IPv4Header* / IPv6Header*) for this pass */
	uint16_t len;
	uint8_t origin;   /* AU_PKT_ORIGIN_* */
	AuVFSNode* in_dev;
	AuVFSNode* out_dev;
} AuPacket;

/*
 * Process-local origin + reinject depth (AA64 loopback).
 * Depth >= 2 stops further local reinject to bound ICMP echo bounce.
 */
extern void AuPacketSetOrigin(uint8_t origin);
extern uint8_t AuPacketGetOrigin(void);
extern int AuPacketLocalEnter(void);  /* returns 0 if depth would exceed cap */
extern void AuPacketLocalLeave(void);
extern int AuPacketLocalDepth(void);

#endif
