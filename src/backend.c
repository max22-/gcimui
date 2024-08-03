#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include "ui.h"

void ui_draw_rectangle(int x, int y, int w, int h, ui_color color) {
    DrawRectangleLines(x, y, w, h, (Color){.r = color.r, .g = color.g, .b = color.b, .a = color.a});
}

void ui_fill_rectangle(int x, int y, int w, int h, ui_color color) {
    DrawRectangle(x, y, w, h, (Color){.r = color.r, .g = color.g, .b = color.b, .a = color.a});
}

void ui_draw_text(const char *msg, int x, int y, int font_size, ui_color color) {
    DrawText(msg, x, y, font_size, (Color){.r = color.r, .g = color.g, .b = color.b, .a = color.a});
}

int ui_get_text_width(const char *text, int font_size) {
    return MeasureText(text, font_size);
}

unsigned long ui_millis() {
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

void handle_keys(ui_ctx *ctx) {
    ui_set_key_state(ctx, UI_KEY_UP, IsKeyDown(KEY_UP));
    ui_set_key_state(ctx, UI_KEY_DOWN, IsKeyDown(KEY_DOWN));
    ui_set_key_state(ctx, UI_KEY_LEFT, IsKeyDown(KEY_LEFT));
    ui_set_key_state(ctx, UI_KEY_RIGHT, IsKeyDown(KEY_RIGHT));
    ui_set_key_state(ctx, UI_KEY_ENTER, IsKeyDown(KEY_ENTER));
    ui_set_key_state(ctx, UI_KEY_BACK, IsKeyDown(KEY_BACKSPACE));
}