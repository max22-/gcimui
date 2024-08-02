#include <raylib.h>
#include "ui.h"

void draw_rectangle(int x, int y, int w, int h, ui_color color) {
    DrawRectangleLines(x, y, w, h, (Color){.r = color.r, .g = color.g, .b = color.b, .a = color.a});
}

void fill_rectangle(int x, int y, int w, int h, ui_color color) {
    DrawRectangle(x, y, w, h, (Color){.r = color.r, .g = color.g, .b = color.b, .a = color.a});
}

