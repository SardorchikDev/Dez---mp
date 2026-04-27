#include "player.h"
#include "ui.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libgen.h>
#include <limits.h>

void print_usage(const char *prog) {
    printf("DEZ Music Player v2.0\n");
    printf("Usage: %s [audio_file_or_directory]\n", prog);
    printf("\nControls:\n");
    printf("  Space     - Play/Pause\n");
    printf("  X         - Stop\n");
    printf("  Q         - Quit\n");
    printf("  +/-       - Volume Up/Down\n");
    printf("  </>       - Seek +/- 10s\n");
    printf("  Left/Right- Seek +/- 1s\n");
    printf("  L         - Toggle Loop\n");
    printf("  H         - Toggle Shuffle\n");
    printf("  R         - Toggle Repeat\n");
    printf("  Up/Down   - Next/Previous Track\n");
    printf("  B         - File Browser\n");
    printf("  I         - Radio Browser\n");
    printf("  S         - Save Playlist\n");
    printf("  D         - Load Playlist\n");
}

int is_audio_file(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return 0;
    
    const char *audio_exts[] = {
        ".wav", ".mp3", ".ogg", ".flac", ".aac", 
        ".wma", ".m4a", ".opus", ".webm", ".mid", NULL
    };
    
    for (int i = 0; audio_exts[i]; i++) {
        if (strcasecmp(ext, audio_exts[i]) == 0) return 1;
    }
    return 0;
}

void show_radio_browser(MusicPlayer *player) {
    if (!player || player->radio_count == 0) return;
    
    int selection = 0;
    int running = 1;
    int scroll_offset = 0;

    while (running) {
        clear();
        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);

        mvprintw(0, 1, "╔════════════════════════════════════════╗");
        mvprintw(1, 1, "║       📻  Radio Station Browser        ║");
        mvprintw(2, 1, "╚════════════════════════════════════════╝");
        
        int display_count = max_y - 8;
        if (display_count < 1) display_count = 1;

        if (selection < scroll_offset) {
            scroll_offset = selection;
        } else if (selection >= scroll_offset + display_count) {
            scroll_offset = selection - display_count + 1;
        }

        for (int i = scroll_offset; i < player->radio_count && 
             i < scroll_offset + display_count; i++) {
            if (i == selection) {
                attron(A_REVERSE | A_BOLD);
            }
            
            char line[512];
            snprintf(line, sizeof(line), "%-30s [%s] %dkbps",
                    player->radio_stations[i].name,
                    player->radio_stations[i].genre,
                    player->radio_stations[i].bitrate);
            
            mvprintw(4 + (i - scroll_offset), 2, "%.*s", max_x - 4, line);
            
            if (i == selection) {
                attroff(A_REVERSE | A_BOLD);
            }
        }

        mvprintw(max_y - 3, 2, "Use Up/Down to navigate, Enter to play, Q to go back");
        mvprintw(max_y - 2, 2, "Stations: %d  Selected: %d/%d", 
                player->radio_count, selection + 1, player->radio_count);
        refresh();

        int ch = ui_get_key();
        switch (ch) {
            case KEY_UP:
                if (selection > 0) selection--;
                break;
            case KEY_DOWN:
                if (selection < player->radio_count - 1) selection++;
                break;
            case '\n':
            case KEY_ENTER:
                player_play_radio(player, selection);
                running = 0;
                break;
            case 'q':
            case 'Q':
                running = 0;
                break;
        }
    }
}

void show_file_browser(MusicPlayer *player) {
    if (!player) return;
    
    char current_dir[PATH_MAX] = ".";
    char **entries = NULL;
    int *entry_types = NULL;
    int entry_count = 0;
    int selection = 0;
    int scroll_offset = 0;
    int running = 1;

    while (running) {
        clear();
        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);

        char title[PATH_MAX + 20];
        snprintf(title, sizeof(title), "[DIR] %s", current_dir);
        int title_len = strlen(title);
        if (title_len > max_x - 4) {
            title[max_x - 7] = '\0';
            strcat(title, "...");
        }
        
        mvhline(0, 0, '=', max_x);
        mvprintw(1, 1, " %-*s ", max_x - 2, title);
        mvhline(2, 0, '=', max_x);

        if (entries) {
            for (int i = 0; i < entry_count; i++) {
                free(entries[i]);
            }
            free(entries);
            free(entry_types);
        }
        entries = NULL;
        entry_types = NULL;
        entry_count = 0;

        DIR *dir = opendir(current_dir);
        if (!dir) {
            mvprintw(4, 2, "Cannot open directory: %s", current_dir);
            mvprintw(max_y - 2, 2, "Press Q to quit");
            refresh();
            int ch;
            nodelay(stdscr, FALSE);
            while ((ch = getch()) != 'q' && ch != 'Q');
            nodelay(stdscr, TRUE);
            return;
        }

        entries = realloc(entries, sizeof(char*) * 1);
        entry_types = realloc(entry_types, sizeof(int) * 1);
        entries[0] = strdup("..");
        entry_types[0] = 1;
        entry_count = 1;

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || 
                strcmp(entry->d_name, "..") == 0) continue;

            char path[PATH_MAX];
            snprintf(path, sizeof(path), "%s/%s", current_dir, entry->d_name);
            struct stat st;
            int is_dir = (stat(path, &st) == 0 && S_ISDIR(st.st_mode));

            if (is_dir || is_audio_file(entry->d_name)) {
                entries = realloc(entries, sizeof(char*) * (entry_count + 1));
                entry_types = realloc(entry_types, sizeof(int) * (entry_count + 1));
                entries[entry_count] = strdup(entry->d_name);
                entry_types[entry_count] = is_dir ? 1 : 0;
                entry_count++;
            }
        }
        closedir(dir);

        for (int i = 1; i < entry_count - 1; i++) {
            for (int j = i + 1; j < entry_count; j++) {
                int should_swap = 0;
                
                if (entry_types[i] && !entry_types[j]) {
                    should_swap = 0;
                } else if (!entry_types[i] && entry_types[j]) {
                    should_swap = 1;
                } else if (strcmp(entries[i], entries[j]) > 0) {
                    should_swap = 1;
                }

                if (should_swap) {
                    char *temp = entries[i];
                    entries[i] = entries[j];
                    entries[j] = temp;
                    int temp_type = entry_types[i];
                    entry_types[i] = entry_types[j];
                    entry_types[j] = temp_type;
                }
            }
        }

        int display_count = max_y - 8;
        if (display_count < 1) display_count = 1;

        if (selection < scroll_offset) {
            scroll_offset = selection;
        } else if (selection >= scroll_offset + display_count) {
            scroll_offset = selection - display_count + 1;
        }

        for (int i = scroll_offset; i < entry_count && 
             i < scroll_offset + display_count; i++) {
            if (i == selection) {
                attron(A_REVERSE | A_BOLD);
            }

            mvprintw(4 + (i - scroll_offset), 2, "%c %s", 
                    entry_types[i] ? 'D' : ' ', entries[i]);

            if (i == selection) {
                attroff(A_REVERSE | A_BOLD);
            }
        }

        mvprintw(max_y - 3, 2, "Up/Down:Navigate  Enter:Select  Backspace:Up  Q:Quit");
        mvprintw(max_y - 2, 2, "Files: %d  Selected: %d", entry_count, selection + 1);
        refresh();

        int ch = getch();
        switch (ch) {
            case KEY_UP:
                if (selection > 0) selection--;
                break;
            case KEY_DOWN:
                if (selection < entry_count - 1) selection++;
                break;
            case '\n':
            case '\r':
            case KEY_ENTER:
                {
                    char selected_path[PATH_MAX];
                    snprintf(selected_path, sizeof(selected_path), "%s/%s",
                            current_dir, entries[selection]);

                    if (entry_types[selection]) {
                        if (strcmp(entries[selection], "..") == 0) {
                            char *parent = dirname(strdup(current_dir));
                            strncpy(current_dir, parent, sizeof(current_dir) - 1);
                            free(parent);
                        } else {
                            strncpy(current_dir, selected_path, sizeof(current_dir) - 1);
                        }
                        selection = 0;
                        scroll_offset = 0;
                    } else if (is_audio_file(entries[selection])) {
                        player_stop(player);
                        if (player_load(player, selected_path) == 0) {
                            player_play(player);
                        }
                        running = 0;
                    }
                }
                break;
            case KEY_BACKSPACE:
            case 127:
                {
                    char *parent = dirname(strdup(current_dir));
                    strncpy(current_dir, parent, sizeof(current_dir) - 1);
                    free(parent);
                    selection = 0;
                    scroll_offset = 0;
                }
                break;
            case 'q':
            case 'Q':
                running = 0;
                break;
        }
    }

    if (entries) {
        for (int i = 0; i < entry_count; i++) {
            free(entries[i]);
        }
        free(entries);
        free(entry_types);
    }
}

int scan_directory(const char *dir_path, MusicPlayer *player) {
    if (!dir_path || !player) return 0;
    
    DIR *dir = opendir(dir_path);
    if (!dir) return 0;
    
    int count = 0;
    struct dirent *entry;
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && is_audio_file(entry->d_name)) {
            char full_path[PATH_MAX];
            if ((int)snprintf(full_path, sizeof(full_path), "%s/%s", 
                        dir_path, entry->d_name) < (int)sizeof(full_path)) {
                if (player_add_to_playlist(player, full_path) == 0) {
                    count++;
                }
            }
        }
    }
    
    closedir(dir);
    return count;
}

int main(int argc, char *argv[]) {
    if (argc > 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        print_usage(argv[0]);
        return 0;
    }

    MusicPlayer player;
    memset(&player, 0, sizeof(MusicPlayer));

    if (player_init(&player) != 0) {
        fprintf(stderr, "Failed to initialize player\n");
        return 1;
    }

    player_load_state(&player);

    char scan_dir[PATH_MAX] = ".";
    int files_scanned = 0;
    
    if (argc == 2) {
        struct stat st;
        if (stat(argv[1], &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                strncpy(scan_dir, argv[1], sizeof(scan_dir) - 1);
                files_scanned = scan_directory(scan_dir, &player);
            } else if (is_audio_file(argv[1])) {
                char *dir_part = dirname(strdup(argv[1]));
                strncpy(scan_dir, dir_part, sizeof(scan_dir) - 1);
                free(dir_part);
                files_scanned = scan_directory(scan_dir, &player);
                
                for (int i = 0; i < player.playlist_count; i++) {
                    if (strcmp(player.playlist[i], argv[1]) == 0) {
                        player.current_track = i;
                        break;
                    }
                }
            }
        }
    }
    
    if (files_scanned == 0) {
        files_scanned = scan_directory(".", &player);
    }

    ui_init();

    if (player.playlist_count > 0 && player.current_track >= 0) {
        player_load_track(&player, player.current_track);
    } else if (player.playlist_count > 0) {
        int selection = 0;
        int running = 1;
        
        while (running && player.playlist_count > 0) {
            clear();
            int max_y, max_x;
            getmaxyx(stdscr, max_y, max_x);

            mvprintw(1, (max_x - 30) / 2, "╔════════════════════════════╗");
            mvprintw(2, (max_x - 30) / 2, "║   Select a track to play   ║");
            mvprintw(3, (max_x - 30) / 2, "╚════════════════════════════╝");
            mvprintw(5, 2, "Use ↑/↓ arrows, Enter to select, Q to quit");

            int max_display = max_y - 10;
            int start = 0;
            if (selection >= max_display) {
                start = selection - max_display + 1;
            }
            
            for (int i = start; i < player.playlist_count && 
                 i < start + max_display; i++) {
                if (!player.playlist[i] || !*player.playlist[i]) continue;
                
                if (i == selection) {
                    attron(COLOR_PAIR(9) | A_BOLD);
                }
                
                char *dup = strdup(player.playlist[i]);
                if (dup) {
                    char *basename_file = basename(dup);
                    mvprintw(7 + (i - start), 4, "%c %.*s", 
                            i == selection ? '>' : ' ', max_x - 8, basename_file);
                    free(dup);
                }
                
                if (i == selection) {
                    attroff(COLOR_PAIR(9) | A_BOLD);
                }
            }

            refresh();

            int ch = getch();
            switch (ch) {
                case KEY_UP:
                    selection = (selection - 1 + player.playlist_count) % player.playlist_count;
                    break;
                case KEY_DOWN:
                    selection = (selection + 1) % player.playlist_count;
                    break;
                case '\n':
                case '\r':
                case KEY_ENTER:
                    player.current_track = selection;
                    running = 0;
                    break;
                case 'q':
                case 'Q':
                    ui_cleanup();
                    player_cleanup(&player);
                    return 0;
            }
        }
        
        if (player.current_track >= 0) {
            player_load_track(&player, player.current_track);
        }
    }

    int running = 1;
    while (running) {
        ui_draw(&player, player.current_filename);

        int ch = ui_get_key();
        switch (ch) {
            case KEY_MOUSE:
                {
                    MEVENT event;
                    if (getmouse(&event) == OK) {
                        int max_y, max_x;
                        getmaxyx(stdscr, max_y, max_x);
                        (void)max_y;
                        
                        if (event.y == 7 && event.x >= 3 && event.x <= max_x - 4) {
                            int bar_width = max_x - 10;
                            float click_pos = (float)(event.x - 3) / bar_width;
                            if (click_pos < 0) click_pos = 0;
                            if (click_pos > 1) click_pos = 1;
                            player_seek(&player, click_pos);
                        }
                    }
                }
                break;
            case 'q':
            case 'Q':
                running = 0;
                break;
            case ' ':
                player_toggle(&player);
                break;
            case 'x':
            case 'X':
                player_stop(&player);
                break;
            case '+':
            case '=':
                player_set_volume(&player, player.volume + 0.05f);
                break;
            case '-':
            case '_':
                player_set_volume(&player, player.volume - 0.05f);
                break;
            case '<':
            case ',':
                {
                    float current = player_get_current_time(&player);
                    float new_time = current - 10.0f;
                    if (new_time < 0) new_time = 0;
                    float duration = player_get_duration(&player);
                    if (duration > 0) {
                        player_seek(&player, new_time / duration);
                    }
                }
                break;
            case '>':
            case '.':
                {
                    float current = player_get_current_time(&player);
                    float new_time = current + 10.0f;
                    float duration = player_get_duration(&player);
                    if (new_time > duration) new_time = duration;
                    if (duration > 0) {
                        player_seek(&player, new_time / duration);
                    }
                }
                break;
            case KEY_LEFT:
                {
                    float current = player_get_current_time(&player);
                    float new_time = current - 1.0f;
                    if (new_time < 0) new_time = 0;
                    float duration = player_get_duration(&player);
                    if (duration > 0) {
                        player_seek(&player, new_time / duration);
                    }
                }
                break;
            case KEY_RIGHT:
                {
                    float current = player_get_current_time(&player);
                    float new_time = current + 1.0f;
                    float duration = player_get_duration(&player);
                    if (new_time > duration) new_time = duration;
                    if (duration > 0) {
                        player_seek(&player, new_time / duration);
                    }
                }
                break;
            case 'l':
            case 'L':
                player_toggle_loop(&player);
                break;
            case 'b':
            case 'B':
                show_file_browser(&player);
                break;
            case 'r':
            case 'R':
                player_toggle_repeat(&player);
                break;
            case 'h':
            case 'H':
                player_toggle_shuffle(&player);
                break;
            case 's':
            case 'S':
                player_save_playlist(&player, "playlist.txt");
                break;
            case 'd':
            case 'D':
                player_load_playlist_from_file(&player, "playlist.txt");
                break;
            case 'i':
            case 'I':
                show_radio_browser(&player);
                break;
            case KEY_UP:
                player_prev_track(&player);
                break;
            case KEY_DOWN:
                player_next_track(&player);
                break;
        }
        
        SDL_Delay(50);
    }

    player_save_state(&player);
    ui_cleanup();
    player_cleanup(&player);
    return 0;
}
