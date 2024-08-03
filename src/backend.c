#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "ui.h"

void draw_rectangle(int x, int y, int w, int h, ui_color color) {
    DrawRectangleLines(x, y, w, h, (Color){.r = color.r, .g = color.g, .b = color.b, .a = color.a});
}

void fill_rectangle(int x, int y, int w, int h, ui_color color) {
    DrawRectangle(x, y, w, h, (Color){.r = color.r, .g = color.g, .b = color.b, .a = color.a});
}

void draw_text(const char *msg, int x, int y, int font_size, ui_color color) {
    DrawText(msg, x, y, font_size, (Color){.r = color.r, .g = color.g, .b = color.b, .a = color.a});
}

int get_text_width(const char *text, int font_size) {
    return MeasureText(text, font_size);
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
    if(IsKeyDown(KEY_UP))
        ui_set_key(ctx, UI_KEY_UP);
    if(IsKeyDown(KEY_DOWN))
        ui_set_key(ctx, UI_KEY_DOWN);
    if(IsKeyDown(KEY_LEFT))
        ui_set_key(ctx, UI_KEY_LEFT);
    if(IsKeyDown(KEY_RIGHT))
        ui_set_key(ctx, UI_KEY_RIGHT);
    if(IsKeyDown(KEY_ENTER))
        ui_set_key(ctx, UI_KEY_ENTER);
    if(IsKeyDown(KEY_BACKSPACE))
        ui_set_key(ctx, UI_KEY_BACK);
}