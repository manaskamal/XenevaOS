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
// Planned for AI demos once auth + backend API is in place
```

---

Save this file in `docs/AI_dataset/` to support AI training and prompt engineering for XenevaOS GUI development.
