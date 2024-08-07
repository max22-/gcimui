#include <raylib.h>
#define UI_IMPLEMENTATION
#include "ui.h"
#include "backend.h"
#include <stdio.h>

#define TFT_WIDTH 320
#define TFT_HEIGHT 240


int main(void) {
    InitWindow(TFT_WIDTH, TFT_HEIGHT, "ui");
    SetTargetFPS(60);
    ui_ctx ctx = {0};
    ui_init(&ctx, TFT_WIDTH, TFT_HEIGHT);
    int rows = 20, cols = 5;
    while (!WindowShouldClose()) {
        handle_keys(&ctx);
        BeginDrawing();
        ui_begin(&ctx);
        ClearBackground(BLACK);
        ui_begin_container(&ctx, "root");
        char label[32];
        for(int x = 0; x < cols; x++) {
            ui_begin_container(&ctx, "column");
            for(int y = 0; y < rows; y++) {
                snprintf(label, sizeof(label), "Button %d", y * cols + x);
                if(ui_button(&ctx, label))
                    printf("%s clicked!\n", label);
                ui_nextline(&ctx);
            }
            ui_end_container(&ctx);
        }
        ui_nextline(&ctx);
        static bool checked = false;
        ui_checkbox(&ctx, "checkbox", &checked);
        if(checked) {
            if(ui_button(&ctx, "hidden button"))
                printf("hidden button clicked!\n");
        }
        ui_end_container(&ctx);
        ui_end(&ctx);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}