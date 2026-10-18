#ifndef __LINUX_PROPERTY_H__
#define __LINUX_PROPERTY_H__

/*
 * DCL <linux/property.h> -- the software/fwnode property readers, and
 * dev_fwnode().
 *
 * mainline's header is the interface to device tree, ACPI and software
 * nodes: reading "rs485-rts-delay" out of the DT, walking fwnode graphs,
 * fwnode_property_read_u32_array()'s many wrappers.  Xeneva has no firmware
 * description in the kernel at all -- a board's UART is compiled in -- so
 * there is nothing to read, and all three functions answer that.
 *
 * The answers are chosen by looking at what serial_core.c does with them
 * (uart_set_rs485_config, :3533-3559), rather than invented:
 *
 *   dev_fwnode()                NULL, so `if (!dev_fwnode(dev)) return 0;`
 *                                takes its early exit and never claims a
 *                                property exists.
 *   device_property_read_u32_array()  nonzero (-ENODATA), so the `if (!ret)`
 *                                that would apply rs485-rts-delay timings is
 *                                skipped -- and a half-applied pair of
 *                                delays would be worse than none.
 *   device_property_read_bool()  0, so SER_RS485_RX_DURING_TX is only ever
 *                                set by the flags a caller passes in, never
 *                                by a DT node that is not there.
 *
 * dev_fwnode() lives here rather than in device.h because that is where
 * mainline declares it (include/linux/property.h), and device.h includes this
 * header for it.
 *
 *   upstream  include/linux/property.h  (mainline v7.2)
 */
#include <linux/errno.h>	/* -ENODATA */
#include <linux/types.h>	/* u32 */

struct device;
struct fwnode_handle;

static inline struct fwnode_handle* dev_fwnode(const struct device* dev)
{
	(void)dev;
	return NULL;
}

static inline int device_property_read_u32_array(const struct device* dev,
						 const char* propname,
						 u32* values,
						 size_t nvalues)
{
	(void)dev;
	(void)propname;
	(void)values;
	(void)nvalues;
	return -ENODATA;
}

static inline int device_property_read_bool(const struct device* dev,
					    const char* propname)
{
	(void)dev;
	(void)propname;
	return 0;
}

#endif /* __LINUX_PROPERTY_H__ */
