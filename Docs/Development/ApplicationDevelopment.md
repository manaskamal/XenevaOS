# User Application Development for XenevaOS
Applications in XenevaOS are Xeneva Apps or _XEApps_ for short. The XenevaOS ecosystem is incomplete without user applications. Xeneva applications are the core of the operating system, enabling users to perform essential tasks. While the OS provides the foundational services—such as memory management, process scheduling, and hardware abstraction—user applications define the actual functionality and usability of the system. Without Xeneva applications, XenevaOS remains just a technical framework, lacking practical purpose.

## About this Guide
This documentation provides a step-by-step overview and approach to building applications for XenevaOS. Whether you are developing graphical user applications, system utilities, or multimedia software, this guide will walk you through:
- Understanding XenevaOS APIs and Libraries
- Setting up the Development Environment 
- Creating GUI and CLI Applications
- Interfacing with System Components (***Graphics, Audio, Networking, Filesystem, etc***)

## XenevaOS APIs and Libraries

Xeneva Applications are divided into three categories: _**(i) System Services**_, _**(ii) GUI Applications**_, and _**(iii) Terminal Applications & Utilities**_. 
- ***System Services***: System services run in the background to handle essential system tasks. Users cannot interact with system services directly as they do not have a GUI or TUI interface attached to them. Instead, they can be controlled through an external GUI control system or a Terminal application that uses IPC to communicate with the service. For example: _Net Manager_, _Deodhai Audio Server_, etc.

- ***GUI Applications***: GUI Applications are the applications that users can directly interact with. A XenevaOS GUI application uses the ___Chitralekha Graphics and Widget Library___ as its base. For example: _The Namdapha Desktop Environment_, _Deodhai Window Compositor_, _The File Manager_, etc.

- ***Terminal Applications & Utilities***: Terminal Applications are applications that have attached TTY pipes, through which they communicate to and from the Xeneva Terminal subsystem. These running applications are only visible through a running instance of the Xeneva Terminal UI. For example: _ping_, _play_, etc.

The application environment uses two fundamental libraries called _XEClib_ and _XECrt_. XEClib is the C library used for applications, and XECrt is used as the C runtime library. XECrt is responsible for setting up the basic stack and defining necessary C/C++ runtime functions before jumping to the application entry point. XEClib contains all standard C functions that are used by other libraries and applications. 

### Chitralekha Graphics and Widget Library
The most important and widely used library for GUI application development is the _Chitralekha Graphics and Widget Library_. This library defines fundamental graphics drawing and windowing system functions. The name ___'Chitralekha'___ is derived from Assam's mythological story; we will discuss that in later sections.

#### ***How Chitralekha Works?***
Whenever a GUI application is started, Chitralekha is responsible for communicating with the Window Manager and bringing up a new window for the application. Communication is done through Xeneva's own IPC mechanism called PostBox IPC. The Chitralekha library is dynamically linked to GUI applications. Here is a step-by-step breakdown of how a GUI application starts within Xeneva:
- ***ChBase Object*** — Responsible for creating and initializing primary usable data such as the application's default font, postbox IPC file index, creating a new postbox for the application, and creating a new Chitralekha App instance where all necessary data is stored.
- ***ChWindow Object*** — Responsible for requesting a window of the given size from the Window Manager. Whenever a new application is launched, two types of windows are created: _(i) Server Window_ and _(ii) Client Window_. A portion of the Server Window with necessary information is shared with the Client Window, meaning it can be accessed from the application's process memory. In other words, it uses a Xeneva Shared Memory object to share the portion of the Server Window with the client application. Here is the structure of the shared portion of the Server Window:

```c
typedef struct _sh_win_ {
    ChRect rect[256];   /* used to tell the server about the 
    dirty rects */
    uint32_t rectCount;     /*used to tell the server how many total dirty rects are there */
    bool dirty;   /*mark if the window is dirty and needs to be updated */
    bool updateEntireWindow; /* mark if the entire window needs to be updated */
    
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
- ***ChWidgets Object*** — ChWidget objects are small objects that help the application provide a useful interface to the user. In other words, the ChWidget object is the base class for all types of widgets that are used by the user to perform meaningful tasks within the application. For example, a user clicks a button to launch a floating window, or chooses an on/off button to toggle a setting. ChWidget acts as a base for all types of widgets that share common properties. For example, ChButton and ChRadioGroup share common properties because both widgets are based on the ChWidget object, which includes fundamental properties that make them widgets.

- ***Application Event Loop*** — The application event loop runs continuously to poll its postbox for any external events. Whenever an event message arrives at its postbox, the event loop immediately passes that event to the Application Event Handler. For example, the Window Manager continuously posts mouse events to each application window under which the mouse pointer arrives. Each window's application handles those mouse events and takes appropriate action. 

These four fundamental components make a Xeneva GUI application work.

### Simple GUI Code Snippet
```c
#include <stdint.h>
#include <chitralekha.h>
#include <sys/kefile.h>
#include <sys/iocodes.h>
#include <widgets/base.h>
#include <widgets/button.h>
#include <widgets/window.h>

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

   //setjmp is used, in case a subwindow gets closed,
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

## Application Event Loop
When an application is newly created, it gets a ``handle_id`` from the Window Manager. The ``handle_id`` is specific to each window; i.e., if an application has two windows created, it will have two unique ``handle_id`` values, one for each window. Applications continuously wait for new events during their lifetime, either from the Window Manager, from external sources in the Operating System, or from the application itself. These events can include user interactions such as keyboard input, mouse movements, touch gestures, hand gesture movements, or system notifications. In XenevaOS, events are sent or received through the PostBox IPC kernel module, which is a message queue-based IPC designed for the Xeneva GUI system. Each GUI event received includes the ``handle_id`` attached to it, which the application can use to locate the desired window. The event loop typically retrieves events, dispatches them to the appropriate window, and puts the application into a paused state until the next event arrives. Each application creates its own PostBox within the PostBox IPC module.<br><br>
When a new application is created, ``ChitralekhaStartApp(argc, argv)`` automatically creates a new PostBox for the application and stores the ***file descriptor to the PostBox*** within the ``ChitralekhaApp`` data structure. PostBox IPC can be controlled via _``KeFileIoControl(app->postboxfd, code, &ptr_to_ev_datastruct)``_. Here are some predefined control codes within the PostBox IPC module:

| Code Name | Value in Integer | Description |
|-----------|------------------|-------------|
| ``POSTBOX_CREATE`` | 401 | Creates a new postbox; automatically created by ``ChitralekhaStartApp(argc, argv)``. |
| ``POSTBOX_DESTROY`` | 402 | Destroys a postbox within the PostBox IPC module; the application can no longer use the postbox after executing this command. |
| ``POST_PUT_EVENT`` | 403 | Puts an event message in the application's own postbox. |
| ``POSTBOX_GET_EVENT`` | 404 | Checks for events within its own postbox. |
| ``POSTBOX_CREATE_ROOT`` | 405 | Only usable by the Window Manager; applications cannot use this command. |
| ``POSTBOX_GET_EVENT_ROOT`` | 406 | Only usable by the Window Manager; applications cannot use this command. | 

## Event Structure Format

PostEvent defines the core event message structure used by the PostBox IPC to describe the event occurred for a particular application. It encapsulates essential routing information, including the event type ``type``, destination thread ID ``to_id``, and source thread ID ``from_id``, followed by a payload region. The payload consists of multiple 32-bit data fields (``dword`` through ``dword8``) for parameter passing, along with pointer-based fields (``pValue, pValue1``) and character buffers for transferring structured or textual data. This structure allows a single event format to represent a wide range of GUI, system, and application-level events while maintaining a compact and efficient memory layout. 

Layout of the PostEvent structure:

```c
typedef struct _post_event_ {
    uint8_t type;
    uint16_t to_id;
    uint16_t from_id;
    uint32_t dword;
    uint32_t dword2;
    uint32_t dword3;
    uint32_t dword4;
    uint32_t dword5;
    uint32_t dword6;
    uint32_t dword7;
    uint32_t dword8;
    uint32_t* pValue;
    uint32_t* pValue1;
    char* charValue;
    unsigned char* charValue2;
    char charValue3[100];
}PostEvent;
```

| Field name | Description |
|------------|-------------|
| _``type``_ | Type of this event, for example ``DEODHAI_REPLY_MOUSE_EVENT`` which expands to the integer value 151. |
| _``to_id``_ | Destination thread's ID number. |
| _``from_id``_ | Source thread's ID number. |
| _``dword-dword8``_ | General purpose 32-bit parameter passing fields. |
| _``pValue-pValue1``_ | Used for passing pointer values. |
| _``charValue``_ | Used for passing string values. |
| _``charValue2``_ | Used for passing string and single-byte aligned pointer values. |
| _``charValue3``_ | Used for passing textual data. |

Using _``_KeFileIoControl(postboxfd, POSTBOX_GET_EVENT, &event)``_, the application can receive the event message directly into the pointed memory structure.








