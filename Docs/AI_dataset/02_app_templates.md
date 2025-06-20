# 02_app_templates.md - Prompt-to-App Code Templates

This file maps natural language prompts to real-world XenevaOS C code examples. Use this dataset to power AI prompt-generation tools, developer onboarding, or LLM fine-tuning for automated coding assistants.

---

## 🧩 Basic Window

**Prompt:** Create a 400x300 window with the title "Hello Xeneva"

```c
#include <chitralekha/window.h>

int main() {
    Window* win = create_window(400, 300, "Hello Xeneva");
    show_window(win);
    return 0;
}
```

---

## 🔘 Button with Click Handler

**Prompt:** Add a button that prints "Clicked!" to the terminal

```c
#include <chitralekha/window.h>
#include <chitralekha/button.h>

void on_click_handler() {
    printf("Clicked!\n");
}

int main() {
    Window* win = create_window(500, 400, "Button App");
    Button* btn = create_button(win, "Click Me", 40, 40);
    on_click(btn, on_click_handler);
    show_window(win);
    return 0;
}
```

---

## 🧮 Calculator UI (Add Two Numbers)

**Prompt:** Create a UI with 2 textboxes and a button to add them

```c
#include <chitralekha/window.h>
#include <chitralekha/button.h>
#include <chitralekha/textbox.h>

Textbox *tb1, *tb2, *result;

void add_numbers() {
    int a = atoi(textbox_get(tb1));
    int b = atoi(textbox_get(tb2));
    char buf[32];
    sprintf(buf, "%d", a + b);
    textbox_set(result, buf);
}

int main() {
    Window* win = create_window(400, 300, "Calc");
    tb1 = create_textbox(win, 20, 30, 100, 30);
    tb2 = create_textbox(win, 20, 70, 100, 30);
    result = create_textbox(win, 20, 150, 100, 30);
    Button* btn = create_button(win, "Add", 150, 50);
    on_click(btn, add_numbers);
    show_window(win);
    return 0;
}
```

---

## 🖼 Static Dashboard

**Prompt:** Make a window showing 3 labels for CPU, RAM, and Disk

```c
#include <chitralekha/window.h>
#include <chitralekha/label.h>

int main() {
    Window* win = create_window(500, 300, "System Stats");
    create_label(win, "CPU: 3.2GHz", 20, 30);
    create_label(win, "RAM: 2048MB", 20, 70);
    create_label(win, "Disk: 128GB", 20, 110);
    show_window(win);
    return 0;
}
```

---

## 💬 Chat-Like Message Interface

**Prompt:** Build a chat interface with input textbox and send button

```c
#include <chitralekha/window.h>
#include <chitralekha/button.h>
#include <chitralekha/textbox.h>

Textbox* input;

void send_message() {
    printf("User says: %s\n", textbox_get(input));
    textbox_set(input, "");
}

int main() {
    Window* win = create_window(480, 320, "Chat UI");
    input = create_textbox(win, 20, 230, 300, 30);
    Button* send = create_button(win, "Send", 340, 230);
    on_click(send, send_message);
    show_window(win);
    return 0;
}
```

---

## 🎨 Paint / Canvas App (Concept)

**Prompt:** Make a window where clicking paints a dot

```c
#include <chitralekha/window.h>
#include <chitralekha/canvas.h>

void draw_dot(int x, int y) {
    canvas_draw_circle(x, y, 4, 0xFFFFFF);
}

int main() {
    Window* win = create_window(500, 400, "Paint");
    Canvas* c = create_canvas(win, 0, 0, 500, 400);
    canvas_on_click(c, draw_dot);
    show_window(win);
    return 0;
}
```

---

## 📥 Submit Forms (Future Use)

**Prompt:** UI form with username and password textbox + submit button

```c

```

### 🧮 Calculator App (Real XenevaOS Example)

Prompt:
> Create a calculator app with digit buttons, operator buttons (+, -, ×, ÷), and an output display. Should update the display in real time and support clear and backspace actions.

Completion:
```c
#include <chitralekha/window.h>
#include <chitralekha/button.h>
#include <chitralekha/font.h>
#include <calculator.h>  // Contains CalculatorDisplay struct

void EqualAction(ChWidget* wid, void* ctx) {
    CalculatorDisplay* calc = (CalculatorDisplay*)ctx;
    CalculatorProcess(calc);  // performs the actual calculation
}

int main() {
    ChitralekhaStartApp();

    ChWindow* win = ChCreateWindow(400, 500, "Calculator");
    CalculatorDisplay* display = CalcCreateDisplay(win);
    CalculatorCreateButtonGrid(win, display);

    PostEvent e;
    while (true) {
        if (PostBoxPollEvent(&e)) {
            WindowHandleMessage(win, &e);
        }
    }

    return 0;
}
```

This is an actual example from `XenevaOS/Process/Calculator`, demonstrating modular UI, widget interaction, and display rendering using Chitralekha.

---

### 📆 Calendar App (Real XenevaOS Example)

Prompt:
> Create a dynamic calendar app with monthly views, current date highlighting, and navigation using keyboard.

Completion:
```c
#include <chitralekha/window.h>
#include <widgets/button.h>
#include <sys/_ketime.h>

int current_month, current_year;
uint8_t* calenderBuffer;
ChWindow* mainWin;

void CalenderRepaint(int year, int month, int num_days);
void CalenderHandleKeys(int c) {
    if (c == 'W') current_month--;
    if (c == 'S') current_month++;
    CalenderRepaint(current_year, current_month, 31);
}

int main() {
    app = ChitralekhaStartApp(argc, argv);
    mainWin = ChCreateWindow(app, WINDOW_FLAG_MOVABLE, "Calendar", 100, 100, 400, 400);
    calenderBuffer = malloc(sizeof(DateBox) * 42);
    _KeGetCurrentTime(&t);
    current_month = t.month; current_year = t.year;
    CalenderRepaint(current_year, current_month, 31);
    while (1) {
        PostEvent e; _KeFileIoControl(app->postboxfd, POSTBOX_GET_EVENT, &e);
        WindowHandleMessage(&e);
    }
}
```

This is an actual example from `XenevaOS/Process/Calender`, showcasing real-time date rendering, keyboard-driven navigation, and dynamic UI layout using the Chitralekha framework.

---

### 🎧 AudioPlayer App (Real XenevaOS Example)

Prompt:
> Create a native audio player app that decodes and plays back an MP3 file from disk.

Completion:
```c
#include <libc.h>
#include <minimp3.h>

int main() {
    int fd = open("/media/test.mp3", 0);
    uint8_t buf[4096];
    mp3_decoder_t decoder = mp3_create();
    while (int len = read(fd, buf, 4096)) {
        short pcm[4096];
        mp3_decode(decoder, buf, len, pcm, NULL);
        play_sound(pcm, len);
    }
    mp3_free(decoder);
    close(fd);
    return 0;
}
```
This is an actual example from `XenevaOS/Process/AudioPlayer`, demonstrating real-time MP3 decoding, memory-buffered playback, and integration with the kernel’s audio subsystem using userland libraries like `minimp3`.


---

This file acts as the “playbook” for your AI assistant to turn dev requests into working apps.
