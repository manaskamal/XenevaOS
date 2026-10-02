#ifndef __LINUX_PRINTK_H__
#define __LINUX_PRINTK_H__

#include <Drivers/uart.h>
#include <aucon.h>

#define KERN_EMERG    ""
#define KERN_ALERT    ""
#define KERN_CRIT     ""
#define KERN_ERR      ""
#define KERN_WARNING  ""
#define KERN_NOTICE   ""
#define KERN_INFO     ""
#define KERN_DEBUG    ""

#define printk(fmt, ...) UARTDebugOut(fmt, ##__VA_ARGS__)

#define pr_emerg(fmt, ...)   printk(fmt, ##__VA_ARGS__)
#define pr_alert(fmt, ...)   printk(fmt, ##__VA_ARGS__)
#define pr_crit(fmt, ...)    printk(fmt, ##__VA_ARGS__)
#define pr_err(fmt, ...)     printk(fmt, ##__VA_ARGS__)
#define pr_warn(fmt, ...)    printk(fmt, ##__VA_ARGS__)
#define pr_notice(fmt, ...)  printk(fmt, ##__VA_ARGS__)
#define pr_info(fmt, ...)    printk(fmt, ##__VA_ARGS__)
#define pr_debug(fmt, ...)   do {} while (0)

#define dev_emerg(dev, fmt, ...)  printk(fmt, ##__VA_ARGS__)
#define dev_alert(dev, fmt, ...)  printk(fmt, ##__VA_ARGS__)
#define dev_crit(dev, fmt, ...)   printk(fmt, ##__VA_ARGS__)
#define dev_err(dev, fmt, ...)    printk(fmt, ##__VA_ARGS__)
#define dev_warn(dev, fmt, ...)   printk(fmt, ##__VA_ARGS__)
#define dev_notice(dev, fmt, ...) printk(fmt, ##__VA_ARGS__)
#define dev_info(dev, fmt, ...)   printk(fmt, ##__VA_ARGS__)
#define dev_dbg(dev, fmt, ...)    do {} while (0)

#define panic(fmt, ...) do { printk("PANIC: " fmt, ##__VA_ARGS__); while(1); } while(0)

#endif
