
#include <Board/RPI3bp/rpi3bp.h>
#include <Drivers/uart.h>
#include <aucon.h>

/*
 * AuAA64BoardInitialize -- initialize board specific data
 */
void AuAA64BoardInitialize() {
	AuTextOut("[aurora]: initializing board data \r\n");
#ifdef __TARGET_BOARD_RPI3__
	AuRPI3Initialize();
#endif
}