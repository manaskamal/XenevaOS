# Populating Runtime Configuration File

Runtime configuration are the files that are loaded by kernel to get description about required system files to load. For example available driver files available in this build image and how they are relatable to each hardware device. 

## Aurora Driver Configuration file 
Aurora Driver configuration file is divided into two type. A PCIe based configuration file and board based configuration file. Board based configuration file need to be modified acoording to the target board for example imx8mp based board needs modification according to the hardware it has. And PCIe based configuration file can hold any driver file along with its PCIe subclass code/class code or vendorID/deviceID entry.

## audrv.cnf file 
This file must be present in root folder of ramdisk image. It holds PCIe based driver file entries. For example:
```
(1)[03,128]/virtgpu.dll |
(2)[02,00]/virtnet.dll |
(3)[6900,4185]/virtsnd.dll |
(0)
```
Here virtgpu.dll holds its _[classCode,subClassCode]_ value, while virtsnd.dll holds its _[vendorID/deviceID]_ value. Please not that entry numbers are in integer format, not hexa-decimal format. Again file path must be mentioned properly like "virtgpu.dll" will skip this entry to be loaded by Aurora Driver Manager, but "/virtgpu.dll" entry will be successfully loaded by Aurora Driver Manager because virtgpu.dll is in root folder of the ramdisk. 
### Marker Points:
As you noticed there are certain marker points present in the configuration file. There are three types of marker point, the numeric bullets, [class/subclass] holder and the _end_of_entry_ marker. Numeric bullets starts from (1) and ends at (0) which means no more further entry Driver Manager can skip this entire file. ClassSubClass holder holds either sub class and class code value of PCIe configuration of each device or vendor ID/device ID value of each device. And _END_OF_ENTRY_  _"|"_ marker tells the end of an entry so that Driver Manager can skip to the next entry. 

## board.cnf file

Board configuration file follows little different format than audrv file. This file also needs to be present in the root folder of ramdisk. 
