#pragma once
typedef unsigned int mu_Id;
#define MU_REAL float
typedef void *mu_Font;
typedef MU_REAL mu_Real;

enum MU_OPT {
  MU_OPT_NONE = 0,
  MU_OPT_ALIGNCENTER = (1 << 0),
  MU_OPT_ALIGNRIGHT = (1 << 1),
  MU_OPT_NOINTERACT = (1 << 2),
  MU_OPT_NOFRAME = (1 << 3),
  MU_OPT_NORESIZE = (1 << 4),
  MU_OPT_NOSCROLL = (1 << 5),
  MU_OPT_NOCLOSE = (1 << 6),
  MU_OPT_NOTITLE = (1 << 7),
  MU_OPT_HOLDFOCUS = (1 << 8),
  MU_OPT_AUTOSIZE = (1 << 9),
  MU_OPT_POPUP = (1 << 10),
  MU_OPT_CLOSED = (1 << 11),
  MU_OPT_EXPANDED = (1 << 12)
};

enum FOCUS_STATE {
  FOCUS_STATE_NONE,
  FOCUS_STATE_HOVER,
  FOCUS_STATE_FOCUS,
};
