# XenevaOS Build Instructions

## Prerequisites

<hr>
<br>

Before Building XenevaOS, make sure all the dependencies are installed.  
XenevaOS is build purely in Windows environment.
<br>
<br>

## Visual Studio 2019 Community edition

Visual Studio 2019 Community edition is required to open and edit Xeneva projects. Community edition is free edition of Microsoft Visual Studio. One can also use any paid edition of 2019. If later version of Visual Studio is used projects needs to be configured manually to meet compiler version requirement. This is done by going to : <br> 
```Project -> Properties -> General -> Platform Toolset```<br>
<br>

## Netwind Assembler (Nasm) <br>
<hr>
Download and install Netwind Assembler (nasm). Nasm needs to be integrated with Visual Studio to compile assembly source code. This can be done by following steps: <br>
```Goto Resources dir in XenevaOS repository then copy and extract nasmprops.zip.```<br><br>
```Copy "nasm.props", "nasm.targets", "nasm.xml" from nasmprops.zip to C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Microsoft\VC\v160\BuildCustomizations```
<br>
<br>

## _Setting up USB Flash Drive_ <br><hr>
Format an USB Flash drive (Minimum 2 GiB space required) with <br>
1> GPT partition scheme <br>
2> FAT-32 filesystem with cluster size 4096<br><br>

## _Setting up Environemt Variables_<hr>

Add an User Environment Variable with __" Variable Name : XENEVA_BUILDS "__ and __" Variable Value "__ with the location of the USB Flash Drive. It can be done simply by clicking __"Browse Directory"__ button. 
<br>

## _Copying required resources_ <br><hr>
Go to __Resources\resources__ directory in the repository and copy all the resource files as it is, to __"XENEVA_BUILDS"__ PATH.<br><br>

## Building the Solution<hr>

Before opening the solution please make sure that Netwind Assembler is installed and configured as mentioned above. Open _"Aurora.sln"_ file from the repository and everything should get automatically configured. Now you can:<br>
``Right Click on 'Solution Aurora' from Solution Explorer, then click "Rebuild Solution"`` <br>
This should rebuild entire solution and produce the binaries to _"Build"_ folder of the repository and copy it to __"XENEVA_BUILDS"__ PATH <br><br>

## More Information
<hr>
At this point you should see all the necessary files inside your "XENEVA_BUILDS" PATH for fully functioning VM for XenevaOS. You can visit [Testing environment setup instructions](VMSetup.md).





