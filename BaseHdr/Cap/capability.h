#ifndef CAPABILITY_H
#define CAPABILITY_H



#include <aurora.h>
//#include <Cred/cred.h>
#include <Cred/user.h>
#include <stdint.h>




#define CAP_READ      (1 << 0)
#define CAP_WRITE     (1 << 1)

#define CAP_EXECUTE   (1 << 2)
#define CAP_SEEK      (1 << 3)
#define CAP_IOCTL     (1 << 4)

#define CAP_DUP       (1 << 5)

#define CAP_TRANSFER  (1 << 6)

#define CAP_RIGHTS_NONE      0

#define CAP_FILE_RIGHTS_MASK  (CAP_READ | CAP_WRITE | CAP_SEEK | CAP_IOCTL | CAP_DUP | CAP_TRANSFER)

/* Return codes */
#define CAP_OK            0
#define CAP_ERR_INVALID  -1
#define CAP_ERR_PERM     -2

#define CAP_OBJ_FILE   1

#define CAP_FLAG_NONE         0

#define CAP_FLAG_NO_INHERIT   (1 << 0)

/* Capability entry */
typedef struct _au_capability_ {

	void* object;


	uint8_t object_type;


	CapRights rights;


	UID_NUM owner;


	uint16_t flags;


	unsigned char valid;
}AuCapability;

extern int BordoisilaCapCreate(void* proc, int fd, void* object, uint8_t type, CapRights rights);
extern AuCapability* BordoisilaCapLookup(void* proc, int fd);
extern bool BordoisilaCapCheckRights(void* proc, int fd, CapRights required);
extern int BordoisilaCapRestrict(void* proc, int fd, CapRights new_rights);
extern void BordoisilaCapDestroy(void* proc, int fd);
extern void BordoisilaCapCleanupProcess(void* proc);
extern void BordoisilaCapInheritTable(void* parent, void* child);

#endif
