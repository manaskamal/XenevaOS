#ifndef _LINUX_PCI_IDS_H
#define _LINUX_PCI_IDS_H

/*
 * DCL <linux/pci_ids.h> -- the two vendor entries this tree has a driver
 * for, in upstream's spelling.
 *
 * mainline's header is 2600 lines of vendor/subvendor constants. Only the
 * pair below is named by any file here: bochs.c matches the first entry of
 * its id_table on the whole quadruple (1234:1111, subsystem 1af4:1100) to
 * tell QEMU's stdvga apart from the Simics model, so the subvendor and
 * subdevice constants are the matches' identity, not decoration.
 *
 * Values are upstream's byte for byte (pci_ids.h:2592-2593 in linux-7.2.6);
 * the file grows by demand like the rest of DCL's headers.
 */

#define PCI_SUBVENDOR_ID_REDHAT_QUMRANET 0x1af4
#define PCI_SUBDEVICE_ID_QEMU            0x1100

#endif /* _LINUX_PCI_IDS_H */
