// renderer.cpp
#include <bits/stdc++.h>
#include "renderer.h"
#include "raylib.h"
#include <chrono>
#include <thread>
#include <string>
#include "trie.h"
#define MAX_INPUT_CHARS 9
int nodeRadius = 20;
using namespace std;
mt19937 rd(chrono::steady_clock::now().time_since_epoch().count());
#define FOR(i, a, b) for (int i = (a), _b = (b); i <= _b; i++)

long long Rand(long long l, long long r)
{
    long long res = 0;
    FOR(i, 1, 4)
    res = (res << 15) ^ (rand() & ((1 << 15) - 1));
    return l + res % (r - l + 1);
}

enum HashingMode
{
    LINEAR,
    QUADRATIC,
    DOUBLE_HASHING
};

Renderer::Renderer() {}

bool IsAnyKeyPressed()
{
    bool keyPressed = false;
    int key = GetKeyPressed();
    if ((key >= 32) && (key <= 126))
        keyPressed = true;
    return keyPressed;
}

void DrawHashTable(int hashTable[], int foundIndex, bool found, int colorEffectCounter)
{
    for (int i = 0; i < 67; ++i)
    {
        int x = (i % 10) * 80 + 50;  // 10 circles per row
        int y = (i / 10) * 80 + 200; // 7 rows, shifted down by 150 pixels
        Color circleColor = BLACK;
        if (found && i == foundIndex)
        {
            // Apply green color if found
            circleColor = GREEN;
        }
        // Draw thicker border by drawing multiple concentric circles
        for (double thickness = 0; thickness < 3; thickness += 0.5)
        {
            DrawCircleLines(x, y, 20 + thickness, circleColor);
        }
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
}

void DrawInputBoxes(Rectangle inputBox, Rectangle searchBox, Rectangle removeBox, const char *inputBuffer, const char *searchBuffer, const char *removeBuffer, int framesCounter, bool inputActive, bool searchActive, bool removeActive)
{
    // Draw input box
    DrawRectangleRec(inputBox, LIGHTGRAY);
    DrawText(inputBuffer, 810, 215, 20, BLACK);

    // Draw search box
    DrawRectangleRec(searchBox, LIGHTGRAY);
    DrawText(searchBuffer, 810, 315, 20, BLACK);

    // Draw remove box
    DrawRectangleRec(removeBox, LIGHTGRAY);
    DrawText(removeBuffer, 810, 415, 20, BLACK);

    if (((framesCounter / 20) % 2) == 0)
    {
        if (inputActive && strlen(inputBuffer) < MAX_INPUT_CHARS)
            DrawText("_", 810 + MeasureText(inputBuffer, 20), 215, 20, BLACK);
        if (searchActive && strlen(searchBuffer) < MAX_INPUT_CHARS)
            DrawText("_", 810 + MeasureText(searchBuffer, 20), 315, 20, BLACK);
        if (removeActive && strlen(removeBuffer) < MAX_INPUT_CHARS)
            DrawText("_", 810 + MeasureText(removeBuffer, 20), 415, 20, BLACK);
    }
}

void HandleInput(Rectangle box, char *buffer, int &index, bool &isActive)
{
    Vector2 mousePoint = GetMousePosition();
    if (CheckCollisionPointRec(mousePoint, box) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        isActive = true;
    }
    else if (!CheckCollisionPointRec(mousePoint, box) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        isActive = false;
    }

    if (isActive)
    {
        // Set the window's cursor to the I-Beam
        SetMouseCursor(MOUSE_CURSOR_IBEAM);

        // Get char pressed (unicode character) on the queue
        int key = GetCharPressed();

        // Check if more characters have been pressed on the same frame
        while (key > 0)
        {
            // NOTE: Only allow keys in range [32..125]
            if ((key >= 32) && (key <= 125) && (index < MAX_INPUT_CHARS))
            {
                buffer[index] = (char)key;
                buffer[index + 1] = '\0'; // Add null terminator at the end of the string.
                index++;
            }

            key = GetCharPressed(); // Check next character in the queue
        }

        if (IsKeyPressed(KEY_BACKSPACE))
        {
            index--;
            if (index < 0)
                index = 0;
            buffer[index] = '\0';
        }
    }
    else
    {
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    }
}

bool CheckButton(Rectangle button, const char *text)
{
    Vector2 mousePoint = GetMousePosition();
    bool mouseOnButton = CheckCollisionPointRec(mousePoint, button);

    if (mouseOnButton)
    {
        DrawRectangleRec(button, LIGHTGRAY);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            return true;
        }
    }
    else
    {
        DrawRectangleRec(button, GRAY);
    }

    DrawText(text, button.x + 10, button.y + 10, 20, BLACK);
    return false;
}

int Hash2(int key)
{
    int prime = 61; // A prime number less than the table size
    return prime - (key % prime);
}

void Renderer::RenderHashTable()
{
    static int hashTable[67] = {0};                      // Initialize hash table with zeros
    static char inputBuffer[MAX_INPUT_CHARS + 1] = {0};  // Buffer for input
    static char searchBuffer[MAX_INPUT_CHARS + 1] = {0}; // Buffer for search input
    static char removeBuffer[MAX_INPUT_CHARS + 1] = {0}; // Buffer for remove input
    static int inputIndex = 0;                           // Index for input buffer
    static int searchIndex = 0;                          // Index for search buffer
    static int removeIndex = 0;                          // Index for remove buffer
    static bool found = false;                           // Flag to indicate if the value was found
    static int foundIndex = -1;                          // Index where the value was found
    static bool showNotFoundMessage = false;             // Flag to indicate if the "Value not found" message should be displayed
    static int colorEffectCounter = 0;                   // Counter for color effect
    static HashingMode mode = LINEAR;                    // Default hashing mode

    static bool inputActive = false;
    static bool searchActive = false;
    static bool removeActive = false;

    Rectangle inputBox = {800, 200, 200, 50};
    Rectangle searchBox = {800, 300, 200, 50};
    Rectangle removeBox = {800, 400, 200, 50};
    Rectangle linearButton = {50, 50, 200, 30};
    Rectangle quadraticButton = {270, 50, 200, 30};
    Rectangle doubleHashingButton = {490, 50, 200, 30};
    int framesCounter = 0;

    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Draw mode selection buttons
    if (CheckButton(linearButton, "Linear Probing"))
    {
        mode = LINEAR;
        memset(hashTable, 0, sizeof(hashTable));
        inputIndex = searchIndex = removeIndex = 0;
        inputBuffer[0] = searchBuffer[0] = removeBuffer[0] = '\0';
        found = false;
        foundIndex = -1;
        showNotFoundMessage = false;
    }
    if (CheckButton(quadraticButton, "Quadratic Probing"))
    {
        mode = QUADRATIC;
        memset(hashTable, 0, sizeof(hashTable));
        inputIndex = searchIndex = removeIndex = 0;
        inputBuffer[0] = searchBuffer[0] = removeBuffer[0] = '\0';
        found = false;
        foundIndex = -1;
        showNotFoundMessage = false;
    }
    if (CheckButton(doubleHashingButton, "Double Hashing"))
    {
        mode = DOUBLE_HASHING;
        memset(hashTable, 0, sizeof(hashTable));
        inputIndex = searchIndex = removeIndex = 0;
        inputBuffer[0] = searchBuffer[0] = removeBuffer[0] = '\0';
        found = false;
        foundIndex = -1;
        showNotFoundMessage = false;
    }

    DrawHashTable(hashTable, foundIndex, found, colorEffectCounter);
    DrawInputBoxes(inputBox, searchBox, removeBox, inputBuffer, searchBuffer, removeBuffer, framesCounter, inputActive, searchActive, removeActive);

    HandleInput(inputBox, inputBuffer, inputIndex, inputActive);
    HandleInput(searchBox, searchBuffer, searchIndex, searchActive);
    HandleInput(removeBox, removeBuffer, removeIndex, removeActive);

    // Draw current mode text
    const char *modeText = "";
    switch (mode)
    {
    case LINEAR:
        modeText = "Current Mode: Linear Probing";
        break;
    case QUADRATIC:
        modeText = "Current Mode: Quadratic Probing";
        break;
    case DOUBLE_HASHING:
        modeText = "Current Mode: Double Hashing";
        break;
    }
    DrawText(modeText, 50, 100, 20, RED);

    if (IsKeyPressed(KEY_ENTER))
    {
        showNotFoundMessage = false; // Reset the not found message flag when a new action is performed

        if (inputActive)
        {
            int inputNumber = std::stoi(inputBuffer);
            int hashIndex = inputNumber % 67;
            int i = 0;
            while (hashTable[hashIndex] != 0)
            {
                // Highlight the current probing index
                int x = (hashIndex % 10) * 80 + 50;
                int y = (hashIndex / 10) * 80 + 200;
                for (double thickness = 0; thickness < 3; thickness += 0.2)
                {
                    DrawCircleLines(x, y, 20 + thickness, YELLOW);
                }
                EndDrawing();
                std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Delay for animation
                BeginDrawing();
                ClearBackground(RAYWHITE);

                // Redraw mode selection buttons and current mode text
                DrawRectangleRec(linearButton, GRAY);
                DrawText("Linear Probing", linearButton.x + 10, linearButton.y + 10, 20, BLACK);
                DrawRectangleRec(quadraticButton, GRAY);
                DrawText("Quadratic Probing", quadraticButton.x + 10, quadraticButton.y + 10, 20, BLACK);
                DrawRectangleRec(doubleHashingButton, GRAY);
                DrawText("Double Hashing", doubleHashingButton.x + 10, doubleHashingButton.y + 10, 20, BLACK);
                DrawText(modeText, 50, 100, 20, RED);

                DrawHashTable(hashTable, foundIndex, found, colorEffectCounter);
                DrawInputBoxes(inputBox, searchBox, removeBox, inputBuffer, searchBuffer, removeBuffer, framesCounter, inputActive, searchActive, removeActive);

                switch (mode)
                {
                case LINEAR:
                    hashIndex = (hashIndex + 1) % 67;
                    break;
                case QUADRATIC:
                    i++;
                    hashIndex = (hashIndex + i * i) % 67;
                    break;
                case DOUBLE_HASHING:
                    i++;
                    hashIndex = (hashIndex + i * Hash2(inputNumber)) % 67;
                    break;
                }
            }
            hashTable[hashIndex] = inputNumber;
            inputIndex = 0;
            inputBuffer[0] = '\0';
            found = false; // Reset found flag when inserting a new value
        }
        else if (searchActive)
        {
            int searchNumber = std::stoi(searchBuffer);
            int hashIndex = searchNumber % 67;
            int i = 0;
            found = false;
            while (hashTable[hashIndex] != 0)
            {
                // Highlight the current probing index
                int x = (hashIndex % 10) * 80 + 50;
                int y = (hashIndex / 10) * 80 + 200;
                for (double thickness = 0; thickness < 3; thickness += 0.2)
                {
                    DrawCircleLines(x, y, 20 + thickness, YELLOW);
                }
                EndDrawing();
                std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Delay for animation
                BeginDrawing();
                ClearBackground(RAYWHITE);

                // Redraw mode selection buttons and current mode text
                DrawRectangleRec(linearButton, GRAY);
                DrawText("Linear Probing", linearButton.x + 10, linearButton.y + 10, 20, BLACK);
                DrawRectangleRec(quadraticButton, GRAY);
                DrawText("Quadratic Probing", quadraticButton.x + 10, quadraticButton.y + 10, 20, BLACK);
                DrawRectangleRec(doubleHashingButton, GRAY);
                DrawText("Double Hashing", doubleHashingButton.x + 10, doubleHashingButton.y + 10, 20, BLACK);
                DrawText(modeText, 50, 100, 20, RED);

                DrawHashTable(hashTable, foundIndex, found, colorEffectCounter);
                DrawInputBoxes(inputBox, searchBox, removeBox, inputBuffer, searchBuffer, removeBuffer, framesCounter, inputActive, searchActive, removeActive);

                if (hashTable[hashIndex] == searchNumber)
                {
                    found = true;
                    foundIndex = hashIndex;
                    colorEffectCounter = 0; // Reset color effect counter
                    break;
                }

                switch (mode)
                {
                case LINEAR:
                    hashIndex = (hashIndex + 1) % 67;
                    break;
                case QUADRATIC:
                    i++;
                    hashIndex = (hashIndex + i * i) % 67;
                    break;
                case DOUBLE_HASHING:
                    i++;
                    hashIndex = (hashIndex + i * Hash2(searchNumber)) % 67;
                    break;
                }
            }

            if (!found)
            {
                // Indicate that the value was not found
                showNotFoundMessage = true;
                foundIndex = -1; // Reset found index
            }

            searchIndex = 0;
            searchBuffer[0] = '\0';
        }
        else if (removeActive)
        {
            int removeNumber = std::stoi(removeBuffer);
            int hashIndex = removeNumber % 67;
            int i = 0;
            found = false;
            while (hashTable[hashIndex] != 0)
            {
                // Highlight the current probing index
                int x = (hashIndex % 10) * 80 + 50;
                int y = (hashIndex / 10) * 80 + 200;
                for (double thickness = 0; thickness < 3; thickness += 0.2)
                {
                    DrawCircleLines(x, y, 20 + thickness, YELLOW);
                }
                EndDrawing();
                std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Delay for animation
                BeginDrawing();
                ClearBackground(RAYWHITE);

                // Redraw mode selection buttons and current mode text
                DrawRectangleRec(linearButton, GRAY);
                DrawText("Linear Probing", linearButton.x + 10, linearButton.y + 10, 20, BLACK);
                DrawRectangleRec(quadraticButton, GRAY);
                DrawText("Quadratic Probing", quadraticButton.x + 10, quadraticButton.y + 10, 20, BLACK);
                DrawRectangleRec(doubleHashingButton, GRAY);
                DrawText("Double Hashing", doubleHashingButton.x + 10, doubleHashingButton.y + 10, 20, BLACK);
                DrawText(modeText, 50, 100, 20, RED);

                DrawHashTable(hashTable, foundIndex, found, colorEffectCounter);
                DrawInputBoxes(inputBox, searchBox, removeBox, inputBuffer, searchBuffer, removeBuffer, framesCounter, inputActive, searchActive, removeActive);

                if (hashTable[hashIndex] == removeNumber)
                {
                    hashTable[hashIndex] = 0; // Remove the value
                    found = true;
                    foundIndex = -1; // Reset found index
                    break;
                }

                switch (mode)
                {
                case LINEAR:
                    hashIndex = (hashIndex + 1) % 67;
                    break;
                case QUADRATIC:
                    i++;
                    hashIndex = (hashIndex + i * i) % 67;
                    break;
                case DOUBLE_HASHING:
                    i++;
                    hashIndex = (hashIndex + i * Hash2(removeNumber)) % 67;
                    break;
                }
            }

            if (!found)
            {
                // Indicate that the value was not found
                showNotFoundMessage = true;
            }

            removeIndex = 0;
            removeBuffer[0] = '\0';
        }
    }

    if (showNotFoundMessage)
    {
        DrawText("Value not found", 800, 350, 20, RED);
    }

    if (found)
    {
        colorEffectCounter++; // Increment color effect counter
    }

    EndDrawing();
}
// Hàm vẽ một đường thẳng có mũi tên (đầu mũi tên là tam giác đầy)
void drawArrowLine(Vector2 start, Vector2 end, float thickness, Color color)
{
    // Vẽ đường thẳng nối hai điểm
    DrawLineEx(start, end, thickness, color);
    // Tính vector chỉ phương từ start đến end
    Vector2 direction = {end.x - start.x, end.y - start.y};
    float len = sqrt(direction.x * direction.x + direction.y * direction.y);
    if (len == 0)
        return;
    direction.x /= len;
    direction.y /= len;

    float arrowLength = 10.0f; // Chiều dài của mũi tên
    float arrowWidth = 5.0f;   // Chiều rộng của mũi tên

    // Tạo vector vuông góc với hướng của đường thẳng
    Vector2 perp = {-direction.y, direction.x};

    // Tính các điểm của tam giác mũi tên
    Vector2 arrowTip = end;
    Vector2 arrowLeft = {end.x - direction.x * arrowLength + perp.x * arrowWidth,
                         end.y - direction.y * arrowLength + perp.y * arrowWidth};
    Vector2 arrowRight = {end.x - direction.x * arrowLength - perp.x * arrowWidth,
                          end.y - direction.y * arrowLength - perp.y * arrowWidth};

    // Vẽ tam giác đầy làm đầu mũi tên
    DrawTriangle(arrowTip, arrowRight, arrowLeft, color);
}
void DrawTrie(Trie::Node *node, int screenWidth, int screenHeight)
{
    if (!node)
        return;
    if (node->cnt <= 0)
        return;
    if (node->exist)
    {
        DrawCircle(node->x, node->y, nodeRadius, GREEN);
    }
    for (double thickness = 0.0; thickness < 3.0; thickness += 0.5)
        DrawCircleLinesV({node->x, node->y}, 20 + thickness, node->color);
    int valueTextWidth = MeasureText(node->character.c_str(), 20);
    DrawText(node->character.c_str(), node->x - valueTextWidth / 2, node->y - 10, 20, BLUE);
    // Vẽ đường nối từ nút cha tới các nút con
    for (int i = 0; i < 52; i++)
    {
        if (node->child[i] != NULL)
        {
            Vector2 parentCenter = {node->x, node->y};
            Vector2 childCenter = {node->child[i]->x, node->child[i]->y};

            // Tính vector chỉ phương từ cha đến con
            Vector2 direction = {childCenter.x - parentCenter.x, childCenter.y - parentCenter.y};
            float len = sqrt(direction.x * direction.x + direction.y * direction.y);
            if (len != 0)
            {
                direction.x /= len;
                direction.y /= len;
            }

            // Tính điểm bắt đầu và kết thúc: cách tâm một khoảng bằng bán kính
            Vector2 startEdge = {parentCenter.x + direction.x * nodeRadius, parentCenter.y + direction.y * nodeRadius};
            Vector2 endEdge = {childCenter.x - direction.x * nodeRadius, childCenter.y - direction.y * nodeRadius};

            drawArrowLine(startEdge, endEdge, 2.0f, BLACK);
            DrawTrie(node->child[i], screenWidth, screenHeight);
        }
    }
}

void Renderer::RenderTrie()
{
    static Trie trie;
    static int framesCounter = 0;
    static bool trieInputActive = false; // Flag to indicate if the Trie input box is active
    Rectangle inputBox = {800, 200, 200, 50};
    Rectangle searchBox = {800, 300, 200, 50};
    Rectangle removeBox = {800, 400, 200, 50};
    static int inputIndex = 0;  // Index for input buffer
    static int searchIndex = 0; // Index for search buffer
    static int removeIndex = 0;
    static char inputBuffer[MAX_INPUT_CHARS + 1] = {0};  // Buffer for input
    static char searchBuffer[MAX_INPUT_CHARS + 1] = {0}; // Buffer for search input
    static char removeBuffer[MAX_INPUT_CHARS + 1] = {0}; // Buffer for remove input
    static bool inputActive = false;
    static bool searchActive = false;
    static bool removeActive = false;
    ClearBackground(RAYWHITE);

    DrawInputBoxes(inputBox, searchBox, removeBox, inputBuffer, searchBuffer, removeBuffer, framesCounter, inputActive, searchActive, removeActive);

    HandleInput(inputBox, inputBuffer, inputIndex, inputActive);
    HandleInput(searchBox, searchBuffer, searchIndex, searchActive);
    HandleInput(removeBox, removeBuffer, removeIndex, removeActive);

    if (IsKeyPressed(KEY_ENTER))
    {
        if (inputActive)
        {
            std::string inputString(inputBuffer);
            if (is_valid_input(inputString))
            {
                trie.add_string(inputString);

                // Recompute positions after adding a new string
                float currentX = 40;
                float horizontalSpacing = 80;
                float verticalSpacing = 80;
                trie.compute_positions(trie.root, 1, currentX, horizontalSpacing, verticalSpacing);

                // Clear the input buffer
                inputIndex = 0;
                inputBuffer[0] = '\0';
            }
        }
        else if (removeActive)
        {
            std::string removeString(removeBuffer);
            if (is_valid_input(removeString))
            {
                trie.delete_string(removeString);
                // Recompute positions after removing a string
                float currentX = 40;
                float horizontalSpacing = 80;
                float verticalSpacing = 80;
                trie.compute_positions(trie.root, 1, currentX, horizontalSpacing, verticalSpacing);

                // Clear the remove buffer
                removeIndex = 0;
                removeBuffer[0] = '\0';
            }
        }
        else if (searchActive)
        {
            std::string searchString(searchBuffer);
            if (is_valid_input(searchString))
            {
                if (trie.find_string(searchString))
                {
                    // Highlight the found string
                }
                searchIndex = 0;
                searchBuffer[0] = '\0';
            }
        }
    }
    framesCounter++;
    DrawTrie(trie.root, GetScreenWidth(), GetScreenHeight());
}
