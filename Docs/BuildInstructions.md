# XenevaOS Build Instructions

## Prerequisites

Before Building XenevaOS, make sure all the dependencies are installed and all the steps are performed in sequence order mentioned in this document.<br>  
XenevaOS is build purely in Windows environment.
<br>
<br>

## Visual Studio 2019 Community edition

Visual Studio 2019 Community edition is required to open and edit Xeneva projects. Community edition is free edition of Microsoft Visual Studio. One can also use any paid edition of 2019. If later version of Visual Studio is used projects needs to be configured manually to meet compiler version requirement. This is done by going to : <br>
- Open Visual Studio and open the desired solution <br>
- Select the desired Project from Solution Explorer and right click <br> 
- ```Project -> Properties -> General -> Platform Toolset```<br>
<br>

## Netwide Assembler (NASM) 

Download and install Netwide Assembler (NASM) from [NASM Official website](https://www.nasm.us/). NASM needs to be integrated with Visual Studio to compile assembly source code. This can be done by following steps: <br>
- ```Goto Resources dir in XenevaOS repository then copy and extract nasmprops.zip.```<br>
- ```Copy "nasm.props", "nasm.targets", "nasm.xml" from nasmprops.zip to C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Microsoft\VC\v160\BuildCustomizations```
<br>
<br>

## _Setting up USB Flash Drive_ 
Format an USB Flash drive (Minimum 2 GiB space required) with GPT partition and FAT32 filesystem.
You can use __Refus__ to format an USB Flash drive (_by downloading Refus_) or ```diskpart``` to format the disk available by default on _Windows_
- Open ```cmd.exe``` with _Run as administrator_ mode
- type ```diskpart``` press Enter
- type ```list disk``` press Enter
- Identify your USB disk number from the list 
- type ```select diskX``` replace X with USB disk number
- type ```clean``` to clean the disk
- type ```convert gpt create partition primary``` press Enter
- type ```format fs=fat32 unit=4096 quick```
- type ```assign X``` replace 'X' with a drive letter

## _Setting up Environment Variables_
Required Environment Variables are: <br>
- __XENEVA_BUILDS__ with variable value of location USB Flash Drive <br>

Add an User Environment Variable with __" Variable Name : XENEVA_BUILDS "__ and __" Variable Value "__ with the location of the USB Flash Drive. It can be done simply by clicking __"Browse Directory"__ button. 
<br>

## _Copying required resources_ 
__Please make sure that all the files from resource directory inside XenevaOS repository is copied as it is to __XENEVA_BUILDS__ PATH (_i.e to the USB Flash Drive_) and make sure it is copied prior to build process.__ <br><br>
Xeneva requires resource files to run. Some important resource file are : <br>
- _"audrv.cnf"_ contains PCI/PCIe class-subclass code with driver file location path and required by the _'Kernel'_<br>
- _"ftlst.cnf"_ contains Fonts name and their location path<br>
- _"lnch.cnf"_ contains Application entry and their location path for __AppTray__ to recognise installed applications in order to display them in __AppTray__.<br><br>
Go to __Resources\resources\__ directory in the repository and copy all the resource files as it is, to __"XENEVA_BUILDS"__ PATH (_i.e to the USB Flash Drive_)<br><br>

# Building the Solution

Before opening the solution please make sure that Netwide Assembler (NASM) is installed and configured as mentioned above. Open _"Aurora.sln"_ file from the repository and everything should get automatically configured. 
- In the Solution Explorer, select the solution. It should be named as _'Solution Aurora'_ 
- ```Right Click on 'Solution Aurora' from Solution Explorer, then click "Rebuild Solution" ```<br>
- This should rebuild theentire solution and produce the binaries to _"Build"_ folder of XenevaOS repository<br>
- The Solution will automatically copy all the binaries to __"XENEVA_BUILDS"__ PATH (_i.e USB Flash Drive_) <br><br>

## More Information

At this point you should see all the necessary files inside your __"XENEVA_BUILDS"__ PATH. Now you need to setup VM for XenevaOS. You can visit [Testing environment setup instructions](VMSetup.md).





