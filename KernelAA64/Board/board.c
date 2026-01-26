
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

/*
 * AuAA64BoardSleepUS -- calls board specific sleep function
 */
void AuAA64BoardSleepUS(uint32_t us) {
#ifdef __TARGET_BOARD_RPI3__
	AuRPI3DelayUS(us);
#endif
}

/*
 * AuAA64BoardSleepMS -- calls board specific sleep function
 * in ms
 */
void AuAA64BoardSleepMS(uint32_t ms) {
#ifdef __TARGET_BOARD_RPI3__
	AuRPIDelayMS(ms);
#endif
}