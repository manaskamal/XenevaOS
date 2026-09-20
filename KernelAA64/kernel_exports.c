#include <stdint.h>
#include <stddef.h>

struct kernel_export {
	char* name;
	void* addr;
};

/* Drivers declarations */
extern void AuPCIEAllocMSI(void);
extern void AuPCIEScanClass(void);
extern void AuPCIEWrite(void);
extern void AuPCIERead(void);
extern void dsb_ish(void);
extern void isb_flush(void);
extern void dc_ivac(void);
extern void dc_cvac(void);
extern void GICEnableSPIIRQ(void);
extern void AuGICAllocateSPI(void);
extern void GICRegisterSPIHandler(void);
extern void UARTDebugOut(void);
extern void AuPmmngrAllocPage(void);
extern void AuPmmngrAllocPages(void);
extern void AuPmmngrReleasePage(void);
extern void AuPmmngrReleasePages(void);
extern void AuTextOut(void);
extern void AuAddNetAdapter(void);
extern void AuNetAddConnectedRoute4(void);
extern void AuNetAddDefaultRoute4(void);
extern void AuNetAddConnectedRoute6(void);
extern void AuNetAddDefaultRoute6(void);
extern void AuNetRegisterRxPoll(void);
extern void AuNetRxPoll(void);
extern void AuDnsSetServer4(void);
extern void AuMapMMIO(void);
extern void AuVirtioPCIInit(void);
extern void AuVirtioPCISetupQueue(void);
extern void AuVirtioPCINotifyQueue(void);
extern void AuVirtioPCIPostAvail(void);
extern void strcpy(void);
extern void memset(void);
extern void memcpy(void);
extern void kmalloc(void);
extern void AuEthernetHandle(void);
extern void P2V(void);

struct kernel_export k_exports[] = {
	{"AuPCIEAllocMSI", (void*)AuPCIEAllocMSI},
	{"AuPCIEScanClass", (void*)AuPCIEScanClass},
	{"AuPCIEWrite", (void*)AuPCIEWrite},
	{"AuPCIERead", (void*)AuPCIERead},
	{"dsb_ish", (void*)dsb_ish},
	{"isb_flush", (void*)isb_flush},
	{"dc_ivac", (void*)dc_ivac},
	{"dc_cvac", (void*)dc_cvac},
	{"GICEnableSPIIRQ", (void*)GICEnableSPIIRQ},
	{"AuGICAllocateSPI", (void*)AuGICAllocateSPI},
	{"GICRegisterSPIHandler", (void*)GICRegisterSPIHandler},
	{"UARTDebugOut", (void*)UARTDebugOut},
	{"AuPmmngrAllocPage", (void*)AuPmmngrAllocPage},
	{"AuPmmngrAllocPages", (void*)AuPmmngrAllocPages},
	{"AuPmmngrReleasePage", (void*)AuPmmngrReleasePage},
	{"AuPmmngrReleasePages", (void*)AuPmmngrReleasePages},
	{"P2V", (void*)P2V},
	{"AuTextOut", (void*)AuTextOut},
	{"AuAddNetAdapter", (void*)AuAddNetAdapter},
	{"AuNetAddConnectedRoute4", (void*)AuNetAddConnectedRoute4},
	{"AuNetAddDefaultRoute4", (void*)AuNetAddDefaultRoute4},
	{"AuNetAddConnectedRoute6", (void*)AuNetAddConnectedRoute6},
	{"AuNetAddDefaultRoute6", (void*)AuNetAddDefaultRoute6},
	{"AuNetRegisterRxPoll", (void*)AuNetRegisterRxPoll},
	{"AuNetRxPoll", (void*)AuNetRxPoll},
	{"AuDnsSetServer4", (void*)AuDnsSetServer4},
	{"AuMapMMIO", (void*)AuMapMMIO},
	{"AuVirtioPCIInit", (void*)AuVirtioPCIInit},
	{"AuVirtioPCISetupQueue", (void*)AuVirtioPCISetupQueue},
	{"AuVirtioPCINotifyQueue", (void*)AuVirtioPCINotifyQueue},
	{"AuVirtioPCIPostAvail", (void*)AuVirtioPCIPostAvail},
	{"strcpy", (void*)strcpy},
	{"memset", (void*)memset},
	{"memcpy", (void*)memcpy},
	{"kmalloc", (void*)kmalloc},
	{"AuEthernetHandle", (void*)AuEthernetHandle},
};

int k_exports_count = sizeof(k_exports) / sizeof(struct kernel_export);
