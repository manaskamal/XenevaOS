#ifndef __LINUX_MODULE_H__
#define __LINUX_MODULE_H__

#define module_init(func)
#define module_exit(func)

#define MODULE_LICENSE(x)
#define MODULE_AUTHOR(x)
#define MODULE_DESCRIPTION(x)
#define MODULE_VERSION(x)
#define MODULE_ALIAS(x)
#define MODULE_DEVICE_TABLE(type, table)
#define MODULE_INFO(info, val)
#define THIS_MODULE ((void *)0)


#define try_module_get(m)   (1)
#define module_put(m)       do {} while (0)

#endif
