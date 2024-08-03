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
        char label[32];
        for(int y = 0; y < 4; y++) {
            for(int x = 0; x < 4; x++) {
                snprintf(label, sizeof(label), "Button %d", y * 4 + x);
                if(button(&ctx, label, 10 + x * 80, 10 + y * 25))
                    printf("%s clicked!\n", label);
            }
        }
        EndDrawing();
        ui_end(&ctx);
    }
    CloseWindow();
    return 0;
}