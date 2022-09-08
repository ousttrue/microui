#pragma once
#include "mu_stack.h"
#include <UIRenderFrame.h>

enum class MU_CLIP : unsigned int { NONE, PART, ALL };

const auto MU_CLIPSTACK_SIZE = 32;

class ClipStack {
  mu_Stack<UIRect, MU_CLIPSTACK_SIZE> clip_stack;

public:
  void end() const { assert(clip_stack.size() == 0); }
  UIRect intersect(const UIRect &rect) {
    return rect.intersect(this->clip_stack.back());
  }

  static UIRect unclipped_rect() { return UIRect(0, 0, 0x1000000, 0x1000000); }

  void push_unclipped_rect() { clip_stack.push(unclipped_rect()); }

  void push(UIRect rect) {
    UIRect last = this->clip_stack.back();
    this->clip_stack.push(rect.intersect(last));
  }
  UIRect back() const { return clip_stack.back(); }
  void pop() { this->clip_stack.pop(); }

  MU_CLIP check_clip(UIRect r) {
    UIRect cr = this->clip_stack.back();
    if (r.x > cr.x + cr.w || r.x + r.w < cr.x || r.y > cr.y + cr.h ||
        r.y + r.h < cr.y) {
      return MU_CLIP::ALL;
    }
    if (r.x >= cr.x && r.x + r.w <= cr.x + cr.w && r.y >= cr.y &&
        r.y + r.h <= cr.y + cr.h) {
      return MU_CLIP::NONE;
    }
    return MU_CLIP::PART;
  }
};
