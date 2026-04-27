#ifndef PLAYER_H
#define PLAYER_H

#include <SDL2/SDL_mixer.h>
#include <stddef.h>
#include <limits.h>

#define MAX_TITLE_LEN 256
#define MAX_ARTIST_LEN 256
#define MAX_ALBUM_LEN 256
#define MAX_YEAR_LEN 16
#define MAX_GENRE_LEN 64
#define MAX_LYRIC_TEXT 256
#define MAX_RADIO_STATIONS 50
#define MAX_PLAYLIST_SIZE 1000
#define SPECTRUM_SIZE 256

typedef struct {
    char title[MAX_TITLE_LEN];
    char artist[MAX_ARTIST_LEN];
    char album[MAX_ALBUM_LEN];
    char year[MAX_YEAR_LEN];
    char genre[MAX_GENRE_LEN];
} Metadata;

typedef struct {
    float time;
    char text[MAX_LYRIC_TEXT];
} LyricLine;

typedef struct {
    LyricLine *lines;
    int count;
    int capacity;
} Lyrics;

typedef struct {
    char name[128];
    char url[512];
    char genre[64];
    int bitrate;
} RadioStation;

typedef struct {
    Mix_Music *music;
    float volume;
    double duration;
    int loop;
    int shuffle;
    int repeat;
    char **playlist;
    int playlist_count;
    int playlist_capacity;
    int current_track;
    char current_filename[PATH_MAX];
    Metadata metadata;
    Lyrics lyrics;
    RadioStation *radio_stations;
    int radio_count;
    int current_radio;
    float saved_time;
    float spectrum[SPECTRUM_SIZE];
    int is_radio;
    char radio_url[512];
} MusicPlayer;

int player_init(MusicPlayer *player);
int player_load(MusicPlayer *player, const char *filename);
void player_play(MusicPlayer *player);
void player_pause(MusicPlayer *player);
void player_toggle(MusicPlayer *player);
void player_stop(MusicPlayer *player);
void player_cleanup(MusicPlayer *player);
float player_get_progress(MusicPlayer *player);
void player_set_volume(MusicPlayer *player, float volume);
void player_seek(MusicPlayer *player, float position);
float player_get_current_time(MusicPlayer *player);
float player_get_duration(MusicPlayer *player);
int player_is_playing(MusicPlayer *player);
int player_is_paused(MusicPlayer *player);
void player_toggle_loop(MusicPlayer *player);
void player_load_playlist(MusicPlayer *player, char **files, int count);
void player_load_track(MusicPlayer *player, int index);
void player_next_track(MusicPlayer *player);
void player_prev_track(MusicPlayer *player);
void player_extract_metadata(MusicPlayer *player, const char *filename);
void player_load_lyrics(MusicPlayer *player, const char *filename);
void player_save_playlist(MusicPlayer *player, const char *filename);
void player_load_playlist_from_file(MusicPlayer *player, const char *filename);
void player_init_radio_stations(MusicPlayer *player);
void player_play_radio(MusicPlayer *player, int index);
void player_save_state(MusicPlayer *player);
void player_load_state(MusicPlayer *player);
void player_toggle_shuffle(MusicPlayer *player);
void player_toggle_repeat(MusicPlayer *player);
void player_shuffle_playlist(MusicPlayer *player);
void player_update_spectrum(MusicPlayer *player);
int player_add_to_playlist(MusicPlayer *player, const char *filepath);

#endif
