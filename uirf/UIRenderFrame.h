#pragma once
#include <assert.h>
#include <stdint.h>
#ifdef __cplusplus
#include <algorithm>
#endif

struct UIVec2 {
  int32_t x;
  int32_t y;

#ifdef __cplusplus
  UIVec2() : x(0), y(0) {}
  UIVec2(int32_t x, int32_t y) : x(x), y(y) {}

  UIVec2 operator-() const { return UIVec2(-x, -y); }

  UIVec2 &operator+=(const UIVec2 &rhs) {
    x += rhs.x;
    y += rhs.y;
    return *this;
  }
#endif
};

struct UIRect {
  int32_t x;
  int32_t y;
  int32_t w;
  int32_t h;

#ifdef __cplusplus
  UIRect() : x(0), y(0), w(0), h(0) {}
  UIRect(int32_t x, int32_t y, int32_t w, int32_t h) : x(x), y(y), w(w), h(h) {}

  UIRect move(const UIVec2 &p) const { return UIRect(x - p.x, y - p.y, w, h); }

  UIRect expand(int32_t n) const {
    return UIRect(x - n, y - n, w + n * 2, h + n * 2);
  }

  UIRect intersect(UIRect r2) const {
    int32_t x1 = std::max(this->x, r2.x);
    int32_t y1 = std::max(this->y, r2.y);
    int32_t x2 = std::min(this->x + this->w, r2.x + r2.w);
    int32_t y2 = std::min(this->y + this->h, r2.y + r2.h);
    if (x2 < x1) {
      x2 = x1;
    }
    if (y2 < y1) {
      y2 = y1;
    }
    return UIRect(x1, y1, x2 - x1, y2 - y1);
  }

  bool overlaps_vec2(UIVec2 p) const {
    return p.x >= this->x && p.x < this->x + this->w && p.y >= this->y &&
           p.y < this->y + this->h;
  }

  bool visible() const { return w > 0 && h > 0; }
#endif
};

struct UIColor32 {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;

#ifdef __cplusplus
  UIColor32() : r(0), g(0), b(0), a(0) {}
  UIColor32(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
      : r(r), g(g), b(b), a(a) {}
#endif
};

enum UI_COMMAND {
  UI_COMMAND_CLIP,
  UI_COMMAND_RECT,
  UI_COMMAND_TEXT,
  UI_COMMAND_ICON
};

struct UIClipCommand {
  struct UIRect rect;
};
static_assert(sizeof(struct UIClipCommand) == 16);

struct UIRectCommand {
  struct UIRect rect;
  struct UIColor32 color;
};
static_assert(sizeof(struct UIRectCommand) == 20);

struct UITextCommand {
  struct UIVec2 pos;
  struct UIColor32 color;
  uint32_t length;
  const void *font;

#ifdef __cplusplus
  const uint8_t *begin() const {
    return ((const uint8_t *)this) + sizeof(UITextCommand);
  }
  const uint8_t *end() const { return begin() + length; }
#endif
};
static_assert(sizeof(struct UITextCommand) == 24);

struct UIIconCommand {
  struct UIRect rect;
  int32_t id;
  struct UIColor32 color;
};
static_assert(sizeof(struct UIIconCommand) == 24);

struct UICommandHeader {
  enum UI_COMMAND command;

#ifdef __cplusplus
  UICommandHeader(const UICommandHeader &) = delete;
  UICommandHeader &operator=(const UICommandHeader &) = delete;

  uint32_t size() const {
    switch (this->command) {
    case UI_COMMAND_CLIP:
      return sizeof(UI_COMMAND) + sizeof(UIClipCommand);
    case UI_COMMAND_RECT:
      return sizeof(UI_COMMAND) + sizeof(UIRectCommand);
    case UI_COMMAND_TEXT:
      return sizeof(UI_COMMAND) + sizeof(UITextCommand) + this->text()->length;
    case UI_COMMAND_ICON:
      return sizeof(UI_COMMAND) + sizeof(UIIconCommand);
    default:
      assert(false);
      return 0;
    }
  }

  UIClipCommand *clip() {
    assert(command == UI_COMMAND_CLIP);
    return (UIClipCommand *)(((uint8_t *)this) + 4);
  }

  UIRectCommand *rect() {
    assert(command == UI_COMMAND_RECT);
    return (UIRectCommand *)(((uint8_t *)this) + 4);
  }

  UITextCommand *text() {
    assert(command == UI_COMMAND_TEXT);
    return (UITextCommand *)(((uint8_t *)this) + 4);
  }

  UIIconCommand *icon() {
    assert(command == UI_COMMAND_ICON);
    return (UIIconCommand *)(((uint8_t *)this) + 4);
  }

  const UIClipCommand *clip() const {
    assert(command == UI_COMMAND_CLIP);
    return (UIClipCommand *)(((uint8_t *)this) + 4);
  }

  const UIRectCommand *rect() const {
    assert(command == UI_COMMAND_RECT);
    return (UIRectCommand *)(((uint8_t *)this) + 4);
  }

  const UITextCommand *text() const {
    assert(command == UI_COMMAND_TEXT);
    return (UITextCommand *)(((uint8_t *)this) + 4);
  }

  const UIIconCommand *icon() const {
    assert(command == UI_COMMAND_ICON);
    return (UIIconCommand *)(((uint8_t *)this) + 4);
  }

  uint32_t set_type(UI_COMMAND command, uint32_t text_length) {
    this->command = command;
    if (this->command == UI_COMMAND_TEXT) {
      this->text()->length = text_length;
    }
    auto s = this->size();
    return s;
  }
#endif
};
static_assert(sizeof(struct UICommandHeader) == 4);

struct UICommandRange {
  uint32_t head;
  uint32_t tail;
};

struct UIRenderFrame {
  const struct UICommandRange *command_groups;
  uint32_t command_group_count;
  const uint8_t *command_buffer;

#ifdef __cplusplus
  const UICommandRange *begin() const { return command_groups; }
  const UICommandRange *end() const {
    return command_groups + command_group_count;
  }
#endif
};
