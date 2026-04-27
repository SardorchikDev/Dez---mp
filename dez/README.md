# DEZ Music Player v2.0

A beautiful terminal-based music player with SDL2 audio, ncurses UI, spectrum visualizer, and internet radio support.

![Version](https://img.shields.io/badge/version-2.0-blue)
![License](https://img.shields.io/badge/license-MIT-green)

## ✨ Features

### Audio Playback
- **Multi-Format Support**: WAV, MP3, OGG, FLAC, AAC, WMA, M4A, Opus, WebM, MIDI
- **Gapless Playback**: Smooth transitions between tracks
- **Resume Playback**: Automatically saves and restores position
- **Playlist Support**: Load, save, and manage playlists
- **Shuffle & Repeat**: Advanced playback modes

### User Interface
- **Beautiful TUI**: Modern ncurses interface with colors and boxes
- **Real-time Spectrum Visualizer**: Audio visualization with color gradients
- **Progress Bar**: Interactive seek bar with mouse support
- **File Browser**: Navigate directories to find music
- **Radio Browser**: 15+ pre-configured internet radio stations

### Controls
- **Space**: Play/Pause
- **X**: Stop
- **Q**: Quit
- **+/-**: Volume Up/Down
- **</>**: Seek ±10 seconds
- **←/→**: Seek ±1 second
- **L**: Toggle Loop
- **H**: Toggle Shuffle
- **R**: Toggle Repeat
- **↑/↓**: Previous/Next Track
- **B**: File Browser
- **I**: Radio Browser
- **S**: Save Playlist
- **D**: Load Playlist

## 📦 Dependencies

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y libsdl2-dev libsdl2-mixer-dev libncursesw5-dev pkg-config
```

### Fedora/RHEL
```bash
sudo dnf install SDL2-devel SDL2_mixer-devel ncurses-devel
```

### Arch Linux
```bash
sudo pacman -S sdl2 sdl2_mixer ncurses
```

### macOS
```bash
brew install sdl2 sdl2_mixer ncurses pkg-config
```

## 🚀 Installation

### Quick Build
```bash
cd dez/
make
```

### Install System-wide
```bash
sudo make install
```

### Run
```bash
./build/dez                    # Start with file browser
./build/dez ~/Music            # Scan directory
./build/dez song.mp3           # Play specific file
```

## 🎨 Screenshots

```
╔═══╗╔═╗ ╔═══╗     DEZ Music Player
║ ╔╗║║║╚╗║ ╔╗║
╚═╝╚╝╚══╝ ╚══╝
────────────────────────────────────────────────
┌──────────────────────────────────────────────┐
│                 Now Playing                  │
│                                              │
│           Song Title - Artist Name           │
│           ▶  PLAYING                         │
│                          02:15 / 04:30       │
│           [████████████▒▒▒▒▒▒▒▒▒▒] 50%      │
│           Volume: ████████░░░░░  53%         │
│           [LOOP] [SHUF] [RPT ]               │
└──────────────────────────────────────────────┘
┌──────────────────────────────────────────────┐
│                 Spectrum                     │
│           ▂▃▅▆▇▆▅▃▂▃▅▆▇▆▅▃▂                 │
└──────────────────────────────────────────────┘
┌──────────────────────────────────────────────┐
│                 Playlist                     │
│   ▶ song1.mp3                                │
│     song2.mp3                                │
│     song3.mp3                                │
└──────────────────────────────────────────────┘
Space:Play/Pause  X:Stop  Q:Quit  +/-:Vol
L:Loop  H:Shuffle  R:Repeat  B:Browse  I:Radio
```

## 📁 Project Structure

```
dez/
├── Makefile              # Build configuration
├── README.md             # This file
├── src/
│   ├── main.c            # Application entry point
│   ├── player.c          # Audio playback engine
│   ├── player.h          # Player API
│   ├── ui.c              # Terminal UI rendering
│   └── ui.h              # UI API
└── build/
    └── dez               # Compiled binary
```

## 🔧 Configuration

### State File
DEZ saves your preferences in `.dez_state`:
- Volume level
- Loop/Shuffle/Repeat settings
- Last played track
- Playback position

### Playlist Files
Save/load playlists with `.txt` format:
```
/path/to/song1.mp3
/path/to/song2.mp3
/path/to/song3.mp3
```

## 🎵 Radio Stations

Pre-configured stations include:
- BBC Radio 1 (Pop)
- Smooth Jazz
- Classic FM (Classical)
- Ibiza Global Radio (Electronic)
- SomaFM (Multiple channels)
  - Groove Salad (Ambient)
  - Drone Zone (Ambient)
  - Secret Agent (Lounge)
  - And more...

## 🐛 Troubleshooting

### No Sound
1. Check audio output device: `pavucontrol` (Linux)
2. Verify SDL2 mixer initialization in terminal output
3. Try increasing volume with `+` key

### Compilation Errors
```bash
# Missing pkg-config?
sudo apt-get install pkg-config

# Missing SDL2?
sudo apt-get install libsdl2-dev libsdl2-mixer-dev

# Missing ncurses?
sudo apt-get install libncursesw5-dev
```

### Terminal Display Issues
- Ensure terminal supports colors
- Minimum size: 80x24 characters
- Try different terminal emulator (alacritty, kitty, foot)

## 🤝 Contributing

Contributions welcome! Areas for improvement:
- ID3v2 tag support for better metadata
- Lyrics synchronization
- Equalizer
- Theme customization
- Additional radio stations

## 📝 License

MIT License - See source files for details

## 🙏 Acknowledgments

- SDL2 Team for audio library
- SDL_mixer contributors
- ncurses developers
- SomaFM for radio streams

---

**DEZ** - Terminal Music Player ❤️
