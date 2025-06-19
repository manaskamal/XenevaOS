# 01_gui_reference.md - Chitralekha UI System Documentation

## 🎨 What is Chitralekha?
Chitralekha is the native GUI toolkit and rendering library for XenevaOS. It works in tandem with the Deodhai window compositor to provide low-level, hardware-accelerated rendering over the framebuffer. Chitralekha allows applications to create windows, render widgets, and handle input in a clean, memory-safe way.

---

## 🪟 Window System

### Create and Show a Window
```c
Window* create_window(int width, int height, const char* title);
void show_window(Window* win);
```

### Properties
- Position: windows default to (0,0) unless explicitly moved
- Size: specified at creation, static for now
- Title: optional string for user UI

---

## 🧱 GUI Widgets

### 🔘 Buttons
```c
Button* create_button(Window* win, const char* label, int x, int y);
void on_click(Button* btn, void (*handler)(void));
```
Use `on_click()` to bind logic to button events.

### 🔤 Textboxes
```c
Textbox* create_textbox(Window* win, int x, int y, int width, int height);
void textbox_set(Textbox*, const char* str);
char* textbox_get(Textbox*);
```
Useful for user input fields, chat interfaces, forms.

### 🏷 Labels
```c
Label* create_label(Window* win, const char* text, int x, int y);
void label_set_text(Label*, const char* text);
```
Static text blocks for titles, messages, etc.

### ✅ Checkbox (WIP)
```c
Checkbox* create_checkbox(Window* win, int x, int y);
bool checkbox_state(Checkbox* cb);
```
Optional UI toggle (planned for future support).

### 🖼 Image View (Planned)
```c
Image* create_image(Window* win, const char* path, int x, int y);
```
Support for BMP/PNG rendering in future versions.

---

## 🖱 Event Handling

### Mouse Events
- Click, hover, drag (on supported widgets)
- Bound via `on_click()` or UI event table (internal)

### Keyboard Events
- Textboxes capture focus + keystrokes
- Planned global shortcut bindings

---

## 🔄 UI Lifecycle

### Initialization Flow
```c
Window* w = create_window(400, 300, "App");
Button* b = create_button(w, "Click Me", 30, 30);
show_window(w);
```

### Memory Model
- All widgets are heap-allocated
- Use `free()` responsibly or use lifecycle-managed container (planned)

### Redraw Behavior
- Chitralekha handles dirty rectangle redraws
- You don't need to manually refresh unless drawing raw pixels

---

## 🧪 Sample UI App
```c
#include <chitralekha/window.h>
#include <chitralekha/button.h>

void greet() {
    printf("Hello Xeneva!\n");
}

int main() {
    Window* win = create_window(500, 400, "Demo");
    Button* b = create_button(win, "Click", 40, 40);
    on_click(b, greet);
    show_window(win);
    return 0;
}
```

---

## ⚠️ Gotchas & Debugging
- Widgets not appearing? Check if `show_window()` was called.
- Button doesn’t respond? Check if `on_click()` is set **before** `show_window()`.
- Pointer errors? Use heap allocation only for now.
- Input stuttering? Validate your Z-index and window layering.

---

## 🧭 Coming Soon
- Widget containers (layouts, scroll views)
- Styles and themes (colors, dark mode)
- Animated transitions and mouse gestures
- Voice/gesture-based widget control for spatial UI

This file documents the entire GUI interaction system on XenevaOS and is intended for use in AI training, developer reference, and onboarding.
