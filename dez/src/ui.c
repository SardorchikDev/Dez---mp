#include "ui.h"
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <time.h>
#include <math.h>

static int g_ui_initialized = 0;

void ui_init(void) {
    if (!initscr()) {
        fprintf(stderr, "Failed to initialize ncurses\n");
        exit(1);
    }
    
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);
    
    srand(time(NULL));
    
    mousemask(BUTTON1_CLICKED, NULL);
    mouseinterval(0);
    
    if (has_colors()) {
        start_color();
        use_default_colors();
        
        init_pair(1, COLOR_GREEN, -1);
        init_pair(2, COLOR_RED, -1);
        init_pair(3, COLOR_YELLOW, -1);
        init_pair(4, COLOR_CYAN, -1);
        init_pair(5, COLOR_MAGENTA, -1);
        init_pair(6, COLOR_BLUE, -1);
        init_pair(7, COLOR_WHITE, -1);
        init_pair(8, COLOR_BLACK, COLOR_WHITE);
        init_pair(9, COLOR_WHITE, COLOR_BLUE);
        init_pair(10, COLOR_GREEN, COLOR_BLACK);
        init_pair(11, COLOR_CYAN, COLOR_BLACK);
        
        for (int i = 0; i < 7; i++) {
            init_pair(20 + i, COLOR_RED + i, -1);
        }
    }
    
    g_ui_initialized = 1;
}

void format_time(float seconds, char *buffer, size_t size) {
    if (!buffer || size < 6) return;
    
    int mins = (int)seconds / 60;
    int secs = (int)seconds % 60;
    snprintf(buffer, size, "%02d:%02d", mins, secs);
}

void draw_box(int start_y, int start_x, int width, int height, const char *title) {
    if (width < 4 || height < 2) return;
    
    mvaddch(start_y, start_x, ACS_ULCORNER);
    for (int i = 1; i < width - 1; i++) {
        mvaddch(start_y, start_x + i, ACS_HLINE);
    }
    mvaddch(start_y, start_x + width - 1, ACS_URCORNER);
    
    for (int i = 1; i < height - 1; i++) {
        mvaddch(start_y + i, start_x, ACS_VLINE);
        mvaddch(start_y + i, start_x + width - 1, ACS_VLINE);
    }
    
    mvaddch(start_y + height - 1, start_x, ACS_LLCORNER);
    for (int i = 1; i < width - 1; i++) {
        mvaddch(start_y + height - 1, start_x + i, ACS_HLINE);
    }
    mvaddch(start_y + height - 1, start_x + width - 1, ACS_LRCORNER);
    
    if (title && title[0]) {
        int title_len = strlen(title);
        int title_x = start_x + (width - title_len - 2) / 2;
        if (title_x > start_x) {
            mvaddch(start_y, title_x - 1, ' ');
            mvprintw(start_y, title_x, " %s ", title);
            mvaddch(start_y, title_x + title_len + 1, ' ');
        }
    }
}

void draw_button(int y, int x, const char *label, int active) {
    if (!label) return;
    
    if (active) {
        attron(COLOR_PAIR(9) | A_BOLD);
        mvprintw(y, x, "[*%s]", label);
        attroff(COLOR_PAIR(9) | A_BOLD);
    } else {
        attron(COLOR_PAIR(6));
        mvprintw(y, x, "[ %s ]", label);
        attroff(COLOR_PAIR(6));
    }
}

void draw_spectrum(MusicPlayer *player, int start_y, int start_x, int width, int height) {
    if (!player || width < 6 || height < 3) return;
    
    int bar_count = (width - 2) / 3;
    if (bar_count > SPECTRUM_SIZE) bar_count = SPECTRUM_SIZE;
    if (bar_count < 8) bar_count = 8;
    
    for (int bar = 0; bar < bar_count; bar++) {
        float val = player->spectrum[bar % SPECTRUM_SIZE];
        int bar_height = (int)(val * height);
        if (bar_height > height) bar_height = height;
        if (bar_height < 0) bar_height = 0;
        
        int bar_x = start_x + bar * 3;
        if (bar_x + 2 >= start_x + width) break;
        
        for (int row = 0; row < height; row++) {
            int y = start_y + (height - 1 - row);
            
            if (row < bar_height) {
                int color_level = row * 6 / height;
                if (color_level > 6) color_level = 6;
                
                if (has_colors()) {
                    if (color_level < 2) {
                        attron(COLOR_PAIR(20));
                    } else if (color_level < 4) {
                        attron(COLOR_PAIR(22));
                    } else {
                        attron(COLOR_PAIR(24));
                    }
                }
                
                mvaddch(y, bar_x, '|');
                mvaddch(y, bar_x + 1, '|');
                
                if (has_colors()) {
                    attroff(COLOR_PAIR(20 + color_level));
                }
            }
        }
    }
}

void ui_draw(MusicPlayer *player, const char *filename) {
    if (!stdscr || !player) return;
    
    clear();
    
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    
    if (max_x < 40 || max_y < 15) {
        mvprintw(0, 0, "Terminal too small");
        refresh();
        return;
    }
    
    int header_height = 1;
    int info_height = 8;
    int spectrum_height = 8;
    int playlist_height = max_y - header_height - info_height - spectrum_height - 6;
    int status_height = 3;
    
    int logo_width = 24;
    int logo_x = (max_x - logo_width) / 2;
    if (logo_x < 0) logo_x = 0;
    
    attron(COLOR_PAIR(6) | A_BOLD);
    mvprintw(0, logo_x, "=== DEZ Music Player ===");
    attroff(COLOR_PAIR(6) | A_BOLD);
    
    mvhline(header_height, 0, ACS_HLINE, max_x);
    
    draw_box(header_height + 1, 1, max_x - 2, info_height, " Now Playing ");
    
    attron(COLOR_PAIR(10) | A_BOLD);
    
    char display_title[MAX_TITLE_LEN + MAX_ARTIST_LEN + 10];
    if (player->metadata.title[0] && player->metadata.artist[0]) {
        snprintf(display_title, sizeof(display_title), "%s - %s",
                player->metadata.title, player->metadata.artist);
    } else if (player->metadata.title[0]) {
        snprintf(display_title, sizeof(display_title), "%s", player->metadata.title);
    } else if (filename && *filename) {
        char *base = basename(strdup(filename));
        snprintf(display_title, sizeof(display_title), "%s", base);
        free(base);
    } else {
        snprintf(display_title, sizeof(display_title), "No file loaded");
    }
    
    int title_x = (max_x - (int)strlen(display_title)) / 2;
    if (title_x < 3) title_x = 3;
    if ((int)strlen(display_title) > max_x - 6) {
        display_title[max_x - 9] = '\0';
        strcat(display_title, "...");
    }
    mvprintw(header_height + 2, title_x, "%s", display_title);
    attroff(COLOR_PAIR(10) | A_BOLD);
    
    if (player_is_paused(player)) {
        attron(COLOR_PAIR(3) | A_BOLD);
        mvprintw(header_height + 3, 3, "|| PAUSED");
        attroff(COLOR_PAIR(3) | A_BOLD);
    } else if (player_is_playing(player)) {
        attron(COLOR_PAIR(1) | A_BOLD);
        mvprintw(header_height + 3, 3, ">> PLAYING");
        attroff(COLOR_PAIR(1) | A_BOLD);
    } else {
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(header_height + 3, 3, "[] STOPPED");
        attroff(COLOR_PAIR(2) | A_BOLD);
    }
    
    char current_time_str[16] = "--:--";
    char total_time_str[16] = "--:--";
    float dur = player_get_duration(player);
    if (dur > 0) {
        format_time(player_get_current_time(player), current_time_str, sizeof(current_time_str));
        format_time(dur, total_time_str, sizeof(total_time_str));
    }
    
    mvprintw(header_height + 4, max_x - 20, "%s / %s", current_time_str, total_time_str);
    
    float progress = player_get_progress(player);
    int bar_width = max_x - 10;
    int filled = (int)(progress * bar_width);
    
    mvaddch(header_height + 5, 3, '[');
    attron(COLOR_PAIR(4) | A_BOLD);
    for (int i = 0; i < filled; i++) {
        addch('#');
    }
    attroff(COLOR_PAIR(4) | A_BOLD);
    for (int i = filled; i < bar_width; i++) {
        addch('-');
    }
    mvaddch(header_height + 5, max_x - 4, ']');
    
    int vol_width = 15;
    int vol_filled = (int)(player->volume * vol_width);
    mvprintw(header_height + 6, 3, "Volume: ");
    attron(COLOR_PAIR(5) | A_BOLD);
    for (int i = 0; i < vol_filled; i++) addch('#');
    attroff(COLOR_PAIR(5) | A_BOLD);
    for (int i = vol_filled; i < vol_width; i++) addch('-');
    printw(" %3.0f%%", player->volume * 100);
    
    int mode_x = 3;
    int mode_y = header_height + 7;
    draw_button(mode_y, mode_x, "LOOP", player->loop);
    draw_button(mode_y, mode_x + 10, "SHUF", player->shuffle);
    draw_button(mode_y, mode_x + 20, "RPT", player->repeat);
    
    if (player->is_radio) {
        attron(COLOR_PAIR(11) | A_BOLD);
        mvprintw(mode_y, max_x - 20, "[RADIO]");
        attroff(COLOR_PAIR(11) | A_BOLD);
    }
    
    int spectrum_y = header_height + info_height + 1;
    draw_box(spectrum_y, 1, max_x - 2, spectrum_height, " CAVA ");
    player_update_spectrum(player);
    draw_spectrum(player, spectrum_y + 1, 2, max_x - 4, spectrum_height - 2);
    
    int playlist_y = spectrum_y + spectrum_height + 1;
    if (playlist_height > 4 && player->playlist_count > 0) {
        draw_box(playlist_y, 1, max_x - 2, playlist_height, " Playlist ");
        
        int display_start = 0;
        if (player->current_track >= playlist_height - 2) {
            display_start = player->current_track - (playlist_height - 3);
        }
        
        for (int i = display_start; i < player->playlist_count && 
             i < display_start + playlist_height - 2; i++) {
            int line = playlist_y + 1 + (i - display_start) + 1;
            
            if (i == player->current_track) {
                attron(COLOR_PAIR(9) | A_BOLD);
            }
            
            char *dup = strdup(player->playlist[i]);
            if (dup) {
                char *base = basename(dup);
                int max_len = max_x - 8;
                if ((int)strlen(base) > max_len) {
                    base[max_len - 3] = '\0';
                    strcat(base, "...");
                }
                
                if (i == player->current_track) {
                    mvprintw(line, 3, "> %s", base);
                } else {
                    mvprintw(line, 3, "  %s", base);
                }
                free(dup);
            }
            
            if (i == player->current_track) {
                attroff(COLOR_PAIR(9) | A_BOLD);
            }
        }
    }
    
    int controls_y = max_y - status_height;
    mvhline(controls_y - 1, 0, ACS_HLINE, max_x);
    
    attron(COLOR_PAIR(6) | A_BOLD);
    mvprintw(controls_y, 1, "Space:Play/Pause  X:Stop  Q:Quit  +/-:Vol  </>:Seek10s  Left/Right:Seek1s");
    mvprintw(controls_y + 1, 1, "L:Loop  H:Shuffle  R:Repeat  B:Browse  Up/Down:Tracks  I:Radio");
    attroff(COLOR_PAIR(6) | A_BOLD);
    
    refresh();
}

void ui_cleanup(void) {
    if (g_ui_initialized) {
        endwin();
        g_ui_initialized = 0;
    }
}

int ui_get_key(void) {
    if (!stdscr) return 'q';
    return getch();
}
