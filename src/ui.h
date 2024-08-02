#ifndef UI_H
#define UI_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limits.h>

typedef struct UIContext ui_ctx;
typedef struct UIColor {
    uint8_t r, g, b, a;
} ui_color;

typedef struct UIVec2 {
    int x, y;
} ui_vec2;

ui_ctx new_uicontext();
void ui_begin(ui_ctx *ctx);
void ui_end(ui_ctx *ctx);

/* Macros ******************************************************************* */
#define UI_WIDGETS_MAX 1000
#define GEN_ID (__LINE__)
/* ************************************************************************** */

/* Backend ****************************************************************** */
extern void draw_rectangle(int x, int y, int w, int h, ui_color color);
extern void fill_rectangle(int x, int y, int w, int h, ui_color color);
enum UI_KEY {
    UI_KEY_NONE = 0,
    UI_KEY_UP = 1 << 0,
    UI_KEY_DOWN = 1 << 1,
    UI_KEY_LEFT = 1 << 2,
    UI_KEY_RIGHT = 1 << 3,
    UI_KEY_ENTER = 1 << 4,
    UI_KEY_BACK = 1 << 5
};
void ui_set_key(ui_ctx *ctx, enum UI_KEY pressed_key);
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
bool button(ui_ctx *ctx, int id, int x, int y, int w, int h);
/* ************************************************************************** */


/*[[[cog
import cog
stacks = [
    {'name': 'widgets_locations', 'type': 'widget_location', 'capacity': 'UI_WIDGETS_MAX', 'arg_name': 'w_loc'}
]

def gen_stacks():
    for s in stacks:
        cog.outl(f'    struct {{ int idx; {s["type"]} items[{s["capacity"]}]; size_t capacity; }} {s["name"]};')

def gen_push():
    for s in stacks:
        cog.outl(f'static void ui_{s["name"]}_push(ui_ctx *ctx, {s["type"]} {s["arg_name"]}) {{')
        cog.outl(f'    if(ctx->{s["name"]}.idx < ctx->{s["name"]}.capacity)')
        cog.outl(f'        ctx->{s["name"]}.items[ctx->{s["name"]}.idx++] = {s["arg_name"]};')
        cog.outl('}')

def init_stacks():
    for s in stacks:
        cog.outl(f'    ctx.{s["name"]}.capacity = {s["capacity"]};')

]]]*/
// [[[end]]]


#ifdef UI_IMPLEMENTATION

typedef struct WidgetLocation {
    ui_vec2 vec;
    int id;
} widget_location;

struct UIContext {
    uint8_t pressed_keys;
    int hot_item, active_item;
    /*[[[cog
    gen_stacks()
    ]]]*/
    struct { int idx; widget_location items[UI_WIDGETS_MAX]; size_t capacity; } widgets_locations;
   //[[[end]]]
};

void new_widget(ui_ctx *ctx, int id) {
    if(ctx->hot_item == -1)
        ctx->hot_item = id;
}

/*[[[cog
gen_push()
]]]*/
static void ui_widgets_locations_push(ui_ctx *ctx, widget_location w_loc) {
    if(ctx->widgets_locations.idx < ctx->widgets_locations.capacity)
        ctx->widgets_locations.items[ctx->widgets_locations.idx++] = w_loc;
}
///[[[end]]]

static widget_location *ui_get_widget_location(ui_ctx *ctx, int id) {
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
    int best_id = -1;
    for(int i = 0; i < ctx->widgets_locations.idx; i++) {
        int id = ctx->widgets_locations.items[i].id;
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
    if(best_id != -1)
        ctx->hot_item = best_id;
}

ui_ctx new_uicontext() {
    ui_ctx ctx;
    ctx.pressed_keys = UI_KEY_NONE;
    ctx.hot_item = ctx.active_item = -1;
    /*[[[cog 
    init_stacks()
    ]]]*/
    ctx.widgets_locations.capacity = UI_WIDGETS_MAX;
    //[[[end]]]
    return ctx;
}

void ui_set_key(ui_ctx *ctx, enum UI_KEY pressed_key) {
    ctx->pressed_keys |= pressed_key;
}

void ui_begin(ui_ctx *ctx) {
    ctx->pressed_keys = UI_KEY_NONE;
    //ctx->hot_item = -1;
    ctx->widgets_locations.idx = 0;
}

void ui_end(ui_ctx *ctx) {
    if(!(ctx->pressed_keys & UI_KEY_ENTER))
        ctx->active_item = -1;
    ui_vec2 dir = {0, 0};
    if(ctx->pressed_keys & UI_KEY_UP)
        dir.y--;
    if(ctx->pressed_keys & UI_KEY_DOWN)
        dir.y++;
    if(ctx->pressed_keys & UI_KEY_LEFT)
        dir.x--;
    if(ctx->pressed_keys & UI_KEY_RIGHT)
        dir.x++;
    if(dir.x != 0 || dir.y != 0)
        ui_update_hot_item_by_direction(ctx, dir);
}

/* Widgets ****************************************************************** */

bool button(ui_ctx *ctx, int id, int x, int y, int w, int h) {
    new_widget(ctx, id);
    if(ctx->hot_item == id && (ctx->pressed_keys & UI_KEY_ENTER))
        ctx->active_item = id;
    fill_rectangle(x, y, w, h, UI_COLOR_DARKGREY);
    if(ctx->active_item == id)
        draw_rectangle(x, y, w, h, UI_COLOR_RED);
    else if(ctx->hot_item == id)
        draw_rectangle(x, y, w, h, UI_COLOR_GREEN);
    ui_widgets_locations_push(ctx, (widget_location){.id = id, .vec = (ui_vec2){.x = x, .y = y}});
    return !(ctx->pressed_keys & UI_KEY_ENTER) && ctx->hot_item == id && ctx->active_item == id;
}

#endif
#endif