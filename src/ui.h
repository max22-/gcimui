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

typedef struct UIVec4 {
    int x, y, w, h;
} ui_vec4;

typedef unsigned int ui_id;

void ui_init(ui_ctx *ctx, int screen_width, int screen_height);

void ui_begin(ui_ctx *ctx);
void ui_end(ui_ctx *ctx);
ui_id ui_get_id(ui_ctx *ctx, const void *data, size_t size);
void ui_push_id(ui_ctx *ctx, const void *data, size_t size);
void ui_pop_id(ui_ctx *ctx);

/* Macros ******************************************************************* */
#define UI_WIDGETS_MAX 1000
#define UI_ID_STACK_SIZE 32
#define UI_CONTAINER_STACK_SIZE 32
#define UI_CONTAINER_POOL_SIZE 32
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
#define ui_max(a, b) ((a) > (b) ? (a) : (b))
#define ui_min(a, b) ((a) < (b) ? (a) : (b))
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
#define ui_stack_get_last(s) ((s).items[(s).idx - 1])
#define ui_clear_stack(s) do { (s).idx = 0; } while(0)
#define ui_stack_empty(s) ((s).idx == 0)
/* pools, inspired by microui */
#define ui_pool(T, capacity) struct { struct { ui_id id; int last_update; ui_##T get; } items[capacity]; }
#define ui_pool_items(name) ctx->name.items
#define ui_generate_pool_get_func(T, name)                                          \
    ui_##T* ui_ ## T ## _pool_get(ui_ctx *ctx, ui_id id) {                          \
        /* Try to find the id in the pool */                                        \
        for(int i = 0; i < ARRAY_SIZE(ui_pool_items(name)); i++) {                  \
            if(ui_pool_items(name)[i].id == id)                                     \
                return &ui_pool_items(name)[i].get;                                 \
        }                                                                           \
        int n = -1, f = ctx->frame;                                                 \
        for(int i = 0; i < ARRAY_SIZE(ui_pool_items(name)); i++) {                  \
            if(ui_pool_items(name)[i].last_update < f) {                            \
                f = ui_pool_items(name)[i].last_update;                             \
                n = i;                                                              \
            }                                                                       \
        }                                                                           \
        ui_assert(n > -1);                                                          \
        ui_pool_items(name)[n].id = id;                                             \
        ui_pool_items(name)[n].last_update = ctx->frame;                            \
        memset(&ui_pool_items(name)[n].get, 0, sizeof(ui_pool_items(name)[n].get)); \
        return &ui_pool_items(name)[n].get;                                         \
    }

/* ************************************************************************** */

/* Backend ****************************************************************** */
extern void ui_draw_rectangle(int x, int y, int w, int h, ui_color color);
extern void ui_fill_rectangle(int x, int y, int w, int h, ui_color color);
extern void ui_draw_text(const char *msg, int x, int y, int font_size, ui_color color);
extern int ui_get_text_width(const char *text, int font_size);
extern void ui_clip(ui_vec4 rect);
extern void ui_clip_end(void);
extern unsigned long ui_millis(void);
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
#define UI_COLOR_WHITE (ui_color){.r = 255, .g = 255, .b = 255, .a = 255}
#define UI_COLOR_RED (ui_color){.r = 255, .g = 0, .b = 0, .a = 255}
#define UI_COLOR_GREEN (ui_color){.r = 0, .g = 255, .b = 0, .a = 255}
#define UI_COLOR_BLUE (ui_color){.r = 0, .g = 0, .b = 255, .a = 255}
#define UI_COLOR_LIGHTGREY (ui_color){.r = 211, .g = 211, .b = 211, .a = 255}
#define UI_COLOR_DARKGREY (ui_color){.r = 128, .g = 128, .b = 128, .a = 255}
/* ************************************************************************** */

/* Widgets ****************************************************************** */
bool ui_button(ui_ctx *ctx, const char *label);
void ui_begin_container(ui_ctx *ctx, const char *name);
void ui_end_container(ui_ctx *ctx);
void ui_nextline(ui_ctx *ctx);
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

typedef struct Container {
    ui_vec2 origin;
    ui_vec2 cursor;
    int max_x, max_y; // bounding box of the container's content (max_y is used to begin a new row)
} ui_container;

typedef struct Style {
    int spacing;
    int slider_width;
} ui_style;

// Warning : make sure that the context is valid when zero-initialized !!!
struct UIContext {
    struct Input input;
    ui_id hot_item, active_item;
    int frame;
    ui_style style;
    ui_stack(widget_location, UI_WIDGETS_MAX) widgets_locations;
    ui_stack(ui_id, UI_ID_STACK_SIZE) id_stack;
    ui_stack(ui_id, UI_CONTAINER_STACK_SIZE) container_stack;
    ui_pool(container, UI_CONTAINER_POOL_SIZE) container_pool;
    ui_container *current_container;
    ui_vec2 content_size;
    ui_vec2 screen_size;
    ui_vec2 scroll; /* global scrolling (i decided to not support per-container scrolling)*/
};

void draw_h_slider(ui_ctx *ctx);
void draw_v_slider(ui_ctx *ctx);

void ui_init(ui_ctx *ctx, int screen_width, int screen_height) {
    ctx->screen_size.x = screen_width;
    ctx->screen_size.y = screen_height;
}

ui_generate_pool_get_func(container, container_pool)

static ui_style ui_default_style = {
    .spacing = 4,
    .slider_width = 10
};

static void ui_new_selectable_widget(ui_ctx *ctx, ui_id id, ui_vec4 bounds) {
    if(ctx->hot_item == 0)
        ctx->hot_item = id;
    if(ctx->hot_item == id) {
        int dx = ctx->screen_size.x - (bounds.x + bounds.w);
        int dy = ctx->screen_size.y - (bounds.y + bounds.h);
        if(dx < 0) ctx->scroll.x += dx;
        if(dy < 0) ctx->scroll.y += dy;
        if(bounds.x < 0) ctx->scroll.x -= bounds.x;
        if(bounds.y < 0) ctx->scroll.y -= bounds.y;
    }
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
    ui_clear_stack(ctx->id_stack);
    ctx->style = ui_default_style;
    ctx->content_size = (ui_vec2){.x = 0, .y = 0};
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
    ctx->frame++;
}

void ui_end(ui_ctx *ctx) {
    if(ctx->content_size.x > ctx->screen_size.x)
        draw_h_slider(ctx);
    if(ctx->content_size.x > ctx->screen_size.x)
        draw_v_slider(ctx);
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
    ui_assert(ui_stack_empty(ctx->container_stack));
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
    ui_id res = ui_stack_empty(ctx->id_stack) ? FNV_OFFSET_BASIS : ui_stack_get_last(ctx->id_stack);
    res = hash(res, data, size);
    return res;
}

void ui_push_id(ui_ctx *ctx, const void *data, size_t size) {
    ui_push(ctx->id_stack, ui_get_id(ctx, data, size));
}

void ui_pop_id(ui_ctx *ctx) {
    ui_pop(ctx->id_stack);
}

/* Containers *************************************************************** */

void ui_update_cursor(ui_ctx *ctx, int w, int h) {
    ui_container *container = ctx->current_container;
    if(container != NULL) {
        container->cursor.x += w + ctx->style.spacing;
        container->max_x = ui_max(container->max_x, container->cursor.x);
        container->max_y = ui_max(container->max_y, container->cursor.y + h);
    }  
}

void ui_push_container(ui_ctx *ctx, ui_id id) {
    ui_container *parent = ctx->current_container;
    ui_push(ctx->container_stack, id);
    ui_container *new_container = ui_container_pool_get(ctx, id);
    if(parent == NULL)
        new_container->origin = (ui_vec2){.x = 0, .y = 0};
    else
        new_container->origin = parent->cursor;
    new_container->cursor = (ui_vec2){.x = 0, .y = 0};
    new_container->max_x = 0;
    new_container->max_y = 0;
    ctx->current_container = new_container;
}

void ui_pop_container(ui_ctx *ctx) {
    ui_container *container = ctx->current_container;
    ui_assert(container != NULL);
    int dx = container->max_x;
    int dy = container->max_y;
    ui_pop(ctx->container_stack);
    if(ui_stack_empty(ctx->container_stack)) {
        ctx->current_container = NULL;
        ctx->content_size = (ui_vec2){.x = dx, .y = dy};
    }
    else {
        ctx->current_container = ui_container_pool_get(ctx, ui_stack_get_last(ctx->container_stack));
    }
    ui_update_cursor(ctx, dx, dy);
}

/* Widgets ****************************************************************** */

bool ui_button(ui_ctx *ctx, const char *label) {
    ui_id id = ui_get_id(ctx, label, strlen(label));
    const int w = ui_get_text_width(label, UI_FONT_SIZE) + 2 * UI_MARGIN;
    const int h = UI_FONT_SIZE + 2 * UI_MARGIN;
    ui_container *container = ctx->current_container;
    ui_assert(container  != NULL);
    const int x = container->origin.x + ctx->scroll.x + container->cursor.x;
    const int y = container->origin.y + ctx->scroll.y + container->cursor.y;
    ui_new_selectable_widget(ctx, id, (ui_vec4){.x = x, .y = y, .w = w, .h = h});
    if(ctx->hot_item == id && ui_key_event(ctx, UI_KEY_ENTER))
        ctx->active_item = id;
    ui_fill_rectangle(x, y, w, h, UI_COLOR_DARKGREY);
    if(ctx->active_item == id)
        ui_draw_rectangle(x, y, w, h, UI_COLOR_RED);
    else if(ctx->hot_item == id)
        ui_draw_rectangle(x, y, w, h, UI_COLOR_GREEN);
    ui_draw_text(label, x + UI_MARGIN, y + UI_MARGIN, UI_FONT_SIZE, UI_COLOR_BLACK);
    ui_push(ctx->widgets_locations, ((widget_location){.id = id, .vec = (ui_vec2){.x = x, .y = y}}));
    ui_update_cursor(ctx, w, h);
    return !ui_key_event(ctx, UI_KEY_ENTER) && ctx->hot_item == id && ctx->active_item == id;
}

bool ui_checkbox(ui_ctx *ctx, const char *label, bool *checked) {
    ui_id id = ui_get_id(ctx, label, strlen(label));
    ui_container *container = ctx->current_container;
    ui_assert(container  != NULL);
    const int x = container->origin.x + ctx->scroll.x + container->cursor.x;
    const int y = container->origin.y + ctx->scroll.y + container->cursor.y;
    const int w = 20, h = 20;
    ui_new_selectable_widget(ctx, id, (ui_vec4){.x = x, .y = y, .w = w, .h = h});
    if(ctx->hot_item == id && ui_key_event(ctx, UI_KEY_ENTER))
        ctx->active_item = id;
    if(*checked)
        ui_fill_rectangle(x, y, w, h, UI_COLOR_DARKGREY);
    if(ctx->active_item == id)
        ui_draw_rectangle(x, y, w, h, UI_COLOR_RED);
    else if(ctx->hot_item == id)
        ui_draw_rectangle(x, y, w, h, UI_COLOR_GREEN);
    else
        ui_draw_rectangle(x, y, w, h, UI_COLOR_WHITE);
    ui_push(ctx->widgets_locations, ((widget_location){.id = id, .vec = (ui_vec2){.x = x, .y = y}}));
    ui_update_cursor(ctx, w, h);
    bool clicked = !ui_key_event(ctx, UI_KEY_ENTER) && ctx->hot_item == id && ctx->active_item == id;
    if(clicked)
        *checked = !*checked;
    return clicked;
}

void ui_begin_container(ui_ctx *ctx, const char *name) {
    ui_id id = ui_get_id(ctx, name, strlen(name));
    ui_push_container(ctx, id);
    ui_container *container = ctx->current_container;
}

void ui_end_container(ui_ctx *ctx) {
    ui_pop_container(ctx);
}

void ui_nextline(ui_ctx *ctx) {
    ui_container *container = ctx->current_container;
    ui_assert(container != NULL);
    container->cursor.y = container->max_y + ctx->style.spacing;
    container->cursor.x = 0;
}

/* Special widgets ********************************************************** */

void draw_h_slider(ui_ctx *ctx) {
    int slider_w = ctx->style.slider_width;
    int screen_w = ctx->screen_size.x;
    int screen_h = ctx->screen_size.y;
    ui_fill_rectangle(0, screen_h - slider_w, screen_w, slider_w, UI_COLOR_DARKGREY);
    int x = -ctx->scroll.x * screen_w / ctx->content_size.x;
    int w = screen_w * screen_w / ctx->content_size.x;
    ui_fill_rectangle(x, screen_h - slider_w, w, slider_w, UI_COLOR_LIGHTGREY);
}

void draw_v_slider(ui_ctx *ctx) {
    int slider_w = ctx->style.slider_width;
    int screen_w = ctx->screen_size.x;
    int screen_h = ctx->screen_size.y;
    ui_fill_rectangle(screen_w - slider_w, 0, slider_w, screen_h, UI_COLOR_DARKGREY);
    int y = -ctx->scroll.y * screen_h / ctx->content_size.y;
    int h = screen_h * screen_h / ctx->content_size.y;
    ui_fill_rectangle(screen_w - slider_w, y, slider_w, h, UI_COLOR_LIGHTGREY);
}

/* ************************************************************************** */


#endif
#endif