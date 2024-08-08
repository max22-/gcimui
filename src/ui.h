#ifndef UI_H
#define UI_H

#include <cstdint>
#include <cstddef>
#include <climits>
#include <cstring>
#include <vector>
#include <unordered_map>

#define ui_assert(x)                                                            \
    do {                                                                        \
        if(!(x))                                                                \
            ui_error("%s:%d: assertion '%s' failed", __FILE__, __LINE__, #x);   \
    } while(0)

namespace UI {

typedef unsigned int ui_id;

class Color {
public:
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a) {}
    static Color black() { return Color(0, 0, 0, 255); }
    static Color white() { return Color(255, 255, 255, 255); }
    static Color red() { return Color(255, 0, 0, 255); }
    static Color green() { return Color(0, 255, 0, 255); }
    static Color blue() { return Color(0, 0, 255, 255); }
    static Color light_grey() { return Color(211, 211, 211, 255); }
    static Color dark_grey() { return Color(128, 128, 128, 255); }
    uint8_t r = 0, g = 0, b = 0, a = 0;
};

/* Colors ******************************************************************* */
#define UI_COLOR_BLACK (ui_color){.r = 0, .g = 0, .b = 0, .a = 255}
#define UI_COLOR_WHITE (ui_color){.r = 255, .g = 255, .b = 255, .a = 255}
#define UI_COLOR_RED (ui_color){.r = 255, .g = 0, .b = 0, .a = 255}
#define UI_COLOR_GREEN (ui_color){.r = 0, .g = 255, .b = 0, .a = 255}
#define UI_COLOR_BLUE (ui_color){.r = 0, .g = 0, .b = 255, .a = 255}
#define UI_COLOR_LIGHTGREY (ui_color){.r = 211, .g = 211, .b = 211, .a = 255}
#define UI_COLOR_DARKGREY (ui_color){.r = 128, .g = 128, .b = 128, .a = 255}
/* ************************************************************************** */

template <typename T>
class Vec2 {
public:
    Vec2() : x(0), y(0) {}
    Vec2(T x, T y) : x(x), y(y) {}
    Vec2 operator+(Vec2 const& other) {
        Vec2 res;
        res.x = x + other.x;
        res.y = y + other.y;
        return res;
    }
    Vec2 operator-(Vec2 const& other) {
        Vec2 res;
        res.x = x - other.x;
        res.y = y - other.y;
        return res;
    }
    T dot(Vec2 other) {
        return x * other.x + y * other.y;
    }
    int mag_squared() {
        return x * x + y * y;
    }
    T x = 0, y = 0;
};

template <typename T>
class Rectangle {
public:
    Rectangle(T x, T y, T w, T h) : x(x), y(y), w(w), h(h) {}
    Rectangle(Vec2<T> xy, Vec2<T> wh) : x(xy.x), y(xy.y), w(wh.x), h(wh.y) {}
    Vec2<T> xy() { return Vec2<T>(x, y); }
    Vec2<T> wh() { return Vec2<T>(w, h); }
    T x = 0, y = 0, w = 0, h = 0;
};

} // namespace UI

/* Backend ****************************************************************** */
extern void ui_draw_rectangle(UI::Rectangle<int> rect, UI::Color color);
extern void ui_fill_rectangle(UI::Rectangle<int> rect, UI::Color color);
extern void ui_draw_text(const char *msg, UI::Vec2<int> pos, int font_size, UI::Color color);
extern int ui_get_text_width(const char *text, int font_size);
extern void ui_clip(UI::Rectangle<int> rect);
extern void ui_clip_end(void);
extern unsigned long ui_millis(void);
extern void ui_error(const char *fmt, ...);
/* ************************************************************************** */

namespace UI {

enum KEY {
    NONE,
    UP,
    DOWN,
    LEFT,
    RIGHT,
    ENTER,
    BACK,
};

class Input {
public:
    void set_key_state(enum KEY key, bool state) {
        uint8_t mask = ~(1 << key);
        new_state &= mask;
        new_state |= (state ? 1 : 0) << key;
    }
    void update() {
        events = ~state & new_state; // detect rising edge
        if(events != 0)
            timestamp = ui_millis(); // we reset the timestamp when new keys are pressed
        if(new_state == 0)
            is_repeat = false;
        if(is_repeat) { // a repetition has already started
            if(ui_millis() - timestamp > key_repeat_interval) {
                events |= new_state;
                timestamp = ui_millis();
            }
        } else { // we check if we need to start a key repetition
            if((state & new_state) && ui_millis() - timestamp > key_repeat_delay) {
                events |= new_state;
                is_repeat = true;
                timestamp = ui_millis();
            }
        }
    }
    bool key_event(enum KEY key) {
        return (events & (1 << key)) != 0;
    }
    void end_frame() {
        state = new_state;
    }
private:
    uint8_t state = 0, new_state = 0, events = 0; // state: saved key states, new_state: updated key states, events: detects rising edges of keys
    unsigned long timestamp = 0;
    bool is_repeat = false;
    const unsigned long key_repeat_delay = 500, key_repeat_interval = 30; // in milliseconds
};

class Style {
public:
    int spacing = 4;
    int slider_width = 10;
    int margin = 2; // margin for text inside buttons
    int font_size = 16;
};

/* id stuff, inspired by microui ******************************************** */
class IDStack {
public:
    ui_id get_id(const void *data, size_t size) {
        ui_id res = stack.empty() ? FNV_OFFSET_BASIS : stack.back();
        res = hash(res, data, size);
        return res;
    }
    void push(const void *data, size_t size) {
        stack.push_back(get_id(data, size));
    }
    void pop() {
        stack.pop_back();
    }
    void clear() {
        stack.clear();
    }
private:
    /* 32bit fnv-1a hash */
    ui_id hash(ui_id id, const void *data, size_t size) {
        const unsigned char *p = (const unsigned char*)data;
        while(size--) 
            id = (id ^ *p++) * FNV_PRIME;
        return id;
    }
    const ui_id FNV_PRIME = 16777619, FNV_OFFSET_BASIS = 2166136261;
    std::vector<ui_id> stack;
};


class Container {
public:
    Container(Vec2<int> origin) : bounds(origin.x, origin.y, 0, 0), cursor(0, 0) {}
    void update_cursor(Vec2<int> wh) { 
        cursor.x += wh.x;
        bounds.w = std::max(bounds.w, cursor.x);
        bounds.h = std::max(bounds.h, cursor.y + wh.y);
    }
    void next_line() {
        cursor.y = bounds.h;
        cursor.x = 0;
    }
    Rectangle<int> bounds;
    Vec2<int> cursor;
};


class Context {
public:
    static Context& get() {
        static Context instance;
        return instance;
    }
    Context(Context const&) = delete;
    void operator=(Context const&) = delete;

    void init(int screen_width, int screen_height) {
        screen_size = Vec2<int>(screen_width, screen_height);
        hot_item = 0;
        active_item = 0;
        frame = 0;
    }

    void set_key_state(enum KEY key, bool state) {
        input.set_key_state(key, state);
    }
    
    void begin_frame() {
        hot_item_exists = false;
        widgets_locations.clear();
        id_stack.clear();
        style = Style();
        content_size = Vec2<int>(0, 0);
        input.update();
        frame++;
    }

    void end_frame() {
        if(content_size.x > screen_size.x)
            draw_h_slider();
        if(content_size.x > screen_size.x)
            draw_v_slider();
        if(!input.key_event(KEY::ENTER))
            active_item = 0;
        Vec2<int> dir;
        if(input.key_event(KEY::UP))
            dir.y--;
        if(input.key_event(KEY::DOWN))
            dir.y++;
        if(input.key_event(KEY::LEFT))
            dir.x--;
        if(input.key_event(KEY::RIGHT))
            dir.x++;
        if(dir.x != 0 || dir.y != 0)
            update_hot_item_by_direction(dir);
        input.end_frame();
        ui_assert(container_stack.empty());
        if(!hot_item_exists)
            hot_item = 0;
    }

    /* Widgets ****************************************************************** */

    bool button(const char *label) {
        ui_id id = id_stack.get_id(label, strlen(label));
        const int w = ui_get_text_width(label, style.font_size) + 2 * style.margin;
        const int h = style.font_size + 2 * style.margin;
        Vec2<int> wh(w, h);
        Container *container = current_container();
        ui_assert(container  != NULL);
        Vec2<int> origin = container->bounds.xy();
        Vec2<int> xy = origin + scroll + container->cursor;
        Rectangle<int> rect(xy, wh);
        new_selectable_widget(id, rect);
        if(hot_item == id && input.key_event(KEY::ENTER))
            active_item = id;
        ui_fill_rectangle(rect, Color::dark_grey());
        if(active_item == id)
            ui_draw_rectangle(rect, Color::red());
        else if(hot_item == id)
            ui_draw_rectangle(rect, Color::green());
        ui_draw_text(label, xy + Vec2<int>(style.margin, style.margin), style.font_size, Color::black());
        widgets_locations[id] = xy;
        update_cursor(wh);
        return !input.key_event(KEY::ENTER) && hot_item == id && active_item == id;
    }

    bool checkbox(const char *label, bool *checked) {
        ui_id id = id_stack.get_id(label, strlen(label));
        Container *container = current_container();
        Vec2<int> origin = container->bounds.xy();
        Vec2<int> xy = origin + scroll + container->cursor;
        Vec2<int> wh(20, 20);
        Rectangle<int> rect(xy, wh);
        new_selectable_widget(id, rect);
        if(hot_item == id && input.key_event(KEY::ENTER))
            active_item = id;
        if(*checked)
            ui_fill_rectangle(rect, Color::dark_grey());
        if(active_item == id)
            ui_draw_rectangle(rect, Color::red());
        else if(hot_item == id)
            ui_draw_rectangle(rect, Color::green());
        else
            ui_draw_rectangle(rect, Color::white());
        widgets_locations[id] = xy;
        update_cursor(wh);
        bool clicked = !input.key_event(KEY::ENTER) && hot_item == id && active_item == id;
        if(clicked)
            *checked = !*checked;
        return clicked;
    }

    void begin_container(const char *name) {
        push_container();
    }

    void end_container() {
        pop_container();
    }

    void nextline() {
        current_container()->next_line();
    }

    

private:
    Context() {}

    void new_selectable_widget(ui_id id, Rectangle<int> bounds) {
        if(hot_item == 0)
            hot_item = id;
        if(hot_item == id) {
            hot_item_exists = true;
            int dx = screen_size.x - (bounds.x + bounds.w);
            int dy = screen_size.y - (bounds.y + bounds.h);
            if(dx < 0) scroll.x += dx;
            if(dy < 0) scroll.y += dy;
            if(bounds.x < 0) scroll.x -= bounds.x;
            if(bounds.y < 0) scroll.y -= bounds.y;
        }
    }

    void update_hot_item_by_direction(Vec2<int> dir) {
        if(hot_item == 0) return;
        if(widgets_locations.count(hot_item) == 0) return;
        Vec2<int> hot_item_loc = widgets_locations[hot_item];
        int best_distance = INT_MAX;
        ui_id best_id = 0;
        for(auto& it: widgets_locations) {
            ui_id id = it.first;
            if(id == hot_item)
                continue;
            Vec2<int> loc = it.second;
            int distance = (loc - hot_item_loc).mag_squared();
            int dot = (loc - hot_item_loc).dot(dir);
            if(dot > 0 && distance < best_distance) {
                best_distance = distance;
                best_id = id;
            }
        }
        if(best_id != 0)
            hot_item = best_id;
    }

    // Containers

    void update_cursor(Vec2<int> wh) {
        if(container_stack.empty()) return;
        container_stack.back()->update_cursor(wh);
    }

    void push_container() {
        Container *parent = container_stack.empty() ? nullptr : container_stack.back();
        Container *new_container = nullptr;
        if(parent == nullptr)
            new_container = new Container(Vec2<int>(0,0));
        else
            new_container = new Container(parent->cursor);
        container_stack.push_back(new_container);
    }

    void pop_container() {
        Container *container = current_container();
        container_stack.pop_back();
        Vec2<int> wh = container->bounds.wh();
        if(container_stack.empty()) {
            content_size = wh;
        }
        update_cursor(wh);
    }

    Container *current_container() {
        ui_assert(!container_stack.empty());
        return container_stack.back();
    }

    // Special widgets

    void draw_h_slider() {
        int slider_w = style.slider_width;
        int screen_w = screen_size.x;
        int screen_h = screen_size.y;
        ui_fill_rectangle(Rectangle<int>(0, screen_h - slider_w, screen_w, slider_w), Color::dark_grey());
        int x = -scroll.x * screen_w / content_size.x;
        int w = screen_w * screen_w / content_size.x;
        ui_fill_rectangle(Rectangle<int>(x, screen_h - slider_w, w, slider_w), Color::light_grey());
    }

    void draw_v_slider() {
        int slider_w = style.slider_width;
        int screen_w = screen_size.x;
        int screen_h = screen_size.y;
        ui_fill_rectangle(Rectangle<int>(screen_w - slider_w, 0, slider_w, screen_h), Color::dark_grey());
        int y = -scroll.y * screen_h / content_size.y;
        int h = screen_h * screen_h / content_size.y;
        ui_fill_rectangle(Rectangle<int>(screen_w - slider_w, y, slider_w, h), Color::light_grey());
    }
    struct Input input;
    ui_id hot_item, active_item;
    bool hot_item_exists;
    int frame;
    Style style;
    std::unordered_map<ui_id, Vec2<int>> widgets_locations;
    IDStack id_stack;
    std::vector<Container*> container_stack;
    Vec2<int> content_size;
    Vec2<int> screen_size;
    Vec2<int> scroll; /* global scrolling (i decided to not support per-container scrolling)*/
};

















/* Macros ******************************************************************* */

#define UI_CONTAINER_POOL_SIZE 32

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))



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

} // namespace UI

#endif