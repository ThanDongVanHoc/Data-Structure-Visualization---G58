// renderer.cpp
#include "renderer.h"
#include "raylib.h"
#include <chrono>
#include <thread>
#include <string>
#define MAX_INPUT_CHARS 9

Renderer::Renderer() {}

void Renderer::renderLinkedList(LinkedList *list)
{
    BeginDrawing();
    ClearBackground(RAYWHITE);
    Node *temp = list->head;
    int x = 100;
    int y = 200;
    while (temp)
    {
        DrawRectangle(x, y, 50, 50, BLUE);
        {
            DrawLine(x + 50, y + 25, x + 100, y + 25, BLACK);
        }
        x += 100;
        temp = temp->next;
    }
    EndDrawing();
}

void Renderer::renderAVL()
{
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawText("AVL Tree Visualization", 300, 250, 20, BLACK);
    EndDrawing();
}

bool IsAnyKeyPressed()
{
    bool keyPressed = false;
    int key = GetKeyPressed();
    if ((key >= 32) && (key <= 126))
        keyPressed = true;
    return keyPressed;
}

void Renderer::RenderHashTable()
{
    static int hashTable[97] = {0};                     // Initialize hash table with zeros
    static char inputBuffer[MAX_INPUT_CHARS + 1] = {0}; // Buffer for input
    static int inputIndex = 0;                          // Index for input buffer

    Rectangle inputBox = {800, 200, 200, 50};
    bool mouseOnText = false;
    int framesCounter = 0;

    BeginDrawing();
    ClearBackground(RAYWHITE);

    for (int i = 0; i < 97; ++i)
    {
        int x = (i % 10) * 80 + 50; // 10 circles per row
        int y = (i / 10) * 80 + 50; // 9 rows
        DrawCircleLines(x, y, 20, BLACK);
        std::string text = std::to_string(i);
        int textWidth = MeasureText(text.c_str(), 20);
        DrawText(text.c_str(), x - textWidth / 2, y + 30, 20, RED);

        if (hashTable[i] != 0)
        {
            std::string valueText = std::to_string(hashTable[i]);
            int valueTextWidth = MeasureText(valueText.c_str(), 20);
            DrawText(valueText.c_str(), x - valueTextWidth / 2, y - 10, 20, BLUE);
        }
    }

    // Draw input box
    DrawRectangleRec(inputBox, LIGHTGRAY);
    DrawText(inputBuffer, 810, 215, 20, BLACK);

    // Handle input if mouse is over the input box
    Vector2 mousePoint = GetMousePosition();
    if (CheckCollisionPointRec(mousePoint, inputBox))
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
            // NOTE: Only allow keys in range [32..125]
            if ((key >= 32) && (key <= 125) && (inputIndex < MAX_INPUT_CHARS))
            {
                inputBuffer[inputIndex] = (char)key;
                inputBuffer[inputIndex + 1] = '\0'; // Add null terminator at the end of the string.
                inputIndex++;
            }

            key = GetCharPressed(); // Check next character in the queue
        }

        if (IsKeyPressed(KEY_BACKSPACE))
        {
            inputIndex--;
            if (inputIndex < 0)
                inputIndex = 0;
            inputBuffer[inputIndex] = '\0';
        }
    }
    else
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);

    if (mouseOnText)
        framesCounter++;
    else
        framesCounter = 0;

    if (mouseOnText)
    {
        if (inputIndex < MAX_INPUT_CHARS)
        {
            // Draw blinking underscore char
            if (((framesCounter / 20) % 2) == 0)
                DrawText("_", 810 + MeasureText(inputBuffer, 20), 215, 20, BLACK);
        }
        else
            DrawText("Press BACKSPACE to delete chars...", 230, 300, 20, GRAY);
    }

    if (IsKeyPressed(KEY_ENTER))
    {
        int inputNumber = std::stoi(inputBuffer);
        int hashIndex = inputNumber % 97;
        while (hashTable[hashIndex] != 0)
        {
            // Highlight the current probing index
            int x = (hashIndex % 10) * 80 + 50;
            int y = (hashIndex / 10) * 80 + 50;
            DrawCircle(x, y, 20, YELLOW);
            EndDrawing();
            std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Delay for animation
            BeginDrawing();
            ClearBackground(RAYWHITE);

            // Redraw the hash table
            for (int i = 0; i < 97; ++i)
            {
                int x = (i % 10) * 80 + 50; // 10 circles per row
                int y = (i / 10) * 80 + 50; // 9 rows
                DrawCircleLines(x, y, 20, BLACK);
                std::string text = std::to_string(i);
                int textWidth = MeasureText(text.c_str(), 20);
                DrawText(text.c_str(), x - textWidth / 2, y + 30, 20, RED);

                if (hashTable[i] != 0)
                {
                    std::string valueText = std::to_string(hashTable[i]);
                    int valueTextWidth = MeasureText(valueText.c_str(), 20);
                    DrawText(valueText.c_str(), x - valueTextWidth / 2, y - 10, 20, BLUE);
                }
            }

            // Draw input box
            DrawRectangleRec(inputBox, LIGHTGRAY);
            DrawText(inputBuffer, 810, 215, 20, BLACK);

            hashIndex = (hashIndex + 1) % 97;
        }
        hashTable[hashIndex] = inputNumber;
        inputIndex = 0;
        inputBuffer[0] = '\0';
    }

    EndDrawing();
}
