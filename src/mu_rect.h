#pragma once

struct mu_Rect {
  int x, y, w, h;
  mu_Rect() : x(0), y(0), w(0), h(0) {}
  mu_Rect(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}
  mu_Rect expand(int n) const {
    return mu_Rect(x - n, y - n, w + n * 2, h + n * 2);
  }
};
