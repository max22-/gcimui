#include <raylib.h>
#include "ui.h"

void draw_rectangle(int x, int y, int w, int h, ui_color color) {
    DrawRectangleLines(x, y, w, h, (Color){.r = color.r, .g = color.g, .b = color.b, .a = color.a});
}

void fill_rectangle(int x, int y, int w, int h, ui_color color) {
    DrawRectangle(x, y, w, h, (Color){.r = color.r, .g = color.g, .b = color.b, .a = color.a});
}

void handle_keys(ui_ctx *ctx) {
    if(IsKeyDown(KEY_UP))
        ui_set_key(&ctx, UI_KEY_UP);
    if(IsKeyDown(KEY_DOWN))
        ui_set_key(&ctx, UI_KEY_DOWN);
    if(IsKeyDown(KEY_LEFT))
        ui_set_key(&ctx, UI_KEY_LEFT);
    if(IsKeyDown(KEY_RIGHT))
        ui_set_key(&ctx, UI_KEY_RIGHT);
    if(IsKeyDown(KEY_ENTER))
        ui_set_key(&ctx, UI_KEY_ENTER);
    if(IsKeyDown(KEY_BACKSPACE))
        ui_set_key(&ctx, UI_KEY_BACK);
}