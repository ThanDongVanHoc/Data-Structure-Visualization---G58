#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "raylib.h"

// Shared visualization constants
static const float NODE_RADIUS = 25.0f;
static const float HORIZONTAL_SPACING = 100.0f;
static const float VERTICAL_SPACING = 100.0f;
static const float NODE_SPACING = 120.0f;
static const float START_X = 200.0f;
static const float START_Y = 400.0f;

// Physics constants
static const float SPRING_CONSTANT = 0.03f;
static const float DAMPING = 0.75f;
static const float MAX_VELOCITY = 5.0f;
static const float SETTLE_DISTANCE = 0.5f;
static const float SETTLE_VELOCITY = 0.05f;

// Colors
static const Color TEXT_COLOR = (Color){ 0, 0, 0, 255 };
static const Color NODE_DEFAULT_COLOR = (Color){ 97, 205, 207, 255 }; // Xanh lá mặc định cho node
static const Color NODE_BORDER_COLOR = (Color){ 0, 0, 0, 255 };
static const Color HIGHLIGHT_COLOR = (Color){ 243, 156, 18, 255 };
static const Color HIGHLIGHT_SEARCH = (Color){ 142, 68, 173, 255 };
static const Color HIGHLIGHT_INSERT = (Color){ 46, 213, 115, 255 }; // Xanh lá đậm cho insert
static const Color HIGHLIGHT_ROTATE = (Color){ 253, 203, 110, 255 }; // Vàng cho rotate
static const Color HIGHLIGHT_DELETE = (Color){ 214, 48, 49, 255 }; // Đỏ cho delete
static const Color HIGHLIGHT_FIND = (Color){ 249, 127, 81, 255 }; // Cam nhạt cho find
static const Color HIGHLIGHT_TRAVERSE = (Color){ 125, 95, 255, 255 }; // Tím cho duyệt
static const Color HEIGHT_TEXT_COLOR = (Color){ 0, 0, 0, 255 };
static const Color NODE_HIGHLIGHT_INSERT = (Color){ 46, 204, 113, 255 };
static const Color NODE_HIGHLIGHT_DELETE = (Color){ 231, 76, 60, 255 };
static const Color NODE_HIGHLIGHT_SEARCH = (Color){ 241, 196, 15, 255 };
static const Color NODE_HIGHLIGHT_CURRENT = (Color){ 155, 89, 182, 255 };
static const Color ARROW_COLOR = (Color){ 149, 165, 166, 255 };

// Background colors
static const Color BACKGROUND_COLOR_TOP = (Color){ 241, 245, 249, 255 };
static const Color BACKGROUND_COLOR_BOTTOM = (Color){ 226, 237, 248, 255 };

#endif // CONSTANTS_H
