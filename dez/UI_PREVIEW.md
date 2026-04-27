# DEZ Music Player - UI Preview

## What You'll See (All ASCII - No Broken Characters!)

```
  ______   ________  ________  
 |_   _\.|_   __  ||  __   _| 
   | | `. \ | |_ \_||_/  / /        DEZ Music Player
==============================================================================
+----------------------------------------------------------------------------+
|                         Now Playing                                        |
|                                                                            |
|                    Song Title - Artist Name                                |
|                    >> PLAYING                                   02:15 / 04:30 |
|                    [################--------------------------------] 50%  |
|                    Volume: ########-------  53%                          |
|                    [*LOOP] [ SHUF] [ RPT ]          [RADIO]              |
+----------------------------------------------------------------------------+
+----------------------------------------------------------------------------+
|                         Spectrum                                           |
|                    ||| ||| ||||| |||| |||                                |
|                    |||| |||| |||||||||| ||||                             |
+----------------------------------------------------------------------------+
+----------------------------------------------------------------------------+
|                         Playlist                                           |
|   > song1.mp3                                                              |
|     song2.mp3                                                              |
|     song3.mp3                                                              |
|     song4.mp3                                                              |
+----------------------------------------------------------------------------+
Space:Play/Pause  X:Stop  Q:Quit  +/-:Vol  </>:Seek10s  Left/Right:Seek1s
L:Loop  H:Shuffle  R:Repeat  B:Browse  Up/Down:Tracks  I:Radio
```

## File Browser
```
==============================================================================
 [DIR] /home/user/Music                                        
==============================================================================
D  ..
D  Albums
D  Singles
D  Podcasts
   artist - song1.mp3
   artist - song2.mp3
   favorite.mp3
   track.wav

Up/Down:Navigate  Enter:Select  Backspace:Up  Q:Quit
Files: 8  Selected: 5/8
```

## Radio Browser
```
Radio Stations
============================================================
BBC Radio 1                      [Pop] 128kbps
Smooth Jazz                      [Jazz] 128kbps
Classic FM                       [Classical] 128kbps
Ibiza Global Radio               [Electronic] 128kbps
SomaFM Groove Salad              [Ambient] 128kbps
SomaFM Drone Zone                [Ambient] 128kbps
SomaFM Secret Agent              [Lounge] 128kbps

Use Up/Down to navigate, Enter to play, Q to go back
Stations: 15  Selected: 1/15
```

## Status Indicators

- `>> PLAYING` - Currently playing (green)
- `|| PAUSED` - Playback paused (yellow)
- `[] STOPPED` - Playback stopped (red)
- `[*LOOP]` - Loop mode enabled (highlighted)
- `[RADIO]` - Currently playing radio stream

## Progress Bar Symbols

- `[` `]` - Bar boundaries
- `#` - Filled portion
- `-` - Empty portion

## Volume Bar

- `#` - Volume level
- `-` - Empty portion

## All Characters Used (100% Terminal Compatible!)

- Standard ASCII letters and numbers
- Basic punctuation: `+ - = [ ] < > / \`
- ncurses line drawing: `ACS_HLINE`, `ACS_VLINE`, `ACS_ULCORNER`, etc.
- No Unicode, no emoji, no special characters!

## Terminal Requirements

- Any terminal that supports ncurses
- Minimum size: 80x24 characters
- Color support (optional, but recommended)
- Works in: xterm, gnome-terminal, konsole, alacritty, kitty, foot, tmux, screen

## No More Broken Display!

✅ All Unicode characters removed
✅ Using only standard ASCII
✅ ncurses box drawing for borders
✅ Works in ALL terminals
✅ No more `~U~t` or garbage characters!
