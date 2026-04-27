#!/bin/sh
set -e

INSTALL_DIR="${INSTALL_DIR:-/usr/local/bin}"
BINARY_NAME="dez"
PROJECT_NAME="dez-mp"
SOURCE_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$SOURCE_DIR/dez/build"

detect_os() {
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        case "$ID" in
            ubuntu|debian|linuxmint)
                echo "debian"
                ;;
            fedora|rhel|centos)
                echo "fedora"
                ;;
            arch|manjaro|endeavouros)
                echo "arch"
                ;;
            darwin)
                echo "macos"
                ;;
            *)
                echo "unknown"
                ;;
        esac
    elif [ "$(uname -s)" = "Darwin" ]; then
        echo "macos"
    else
        echo "unknown"
    fi
}

install_deps_debian() {
    printf "Installing dependencies for Debian/Ubuntu... "
    if command -v apt-get >/dev/null 2>&1; then
        apt-get update -qq && apt-get install -y -qq libsdl2-dev libsdl2-mixer-dev libncursesw5-dev pkg-config gcc make >/dev/null 2>&1 || {
            echo "Failed to install dependencies. Please run as root or with sudo."
            exit 1
        }
        echo "OK"
    else
        echo "apt-get not found"
        exit 1
    fi
}

install_deps_fedora() {
    printf "Installing dependencies for Fedora/RHEL... "
    if command -v dnf >/dev/null 2>&1; then
        dnf install -y -q SDL2-devel SDL2_mixer-devel ncurses-devel pkg-config gcc make >/dev/null 2>&1 || {
            echo "Failed to install dependencies. Please run as root or with sudo."
            exit 1
        }
        echo "OK"
    else
        echo "dnf not found"
        exit 1
    fi
}

install_deps_arch() {
    printf "Installing dependencies for Arch Linux... "
    if command -v pacman >/dev/null 2>&1; then
        pacman -Sy --noconfirm sdl2 sdl2_mixer ncurses pkg-config gcc make >/dev/null 2>&1 || {
            echo "Failed to install dependencies. Please run as root or with sudo."
            exit 1
        }
        echo "OK"
    else
        echo "pacman not found"
        exit 1
    fi
}

install_deps_macos() {
    printf "Installing dependencies for macOS... "
    if command -v brew >/dev/null 2>&1; then
        brew install sdl2 sdl2_mixer ncurses pkg-config gcc make 2>/dev/null || {
            echo "Failed to install dependencies."
            exit 1
        }
        echo "OK"
    else
        echo "brew not found"
        exit 1
    fi
}

install_deps() {
    OS=$(detect_os)
    case "$OS" in
        debian)
            install_deps_debian
            ;;
        fedora)
            install_deps_fedora
            ;;
        arch)
            install_deps_arch
            ;;
        macos)
            install_deps_macos
            ;;
        *)
            echo "Unsupported OS. Please install manually: libsdl2-dev, libsdl2-mixer-dev, libncursesw5-dev, pkg-config, gcc, make"
            exit 1
            ;;
    esac
}

compile() {
    printf "Compiling %s... " "$PROJECT_NAME"
    cd "$SOURCE_DIR/dez"
    make clean >/dev/null 2>&1 || true
    make >/dev/null 2>&1 || {
        echo "FAILED"
        echo "Compilation failed. Check errors above."
        exit 1
    }
    echo "OK"
}

install_binary() {
    printf "Installing %s to %s... " "$BINARY_NAME" "$INSTALL_DIR"
    mkdir -p "$INSTALL_DIR"
    if [ -w "$INSTALL_DIR" ] || [ -w "$(dirname "$INSTALL_DIR")" ]; then
        cp "$BUILD_DIR/$BINARY_NAME" "$INSTALL_DIR/$BINARY_NAME"
        chmod 755 "$INSTALL_DIR/$BINARY_NAME"
        echo "OK"
    else
        echo "Need sudo"
        sudo cp "$BUILD_DIR/$BINARY_NAME" "$INSTALL_DIR/$BINARY_NAME"
        sudo chmod 755 "$INSTALL_DIR/$BINARY_NAME"
        echo "OK (with sudo)"
    fi
}

main() {
    echo "=========================================="
    echo "  $PROJECT_NAME Music Player Installer"
    echo "=========================================="
    echo ""

    if [ -f "$BUILD_DIR/$BINARY_NAME" ] && [ "$1" != "--force" ]; then
        echo "Binary already exists. Use --force to reinstall."
    else
        install_deps
        compile
    fi

    install_binary

    echo ""
    echo "=========================================="
    echo "  Installation Complete!"
    echo "=========================================="
    echo ""
    echo "Usage:"
    echo "  dez                      # Start with file browser"
    echo "  dez ~/Music              # Play directory"
    echo "  dez song.mp3             # Play file"
    echo "  dez http://stream.url    # Play internet radio"
    echo ""
    echo "Controls:"
    echo "  Space  Play/Pause"
    echo "  X      Stop"
    echo "  Q      Quit"
    echo "  +/-    Volume"
    echo "  ←/→    Seek"
    echo "  ↑/↓    Prev/Next track"
    echo ""
    echo "=========================================="
}

main "$@"