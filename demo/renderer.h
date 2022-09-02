#pragma once

void r_init(void);
int r_get_text_width(const char *text, int len);
int r_get_text_height(void);
void render(int width, int height, float bg[4],
            const struct UIRenderFrame *command);
