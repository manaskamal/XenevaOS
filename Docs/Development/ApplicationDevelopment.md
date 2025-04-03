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
- ***ChWindow Object*** -- Responsible  for requesting window of given size from the Window Manager. Whenever new application is launched, two types of Windows are created, _(i) Server Window (ii)Client Window_. A portion of Server Window with necessary informations are shared with the Client Window, i.e it can be accessed from the application process memory. In other words, it uses Xeneva Shared Memory object to share the portion of Server window with the client application. Here's the structure of shared portion of the Server window:

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
- ***ChWidgets Object ---*** ChWidget object are small objects that helps application provide valuable interface to user. In other words, ChWidget Object is base class of all types of widget that are used by user to make some meaningful task within the application. For example, user clicks a button to launch a floating window, user choosed an onoff button to turn on/off some settings. ChWidget act as a base for all type of widgets that share some common properties. For example, ChButton and ChRadioGroup will share some common properties because both the widgets are based on ChWidget object which includes fundamental properties, that makes it a widget.

- ***Application Event Loop---*** Application event loop runs continuously to poll its postbox for any outsource events. Whenever an event message arrives at its postbox, the event loop immediately pass that event to Application Event Handler. For example, the Window Manager continuously posts mouse event to each application window under which mouse pointer arrives. Each Window's application handles those mouse event and takes valuable action. 

This four fundamental component makes Xeneva GUI application working. 

### Simple GUI Code Snippet
```
#include <stdint.h>
#include <chitralekha.h>
#include <sys\_kefile.h>
#include <sys\iocodes.h>
#include <widgets\base.h>
#include <widgets\button.h>
#include <widgets\window.h>

ChitralekhaApp* app;
ChWindow* mainWin;

/*
 * WindowHandleMessage -- application event message
 * handler
 * @param e -- Pointer to PostEvent message
 */
void WindowHandleMessage(PostEvent *e){
    switch(e->type){

        // handle mouse event
        case DEODHAI_REPLY_MOUSE_EVENT:{
            ChWindowHandleMouse(mainWin, e->dword, e->dword2, e->dword3);
            memset(e, 0, sizeof(PostEvent));
            break;
        }

        // handle key event
        case DEODHAI_REPLY_KEY_EVENT:{
            int code = e->dword;
            ChitralekhaProcessKey(code);
            char c = ChitralekhaKeyToASCII(code);
            //handle the key event
            memset(e, 0, sizeof(PostEvent));
            break;
        }
    }
}

/*
 * Application's entry point
 */
int main(int argc, char* argv[]){
   app = ChitralekhaStartApp(argc, argv);
   mainWin = ChCreateWindow(app, WINDOW_FLAG_MOVABLE, "App Title", 100,100,500,400);

   //Create a button in 10-x position and 60-y position
   //with the size of 50 as width, 35 as height

   ChButton *button = ChCreateButton(10,60,50,35,"Hello World!");

   //now add the button widget to the mainWin that
   //we've created earlier

   ChWindowAddWidget(mainWin, (ChWidget*)button);

   //Now render the entire window on the application side
   //and tell the window manager to compose it on the 
   //display

   ChWindowPaint(mainWin);

   PostEvent e;
   memset(&e, 0, sizeof(PostEvent));

   //setjmp is used, incase if a subwindow get closed
   //the application jumps directly to the event loop's
   //instruction

   setjmp(mainWin->jump);
   while(1){
      int err = _KeFileIoControl(app->postboxfd,   POSTBOX_GET_EVENT, &e);
      WindowHandleMessage(&e);
      if (err == POSTBOX_NO_EVENT)
         _KePauseThread();
   }
}
```





