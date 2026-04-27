CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 `pkg-config --cflags sdl2 SDL2_mixer`
LDFLAGS = `pkg-config --libs sdl2 SDL2_mixer` -lncursesw -lm

SRC_DIR = src
BUILD_DIR = build

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
TARGET = $(BUILD_DIR)/dez

.PHONY: all clean install uninstall run

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)
	@echo "Build complete: $(TARGET)"
	@echo "Run with: ./$(TARGET) [audio_file_or_directory]"

clean:
	rm -rf $(BUILD_DIR)
	rm -f .dez_state
	@echo "Clean complete"

install: $(TARGET)
	install -d $(DESTDIR)/usr/local/bin
	install -m 755 $(TARGET) $(DESTDIR)/usr/local/bin/dez
	@echo "Installed to $(DESTDIR)/usr/local/bin/dez"

uninstall:
	rm -f $(DESTDIR)/usr/local/bin/dez
	@echo "Uninstalled"

run: $(TARGET)
	./$(TARGET)

debug: CFLAGS += -g -DDEBUG
debug: clean $(TARGET)
