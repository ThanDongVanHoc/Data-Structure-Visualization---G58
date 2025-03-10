// main.cpp
#include "linked_list.h"
#include "renderer.h"
#include "raylib.h"
#include <string>
#define MAX_INPUT_CHARS 9
int main()
{
    InitWindow(1200, 900, "Data Structure Visualization");
    SetTargetFPS(60);
    LinkedList list;
    Renderer renderer;
    std::string inputText = ""; // Lưu số nhập vào
    int frameCount = 0;
    Rectangle insertButton = {320, 45, 80, 30}; // Vị trí và kích thước nút Insert
    char name[MAX_INPUT_CHARS + 1] = "\0";      // NOTE: One extra space required for null terminator char '\0'
    int letterCount = 0;
    Rectangle textBox = {800, 180, 225, 50};
    bool mouseOnText = false;
    while (!WindowShouldClose())
    {
        /*
        frameCount++;

        if (CheckCollisionPointRec(GetMousePosition(), textBox))
            mouseOnText = true;
        else
            mouseOnText = false;
        if (mouseOnText)
        {
            // Set the window's cursor to the I-Beam
            SetMouseCursor(MOUSE_CURSOR_IBEAM);

            // Get char pressed (unicode character) on the queue
            int key = GetCharPressed();

            // Check if more characters have been pressed on the same frame
            while (key > 0)
            {
                if ((key >= 32) && (key <= 125) && (letterCount < MAX_INPUT_CHARS))
                {
                    name[letterCount] = (char)key;
                    name[letterCount + 1] = '\0'; // Add null terminator at the end of the string.
                    letterCount++;
                }

                key = GetCharPressed(); // Check next character in the queue
            }
            if (IsKeyPressed(KEY_BACKSPACE))
            {
                letterCount--;
                if (letterCount < 0)
                    letterCount = 0;
                name[letterCount] = '\0';
            }
        }
        else
            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        if (mouseOnText)
            frameCount++;
        else
            frameCount = 0;
        ClearBackground(RAYWHITE);
        DrawRectangleRec(textBox, LIGHTGRAY);
        if (mouseOnText)
            DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, RED);
        else
            DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, DARKGRAY);
        DrawText(name, (int)textBox.x + 5, (int)textBox.y + 8, 40, MAROON);
        if (mouseOnText)
        {
            if (letterCount < MAX_INPUT_CHARS)
            {
                // Draw blinking underscore char
                if (((frameCount / 20) % 2) == 0)
                    DrawText("_", (int)textBox.x + 8 + MeasureText(name, 40), (int)textBox.y + 12, 40, MAROON);
            }
        }*/
        renderer.RenderHashTable();
    }

    CloseWindow();
    return 0;
}
