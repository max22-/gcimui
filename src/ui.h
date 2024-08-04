#ifndef UI_H
#define UI_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limits.h>
#include <string.h>

typedef struct UIContext ui_ctx;
typedef struct UIColor {
    uint8_t r, g, b, a;
} ui_color;

typedef struct UIVec2 {
    int x, y;
} ui_vec2;

typedef unsigned int ui_id;

void ui_begin(ui_ctx *ctx);
void ui_end(ui_ctx *ctx);
ui_id ui_get_id(ui_ctx *ctx, const void *data, size_t size);
void ui_push_id(ui_ctx *ctx, const void *data, size_t size);
void ui_pop_id(ui_ctx *ctx);

/* Macros ******************************************************************* */
#define UI_WIDGETS_MAX 1000
#define UI_ID_STACK_SIZE 32
#define UI_MARGIN 2
#define UI_FONT_SIZE 16
#define UI_KEY_REPEAT_DELAY 500 // ms
#define UI_KEY_REPEAT_INTERVAL 30 // ms
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define ui_assert(x)                                                            \
    do {                                                                        \
        if(!(x))                                                                \
            ui_error("%s:%d: assertion '%s' failed", __FILE__, __LINE__, #x);   \
    } while(0)
/* stack stuff, inspired by microui */
#define ui_stack(T, capacity) struct { int idx; T items[capacity]; }
#define ui_push(s, val)                             \
    do {                                            \
        ui_assert((s).idx < ARRAY_SIZE((s).items)); \
        (s).items[(s).idx] = (val);                 \
        (s).idx++;                                  \
    } while(0)
#define ui_pop(s)               \
    do {                        \
        ui_assert((s).idx > 0); \
        (s).idx--;              \
    } while(0)
#define ui_clear_stack(s) do { (s).idx = 0; } while(0)

/* ************************************************************************** */

/* Backend ****************************************************************** */
extern void ui_draw_rectangle(int x, int y, int w, int h, ui_color color);
extern void ui_fill_rectangle(int x, int y, int w, int h, ui_color color);
extern void ui_draw_text(const char *msg, int x, int y, int font_size, ui_color color);
extern int ui_get_text_width(const char *text, int font_size);
extern unsigned long ui_millis();
extern void ui_error(const char *fmt, ...);
enum UI_KEY {
    UI_KEY_NONE,
    UI_KEY_UP,
    UI_KEY_DOWN,
    UI_KEY_LEFT,
    UI_KEY_RIGHT,
    UI_KEY_ENTER,
    UI_KEY_BACK,
};
void ui_set_key_state(ui_ctx *ctx, enum UI_KEY key, bool state);
/* ************************************************************************** */

/* Colors ******************************************************************* */
#define UI_COLOR_BLACK (ui_color){.r = 0, .g = 0, .b = 0, .a = 255}
#define UI_COLOR_RED (ui_color){.r = 255, .g = 0, .b = 0, .a = 255}
#define UI_COLOR_GREEN (ui_color){.r = 0, .g = 255, .b = 0, .a = 255}
#define UI_COLOR_BLUE (ui_color){.r = 0, .g = 0, .b = 255, .a = 255}
#define UI_COLOR_LIGHTGREY (ui_color){.r = 211, .g = 211, .b = 211, .a = 255}
#define UI_COLOR_DARKGREY (ui_color){.r = 128, .g = 128, .b = 128, .a = 255}
/* ************************************************************************** */

/* Widgets ****************************************************************** */
bool button(ui_ctx *ctx, const char *label, int x, int y);
/* ************************************************************************** */


#ifdef UI_IMPLEMENTATION

struct Input {
    uint8_t state, new_state, events; // state: saved key states, new_state: updated key states, events: detects rising edges of keys
    unsigned long timestamp;
    bool is_repeat;
};

typedef struct WidgetLocation {
    ui_vec2 vec;
    ui_id id;
} widget_location;

// Warning : make sure that the context is valid when zero-initialized !!!
struct UIContext {
    struct Input input;
    ui_id hot_item, active_item;
    ui_stack(widget_location, UI_WIDGETS_MAX) widgets_locations;
    ui_stack(ui_id, UI_ID_STACK_SIZE) id_stack;
};

void new_widget(ui_ctx *ctx, ui_id id) {
    if(ctx->hot_item == 0)
        ctx->hot_item = id;
}

static widget_location *ui_get_widget_location(ui_ctx *ctx, ui_id id) {
    for(int i = 0; i < ctx->widgets_locations.idx; i++) {
        widget_location *loc = &ctx->widgets_locations.items[i];
        if(loc->id == id)
            return loc;
    }
    return NULL;
}

static int ui_vec2_dot(ui_vec2 v1, ui_vec2 v2) {
    return v1.x * v2.x + v1.y * v2.y;
}

static ui_vec2 ui_vec2_diff(ui_vec2 v1, ui_vec2 v2) {
    return (ui_vec2){.x = v1.x - v2.x, .y = v1.y - v2.y};
}

static int ui_vec2_norm_squared(ui_vec2 v) {
    return v.x * v.x + v.y * v.y;
}

static void ui_update_hot_item_by_direction(ui_ctx *ctx, ui_vec2 dir) {
    if(ctx->hot_item == -1) return;
    widget_location *wl = ui_get_widget_location(ctx, ctx->hot_item);
    if(!wl) return;
    ui_vec2 hot_item_loc = wl->vec;
    int best_distance = INT_MAX;
    ui_id best_id = 0;
    for(int i = 0; i < ctx->widgets_locations.idx; i++) {
        ui_id id = ctx->widgets_locations.items[i].id;
        if(id == ctx->hot_item)
            continue;
        ui_vec2 loc = ctx->widgets_locations.items[i].vec;
        int distance = ui_vec2_norm_squared(ui_vec2_diff(loc, hot_item_loc));
        int dot = ui_vec2_dot(ui_vec2_diff(loc, hot_item_loc), dir);
        if(dot > 0 && distance < best_distance) {
            best_distance = distance;
            best_id = id;
        }
    }
    if(best_id != 0)
        ctx->hot_item = best_id;
}

static inline bool ui_key_event(ui_ctx *ctx, enum UI_KEY key) {
    return (ctx->input.events & (1 << key)) != 0;
}

void ui_set_key_state(ui_ctx *ctx, enum UI_KEY key, bool state) {
    uint8_t mask = ~(1 << key);
    ctx->input.new_state &= mask;
    ctx->input.new_state |= (state ? 1 : 0) << key;
}

void ui_begin(ui_ctx *ctx) {
    ui_clear_stack(ctx->widgets_locations);
    { /* Input handling ************** */
        ctx->input.events = ~ctx->input.state & ctx->input.new_state; // detect rising edge
        if(ctx->input.events != 0)
            ctx->input.timestamp = ui_millis(); // we reset the timestamp when new keys are pressed
        if(ctx->input.new_state == 0)
            ctx->input.is_repeat = false;
        if(ctx->input.is_repeat) { // a repetition has already started
            if(ui_millis() - ctx->input.timestamp > UI_KEY_REPEAT_INTERVAL) {
                ctx->input.events |= ctx->input.new_state;
                ctx->input.timestamp = ui_millis();
            }
        } else { // we check if we need to start a key repetition
            if((ctx->input.state & ctx->input.new_state) && ui_millis() - ctx->input.timestamp > UI_KEY_REPEAT_DELAY) {
                ctx->input.events |= ctx->input.new_state;
                ctx->input.is_repeat = true;
                ctx->input.timestamp = ui_millis();
            }
        }
    } /* End of input handling ******* */

}

void ui_end(ui_ctx *ctx) {
    if(!ui_key_event(ctx, UI_KEY_ENTER))
        ctx->active_item = 0;
    ui_vec2 dir = {0, 0};
    if(ui_key_event(ctx, UI_KEY_UP))
        dir.y--;
    if(ui_key_event(ctx, UI_KEY_DOWN))
        dir.y++;
    if(ui_key_event(ctx, UI_KEY_LEFT))
        dir.x--;
    if(ui_key_event(ctx, UI_KEY_RIGHT))
        dir.x++;
    if(dir.x != 0 || dir.y != 0)
        ui_update_hot_item_by_direction(ctx, dir);
    ctx->input.state = ctx->input.new_state;
}

/* id stuff, inspired by microui ******************************************** */

/* 32bit fnv-1a hash */
#define FNV_PRIME 16777619
#define FNV_OFFSET_BASIS 2166136261
static ui_id hash(ui_id id, const void *data, size_t size) {
    const unsigned char *p = data;
    while(size--) 
        id = (id ^ *p++) * FNV_PRIME;
    return id;
}

ui_id ui_get_id(ui_ctx *ctx, const void *data, size_t size) {
    int idx = ctx->id_stack.idx;
    ui_id res = (idx > 0) ? ctx->id_stack.items[idx - 1] : FNV_OFFSET_BASIS;
    res = hash(res, data, size);
    return res;
}

void ui_push_id(ui_ctx *ctx, const void *data, size_t size) {
    ui_push(ctx->id_stack, ui_get_id(ctx, data, size));
}

void ui_pop_id(ui_ctx *ctx) {
    ui_pop(ctx->id_stack);
}

/* Widgets ****************************************************************** */

bool button(ui_ctx *ctx, const char *label, int x, int y) {
    ui_id id = ui_get_id(ctx, label, strlen(label));
    new_widget(ctx, id);
    if(ctx->hot_item == id && ui_key_event(ctx, UI_KEY_ENTER))
        ctx->active_item = id;
    int w = ui_get_text_width(label, UI_FONT_SIZE) + 2 * UI_MARGIN;
    int h = UI_FONT_SIZE + 2 * UI_MARGIN;
    ui_fill_rectangle(x, y, w, h, UI_COLOR_DARKGREY);
    if(ctx->active_item == id)
        ui_draw_rectangle(x, y, w, h, UI_COLOR_RED);
    else if(ctx->hot_item == id)
        ui_draw_rectangle(x, y, w, h, UI_COLOR_GREEN);
    ui_draw_text(label, x + UI_MARGIN, y + UI_MARGIN, UI_FONT_SIZE, UI_COLOR_BLACK);
    ui_push(ctx->widgets_locations, ((widget_location){.id = id, .vec = (ui_vec2){.x = x, .y = y}}));
    return !ui_key_event(ctx, UI_KEY_ENTER) && ctx->hot_item == id && ctx->active_item == id;
}

#endif
#endif