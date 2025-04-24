#include "raylib.h"
#include "UI.h"
#include <cmath>

// Constants for UI styling
const Color UI_PRIMARY = {67, 134, 245, 255};   // Modern blue
const Color UI_PRIMARY_DARK = {45, 112, 223, 255}; // Darker blue
const Color UI_PRIMARY_LIGHT = {89, 156, 255, 255}; // Lighter blue
const Color UI_SECONDARY = {142, 68, 173, 255}; // Purple
const Color UI_SUCCESS = {46, 204, 113, 255};   // Green
const Color UI_DANGER = {231, 76, 60, 255};     // Red
const Color UI_WARNING = {241, 196, 15, 255};   // Yellow
const Color UI_INFO = {52, 152, 219, 255};      // Light blue
const Color UI_LIGHT = {248, 249, 250, 255};    // Light gray
const Color UI_DARK = {40, 44, 52, 255};        // Dark gray
const Color UI_TEXT = {33, 37, 41, 255};        // Text color
const Color UI_TEXT_LIGHT = {255, 255, 255, 255}; // Light text

// Neumorphic styling constants
const float SHADOW_OFFSET = 4.0f;
const float SHADOW_BLUR = 8.0f;
const float BUTTON_ROUNDING = 0.3f;

// Helper function to draw a shadow
void DrawShadow(Rectangle bounds, float rounding, float blur, Color shadowColor) {
    DrawRectangleRounded(
        (Rectangle){
            bounds.x + SHADOW_OFFSET,
            bounds.y + SHADOW_OFFSET,
            bounds.width,
            bounds.height
        },
        rounding, blur, shadowColor
    );
}

// Draw a nice neumorphic button with various states
void DrawNeumorphicButton(Rectangle bounds, const char* text, Color baseColor, bool isHovered, bool isPressed) {
    // Base colors
    Color lightShadow = ColorAlpha(WHITE, 0.7f);
    Color darkShadow = ColorAlpha(BLACK, 0.15f);
    Color textColor = WHITE;
    
    // Shadow offset
    float shadowOffset = 6.0f;
    
    if (!isPressed && !isHovered) {
        // Normal state - outer shadow effect (raised)
        DrawRectangleRounded(
            (Rectangle){
                bounds.x + shadowOffset, 
                bounds.y + shadowOffset, 
                bounds.width, 
                bounds.height
            },
            BUTTON_ROUNDING, 8, darkShadow
        );
        
        DrawRectangleRounded(
            (Rectangle){
                bounds.x - 2, 
                bounds.y - 2, 
                bounds.width, 
                bounds.height
            },
            BUTTON_ROUNDING, 8, lightShadow
        );
        
        // Main button
        DrawRectangleRounded(bounds, BUTTON_ROUNDING, 8, baseColor);
    } else {
        // Hover/pressed state - inset shadow effect (depressed)
        Color pressedColor = ColorBrightness(baseColor, -0.1f);
        
        // Inner shadow
        DrawRectangleRounded(bounds, BUTTON_ROUNDING, 8, pressedColor);
        
        // Inset shadow at top-left (darker)
        Vector2 innerShadowTL[3] = {
            (Vector2){ bounds.x + 4, bounds.y + 4 },
            (Vector2){ bounds.x + 12, bounds.y + 4 },
            (Vector2){ bounds.x + 4, bounds.y + 12 }
        };
        DrawTriangle(innerShadowTL[0], innerShadowTL[1], innerShadowTL[2], ColorAlpha(BLACK, 0.1f));
        
        // Light edge at bottom-right
        Vector2 innerShadowBR[3] = {
            (Vector2){ bounds.x + bounds.width - 4, bounds.y + bounds.height - 4 },
            (Vector2){ bounds.x + bounds.width - 12, bounds.y + bounds.height - 4 },
            (Vector2){ bounds.x + bounds.width - 4, bounds.y + bounds.height - 12 }
        };
        DrawTriangle(innerShadowBR[0], innerShadowBR[1], innerShadowBR[2], ColorAlpha(WHITE, 0.05f));
        
        // Add subtle border
        DrawRectangleRoundedLines(bounds, BUTTON_ROUNDING, 8, ColorAlpha(BLACK, 0.1f));
        
        // Translate bounds slightly to create pressed effect
        bounds.x += 1;
        bounds.y += 1;
    }
    
    // Draw text centered
    int fontSize = 20;
    int textWidth = MeasureText(text, fontSize);
    int textX = bounds.x + (bounds.width - textWidth) / 2;
    int textY = bounds.y + (bounds.height - fontSize) / 2;
    
    // Draw text with subtle shadow
    DrawText(text, textX + 1, textY + 1, fontSize, ColorAlpha(BLACK, 0.2f));
    DrawText(text, textX, textY, fontSize, textColor);
}

// Draw an enhanced back button with better styling
void DrawEnhancedBackButton(Rectangle bounds, bool isHovered, bool isPressed) {
    Color btnColor = isPressed ? UI_PRIMARY_DARK : 
                    (isHovered ? ColorBrightness(UI_PRIMARY, 0.1f) : UI_PRIMARY);
    
    // Shadow for elevation effect
    DrawShadow(bounds, BUTTON_ROUNDING, 8, ColorAlpha(BLACK, 0.2f));
    
    // Main button
    DrawRectangleRounded(bounds, BUTTON_ROUNDING, 8, btnColor);
    
    // Draw arrow icon
    float arrowSize = bounds.height * 0.5f;
    float centerY = bounds.y + bounds.height/2;
    
    // Arrow stem
    DrawLineEx(
        (Vector2){bounds.x + bounds.width * 0.3f + arrowSize * 0.8f, centerY},
        (Vector2){bounds.x + bounds.width * 0.3f, centerY},
        3.0f, WHITE
    );
    
    // Arrow head
    Vector2 arrowHead[3] = {
        (Vector2){bounds.x + bounds.width * 0.3f, centerY},
        (Vector2){bounds.x + bounds.width * 0.3f + arrowSize * 0.4f, centerY - arrowSize * 0.4f},
        (Vector2){bounds.x + bounds.width * 0.3f + arrowSize * 0.4f, centerY + arrowSize * 0.4f}
    };
    DrawTriangle(arrowHead[0], arrowHead[1], arrowHead[2], WHITE);
    
    // Draw text
    const char* text = "Back";
    int fontSize = 20;
    DrawText(text, bounds.x + bounds.width * 0.3f + arrowSize, centerY - fontSize/2, fontSize, WHITE);
    
    // Add subtle highlight on top edge
    DrawLineEx(
        (Vector2){bounds.x + BUTTON_ROUNDING * bounds.height, bounds.y + 1},
        (Vector2){bounds.x + bounds.width - BUTTON_ROUNDING * bounds.height, bounds.y + 1},
        2.0f, ColorAlpha(WHITE, 0.3f)
    );
}

// Draw a forward navigation button with arrow
void DrawForwardButton(Rectangle bounds, bool isActive, bool isHovered, bool isPressed) {
    Color baseColor = isActive ? 
                    (isPressed ? UI_PRIMARY_DARK : 
                    (isHovered ? ColorBrightness(UI_PRIMARY, 0.1f) : UI_PRIMARY)) : 
                    ColorAlpha((Color){200, 200, 200, 200}, 0.8f); // Disabled state
    
    // Shadow
    DrawShadow(bounds, BUTTON_ROUNDING, 8, ColorAlpha(BLACK, 0.15f));
    
    // Main button
    DrawRectangleRounded(bounds, BUTTON_ROUNDING, 8, baseColor);
    
    // Draw arrow
    float arrowWidth = bounds.width * 0.4f;
    float arrowHeight = bounds.height * 0.4f;
    float centerX = bounds.x + bounds.width/2;
    float centerY = bounds.y + bounds.height/2;
    
    // Right-pointing triangle (filled)
    Vector2 points[3] = {
        (Vector2){centerX - arrowWidth/2, centerY - arrowHeight/2},
        (Vector2){centerX + arrowWidth/2, centerY},
        (Vector2){centerX - arrowWidth/2, centerY + arrowHeight/2}
    };
    DrawTriangle(points[0], points[1], points[2], WHITE);
    
    // Add subtle highlight on top edge
    DrawLineEx(
        (Vector2){bounds.x + BUTTON_ROUNDING * bounds.height, bounds.y + 1},
        (Vector2){bounds.x + bounds.width - BUTTON_ROUNDING * bounds.height, bounds.y + 1},
        2.0f, ColorAlpha(WHITE, 0.3f)
    );
}

// Draw a backward navigation button with arrow
void DrawBackwardButton(Rectangle bounds, bool isActive, bool isHovered, bool isPressed) {
    Color baseColor = isActive ? 
                    (isPressed ? UI_PRIMARY_DARK : 
                    (isHovered ? ColorBrightness(UI_PRIMARY, 0.1f) : UI_PRIMARY)) : 
                    ColorAlpha((Color){200, 200, 200, 200}, 0.8f); // Disabled state
    
    // Shadow
    DrawShadow(bounds, BUTTON_ROUNDING, 8, ColorAlpha(BLACK, 0.15f));
    
    // Main button
    DrawRectangleRounded(bounds, BUTTON_ROUNDING, 8, baseColor);
    
    // Draw arrow
    float arrowWidth = bounds.width * 0.4f;
    float arrowHeight = bounds.height * 0.4f;
    float centerX = bounds.x + bounds.width/2;
    float centerY = bounds.y + bounds.height/2;
    
    // Left-pointing triangle (filled)
    Vector2 points[3] = {
        (Vector2){centerX + arrowWidth/2, centerY - arrowHeight/2},
        (Vector2){centerX - arrowWidth/2, centerY},
        (Vector2){centerX + arrowWidth/2, centerY + arrowHeight/2}
    };
    DrawTriangle(points[0], points[1], points[2], WHITE);
    
    // Add subtle highlight on top edge
    DrawLineEx(
        (Vector2){bounds.x + BUTTON_ROUNDING * bounds.height, bounds.y + 1},
        (Vector2){bounds.x + bounds.width - BUTTON_ROUNDING * bounds.height, bounds.y + 1},
        2.0f, ColorAlpha(WHITE, 0.3f)
    );
}

// Draw play/pause button
void DrawPlayPauseButton(Rectangle bounds, bool isPlaying, bool isHovered, bool isPressed) {
    Color baseColor = isPlaying ? 
                    UI_SUCCESS : // Green when playing
                    UI_INFO;     // Blue when paused
    
    if (isPressed) baseColor = ColorBrightness(baseColor, -0.2f);
    else if (isHovered) baseColor = ColorBrightness(baseColor, 0.1f);
    
    // Shadow
    DrawShadow(bounds, BUTTON_ROUNDING, 8, ColorAlpha(BLACK, 0.15f));
    
    // Main button
    DrawRectangleRounded(bounds, BUTTON_ROUNDING, 8, baseColor);
    
    // Draw play/pause icon
    float iconSize = bounds.width * 0.4f;
    float centerX = bounds.x + bounds.width/2;
    float centerY = bounds.y + bounds.height/2;
    
    if (isPlaying) {
        // Pause icon (two bars)
        float barWidth = iconSize * 0.25f;
        float barHeight = iconSize * 0.7f;
        float barSpacing = iconSize * 0.2f;
        
        DrawRectangleRec(
            (Rectangle){
                centerX - barSpacing - barWidth, 
                centerY - barHeight/2, 
                barWidth, 
                barHeight
            }, 
            WHITE
        );
        
        DrawRectangleRec(
            (Rectangle){
                centerX + barSpacing, 
                centerY - barHeight/2, 
                barWidth, 
                barHeight
            }, 
            WHITE
        );
    } else {
        // Play icon (triangle)
        Vector2 points[3] = {
            (Vector2){centerX - iconSize/3, centerY - iconSize/2},
            (Vector2){centerX + iconSize/2, centerY},
            (Vector2){centerX - iconSize/3, centerY + iconSize/2}
        };
        DrawTriangle(points[0], points[1], points[2], WHITE);
    }
    
    // Add subtle highlight on top edge
    DrawLineEx(
        (Vector2){bounds.x + BUTTON_ROUNDING * bounds.height, bounds.y + 1},
        (Vector2){bounds.x + bounds.width - BUTTON_ROUNDING * bounds.height, bounds.y + 1},
        2.0f, ColorAlpha(WHITE, 0.3f)
    );
}

// Draw a modern progress bar with scrubber
void DrawModernProgressBar(Rectangle bounds, float progress, bool isDragging) {
    progress = progress < 0.0f ? 0.0f : (progress > 1.0f ? 1.0f : progress);
    
    // Draw background shadow
    DrawRectangleRounded(
        (Rectangle){
            bounds.x + 2, 
            bounds.y + 2, 
            bounds.width, 
            bounds.height
        },
        0.5f, 8, ColorAlpha(BLACK, 0.1f)
    );
    
    // Draw track background
    DrawRectangleRounded(
        bounds,
        0.5f, 8, (Color){227, 237, 247, 255}
    );
    
    // Draw progress fill with gradient
    DrawRectangleGradientH(
        bounds.x, bounds.y, 
        bounds.width * progress, bounds.height,
        UI_PRIMARY,
        UI_PRIMARY_LIGHT
    );
    
    // Apply rounded corners to fill area
    DrawRectangleRounded(
        (Rectangle){
            bounds.x, 
            bounds.y, 
            bounds.width * progress, 
            bounds.height
        },
        0.5f, 8, ColorAlpha((Color){0, 0, 0, 0}, 0.0f) // Transparent to preserve gradient
    );
    
    // Draw scrubber handle with neumorphic style
    float handleX = bounds.x + bounds.width * progress;
    float handleRadius = bounds.height * 0.8f;
    
    // Handle shadow
    DrawCircle(
        handleX + 2, 
        bounds.y + bounds.height/2 + 2, 
        handleRadius, 
        ColorAlpha(BLACK, 0.15f)
    );
    
    // Handle main circle
    Color handleColor = isDragging ? 
                      UI_PRIMARY_DARK : // Active blue
                      (Color){255, 255, 255, 255}; // White
    DrawCircle(
        handleX, 
        bounds.y + bounds.height/2, 
        handleRadius, 
        handleColor
    );
    
    // Handle outline
    DrawCircleLines(
        handleX, 
        bounds.y + bounds.height/2, 
        handleRadius, 
        ColorAlpha(UI_PRIMARY, 0.5f)
    );
}

// Draw text with optional shadow
void DrawTextWithShadow(const char* text, int posX, int posY, int fontSize, Color textColor, Color shadowColor) {
    DrawText(text, posX + 1, posY + 1, fontSize, shadowColor);
    DrawText(text, posX, posY, fontSize, textColor);
}

// Draw a modern input box with animation
void DrawInputBox(Rectangle bounds, const char* text, bool isActive, int cursorPos, bool showCursor) {
    Color bgColor = isActive ? ColorAlpha(UI_PRIMARY, 0.1f) : UI_LIGHT;
    Color borderColor = isActive ? UI_PRIMARY : ColorAlpha(UI_DARK, 0.3f);
    
    // Draw shadow
    DrawShadow(bounds, BUTTON_ROUNDING, 8, ColorAlpha(BLACK, 0.05f));
    
    // Draw background
    DrawRectangleRounded(bounds, BUTTON_ROUNDING, 8, bgColor);
    
    // Draw border
    DrawRectangleRoundedLines(bounds, BUTTON_ROUNDING, 8, borderColor);
    
    // Draw text
    if (text && text[0] != '\0') {
        DrawText(text, bounds.x + 10, bounds.y + (bounds.height - 20)/2, 20, UI_TEXT);
        
        // Draw cursor if active and blinking
        if (isActive && showCursor) {
            // Calculate cursor position
            std::string textBeforeCursor(text, cursorPos);
            int cursorX = bounds.x + 10 + MeasureText(textBeforeCursor.c_str(), 20);
            DrawRectangle(cursorX, bounds.y + (bounds.height - 20)/2, 2, 20, UI_PRIMARY);
        }
    }
}

// Draw title text with a subtle underline
void DrawTitleText(const char* text, int posX, int posY, int fontSize, Color textColor) {
    int textWidth = MeasureText(text, fontSize);
    
    // Draw text
    DrawText(text, posX, posY, fontSize, textColor);
    
    // Draw underline
    DrawLineEx(
        (Vector2){posX, posY + fontSize + 5},
        (Vector2){posX + textWidth, posY + fontSize + 5},
        2,
        ColorAlpha(textColor, 0.3f)
    );
}

// Draw a card container with shadow and optional title
void DrawCard(Rectangle bounds, const char* title, Color bgColor) {
    // Draw shadow
    DrawRectangleRounded(
        (Rectangle){
            bounds.x + SHADOW_OFFSET,
            bounds.y + SHADOW_OFFSET,
            bounds.width,
            bounds.height
        },
        BUTTON_ROUNDING, 8, ColorAlpha(BLACK, 0.2f)
    );
    
    // Draw card background
    DrawRectangleRounded(bounds, BUTTON_ROUNDING, 8, bgColor);
    
    // Draw title if provided
    if (title && title[0] != '\0') {
        int fontSize = 24;
        DrawText(title, bounds.x + 20, bounds.y + 15, fontSize, UI_TEXT);
        
        // Draw separator line
        DrawLineEx(
            (Vector2){bounds.x + 20, bounds.y + 15 + fontSize + 10},
            (Vector2){bounds.x + bounds.width - 20, bounds.y + 15 + fontSize + 10},
            1,
            ColorAlpha(UI_TEXT, 0.2f)
        );
    }
}
