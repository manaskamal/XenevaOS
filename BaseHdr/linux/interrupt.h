#ifndef __LINUX_INTERRUPT_H__
#define __LINUX_INTERRUPT_H__

/*
 * DCL <linux/interrupt.h> -- request_irq()/free_irq() over Xeneva's GIC.
 *
 * The GIC already keeps a per-SPI callback table (KernelAA64/Hal/gic.c:
 * GICRegisterSPIHandler / GICCallSPIHandler, typed void (*)(int spi)), so the
 * adapter is small: record (irq -> handler, dev_id) and install one dispatcher
 * as that SPI's handler. Bodies land in DCL/linux_irq_shim.c.
 *
 * IRQ numbers: this is where a faithful adapter earns its keep. On ARM64 the
 * GIC numbers SGIs 0-15, PPIs 16-31 and SPIs from 32, and Linux's irq number
 * on these boards is the HW irq, so the SPI id is irq - 32. A negative result
 * means somebody asked for an SGI/PPI -- a per-CPU line no serial driver owns
 * -- and is refused rather than quietly registered on the wrong vector.
 *
 * One handler per irq. IRQF_SHARED is accepted because 8250_port.c:2314 sets
 * it unconditionally for iotype PORT_UNKNOWN, but nothing in this tree shares
 * an irq yet: a fabricated share list would be the kind of quiet wrongness
 * that surfaces later as a lost interrupt. If a second driver ever asks, that
 * is the moment to grow it.
 *
 * irqreturn_t and IRQ_NONE/IRQ_HANDLED/IRQ_WAKE_THREAD are already in
 * <linux/kernel.h>, as are the delay primitives -- see <linux/delay.h>.
 */

#include <linux/kernel.h>	/* irqreturn_t, IRQ_NONE/IRQ_HANDLED */

typedef irqreturn_t (*irq_handler_t)(int irq, void* dev_id);

/* mainline include/linux/interrupt.h -- values, so flag words survive copy */
#define IRQF_TRIGGER_NONE   0x00000000
#define IRQF_TRIGGER_RISING 0x00000001
#define IRQF_TRIGGER_FALLING 0x00000002
#define IRQF_TRIGGER_HIGH   0x00000004
#define IRQF_TRIGGER_LOW    0x00000008
#define IRQF_TRIGGER_MASK   (IRQF_TRIGGER_HIGH | IRQF_TRIGGER_LOW | \
							IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING)
#define IRQF_SHARED         0x00000080

/* mainline: a nonzero handler result is "handled" unless it was IRQ_NONE */
#define IRQ_RETVAL(x) ((x) != IRQ_NONE)

int request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags,
				const char* name, void* dev_id);
void free_irq(unsigned int irq, void* dev_id);

void enable_irq(unsigned int irq);
void disable_irq(unsigned int irq);
void synchronize_irq(unsigned int irq);

/*
 * Bounds every port-irq validity check in serial_core.c:966 and
 * 8250_port.c:3144, so it must be larger than any irq we could ever hand out.
 * 1024 covers the full GICv3 SPI range with room to spare.
 */
int irq_get_nr_irqs(void);

#endif /* __LINUX_INTERRUPT_H__ */
