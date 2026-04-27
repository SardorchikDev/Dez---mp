# DEZ Music Player - CAVA-Style Visualizer

## New Visualizer Design (CAVA-inspired)

Instead of the old "Spectrum" display, you now get vertical bars that animate like CAVA!

```
=== DEZ Music Player ===
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
|                            CAVA                                            |
|                                                                            |
|        |     |  |        |     |                                          |
|        |  |  |  |  |     |  |  |  |     |                                 |
|     |  |  |  |  |  |  |  |  |  |  |  |  |  |                              |
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |                       |
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |                 |
|  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |              |
|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|__|
+----------------------------------------------------------------------------+
+----------------------------------------------------------------------------+
|                         Playlist                                           |
|   > song1.mp3                                                              |
|     song2.mp3                                                              |
+----------------------------------------------------------------------------+
Space:Play/Pause  X:Stop  Q:Quit  +/-:Vol  </>:Seek10s  Left/Right:Seek1s
```

## Visualizer Features

### 🎨 Color Gradient
- **Red** (bottom) - Low frequencies
- **Yellow/Green** (middle) - Mid frequencies  
- **Cyan/Blue** (top) - High frequencies

### 📊 Bar Design
- **20-30 vertical bars** across the screen
- **2 characters wide** per bar
- **1 character spacing** between bars
- **8 rows tall** for good visibility

### 🌊 Animation
- **Smooth transitions** between values
- **Natural decay** when music stops
- **Wave-like patterns** that flow across bars
- **Random variation** for organic feel
- **Neighbor smoothing** for cohesive look

### 🎵 Behavior
- **Bars jump up** with the beat
- **Gradual fall** like real audio visualizers
- **Stops when paused** (bars fade to zero)
- **Restarts smoothly** when playing resumes

## CAVA Comparison

| Feature | CAVA | DEZ Visualizer |
|---------|------|----------------|
| Vertical bars | ✅ | ✅ |
| Color gradient | ✅ | ✅ |
| Smooth animation | ✅ | ✅ |
| Frequency bands | ✅ | ✅ (simulated) |
| Terminal based | ✅ | ✅ |
| Real audio input | ✅ | ⚠️ (simulated)* |

*Note: Currently uses simulated audio data. Real FFT analysis can be added in future versions.

## Technical Details

### Bar Calculation
```c
bar_count = (width - 2) / 3;  // Bars based on terminal width
bar_width = 2;                 // 2 chars wide
spacing = 1;                   // 1 char gap
height = 8;                    // 8 rows tall
```

### Smoothing Algorithm
```c
// Exponential smoothing for natural movement
current = current * 0.7 + target * 0.3;

// Neighbor averaging for cohesive look
target[i] = target[i] * 0.7 + (target[i-1] + target[i+1]) / 2 * 0.3;
```

### Color Levels
```
Level 0-1: Red (COLOR_RED)
Level 2-3: Yellow/Green (COLOR_YELLOW)
Level 4-5: Cyan (COLOR_CYAN)
Level 6:   Blue (COLOR_BLUE)
```

## Update Rate

- **20 FPS** (50ms update interval)
- **Smooth 60 FPS feel** due to interpolation
- **Low CPU usage** - efficient calculations

## Terminal Compatibility

✅ Works in all terminals that support:
- ncurses colors
- Minimum 80x24 size
- Standard ASCII characters

## Future Enhancements (Optional)

1. **Real FFT Analysis** - Use actual audio data
2. **More bar modes** - Waves, lines, dots
3. **Customizable colors** - Theme support
4. **Sensitivity control** - Adjust bar height
5. **Mono/Stereo mode** - Split channels

---

**Enjoy the CAVA-style visualizer in DEZ! 🎵**
