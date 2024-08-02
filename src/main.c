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
    ui_ctx ctx = new_uicontext();
    while (!WindowShouldClose()) {
        ui_begin(&ctx);
        handle_keys(&ctx);
        ClearBackground(BLACK);
        if(button(&ctx, GEN_ID, 10, 10, 50, 20))
            printf("Button 1 clicked!\n");
        if(button(&ctx, GEN_ID, 70, 10, 50, 20))
            printf("Button 2 clicked!\n");
        if(button(&ctx, GEN_ID, 70, 40, 50, 20))
            printf("Button 3 clicked!\n");
        EndDrawing();
        ui_end(&ctx);
    }
    CloseWindow();
    return 0;
}