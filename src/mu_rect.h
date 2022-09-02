#pragma once
#include "mu_vec2.h"

struct mu_Rect {
  int x, y, w, h;
  mu_Rect() : x(0), y(0), w(0), h(0) {}
  mu_Rect(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}

  static const mu_Rect unclipped_rect;

  mu_Rect expand(int n) const {
    return mu_Rect(x - n, y - n, w + n * 2, h + n * 2);
  }

  mu_Rect intersect(mu_Rect r2) const {
    int x1 = mu_max(this->x, r2.x);
    int y1 = mu_max(this->y, r2.y);
    int x2 = mu_min(this->x + this->w, r2.x + r2.w);
    int y2 = mu_min(this->y + this->h, r2.y + r2.h);
    if (x2 < x1) {
      x2 = x1;
    }
    if (y2 < y1) {
      y2 = y1;
    }
    return mu_Rect(x1, y1, x2 - x1, y2 - y1);
  }

  bool overlaps_vec2(mu_Vec2 p) const {
    return p.x >= this->x && p.x < this->x + this->w && p.y >= this->y &&
           p.y < this->y + this->h;
  }
};

inline const mu_Rect mu_Rect::unclipped_rect = mu_Rect(0, 0, 0x1000000, 0x1000000);
