#include "player.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <libgen.h>

static int g_audio_initialized = 0;



int player_init(MusicPlayer *player) {
    if (!player) return -1;
    
    memset(player, 0, sizeof(MusicPlayer));
    
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "SDL audio init failed: %s\n", SDL_GetError());
        return -1;
    }
    
    int audio_rate = 44100;
    Uint16 audio_format = MIX_DEFAULT_FORMAT;
    int audio_channels = 2;
    int audio_buffers = 2048;
    
    if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) < 0) {
        fprintf(stderr, "SDL_mixer init failed: %s\n", Mix_GetError());
        SDL_Quit();
        return -1;
    }
    
    Mix_AllocateChannels(16);
    Mix_SetPostMix(NULL, NULL);
    
    player->music = NULL;
    player->volume = 0.8f;
    player->duration = 0.0;
    player->loop = 0;
    player->shuffle = 0;
    player->repeat = 0;
    player->playlist = NULL;
    player->playlist_count = 0;
    player->playlist_capacity = 0;
    player->current_track = -1;
    player->current_filename[0] = '\0';
    player->lyrics.lines = NULL;
    player->lyrics.count = 0;
    player->lyrics.capacity = 0;
    player->radio_stations = NULL;
    player->radio_count = 0;
    player->current_radio = -1;
    player->saved_time = 0.0f;
    player->is_radio = 0;
    player->radio_url[0] = '\0';
    
    memset(player->spectrum, 0, sizeof(player->spectrum));
    
    player_init_radio_stations(player);
    
    g_audio_initialized = 1;
    fprintf(stdout, "Audio initialized: %dHz, %d channels\n", audio_rate, audio_channels);
    
    return 0;
}

int player_load(MusicPlayer *player, const char *filename) {
    if (!player || !filename || !*filename) {
        return -1;
    }
    
    if (!g_audio_initialized) {
        fprintf(stderr, "Audio not initialized\n");
        return -1;
    }
    
    player_stop(player);
    
    if (player->music) {
        Mix_FreeMusic(player->music);
        player->music = NULL;
    }
    
    player->is_radio = 0;
    player->radio_url[0] = '\0';
    
    player->music = Mix_LoadMUS(filename);
    if (!player->music) {
        fprintf(stderr, "Failed to load '%s': %s\n", filename, Mix_GetError());
        return -1;
    }
    
    player->duration = Mix_MusicDuration(player->music);
    if (player->duration < 0) {
        player->duration = 0.0;
    }
    
    strncpy(player->current_filename, filename, sizeof(player->current_filename) - 1);
    player->current_filename[sizeof(player->current_filename) - 1] = '\0';
    
    player_extract_metadata(player, filename);
    player_load_lyrics(player, filename);
    
    Mix_VolumeMusic((int)(player->volume * MIX_MAX_VOLUME));
    
    fprintf(stdout, "Loaded: %s (duration: %.2fs)\n", filename, player->duration);
    
    return 0;
}

int player_load_stream(MusicPlayer *player, const char *url, const char *name) {
    if (!player || !url || !name) {
        return -1;
    }
    
    player_stop(player);
    
    if (player->music) {
        Mix_FreeMusic(player->music);
        player->music = NULL;
    }
    
    player->music = Mix_LoadMUS(url);
    if (!player->music) {
        fprintf(stderr, "Failed to load stream '%s': %s\n", url, Mix_GetError());
        return -1;
    }
    
    player->is_radio = 1;
    player->duration = 0.0;
    strncpy(player->radio_url, url, sizeof(player->radio_url) - 1);
    snprintf(player->metadata.title, sizeof(player->metadata.title), "%s", name);
    player->metadata.artist[0] = '\0';
    player->metadata.album[0] = '\0';
    strncpy(player->current_filename, name, sizeof(player->current_filename) - 1);
    
    Mix_VolumeMusic((int)(player->volume * MIX_MAX_VOLUME));
    
    fprintf(stdout, "Loaded radio stream: %s\n", name);
    
    return 0;
}

void player_play(MusicPlayer *player) {
    if (!player || !player->music) {
        fprintf(stderr, "No music loaded\n");
        return;
    }
    
    int loops = player->loop ? -1 : 0;
    
    if (Mix_PlayMusic(player->music, loops) < 0) {
        fprintf(stderr, "Play failed: %s\n", Mix_GetError());
        return;
    }
    
    Mix_VolumeMusic((int)(player->volume * MIX_MAX_VOLUME));
    
    if (player->saved_time > 0 && player->duration > 0) {
        Mix_SetMusicPosition(player->saved_time);
        player->saved_time = 0;
    }
    
    fprintf(stdout, "Playing (loop=%d, volume=%.0f%%)\n", player->loop, player->volume * 100);
}

void player_pause(MusicPlayer *player) {
    if (!player || !player->music) return;
    Mix_PauseMusic();
}

void player_toggle(MusicPlayer *player) {
    if (!player || !player->music) return;
    
    if (Mix_PlayingMusic() == 0) {
        player_play(player);
    } else if (Mix_PausedMusic()) {
        Mix_ResumeMusic();
    } else {
        Mix_PauseMusic();
    }
}

void player_stop(MusicPlayer *player) {
    if (!player || !player->music) return;
    
    if (Mix_PlayingMusic() || Mix_PausedMusic()) {
        double pos = Mix_GetMusicPosition(player->music);
        if (pos >= 0) {
            player->saved_time = (float)pos;
        }
    }
    
    Mix_HaltMusic();
}

void player_cleanup(MusicPlayer *player) {
    if (!player) return;
    
    if (player->music) {
        Mix_FreeMusic(player->music);
        player->music = NULL;
    }
    
    if (player->lyrics.lines) {
        free(player->lyrics.lines);
        player->lyrics.lines = NULL;
        player->lyrics.count = 0;
        player->lyrics.capacity = 0;
    }
    
    if (player->playlist) {
        for (int i = 0; i < player->playlist_count; i++) {
            free(player->playlist[i]);
        }
        free(player->playlist);
        player->playlist = NULL;
        player->playlist_count = 0;
        player->playlist_capacity = 0;
    }
    
    if (player->radio_stations) {
        free(player->radio_stations);
        player->radio_stations = NULL;
        player->radio_count = 0;
    }
    
    if (g_audio_initialized) {
        Mix_CloseAudio();
        SDL_Quit();
        g_audio_initialized = 0;
    }
    
    fprintf(stdout, "Player cleanup complete\n");
}

float player_get_progress(MusicPlayer *player) {
    if (!player || player->duration <= 0) return 0.0f;
    
    double current = Mix_GetMusicPosition(player->music);
    if (current < 0) return 0.0f;
    
    return (float)(current / player->duration);
}

void player_set_volume(MusicPlayer *player, float volume) {
    if (!player) return;
    
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    
    player->volume = volume;
    Mix_VolumeMusic((int)(volume * MIX_MAX_VOLUME));
}

void player_seek(MusicPlayer *player, float position) {
    if (!player || !player->music || player->duration <= 0) return;
    
    if (position < 0.0f) position = 0.0f;
    if (position > 1.0f) position = 1.0f;
    
    double seek_pos = position * player->duration;
    
    if (Mix_SetMusicPosition(seek_pos) < 0) {
        fprintf(stderr, "Seek not supported for this format\n");
    }
}

float player_get_current_time(MusicPlayer *player) {
    if (!player || !player->music) return 0.0f;
    
    double pos = Mix_GetMusicPosition(player->music);
    return (pos >= 0) ? (float)pos : 0.0f;
}

float player_get_duration(MusicPlayer *player) {
    if (!player) return 0.0f;
    return (float)player->duration;
}

int player_is_playing(MusicPlayer *player) {
    if (!player) return 0;
    return Mix_PlayingMusic() && !Mix_PausedMusic();
}

int player_is_paused(MusicPlayer *player) {
    if (!player) return 0;
    return Mix_PausedMusic();
}

void player_toggle_loop(MusicPlayer *player) {
    if (!player) return;
    player->loop = !player->loop;
    
    if (Mix_PlayingMusic()) {
        player_play(player);
    }
}

void player_extract_metadata(MusicPlayer *player, const char *filename) {
    if (!player || !filename || !*filename) return;
    
    memset(&player->metadata, 0, sizeof(Metadata));
    
    FILE *f = fopen(filename, "rb");
    if (!f) {
        goto fallback_filename;
    }
    
    char tag[4] = {0};
    if (fseek(f, -128, SEEK_END) == 0 && fread(tag, 1, 3, f) == 3 && strcmp(tag, "TAG") == 0) {
        char title[31] = {0}, artist[31] = {0}, album[31] = {0}, year[5] = {0};
        
        fread(title, 1, 30, f);
        fread(artist, 1, 30, f);
        fread(album, 1, 30, f);
        fread(year, 1, 4, f);
        
        char *trim = NULL;
        
        if (title[0]) {
            trim = title + strlen(title) - 1;
            while (trim >= title && *trim == ' ') *trim-- = '\0';
            strncpy(player->metadata.title, title, sizeof(player->metadata.title) - 1);
        }
        
        if (artist[0]) {
            trim = artist + strlen(artist) - 1;
            while (trim >= artist && *trim == ' ') *trim-- = '\0';
            strncpy(player->metadata.artist, artist, sizeof(player->metadata.artist) - 1);
        }
        
        if (album[0]) {
            trim = album + strlen(album) - 1;
            while (trim >= album && *trim == ' ') *trim-- = '\0';
            strncpy(player->metadata.album, album, sizeof(player->metadata.album) - 1);
        }
        
        if (year[0]) {
            trim = year + strlen(year) - 1;
            while (trim >= year && *trim == ' ') *trim-- = '\0';
            strncpy(player->metadata.year, year, sizeof(player->metadata.year) - 1);
        }
    }
    
    fclose(f);
    
    if (player->metadata.title[0] == '\0') {
        fallback_filename:
        char *dup = strdup(filename);
        if (dup) {
            char *base = basename(dup);
            char *dash = strstr(base, " - ");
            
            if (dash) {
                *dash = '\0';
                if (player->metadata.artist[0] == '\0') {
                    strncpy(player->metadata.artist, base, sizeof(player->metadata.artist) - 1);
                }
                
                char *ext = strrchr(dash + 3, '.');
                if (ext) *ext = '\0';
                strncpy(player->metadata.title, dash + 3, sizeof(player->metadata.title) - 1);
            } else {
                char *ext = strrchr(base, '.');
                if (ext) *ext = '\0';
                strncpy(player->metadata.title, base, sizeof(player->metadata.title) - 1);
            }
            
            free(dup);
        }
    }
}

void player_toggle_shuffle(MusicPlayer *player) {
    if (!player) return;
    player->shuffle = !player->shuffle;
    
    if (player->shuffle && player->playlist_count > 1) {
        player_shuffle_playlist(player);
    }
}

void player_toggle_repeat(MusicPlayer *player) {
    if (!player) return;
    player->repeat = !player->repeat;
}

void player_shuffle_playlist(MusicPlayer *player) {
    if (!player || player->playlist_count < 2) return;
    
    for (int i = player->playlist_count - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        char *temp = player->playlist[i];
        player->playlist[i] = player->playlist[j];
        player->playlist[j] = temp;
    }
    
    for (int i = 0; i < player->playlist_count; i++) {
        if (strcmp(player->playlist[i], player->current_filename) == 0) {
            player->current_track = i;
            break;
        }
    }
}

int player_add_to_playlist(MusicPlayer *player, const char *filepath) {
    if (!player || !filepath) return -1;
    
    if (player->playlist_count >= player->playlist_capacity) {
        int new_capacity = player->playlist_capacity == 0 ? 64 : player->playlist_capacity * 2;
        char **new_playlist = realloc(player->playlist, sizeof(char*) * new_capacity);
        
        if (!new_playlist) {
            fprintf(stderr, "Out of memory\n");
            return -1;
        }
        
        player->playlist = new_playlist;
        player->playlist_capacity = new_capacity;
    }
    
    char *dup = strdup(filepath);
    if (!dup) {
        return -1;
    }
    
    player->playlist[player->playlist_count] = dup;
    player->playlist_count++;
    
    return 0;
}

void player_load_playlist(MusicPlayer *player, char **files, int count) {
    if (!player || !files || count <= 0) return;
    
    if (player->playlist) {
        for (int i = 0; i < player->playlist_count; i++) {
            free(player->playlist[i]);
        }
        free(player->playlist);
    }
    
    player->playlist = files;
    player->playlist_count = count;
    player->playlist_capacity = count;
    player->current_track = 0;
}

void player_load_track(MusicPlayer *player, int index) {
    if (!player || index < 0 || index >= player->playlist_count) return;
    
    player->current_track = index;
    
    int was_playing = player_is_playing(player);
    
    if (player_load(player, player->playlist[index]) == 0) {
        if (was_playing) {
            player_play(player);
        }
    }
}

void player_next_track(MusicPlayer *player) {
    if (!player || player->playlist_count == 0) return;
    
    if (player->repeat) {
        player_load_track(player, player->current_track);
        return;
    }
    
    int next = (player->current_track + 1) % player->playlist_count;
    
    if (next == 0 && !player->loop) {
        player_stop(player);
        return;
    }
    
    player_load_track(player, next);
}

void player_prev_track(MusicPlayer *player) {
    if (!player || player->playlist_count == 0) return;
    
    int prev = (player->current_track - 1 + player->playlist_count) % player->playlist_count;
    player_load_track(player, prev);
}

void player_load_lyrics(MusicPlayer *player, const char *filename) {
    if (!player || !filename) return;
    
    if (player->lyrics.lines) {
        free(player->lyrics.lines);
        player->lyrics.lines = NULL;
        player->lyrics.count = 0;
        player->lyrics.capacity = 0;
    }
    
    char lrc_filename[PATH_MAX];
    snprintf(lrc_filename, sizeof(lrc_filename), "%.*s.lrc", 
             (int)(sizeof(lrc_filename) - 5), filename);
    
    FILE *file = fopen(lrc_filename, "r");
    if (!file) return;
    
    int capacity = 64;
    player->lyrics.lines = malloc(sizeof(LyricLine) * capacity);
    if (!player->lyrics.lines) {
        fclose(file);
        return;
    }
    player->lyrics.capacity = capacity;
    player->lyrics.count = 0;
    
    char line[512];
    while (fgets(line, sizeof(line), file)) {
        char *bracket = strchr(line, '[');
        if (!bracket) continue;
        
        char *close = strchr(bracket, ']');
        if (!close) continue;
        
        if (player->lyrics.count >= player->lyrics.capacity) {
            int new_capacity = player->lyrics.capacity * 2;
            LyricLine *new_lines = realloc(player->lyrics.lines, sizeof(LyricLine) * new_capacity);
            if (!new_lines) break;
            player->lyrics.lines = new_lines;
            player->lyrics.capacity = new_capacity;
        }
        
        char time_str[32];
        int time_len = close - bracket - 1;
        if (time_len >= (int)sizeof(time_str)) time_len = sizeof(time_str) - 1;
        strncpy(time_str, bracket + 1, time_len);
        time_str[time_len] = '\0';
        
        int min = 0, sec = 0, ms = 0;
        if (sscanf(time_str, "%d:%d.%d", &min, &sec, &ms) >= 2) {
            float time = min * 60.0f + sec + ms / 100.0f;
            
            char *text = close + 1;
            size_t len = strlen(text);
            while (len > 0 && (text[len-1] == '\n' || text[len-1] == '\r')) {
                text[--len] = '\0';
            }
            
            player->lyrics.lines[player->lyrics.count].time = time;
            strncpy(player->lyrics.lines[player->lyrics.count].text, text, 
                    sizeof(player->lyrics.lines[0].text) - 1);
            player->lyrics.count++;
        }
    }
    
    fclose(file);
}

void player_save_playlist(MusicPlayer *player, const char *filename) {
    if (!player || !filename || player->playlist_count == 0) return;
    
    FILE *file = fopen(filename, "w");
    if (!file) return;
    
    for (int i = 0; i < player->playlist_count; i++) {
        fprintf(file, "%s\n", player->playlist[i]);
    }
    
    fclose(file);
}

void player_load_playlist_from_file(MusicPlayer *player, const char *filename) {
    if (!player || !filename) return;
    
    FILE *file = fopen(filename, "r");
    if (!file) return;
    
    if (player->playlist) {
        for (int i = 0; i < player->playlist_count; i++) {
            free(player->playlist[i]);
        }
        free(player->playlist);
        player->playlist = NULL;
        player->playlist_count = 0;
        player->playlist_capacity = 0;
    }
    
    int capacity = 64;
    player->playlist = malloc(sizeof(char*) * capacity);
    if (!player->playlist) {
        fclose(file);
        return;
    }
    player->playlist_capacity = capacity;
    player->playlist_count = 0;
    
    char line[PATH_MAX];
    while (fgets(line, sizeof(line), file)) {
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) {
            line[--len] = '\0';
        }
        
        if (len == 0) continue;
        
        if (player->playlist_count >= player->playlist_capacity) {
            int new_capacity = player->playlist_capacity * 2;
            char **new_playlist = realloc(player->playlist, sizeof(char*) * new_capacity);
            if (!new_playlist) break;
            player->playlist = new_playlist;
            player->playlist_capacity = new_capacity;
        }
        
        char *dup = strdup(line);
        if (!dup) break;
        
        player->playlist[player->playlist_count] = dup;
        player->playlist_count++;
    }
    
    fclose(file);
    
    if (player->playlist_count > 0) {
        player->current_track = 0;
        player_load(player, player->playlist[0]);
    }
}

void player_init_radio_stations(MusicPlayer *player) {
    if (!player) return;
    
    static const struct {
        const char *name;
        const char *url;
        const char *genre;
    } stations[] = {
        {"BBC Radio 1", "http://stream.live.vc.bbcmedia.co.uk/bbc_radio_one", "Pop"},
        {"Smooth Jazz", "http://smoothjazz.com/stream", "Jazz"},
        {"Classic FM", "http://media-ice.musicradio.com/ClassicFMMP3", "Classical"},
        {"Ibiza Global Radio", "http://ibizaglobalradio.streaming-pro.com:8024/ibizaglobalradio.mp3", "Electronic"},
        {"SomaFM Groove Salad", "https://ice1.somafm.com/groovesalad-128-mp3", "Ambient"},
        {"SomaFM Drone Zone", "https://ice1.somafm.com/dronezone-128-mp3", "Ambient"},
        {"SomaFM Secret Agent", "https://ice1.somafm.com/secretagent-128-mp3", "Lounge"},
        {"SomaFM Indie Pop Rocks", "https://ice1.somafm.com/indiepop-128-mp3", "Indie"},
        {"SomaFM Beat Blender", "https://ice1.somafm.com/beatblender-128-mp3", "Electronic"},
        {"SomaFM Boot Liquor", "https://ice1.somafm.com/bootliquor-128-mp3", "Americana"},
        {"SomaFM Cliqhop", "https://ice1.somafm.com/cliqhop-128-mp3", "Electronic"},
        {"SomaFM Deep Space One", "https://ice1.somafm.com/deepspaceone-128-mp3", "Ambient"},
        {"SomaFM Doomed", "https://ice1.somafm.com/doomed-128-mp3", "Alternative"},
        {"SomaFM Dub Beyond", "https://ice1.somafm.com/dub-128-mp3", "Reggae"},
        {"SomaFM Ear Candy", "https://ice1.somafm.com/earcandy-128-mp3", "Pop"},
    };
    
    int count = sizeof(stations) / sizeof(stations[0]);
    player->radio_stations = malloc(sizeof(RadioStation) * count);
    
    if (!player->radio_stations) {
        player->radio_count = 0;
        return;
    }
    
    for (int i = 0; i < count; i++) {
        strncpy(player->radio_stations[i].name, stations[i].name, 
                sizeof(player->radio_stations[i].name) - 1);
        strncpy(player->radio_stations[i].url, stations[i].url, 
                sizeof(player->radio_stations[i].url) - 1);
        strncpy(player->radio_stations[i].genre, stations[i].genre, 
                sizeof(player->radio_stations[i].genre) - 1);
        player->radio_stations[i].bitrate = 128;
    }
    
    player->radio_count = count;
    player->current_radio = -1;
}

void player_play_radio(MusicPlayer *player, int index) {
    if (!player || index < 0 || index >= player->radio_count) return;
    
    if (player_load_stream(player, player->radio_stations[index].url, 
                           player->radio_stations[index].name) == 0) {
        player->current_radio = index;
        player_play(player);
    }
}

void player_save_state(MusicPlayer *player) {
    if (!player) return;
    
    FILE *f = fopen(".dez_state", "w");
    if (!f) return;
    
    fprintf(f, "volume %.2f\n", player->volume);
    fprintf(f, "loop %d\n", player->loop);
    fprintf(f, "shuffle %d\n", player->shuffle);
    fprintf(f, "repeat %d\n", player->repeat);
    fprintf(f, "current_track %d\n", player->current_track);
    fprintf(f, "current_time %.2f\n", player_get_current_time(player));
    
    fclose(f);
}

void player_load_state(MusicPlayer *player) {
    if (!player) return;
    
    FILE *f = fopen(".dez_state", "r");
    if (!f) return;
    
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        float vol;
        int val;
        float time;
        
        if (sscanf(line, "volume %f", &vol) == 1) {
            player->volume = vol;
            continue;
        }
        if (sscanf(line, "loop %d", &val) == 1) {
            player->loop = val;
            continue;
        }
        if (sscanf(line, "shuffle %d", &val) == 1) {
            player->shuffle = val;
            continue;
        }
        if (sscanf(line, "repeat %d", &val) == 1) {
            player->repeat = val;
            continue;
        }
        if (sscanf(line, "current_track %d", &val) == 1) {
            player->current_track = val;
            continue;
        }
        if (sscanf(line, "current_time %f", &time) == 1) {
            player->saved_time = time;
        }
    }
    
    fclose(f);
}

void player_update_spectrum(MusicPlayer *player) {
    if (!player) return;
    
    static int last_update = 0;
    static float target_spectrum[SPECTRUM_SIZE] = {0};
    static float current_spectrum[SPECTRUM_SIZE] = {0};
    
    int now = SDL_GetTicks();
    if (now - last_update < 50) {
        for (int i = 0; i < SPECTRUM_SIZE; i++) {
            current_spectrum[i] *= 0.92f;
            if (current_spectrum[i] < 0.01f) current_spectrum[i] = 0;
            player->spectrum[i] = current_spectrum[i];
        }
        return;
    }
    last_update = now;
    
    int is_playing = player_is_playing(player);
    
    for (int i = 0; i < SPECTRUM_SIZE; i++) {
        if (is_playing) {
            float base = (i % 8) * 0.1f + 0.2f;
            float wave = sinf(now * 0.005f + i * 0.4f) * 0.3f + 0.5f;
            float noise = (rand() % 100) / 100.0f * 0.4f;
            target_spectrum[i] = base * wave + noise;
            
            if (i > 0 && i < SPECTRUM_SIZE - 1) {
                float neighbor_avg = (target_spectrum[i-1] + target_spectrum[i+1]) / 2;
                target_spectrum[i] = target_spectrum[i] * 0.7f + neighbor_avg * 0.3f;
            }
        } else {
            target_spectrum[i] = 0;
        }
        
        float smoothing = 0.3f;
        current_spectrum[i] = current_spectrum[i] * (1 - smoothing) + target_spectrum[i] * smoothing;
        
        if (current_spectrum[i] < 0.01f) current_spectrum[i] = 0;
        
        player->spectrum[i] = current_spectrum[i];
    }
}
