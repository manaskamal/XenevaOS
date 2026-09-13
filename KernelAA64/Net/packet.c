/**
 * @file packet.c
 * Thin AuPacket origin / local-reinject depth helpers (loopback).
 */

#include <Net/packet.h>

static uint8_t au_pkt_origin = AU_PKT_ORIGIN_WIRE;
static int au_pkt_local_depth = 0;

#define AU_PKT_LOCAL_DEPTH_CAP 2

void AuPacketSetOrigin(uint8_t origin) {
	au_pkt_origin = origin;
}

uint8_t AuPacketGetOrigin(void) {
	return au_pkt_origin;
}

int AuPacketLocalEnter(void) {
	if (au_pkt_local_depth >= AU_PKT_LOCAL_DEPTH_CAP)
		return 0;
	au_pkt_local_depth++;
	au_pkt_origin = AU_PKT_ORIGIN_LOCAL;
	return 1;
}

void AuPacketLocalLeave(void) {
	if (au_pkt_local_depth > 0)
		au_pkt_local_depth--;
	if (au_pkt_local_depth == 0)
		au_pkt_origin = AU_PKT_ORIGIN_WIRE;
}

int AuPacketLocalDepth(void) {
	return au_pkt_local_depth;
}
