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
    ui_ctx ctx = ui_context_new();
    while (!WindowShouldClose()) {
        ui_begin(&ctx);
        handle_keys(&ctx);
        ClearBackground(BLACK);
        if(button(&ctx, "Button 1", 10, 10))
            printf("Button 1 clicked!\n");
        if(button(&ctx, "Button 2", 100, 10))
            printf("Button 2 clicked!\n");
        if(button(&ctx, "Button 3", 100, 40))
            printf("Button 3 clicked!\n");
        EndDrawing();
        ui_end(&ctx);
    }
    CloseWindow();
    return 0;
}