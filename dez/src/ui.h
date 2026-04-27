#ifndef UI_H
#define UI_H

#include <ncurses.h>
#include "player.h"

void ui_init(void);
void ui_draw(MusicPlayer *player, const char *filename);
void ui_cleanup(void);
int ui_get_key(void);
void format_time(float seconds, char *buffer, size_t size);
void draw_spectrum(MusicPlayer *player, int start_y, int start_x, int width, int height);
void draw_box(int start_y, int start_x, int width, int height, const char *title);
void draw_button(int y, int x, const char *label, int active);

#endif
