#ifndef DRAW_COMMAND_H
#define DRAW_COMMAND_H

#include "raylib.h"
#include <string>
#include <vector>

enum class DrawType {
    Rectangle,
    Line,
    Text,
    Triangle,
    Circle,
    RectangleLines,
    PseudoCode,  // Add this new type for pseudocode
    TextWithFont  // Add this type for custom font text
};

struct DrawCommand {
    DrawType type;
    Vector2 position;
    Vector4 size; // x, y for Line's second point, z for Text's value, w for rotation
    float rotation; // For Text: fontSize, for Lines: thickness
    Color color;
    std::string textStr; // Store the text as std::string for TextWithFont
    Font font; // For TextWithFont
    float value; // For legacy values
    
    static DrawCommand CreateRectangle(Vector2 pos, Vector2 size, Color color) {
        DrawCommand cmd;
        cmd.type = DrawType::Rectangle;
        cmd.position = pos;
        cmd.size = {size.x, size.y, 0, 0};
        cmd.color = color;
        return cmd;
    }
    
    static DrawCommand CreateLine(Vector2 start, Vector2 end, float thickness, Color color) {
        DrawCommand cmd;
        cmd.type = DrawType::Line;
        cmd.position = start;
        cmd.size = {end.x - start.x, end.y - start.y, 0, 0};
        cmd.rotation = thickness;
        cmd.color = color;
        return cmd;
    }

    static DrawCommand CreateText(const char* text, Vector2 pos, float fontSize, Color color) {
        DrawCommand cmd;
        cmd.type = DrawType::Text;
        cmd.position = pos;
        cmd.size = {0, 0, fontSize, 0};
        cmd.rotation = fontSize;
        cmd.color = color;
        cmd.value = 0;
        if (text) {
            cmd.textStr = text;
        }
        return cmd;
    }

    static DrawCommand CreateRectangleLines(Vector2 pos, Vector2 size, Color color) {
        DrawCommand cmd;
        cmd.type = DrawType::RectangleLines;
        cmd.position = pos;
        cmd.size = {size.x, size.y, 0, 0};
        cmd.color = color;
        return cmd;
    }

    static DrawCommand CreateTriangle(Vector2 p1, Vector2 p2, Vector2 p3, Color color) {
        DrawCommand cmd;
        cmd.type = DrawType::Triangle;
        cmd.position = p1;
        cmd.size = {p2.x, p2.y, p3.x, p3.y};
        cmd.color = color;
        return cmd;
    }

    // Add factory method for TextWithFont
    static DrawCommand CreateTextWithFont(Font font, const char* text, Vector2 pos, float fontSize, float spacing, Color color) {
        DrawCommand cmd;
        cmd.type = DrawType::TextWithFont;
        cmd.position = pos;
        cmd.size = {0, 0, fontSize, spacing};
        cmd.rotation = 0;
        cmd.color = color;
        cmd.font = font;
        if (text) {
            cmd.textStr = text; // Store the text as std::string
        }
        return cmd;
    }
    
    // Add factory method for Pseudocode
    static DrawCommand CreatePseudoCode(const char* operation, int highlightedLine, Color color) {
        DrawCommand cmd;
        cmd.type = DrawType::PseudoCode;
        cmd.position = {0, 0}; // Position will be handled by the drawing function
        cmd.size = {0, 0, (float)highlightedLine, 0}; // Store highlighted line in size.z
        cmd.color = color;
        if (operation) {
            cmd.textStr = operation; // Store the operation name
        }
        return cmd;
    }
};

#endif
