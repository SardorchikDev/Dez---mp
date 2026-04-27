#!/bin/bash
# DEZ Music Player - Quick Start Script

echo "╔════════════════════════════════════════╗"
echo "║     DEZ Music Player - Quick Start     ║"
echo "╚════════════════════════════════════════╝"
echo ""

# Check if build exists
if [ ! -f "build/dez" ]; then
    echo "Building DEZ Music Player..."
    make
    if [ $? -ne 0 ]; then
        echo "Build failed! Install dependencies:"
        echo "  Ubuntu/Debian: sudo apt-get install libsdl2-dev libsdl2-mixer-dev libncursesw5-dev"
        echo "  Fedora: sudo dnf install SDL2-devel SDL2_mixer-devel ncurses-devel"
        echo "  Arch: sudo pacman -S sdl2 sdl2_mixer ncurses"
        exit 1
    fi
fi

echo "✓ Build complete"
echo ""

# Check for music directory
MUSIC_DIR="${1:-$HOME/Music}"

if [ -d "$MUSIC_DIR" ]; then
    AUDIO_FILES=$(find "$MUSIC_DIR" -maxdepth 1 -type f \( -iname "*.mp3" -o -iname "*.wav" -o -iname "*.ogg" -o -iname "*.flac" \) 2>/dev/null | wc -l)
    
    if [ "$AUDIO_FILES" -gt 0 ]; then
        echo "✓ Found $AUDIO_FILES audio files in $MUSIC_DIR"
        echo ""
        echo "Starting DEZ Music Player..."
        echo ""
        ./build/dez "$MUSIC_DIR"
    else
        echo "⚠ No audio files found in $MUSIC_DIR"
        echo ""
        echo "Starting DEZ in file browser mode..."
        echo "Use 'B' key to browse for music files"
        echo ""
        ./build/dez
    fi
else
    echo "⚠ Music directory not found: $MUSIC_DIR"
    echo ""
    echo "Starting DEZ in file browser mode..."
    echo "Use 'B' key to browse for music files"
    echo ""
    ./build/dez
fi
