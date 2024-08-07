#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include "backend.h"

static Color to_raylib(UI::Color color) {
    Color c;
    c.r = color.r;
    c.g = color.g;
    c.b = color.b;
    c.a = color.a;
    return c;
}

void ui_draw_rectangle(UI::Rectangle<int> rect, UI::Color color) {
    DrawRectangleLines(rect.x, rect.y, rect.w, rect.h, to_raylib(color));
}

void ui_fill_rectangle(UI::Rectangle<int> rect, UI::Color color) {
    DrawRectangle(rect.x, rect.y, rect.w, rect.h, to_raylib(color));
}

void ui_draw_text(const char *msg, UI::Vec2<int> pos, int font_size, UI::Color color) {
    DrawText(msg, pos.x, pos.y, font_size, to_raylib(color));
}

int ui_get_text_width(const char *text, int font_size) {
    return MeasureText(text, font_size);
}

void ui_clip(UI::Rectangle<int> rect) {
    BeginScissorMode(rect.x, rect.y, rect.w, rect.h);
}

void ui_clip_end(void) {
    EndScissorMode();
}

unsigned long ui_millis(void) {
    return GetTime() * 1000;
}

void ui_error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    abort();
}


void handle_keys(UI::Context& ui) {
    ui.set_key_state(UI::KEY::UP, IsKeyDown(KEY_UP));
    ui.set_key_state(UI::KEY::DOWN, IsKeyDown(KEY_DOWN));
    ui.set_key_state(UI::KEY::LEFT, IsKeyDown(KEY_LEFT));
    ui.set_key_state(UI::KEY::RIGHT, IsKeyDown(KEY_RIGHT));
    ui.set_key_state(UI::KEY::ENTER, IsKeyDown(KEY_ENTER));
    ui.set_key_state(UI::KEY::BACK, IsKeyDown(KEY_BACKSPACE));
}