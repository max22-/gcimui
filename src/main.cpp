#include <raylib.h>
#include "ui.h"
#include "backend.h"
#include <stdio.h>

#define TFT_WIDTH 320
#define TFT_HEIGHT 240


int main(void) {
    InitWindow(TFT_WIDTH, TFT_HEIGHT, "ui");
    SetTargetFPS(60);
    UI::Context ui(UI::Vec2<int>(TFT_WIDTH, TFT_HEIGHT));
    int rows = 20, cols = 5;
    while (!WindowShouldClose()) {
        handle_keys(ui);
        BeginDrawing();
        ui.begin_frame();
        ClearBackground(BLACK);
        ui.begin_container("root");
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
            if(ui.button("hidden button"))
                printf("hidden button clicked!\n");
        }
        ui.end_container();
        ui.end_frame();
        EndDrawing();
    }
    CloseWindow();
    return 0;
}