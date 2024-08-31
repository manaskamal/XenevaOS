## Setting up Virtual Machine for Xeneva

## Notice

Xeneva is well tested only on VMware Workstation and Virtual Box with vmdk file that maps PhysicalDisk (_The USB Flash drive containing all required binaries_). Refer to [Setting up USB Flash Drive](BuildInstructions.md) for more information. Every thing should be set up as mentioned in this document. ***You can skip to [Virtual Machine Creation](#creating-the-virtual-machine-in-virtual-box) if you have downloaded the release vhd file***.

## Creating the .vmdk file 

In Virtual Box's VMs we will use .vmdk file in AHCI Controllers Port 1. To Create vmdk we will use Virtual Box __VBoxManage.exe__ tool present in location ```C:\Program Files\Oracle\VirtualBox```. You can add this location to Windows Environment Variable __PATH__ for accessing the tool from any location inside your local Machine.<br>
- Check for Drive Number of USB Flash Drive using ```diskmgmt.msc```. On Linux or macOS, you can use ```lsblk``` or ```diskutil list``` respectively <br>
- Start ```cmd.exe``` in _Run as administrator_ mode <br>
- Type ```VBoxManage createmedium -filename "Your\path\to\filename.vmdk" --variant RawDisk --format=VMDK --property RawDrive=\\.\PhysicalDriveX```<br>
Repace ```X``` with your drive number of USB Flash drive
- Example here:
```VBoxManage createmedium -filename "E:\usb.vmdk" --variant RawDisk --format=VMDK --property RawDrive=\\.\PhysicalDrive1```. Here PhysicalDrive1 is my USB Flash Drive<br>
- Upto here you should be able to create __.vmdk__ file mapped to Physical Drive 

## Creating the Virtual Machine in Virtual Box
- Create a new Virtual Machine in Virtual Box. ```{Machine - New}```
- Give a suitable name to the VM and select ```Linux``` from Type and ```Ubuntu (64-bit)``` from Version and click __Next__
- Set Base Memory to __2048 MB__ and _Processor Count to 3_ and check the _Enable EFI(special OSses only)_ and click Next
- Select __Do Not Add a Virtual Hard Disk__ option and Click Next
- Click __Finish__ to complete the process.
- Upto here the new Virtual Machine should appear inside Virtual Box. 
- Now Open Settings of newly created Virtual Machine
- Go to System and select __ICH9__ from the _Chipset_ , __USB Tablet__ from the _Pointing Device_ and __Enable I/O APIC__ from _Extended Features_
- Go to Storage and _Add a controller_ of __AHCI(SATA)__. This should create a new controller in the _Storage Device_ list.
- Now _Add a Hard disk_ inside newly created Controller then click _Add_ button from Hard Disk Selector window and select the __vmdk__ that is created using the steps mentioned above, or directly the downloaded __vhd__ file, if you have downloaded Xeneva from official release page.

- Go to _Audio_ and Windows Direct Sound in Host Audio Driver and Intel HD Audio from Audio Controller and check _Enable Audio Output_ and _Enable Audio Input_
- Go to USB and select _Enable USB Controller_ and select _USB 3.0 (XHCI) Controller_
- Upto here XenevaOS is ready to run

## Using Xeneva Terminal (_only for v1.0_)
When the OS boots up it automatically starts the Window Manager and launches Xeneva Terminal application. Desktop should be started manually. Follow the steps to start the desktop
- Type ```xelnch``` and press _Enter_, this starts the _AppTray_ process
- Send a signal ```CTRL+C``` to the terminal. 
- Type ```nmdapha``` and press _Enter_, to start the Namdapha Desktop bar
- Now you should able open the _AppTray_ by clicking the Xeneva Logo in the Namdapha Bar 
- Now launch applications from the _AppTray_

## Starting _play_ application to play audio
XenevaOS has builtin play application which can play audio files of 48kHZ - 16bit format .wav files
- In Xeneva Terminal type ```play -file /music/folk.wav``` this will play the Xeneva default music file present in the music folder
- You can send signal to the Terminal to bring back the shell input

## Conclusion
Currently components of Xeneva should be started separately, automatic starting of components is not supported.You can attach a serial terminal to print Xeneva's internal messages which helps debugging the system on various crashes. See [How to setup a serial Terminal for Xeneva](SerialTerminalAttach.md) for more information
