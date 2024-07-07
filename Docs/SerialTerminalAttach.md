## Attaching a Serial Terminal 

Serial Terminal is essential for debugging the system while it's running. Serial debugging involves sending debug output from the system over the serial port to a connected terminal.
By default Xeneva kernel and userspace sends various debug output over serial port. Serial Terminal can be used to see debug output from Xeneva.

## NOTE
XenevaOS sends all kernel panic informations over serial port. It is necessary to setup serial debugging environment to get a meaningful insights of panic messages.

## Prerequisites

- _"vmwaregateway.exe"_ is a VMware utility often used for debuggin guest operating systems. You can get it from [here](https://www.l4ka.org/159.php).
- _HyperTerminal_ is a terminal emulation program that allows users to connect to remote systems using serial ports, modems, or TCP/IP networks. You can also use PuTTY, Tera Term or RealTerm alternative to _HyperTerminal_. 

## Starting up Serial Terminal
- Open command prompt on your host machine
- Run _'vmwaregateway.exe' with '/t' option <br>
```vmwaregateway.exe /t```
- Now open HyperTerminal and give a suitable name for
the session, for example "debug" and press 'OK'
- Now _'Connect to'_ dialogue should open up automatically, type __"localhost"__ in Host Address field
- Set Port Number to 567 and Connect Using to _TCP/IP(Winsock)_
- Now Hyper Terminal is ready to be used, to start serial debugging, follow the steps to connect virtual machine to the serial terminal

## Connecting to Serial Terminal in VMware 
- Make sure VMware workstation is installed on your host machine.
- Go to the virtual machine settings in VMware 
- Add a new serial port
- Select "Use Named Pipe".
- Type the name for the pipe, _"\\.\pipe\vmwaredebug"_
- Set the "Near end" to _"This end is the client"_
- Set the "Far end" to _"The other end is an application"_
- Click "OK" to finish the process

## Connecting to Serial Terminal in Virtual Box
- Make sure Virtual Box is installed on your host machine.
- Go to the virtual machine settings in Virtual Box
- Go to Serial Port and open the Port 1 tab (_by default Port 1 tab is opened_)
- Check the _'Enable Serial Port'_
- Set the Port Number to __'COM1'__ and Port Mode to __Host Pipe__
- Check _'Connect to existing pipe/socket'_
- Type _"\\.\pipe\vmwaredebug"_ in Path/Address

## Conclusion
Please make sure each steps are performed correctly to get Serial Terminal connected to the virtual machine. This is a reliable process to monitor XenevaOS's behavior and diagnose issues. Any kernel exceptions or panic will only be visible over Serial Terminal only. By setting it up properly it can give valuable insights of XenevaOS's debug outputs.

