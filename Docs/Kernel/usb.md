# USB System in Xeneva

The USB system in Xeneva is designed with modularity and flexibility in mind, which allows the communication between higher-level applications with USB devices and the Kernel with USB devices. The system is composed of three different layer running separately from each other: the __Host Controller Driver__, the __Core USB Driver__ and the __USB Class Drivers__. Each components are loaded separately by the kernel as module to ensure clean separation and ease of development.

## Architecture Overview

- **Host Controller Driver** : Host Controller Driver is responsible for directly interacting with the hardware. It handles tasks such as initializing the controller itself, managing port attach and dettach events, managing endpoint descriptors and transferring data to and from the USB devices to USB class drivers. This driver is separate from the USB core driver to maintain a clean abstraction layer.

- **USB Core Driver** : The USB core driver acts as a glue layer between the USB class drivers and the host controller driver. It manages device enumeration, commnication protocols, and standard USB operations. When a class driver requests a USB operation, the USB core driver forwards these request to the appropriate host controller driver (e.g. xHCI).

- **USB Class Driver** : USB class drivers implement functionalities specific to certain types of USB devices, such as mass storage devices, keyboards, or audio peripherals. These drivers rely on the USB core driver for performing operations like data transfers or sending control requests.

## USB Device Structure

```
#pragma pack(push,1)
typedef struct _usb_dev_ {
    void* data;
    uint8_t classCode;
    uint8_t subClassCode;
    uint16_t vendorID;
	uint16_t deviceID;
	int configValue;
	uint16_t usbVersion;
	uint8_t address;
	uint8_t protocol;
	void* descriptor;
	int numInterfaces;
	int numEndpoint;
	bool driverInitialized;
	void (*schedule_interrupt)(_usb_dev_* dev, void* ep, uint64_t buffer, void (*callback)(void* dev, void* slot, void* Endp));
	void (*control_transfer)(_usb_dev_* usbdev, const USB_REQUEST_PACKET* request, uint64_t buffer_addr, const size_t len);
	void (*bulk_transfer)(_usb_dev_* usbdev, uint64_t buffer, uint16_t data_len, void* ep);
	void (*get_device_desc)(_usb_dev_* dev, uint64_t buffer, uint16_t len);
	void (*get_string_desc)(_usb_dev_* dev, uint64_t buffer, uint16_t id);
	void (*get_config_desc)(_usb_dev_* dev, uint64_t buffer, uint16_t len, uint8_t id);
	usb_descriptor_t* (*get_descriptor)(_usb_dev_* dev, uint8_t type);
	void* (*get_endpoint)(_usb_dev_* dev, uint8_t ep_type);
	int (*get_max_pack_sz)(_usb_dev_* dev, void* ep);
	void (*set_config_val)(_usb_dev_* dev, uint8_t config_val);
	int (*poll_wait)(_usb_dev_* dev, int wait_type);
	usb_drv_entry ClassEntry;
	usb_drv_unload ClassUnload;
}AuUSBDevice;
#pragma pack(pop)
}
```

USB device struct is the main structure through which communication between USB class driver, USB core driver and USB host controller take place. Whenever new USB device connection happens in host controller, the host controller initializes the device and allocates a new USB device struct for being used by Core driver and class driver. 
