#ifdef __GNUC__

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
extern void GICEnableSPIIRQ(void);
extern void AuGICAllocateSPI(void);
extern void GICRegisterSPIHandler(void);
extern void UARTDebugOut(void);
extern void AuPmmngrAlloc(void);
extern void AuPmmngrAllocBlocks(void);
extern void AuTextOut(void);
extern void AuAddNetAdapter(void);
extern void AuMapMMIO(void);
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
    {"GICEnableSPIIRQ", (void*)GICEnableSPIIRQ},
    {"AuGICAllocateSPI", (void*)AuGICAllocateSPI},
    {"GICRegisterSPIHandler", (void*)GICRegisterSPIHandler},
    {"UARTDebugOut", (void*)UARTDebugOut},
    {"AuPmmngrAlloc", (void*)AuPmmngrAlloc},
    {"AuPmmngrAllocBlocks", (void*)AuPmmngrAllocBlocks},
    {"P2V", (void*)P2V},
    {"AuTextOut", (void*)AuTextOut},
    {"AuAddNetAdapter", (void*)AuAddNetAdapter},
    {"AuMapMMIO", (void*)AuMapMMIO},
    {"strcpy", (void*)strcpy},
    {"memset", (void*)memset},
    {"memcpy", (void*)memcpy},
    {"kmalloc", (void*)kmalloc},
    {"AuEthernetHandle", (void*)AuEthernetHandle},
};

int k_exports_count = sizeof(k_exports) / sizeof(struct kernel_export);

#endif
