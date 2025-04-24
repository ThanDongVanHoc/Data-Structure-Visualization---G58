#ifndef UI_H
#define UI_H

#include "raylib.h"
#include <string>

// Constants for UI styling
extern const Color UI_PRIMARY;
extern const Color UI_PRIMARY_DARK;
extern const Color UI_PRIMARY_LIGHT;
extern const Color UI_SECONDARY;
extern const Color UI_SUCCESS;
extern const Color UI_DANGER;
extern const Color UI_WARNING;
extern const Color UI_INFO;
extern const Color UI_LIGHT;
extern const Color UI_DARK;
extern const Color UI_TEXT;
extern const Color UI_TEXT_LIGHT;

// Helper function to draw a shadow
void DrawShadow(Rectangle bounds, float rounding, float blur, Color shadowColor);

// Draw a nice neumorphic button with various states
void DrawNeumorphicButton(Rectangle bounds, const char* text, Color baseColor, bool isHovered, bool isPressed);

// Draw an enhanced back button with better styling
void DrawEnhancedBackButton(Rectangle bounds, bool isHovered, bool isPressed);

// Draw a forward navigation button with arrow
void DrawForwardButton(Rectangle bounds, bool isActive, bool isHovered, bool isPressed);

// Draw a backward navigation button with arrow
void DrawBackwardButton(Rectangle bounds, bool isActive, bool isHovered, bool isPressed);

// Draw play/pause button
void DrawPlayPauseButton(Rectangle bounds, bool isPlaying, bool isHovered, bool isPressed);

// Draw a modern progress bar with scrubber
void DrawModernProgressBar(Rectangle bounds, float progress, bool isDragging);

// Draw text with optional shadow
void DrawTextWithShadow(const char* text, int posX, int posY, int fontSize, Color textColor, Color shadowColor = BLACK);

// Draw a modern input box with animation
void DrawInputBox(Rectangle bounds, const char* text, bool isActive, int cursorPos = 0, bool showCursor = true);

// Draw title text with a subtle underline
void DrawTitleText(const char* text, int posX, int posY, int fontSize, Color textColor);

// Draw a card container with shadow and optional title
void DrawCard(Rectangle bounds, const char* title, Color bgColor);

#endif // UI_H
