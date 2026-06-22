/**
 * WORK IN PROGRESS, Building Linux Compat Layer for drivers
 */
#ifndef __LINUX_KERNEL_H__
#define __LINUX_KERNEL_H__

#include <stdint.h>
#include <Hal/AA64/aa64cpu.h>
#include <Mm/kmalloc.h>
#include <_null.h>
#include <Hal/AA64/aa64lowlevel.h>
#include <linux/bitmap.h>
#include <linux/list.h>
#include <linux/usb/ch9.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/usb/otg.h>
#include <linux/usb/ch9.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t  __u8;
typedef uint16_t __u16;
typedef uint32_t __u32;
typedef uint64_t __u64;
typedef int8_t __s8;
typedef int16_t __s16;
typedef int32_t __s32;
typedef int64_t __s64;

typedef uint8_t __le8;
typedef uint16_t __le16;
typedef uint32_t __le32;
typedef uint64_t __le64;
typedef uint16_t __be16;
typedef uint32_t __be32;
typedef uint64_t __be64;


#define udelay(us)  AA64SleepUS(us)
#define mdelay(ms) AA64SleepMS(ms)
#define msleep(ms) mdelay(ms)
#define usleep(us) AA64SleepUS(us)

#define GFP_KERNEL 0
#define GFP_ATOMIC 1
#define GFP_DMA 2

#define kmalloc(size, flags) kmalloc(size)
#define kzalloc(size, flags) kcalloc(size)

#define container_of(ptr, type, member) \
     ((type*)((char*)(ptr)- offsetof(type,member)))

#define offsetof(type, member) \
     ((size_t)&((type*)0)->member)

#define IS_ENABLED(opt) (!!(opt))

typedef uintptr_t phys_addr_t;
typedef uintptr_t resource_size_t;
typedef uintptr_t dma_addr_t;
typedef uintptr_t io_addr_t;
typedef int spinlock_t;
typedef int mutex;

/** implementation needed */
#define spin_lock_init(l) do{} while(0)
#define spin_lock_irqsave(l,f) do{(f) = 0;}while(0)
#define spin_unlock_irqrestore(l, f) do{}while(0)
#define mutex_init(m)  do{} while(0)
#define mutex_lock(m)  do{}while(0)
#define mutex_unlock(m)  do{} while(0)

typedef uint64_t sector_t;
typedef uint64_t blkcnt_t;
typedef uint64_t loff_t;
typedef uint32_t uid_t;
typedef uint32_t gid_t;
typedef uint32_t dev_t;
typedef uint32_t ino_t;
typedef uint32_t nlink_t;

typedef unsigned int gfp_t;
typedef unsigned int fmode_t;
typedef unsigned int umode_t;
typedef int irqreturn_t;
typedef unsigned long irq_hw_number_t;
typedef unsigned long pgoff_t;
typedef unsigned long kernel_ulong_t;

#define IRQ_NONE 0
#define IRQ_HANDLED 1
#define IRQ_WAKE_THREAD 2

#define __iomem
#define __user
#define __kernel
#define __force
#define __nocast
#define __safe
#define __chk_user_ptr(x) (void)0
#define __chk_io_ptr(x) (void)0

#define EPERM 1
#define ENOENT 2
#define EIO 5
#define ENOMEM 12
#define EBUSY 16
#define ENODEV 19
#define EINVAL 22
#define ENOSPC 28
#define ETIMEDOUT 110
#define ENOTSUPP 524
#define MAX_ERRNO 4095
#define IS_ERR_VALUE(x) ((x) >= (uintptr_t)-MAX_ERRNO)

#define EDEADLK 35
#define ENAMETOOLONG 36
#define ENOLCK 37
#define ENOSYS 38
#define ENOTEMPTY 39
#define ELOOP 40
#define EWOULDBLOCK EAGAIN
#define ENOMSG 42
#define EIDRM 43
#define ENOSTR 60
#define ENODATA 61
#define ETIME 62
#define ENOSR 63
#define EREMOTE 66
#define ENOLINK 67
#define EPROTO 71
#define EMULTIHOP 72
#define EBADMSG 74
#define EOVERFLOW 75
#define EILSEQ 84
#define EUSERS 87
#define ENOTSOCK 88
#define EDESTADDRREQ 89
#define EMSGSIZE 90
#define ENOPROTOOPT 92
#define EPROTONOSUPPORT 93
#define ESOCKTNOSUPPORT 94
#define EOPNOTSUPP 95
#define EAFNOSUPPORT 97
#define EADDRINUSE 98
#define EADDRNOTAVAIL 99
#define ENETDOWN 100
#define ENETUNREACH 101
#define ENETRESET 102
#define ECONNABORTED 103
#define ECONNRESET 104
#define ECONNRESET 104
#define ENOBUFS 105
#define EISCONN 106
#define ENOTCONN 107
#define ESHUTDOWN 108
#define ETOOMANYREGS 109
#define ETIMEDOUT 110
#define ECONNREFUSED 111
#define EHOSTDOWN 112

#define readb(addr) (*(volatile u8*)((uint8_t*)addr))
#define readw(addr) (*(volatile u16*)((uint8_t*)addr))
#define readl(addr) (*(volatile u32*)((uint8_t*)addr))
#define writeb(v, addr) (*(volatile u8*)((uint8_t*)addr) = (v))
#define writew(v, addr) (*(volatile u16*)((uint8_t*)addr) = (v))
#define writel(v, addr) (*(volatile u32*)((uint8_t*)addr) = (v))

#define BITS_PER_LONG 64
#define BITS_PER_BYTE 8

#define BIT(n) (1UL << (n))
#define BIT_ULL(n) (1ULL << (n))
#define GENMASK(h, l) (((~0UL) << (l)) & (~0UL >> (BITS_PER_LONG-1-(h))))
#define GENMASK_ULL(h,l) (((~0ULL) << (l)) & (~0ULL >> (63-(h))))

typedef int64_t ktime_t;

#define NSEC_PER_USEC 1000LL
#define NSEC_PER_MSEC 1000000LL
#define NSEC_PER_SEC  1000000000LL
#define USEC_PER_MSEC 1000LL
#define USEC_PER_SEC  1000000LL
#define MSEC_PER_SEC  1000LL

#define KTIME_MAX ((ktime_t)~((uint64_t)1<<63))
#define KTIME_MIN (~KTIME_MAX - 1)
#define KTIME_SEC_MAX (KTIME_MAX / NSEC_PER_SEC)

static inline ktime_t ktime_set(long secs, unsigned long nsecs) {
    return (ktime_t)secs * MSEC_PER_SEC + (ktime_t)nsecs;
}

static inline ktime_t ktime_add(ktime_t a, ktime_t b) {
    return a + b;
}

static inline ktime_t ktime_sub(ktime_t a, ktime_t b) {
    return a - b;
}

static inline ktime_t ktime_add_ns(ktime_t a, uint64_t ns) {
    return a + (ktime_t)ns;
}

static inline ktime_t ktime_sub_ns(ktime_t a, uint64_t ns) {
    return a - (ktime_t)ns;
}

static inline ktime_t ktime_add_us(ktime_t a, uint64_t us) {
    return a + (ktime_t)(us * NSEC_PER_USEC);
}

static inline ktime_t ktime_add_ms(ktime_t a, uint64_t ms) {
    return a + (ktime_t)(ms * NSEC_PER_MSEC);
}

static inline int ktime_compare(ktime_t a, ktime_t b) {
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

static inline bool ktime_after(ktime_t a, ktime_t b) {
    return a > b;
}

static inline bool ktime_before(ktime_t a, ktime_t b) {
    return a < b;
}

static inline int64_t ktime_to_ns(ktime_t kt) {
    return (int64_t)kt;
}

static inline int64_t ktime_to_us(ktime_t kt) {
    return (int64_t)kt / NSEC_PER_USEC;
}

static inline int64_t ktime_to_ms(ktime_t kt) {
    return (int64_t)kt / NSEC_PER_MSEC;
}

static inline int64_t ktime_to_s(ktime_t kt) {
    return (int64_t)kt / NSEC_PER_SEC;
}

static inline ktime_t ns_to_ktime(uint64_t ns) {
    return (ktime_t)ns;
}

static inline ktime_t us_to_ktime(uint64_t us) {
    return (ktime_t)(us * NSEC_PER_USEC);
}

static inline ktime_t ms_to_ktime(uint64_t ms) {
    return (ktime_t)(ms * NSEC_PER_MSEC);
}

static inline ktime_t ktime_sub_us(ktime_t a, uint64_t us) {
    return a - (ktime_t)(us * NSEC_PER_USEC);
}

static inline ktime_t ktime_us_delta(ktime_t later, ktime_t earlier) {
    return ktime_to_us(ktime_sub(later, earlier));
}

static inline int64_t ktime_ms_delta(ktime_t later, ktime_t earlier) {
    return ktime_to_ms(ktime_sub(later, earlier));
}

static inline ktime_t ktime_get(void) {
    uint64_t cnt = get_cntpct_el0();
    uint64_t freq = get_cntfrq_el0();

    return (ktime_t)(cnt / (freq / NSEC_PER_SEC));
}

static inline ktime_t ktime_get_mono_fast_ns(void) {
    return ktime_get();
}

static inline uint64_t ktime_get_ns(void) {
    return (uint64_t)ktime_get();
}

#if defined(_MSC_VER)
#define WARN_ON(cond) \
     ((cond) ? (UARTDebugOut("[WARN] %s:%d %s \r\n", __FILE__, __LINE__, #cond)))
#elif defined(__GNUC__) && defined(__clang__)
#define WARN_ON(cond) ({                               \
int __c = !!(cond);                                    \
if (__c){                                              \
UARTDebugOut("[WARN] %s:%d \r\n", __FILE__, __LINE__); \
}                                                      \
})
#endif

#endif



