#include <raylib.h>
#include <cstdio>
#include "ui.h"
#include "backend.h"

#define TFT_WIDTH 320
#define TFT_HEIGHT 240

UI::Context& ui = UI::Context::get();

int page = 0;

void menu() {
    if(ui.button("page 1"))
        page = 1;
    static uint16_t x;
    ui.nextline();
    ui.input_u16("test", &x);
}

void page1() {
    int rows = 20, cols = 5;
    char label[32];
    for(int x = 0; x < cols; x++) {
        ui.begin_container("column");
        for(int y = 0; y < rows; y++) {
            snprintf(label, sizeof(label), "Button %d", y * cols + x);
            if(ui.button(label))
                printf("%s clicked!\n", label);
            ui.nextline();
        }
        ui.end_container();
    }
    ui.nextline();
    static bool checked = false;
    ui.checkbox("checkbox", &checked);
    if(checked) {
        if(ui.button("hidden button")) {
            printf("hidden button clicked!\n");
            page = 0;
        }
    }
}


int main(void) {
    InitWindow(TFT_WIDTH, TFT_HEIGHT, "ui");
    SetTargetFPS(60);
    ui.init(TFT_WIDTH, TFT_HEIGHT);
    while (!WindowShouldClose()) {
        handle_keys(ui);
        BeginDrawing();
        ui.begin_frame();
        ClearBackground(BLACK);
        ui.begin_container("root");
        switch(page) {
            case 0:
                menu();
                break;
            case 1:
                page1();
                break;
        }
        ui.end_container();
        ui.end_frame();
        EndDrawing();
    }
    CloseWindow();
    return 0;
}