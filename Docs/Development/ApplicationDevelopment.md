# User Application Development for XenevaOS
Applications in XenevaOS are Xeneva Apps or _XEApps_ in short. XenevaOS ecosystem is incomplete without user applications. As Xeneva applications are the core of the operating system, enabling users to perform essential tasks. While the OS provides the foundational services- such as memory management, process scheduling, and hardware abstraction- user applications define the actual functionality and usability of the system. Without Xeneva applications, XenevaOS remains just a technical framework, lacking practical purpose.

## About this Guide
This documentation provides a step-by-step overview and approach to building application for XenevaOS. Whether you are developing graphical user applications, system utilities, or multimedia software, this guide will walk you through:
- Understanging XenevaOS APIs and Libraries
- Setting up the Development Environment 
- Creating GUI and CLI Applications
- Interfacing with System Components (***Graphics, Audio, Networking, Filesystem, etc***)

## XenevaOS APIs and Libraries

Xeneva Applications are divided into three category: _**(i) System Service**_, _**(ii) GUI Application**_, _**(iii)Terminal Application & Utilities**_. 
- ***System Services*** : System services runs in background to handle essential system tasks. User cannot interact with system services as it doesn't have a GUI or TUI interface attached to it, rather it can be controlled through extern GUI control system or Terminal application that uses IPC to communicate with system services. For example -- _Net Manager_, _Deodhai Audio Server_.._etc_.

- ***GUI Application***: GUI Applications are the application that user can directly interact with. XenevaOS's GUI application uses ___Chitralekha Graphics and Widget Library___ as its base. For example, _The Namdapha Desktop Environment_, _Deodhai Window Compositor_, _The File Manager ...etc_.

- ***Terminal Application & Utilites***: Terminal Applications are application that has attached TTY pipes, through which it communicate to and from Xeneva Terminal subsystem. This running applications are only visible through a running instance of Xeneva Terminal UI. For example _ping_, _play..etc_.

Application environment uses two most fundamental library called the _XEClib_ and the _XECrt_. XEClib is the C library used for applications and XECrt is used as the C runtime library. XECrt is responsible for setting up the basic stack and defining necessary C/C++ runtime functions before jumping to application entry point. XEClib contains all standard C functions that are used by other libraries and the applications. 

### Chitralekha Graphics and Widget library:
Most important and used library for GUI application development is _Chitralekha Graphics and Widget library_. This library defines fundamental Graphics drawing functions and Windowing system functions. The name ___'Chitralekha'___ is derived from Assam's own mythological story, we'll discuss that in later sections.

#### ***How Chitralekha Works?***
Whenever a GUI application is started, Chitralekha is responsible for communicating to Window manager and bringing up a new window for the application. Communication is done through Xeneva's own IPC mechanism called PostBox IPC. Chitralekha library is dynamically linked to GUI applications and is a dynamic library. Here's a step by step breakdown of how a GUI application starts within Xeneva,
- ***ChBase Object*** -- Responsible for creating and initializing primary usable data's like Application's default font, postbox ipc file index and creating a new postbox for this application and creating a new Chitralekha App instance where all necessary data's are stored
- ***ChWindow Object*** -- Responsible  for requesting a window of given size from the Window Manager. Whenever new application is launched, two types of Windows are created, _(i) Server Window (ii)Client Window_. A portion of Server Window with necessary informations are shared with the Client Window, i.e it can be accessed from the application process memory. In other words, it uses Xeneva Shared Memory object to share the portion of Server window with the client application. Here's the structure of shared portion of the Server window:

``` 
typedef struct _sh_win_ {
    ChRect rect[256];   /* used to tell the server about the 
    dirty rects */
    uint32_t rectCount;     /*used to tell the server how many total dirty rect are there */
    bool dirty;   /*mark if the window is dirty and needs to be updated */
    bool updateEntireWindow; /* mark if the entire windows needs to be updated */
    
    int x;
    int y;
    int width;
    int height;
    bool alpha;
    bool hide;
    double alphaValue;
    bool windowReady;
}SharedWinInfo;
```






