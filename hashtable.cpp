// renderer.cpp
#include "raylib.h"
#include "raygui.h"
#include "gui_window_file_dialog.h"
#include <bits/stdc++.h>
#include <chrono>
#include <thread>
#include <string>
#include <fstream>
#include "var.h"
#include "hashtable.h"
#include <random> // Thêm thư viện random
const int MAX_INPUT_CHARS = 10;

int nodeRadius = 20;
using namespace std;

void resetcolor(std::vector<HashNode> &hashTable)
{
    for (size_t i = 0; i < hashTable.size(); ++i)
    {
        hashTable[i].color = BLACK; // Đặt lại màu sắc cho tất cả các ô
        HashNode *current = hashTable[i].next;
        while (current != nullptr)
        {
            current->color = BLACK; // Đặt lại màu sắc cho tất cả các nút trong danh sách liên kết
            current = current->next;
        }
    }
}
// Hàm vẽ một đường thẳng có mũi tên (đầu mũi tên là tam giác đầy)
void drawArrowLine_hash(Vector2 start, Vector2 end, float thickness, Color color)
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
// Hàm băm phụ cho Double Hashing
int Hash2(int key, int tableSize)
{
    int prime = tableSize - 1;
    while (prime > 1)
    {
        bool isPrime = true;
        for (int i = 2; i * i <= prime; ++i)
        {
            if (prime % i == 0)
            {
                isPrime = false;
                break;
            }
        }
        if (isPrime)
        {
            break;
        }
        --prime;
    }
    return max(1, prime - (key % prime));
}
void inputHashTable(std::vector<HashNode> &hashTable, std::vector<HashTableState> &history, int key, int value, HashingMode mode)
{
    int hashIndex = key % hashTable.size();
    int highlightedLine = 1; // Dòng đầu tiên của pseudocode
    for (int j = 0; j <= 20; j++)
        history.push_back({hashTable, "Insert", highlightedLine});

    if (mode == CHAINING)
    {
        hashTable[hashIndex].color = YELLOW;
        highlightedLine = 3; // Highlight dòng "index = key % size_of(hashTable)"
        for (int j = 0; j <= 20; j++)
            history.push_back({hashTable, "Insert", highlightedLine});

        HashNode *newNode = new HashNode();
        newNode->value = value;
        newNode->isEmpty = false;
        newNode->color = YELLOW;
        newNode->next = nullptr;

        if (hashTable[hashIndex].next == nullptr)
        {
            hashTable[hashIndex].next = newNode;
            highlightedLine = 4; // Highlight dòng "hashTable[index].append(value)"
            for (int j = 0; j <= 20; j++)
                history.push_back({hashTable, "Insert", highlightedLine});
        }
        else
        {
            HashNode *current = hashTable[hashIndex].next;
            while (current->next != nullptr)
            {
                current = current->next;
            }
            current->next = newNode;
            highlightedLine = 5; // Highlight dòng "hashTable[index].append(value)"
            for (int j = 0; j <= 20; j++)
                history.push_back({hashTable, "Insert", highlightedLine});
        }
        return;
    }

    int i = 0;
    while (!hashTable[hashIndex].isEmpty)
    {
        hashTable[hashIndex].color = YELLOW;
        switch (mode)
        {
        case LINEAR:
            highlightedLine = 3; // Highlight dòng "while hashTable[index] is not empty:"
            break;
        case QUADRATIC:
            highlightedLine = 4; // Highlight dòng "while hashTable[index] is not empty:"
            break;
        case DOUBLE_HASHING:
            highlightedLine = 4; // Highlight dòng "while hashTable[index] is not empty:"
            break;
        default:
            break;
        }
        for (int j = 0; j <= 20; j++)
            history.push_back({hashTable, "Insert", highlightedLine});

        if (mode == LINEAR)
            hashIndex = (hashIndex + 1) % hashTable.size();
        else if (mode == QUADRATIC)
            hashIndex = (hashIndex + (++i) * i) % hashTable.size();
        else if (mode == DOUBLE_HASHING)
            hashIndex = (hashIndex + (++i) * Hash2(key, hashTable.size())) % hashTable.size();
        switch (mode)
        {
        case LINEAR:
            highlightedLine = 4; // Highlight dòng "while hashTable[index] is not empty:"
            break;
        case QUADRATIC:
            highlightedLine = 6; // Highlight dòng "while hashTable[index] is not empty:"
            break;
        case DOUBLE_HASHING:
            highlightedLine = 5; // Highlight dòng "while hashTable[index] is not empty:"
            break;
        default:
            break;
        }
        for (int j = 0; j <= 20; j++)
            history.push_back({hashTable, "Insert", highlightedLine});
    }

    hashTable[hashIndex].value = value;
    hashTable[hashIndex].isEmpty = false;
    hashTable[hashIndex].color = YELLOW;
    switch (mode)
    {
    case LINEAR:
        highlightedLine = 5; // Highlight dòng "while hashTable[index] is not empty:"
        break;
    case QUADRATIC:
        highlightedLine = 7; // Highlight dòng "while hashTable[index] is not empty:"
        break;
    case DOUBLE_HASHING:
        highlightedLine = 6; // Highlight dòng "while hashTable[index] is not empty:"
        break;
    default:
        break;
    }
    for (int j = 0; j <= 20; j++)
        history.push_back({hashTable, "Insert", highlightedLine});
}
bool searchHashTable(std::vector<HashNode> &hashTable, std::vector<HashTableState> &history, int key, HashingMode mode, int &foundIndex)
{
    int hashIndex = key % hashTable.size();
    int highlightedLine = 1; // Dòng đầu tiên của pseudocode
    for (int j = 0; j <= 20; j++)

        history.push_back({hashTable, "Search", highlightedLine});

    if (mode == CHAINING)
    {
        HashNode *current = hashTable[hashIndex].next;

        highlightedLine = 2; // Highlight dòng "index = key % size_of(hashTable)"
        for (int j = 0; j <= 20; j++)

            history.push_back({hashTable, "Search", highlightedLine});

        while (current != nullptr)
        {
            current->color = YELLOW;
            highlightedLine = 4; // Highlight dòng "for node in hashTable[index]:"
            for (int j = 0; j <= 20; j++)

                history.push_back({hashTable, "Search", highlightedLine});

            if (current->value == key)
            {
                foundIndex = hashIndex;

                highlightedLine = 6; // Highlight dòng "return index"
                for (int j = 0; j <= 20; j++)

                    history.push_back({hashTable, "Search", highlightedLine});
                return true;
            }
            current = current->next;
        }

        highlightedLine = 7; // Highlight dòng "return -1 // Not found"
        for (int j = 0; j <= 20; j++)

            history.push_back({hashTable, "Search", highlightedLine});
        return false;
    }

    // Các chế độ khác (LINEAR, QUADRATIC, DOUBLE_HASHING)
    int i = 0;
    while (!hashTable[hashIndex].isEmpty)
    {
        hashTable[hashIndex].color = YELLOW;
        switch (mode)
        {
        case LINEAR:
            highlightedLine = 3; // Highlight dòng "while hashTable[index] is not empty:"
            break;
        case QUADRATIC:
            highlightedLine = 4; // Highlight dòng "while hashTable[index] is not empty:"
            break;
        case DOUBLE_HASHING:
            highlightedLine = 4; // Highlight dòng "while hashTable[index] is not empty:"
            break;
        default:
            break;
        }
        for (int j = 0; j <= 20; j++)

            history.push_back({hashTable, "Search", highlightedLine});

        if (hashTable[hashIndex].value == key)
        {
            foundIndex = hashIndex;

            switch (mode)
            {
            case LINEAR:
                highlightedLine = 4; // Highlight dòng "while hashTable[index] is not empty:"
                break;
            case QUADRATIC:
                highlightedLine = 5; // Highlight dòng "while hashTable[index] is not empty:"
                break;
            case DOUBLE_HASHING:
                highlightedLine = 5; // Highlight dòng "while hashTable[index] is not empty:"
                break;
            default:
                break;
            }
            for (int j = 0; j <= 20; j++)

                history.push_back({hashTable, "Search", highlightedLine});
            return true;
        }

        if (mode == LINEAR)
            hashIndex = (hashIndex + 1) % hashTable.size();
        else if (mode == QUADRATIC)
            hashIndex = (hashIndex + (++i) * i) % hashTable.size();
        else if (mode == DOUBLE_HASHING)
            hashIndex = (hashIndex + (++i) * Hash2(key, hashTable.size())) % hashTable.size();
        switch (mode)
        {
        case LINEAR:
            highlightedLine = 6; // Highlight dòng "while hashTable[index] is not empty:"
            break;
        case QUADRATIC:
            highlightedLine = 8; // Highlight dòng "while hashTable[index] is not empty:"
            break;
        case DOUBLE_HASHING:
            highlightedLine = 7; // Highlight dòng "while hashTable[index] is not empty:"
            break;
        default:
            break;
        }
        for (int j = 0; j <= 20; j++)

            history.push_back({hashTable, "Search", highlightedLine});
    }

    switch (mode)
    {
    case LINEAR:
        highlightedLine = 7; // Highlight dòng "while hashTable[index] is not empty:"
        break;
    case QUADRATIC:
        highlightedLine = 9; // Highlight dòng "while hashTable[index] is not empty:"
        break;
    case DOUBLE_HASHING:
        highlightedLine = 8; // Highlight dòng "while hashTable[index] is not empty:"
        break;
    default:
        break;
    }
    for (int j = 0; j <= 20; j++)

        history.push_back({hashTable, "Search", highlightedLine});
    return false;
}
bool removeHashTable(std::vector<HashNode> &hashTable, std::vector<HashTableState> &history, int key, HashingMode mode)
{
    int hashIndex = key % hashTable.size();
    int highlightedLine = 1; // Dòng đầu tiên của pseudocode
    for (int j = 0; j <= 20; j++)
        history.push_back({hashTable, "Remove", highlightedLine});

    if (mode == CHAINING)
    {
        HashNode *current = hashTable[hashIndex].next;
        HashNode *prev = nullptr;

        highlightedLine = 2; // Highlight dòng "index = key % size_of(hashTable)"
        for (int j = 0; j <= 20; j++)
            history.push_back({hashTable, "Remove", highlightedLine});

        while (current != nullptr)
        {
            current->color = YELLOW;
            highlightedLine = 3; // Highlight dòng "for node in hashTable[index]:"
            for (int j = 0; j <= 20; j++)
                history.push_back({hashTable, "Remove", highlightedLine});

            if (current->value == key)
            {
                if (prev == nullptr)
                {
                    hashTable[hashIndex].next = current->next;
                }
                else
                {
                    prev->next = current->next;
                }
                delete current;

                highlightedLine = 5; // Highlight dòng "hashTable[index].remove(node)"
                for (int j = 0; j <= 20; j++)
                    history.push_back({hashTable, "Remove", highlightedLine});
                return true;
            }
            prev = current;
            current = current->next;
        }

        highlightedLine = 6; // Highlight dòng "return false // Not found"
        for (int j = 0; j <= 20; j++)
            history.push_back({hashTable, "Remove", highlightedLine});
        return false;
    }

    // Các chế độ khác (LINEAR, QUADRATIC, DOUBLE_HASHING)
    int i = 0;
    while (!hashTable[hashIndex].isEmpty)
    {
        hashTable[hashIndex].color = YELLOW;
        switch (mode)
        {
        case LINEAR:
            highlightedLine = 3; // Highlight dòng "while hashTable[index] is not empty:"
            break;
        case QUADRATIC:
            highlightedLine = 4; // Highlight dòng "while hashTable[index] is not empty:"
            break;
        case DOUBLE_HASHING:
            highlightedLine = 4; // Highlight dòng "while hashTable[index] is not empty:"
            break;
        default:
            break;
        }
        for (int j = 0; j <= 20; j++)
            history.push_back({hashTable, "Remove", highlightedLine});

        if (hashTable[hashIndex].value == key)
        {
            hashTable[hashIndex].isEmpty = true;

            switch (mode)
            {
            case LINEAR:
                highlightedLine = 4; // Highlight dòng "hashTable[index] = empty"
                break;
            case QUADRATIC:
                highlightedLine = 5; // Highlight dòng "hashTable[index] = empty"
                break;
            case DOUBLE_HASHING:
                highlightedLine = 5; // Highlight dòng "hashTable[index] = empty"
                break;
            default:
                break;
            }
            for (int j = 0; j <= 20; j++)
                history.push_back({hashTable, "Remove", highlightedLine});
            return true;
        }

        if (mode == LINEAR)
            hashIndex = (hashIndex + 1) % hashTable.size();
        else if (mode == QUADRATIC)
            hashIndex = (hashIndex + (++i) * i) % hashTable.size();
        else if (mode == DOUBLE_HASHING)
            hashIndex = (hashIndex + (++i) * Hash2(key, hashTable.size())) % hashTable.size();

        switch (mode)
        {
        case LINEAR:
            highlightedLine = 5; // Highlight dòng "while hashTable[index] is not empty:"
            break;
        case QUADRATIC:
            highlightedLine = 7; // Highlight dòng "while hashTable[index] is not empty:"
            break;
        case DOUBLE_HASHING:
            highlightedLine = 6; // Highlight dòng "while hashTable[index] is not empty:"
            break;
        default:
            break;
        }
        for (int j = 0; j <= 20; j++)
            history.push_back({hashTable, "Remove", highlightedLine});
    }

    switch (mode)
    {
    case LINEAR:
        highlightedLine = 6; // Highlight dòng "return false // Not found"
        break;
    case QUADRATIC:
        highlightedLine = 8; // Highlight dòng "return false // Not found"
        break;
    case DOUBLE_HASHING:
        highlightedLine = 7; // Highlight dòng "return false // Not found"
        break;
    default:
        break;
    }
    for (int j = 0; j <= 20; j++)
        history.push_back({hashTable, "Remove", highlightedLine});
    return false;
}
// Hàm vẽ bảng băm
void DrawHashTable(const std::vector<HashNode> &hashTable, int foundIndex, bool found)
{
    for (int i = 0; i < hashTable.size(); ++i)
    {
        int x = (i % 23) * 80 + 50;
        int y = (i / 23) * 80 + 230;
        Color circleColor = hashTable[i].color;

        // Vẽ hình tròn đại diện cho ô băm
        for (double thickness = 0; thickness < 3; thickness += 0.5)
        {
            DrawCircleLines(x, y, 20 + thickness, circleColor);
        }

        // Hiển thị chỉ số của ô băm
        std::string text = std::to_string(i);
        int textWidth = MeasureText(text.c_str(), GetFont().baseSize);
        DrawTextEx(GetFont(), text.c_str(), {float(x - textWidth / 2), float(y - 50)}, GetFont().baseSize, 1, RED);

        // Nếu ô băm không trống, hiển thị giá trị và danh sách liên kết
        if (!hashTable[i].isEmpty)
        {
            std::string valueText = std::to_string(hashTable[i].value);
            int valueTextWidth = MeasureText(valueText.c_str(), GetFont().baseSize);
            DrawTextEx(GetFont(), valueText.c_str(), {float(x - valueTextWidth / 2), float(y - GetFont().baseSize / 2)}, GetFont().baseSize, 2, BLUE);
        }

        // Vẽ danh sách liên kết (chaining)
        HashNode *child = hashTable[i].next;
        int childX = x;      // Vị trí ngang của nút con
        int childY = y + 80; // Khoảng cách dọc giữa các nút

        while (child != nullptr)
        {
            // Vẽ đường nối giữa các nút
            drawArrowLine_hash({float(x), float(y + 20)}, {float(childX), float(childY - 20)}, 2.0f, BLACK);

            // Vẽ nút con
            for (double thickness = 0; thickness < 3; thickness += 0.5)
            {
                DrawCircleLines(childX, childY, 20 + thickness, child->color);
            }
            std::string childValueText = std::to_string(child->value);
            int childValueTextWidth = MeasureText(childValueText.c_str(), GetFont().baseSize);
            DrawTextEx(GetFont(), childValueText.c_str(), {float(childX - childValueTextWidth / 2), float(childY - GetFont().baseSize / 2)}, GetFont().baseSize, 2, BLUE);

            // Di chuyển đến nút tiếp theo
            y = childY;
            childY += 80;
            child = child->next;
        }
    }
}

// Hàm sao chép một bảng băm
std::vector<HashNode> CopyHashTable(const std::vector<HashNode> &originalHashTable)
{
    std::vector<HashNode> copiedHashTable(originalHashTable.size());
    for (size_t i = 0; i < originalHashTable.size(); ++i)
    {
        copiedHashTable[i] = originalHashTable[i]; // Sao chép từng ô
        if (originalHashTable[i].next != nullptr)
        {
            copiedHashTable[i].next = CopyLinkedList(originalHashTable[i].next); // Sao chép danh sách liên kết nếu có
        }
    }
    return copiedHashTable;
}

// Hàm sao chép danh sách liên kết (cho trường hợp chaining)
HashNode *CopyLinkedList(const HashNode *head)
{
    if (head == nullptr)
    {
        return nullptr;
    }
    HashNode *newHead = new HashNode(*head); // Sao chép nút đầu tiên
    HashNode *current = newHead;
    const HashNode *originalCurrent = head->next;

    while (originalCurrent != nullptr)
    {
        current->next = new HashNode(*originalCurrent); // Sao chép từng nút
        current = current->next;
        originalCurrent = originalCurrent->next;
    }
    return newHead;
}

// Hàm xử lý nhập liệu
void HandleInput_hash(Rectangle box, char *buffer, int &index, bool &isActive)
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
        SetMouseCursor(MOUSE_CURSOR_IBEAM);
        int key = GetCharPressed();
        while (key > 0)
        {
            if ((key >= 32) && (key <= 125) && (index < MAX_INPUT_CHARS))
            {
                buffer[index] = (char)key;
                buffer[index + 1] = '\0';
                index++;
            }
            key = GetCharPressed();
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

// Hàm kiểm tra nút bấm
bool CheckButton_hash(Rectangle button, const char *text)
{
    static Texture2D pauseTexture = LoadTexture("res/pause.png");            // Load texture once
    static Texture2D playTexture = LoadTexture("res/play.png");              // Load texture once
    static Texture2D backwardTexture = LoadTexture("res/previous step.png"); // Load texture once
    static Texture2D beginTexture = LoadTexture("res/go to begin.png");      // Load texture once
    static Texture2D replayTexture = LoadTexture("res/replay.png");          // Load texture once
    static Texture2D forwardTexture = LoadTexture("res/next step.png");      // Load texture once
    static Texture2D endTexture = LoadTexture("res/go to end.png");          // Load texture once

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
        DrawRectangleRec(button, WHITE);
    }

    // Draw the corresponding image based on the button text
    Texture2D *texture = nullptr;

    if (std::string(text) == "pause")
        texture = &pauseTexture;
    else if (std::string(text) == "play")
        texture = &playTexture;
    else if (std::string(text) == "previous step")
        texture = &backwardTexture;
    else if (std::string(text) == "go to begin")
        texture = &beginTexture;
    else if (std::string(text) == "replay")
        texture = &replayTexture;
    else if (std::string(text) == "next step")
        texture = &forwardTexture;
    else if (std::string(text) == "go to end")
        texture = &endTexture;
    else if (std::string(text) == "menu")
    {
        return false;
    }
    else if (std::string(text) == "PSEUDO CODE")
    {
        return false;
    }
    else
    {
        // Draw the text if no texture is found
        if (!mouseOnButton)
            DrawRectangleGradientH(button.x, button.y, button.width, button.height, SKYBLUE, DARKBLUE);
        DrawTextEx(GetFont(), text, {button.x + 10, button.y + 10}, GetFont().baseSize, 1, WHITE);
        return false; // No texture to draw, return false
    }
    if (texture != nullptr)
    {
        // Define the source rectangle (entire texture)
        Rectangle sourceRect = {0, 0, (float)texture->width, (float)texture->height};

        // Define the destination rectangle (scaled size and position)
        Rectangle destRect = {button.x, button.y, button.width, button.height};

        // Draw the resized texture
        DrawTexturePro(*texture, sourceRect, destRect, {0, 0}, 0.0f, WHITE);
    }

    return false;
}

// Hàm chính để render bảng băm
void RenderHashTable()
{

    DrawTextEx(GetFont(), "HASH TABLE", {float(GetScreenWidth() / 2 - 80), 20}, GetFont().baseSize * 1.5, 1, BLUE);
    static std::vector<HashNode> hashTable(67);          // Bảng băm động với kiểu HashNode
    static int tableSize = 67;                           // Kích thước bảng băm
    static char inputBuffer[MAX_INPUT_CHARS + 1] = {0};  // Bộ đệm nhập liệu
    static char searchBuffer[MAX_INPUT_CHARS + 1] = {0}; // Bộ đệm tìm kiếm
    static char removeBuffer[MAX_INPUT_CHARS + 1] = {0}; // Bộ đệm xóa
    static char sizeBuffer[MAX_INPUT_CHARS + 1] = {0};   // Bộ đệm thay đổi kích thước
    static int inputIndex = 0, searchIndex = 0, removeIndex = 0, sizeIndex = 0;
    static bool found = false;
    static int foundIndex = -1;
    static bool showNotFoundMessage = false;
    static HashingMode mode = LINEAR;
    static int framecounter = 0;
    static bool inputActive = false, searchActive = false, removeActive = false, sizeActive = false;
    static std::vector<HashTableState> hashTableHistory; // Lưu lịch sử bảng băm
    static int hashtablestep = 0;                        // Chỉ số lịch sử hiện tại
    Rectangle inputBox = {260, 600, 200, 50};
    Rectangle searchBox = {260, 650, 200, 50};
    Rectangle removeBox = {260, 700, 200, 50};
    Rectangle sizeBox = {260, 750, 200, 50};
    Rectangle linearButton = {50, 80, 215, 50};
    Rectangle quadraticButton = {285, 80, 215, 50};
    Rectangle doubleHashingButton = {520, 80, 215, 50};
    Rectangle chainingButton = {755, 80, 215, 50};

    // Seekbar variables
    static float sliderValue = 0.0f; // Normalized value (0.0 to 1.0)
    const int barWidth = 800;
    const int barHeight = 10;
    const int barX = 560;
    const int barY = 970;
    const int knobRadius = 10;
    static Rectangle Previous_query = {70, 950, 80, 50};
    static Rectangle Next_query = {450, 950, 80, 50};

    static Rectangle pause_box = {380, 950, 40, 40};
    static Rectangle next_step_box = {440, 950, 40, 40};
    static Rectangle prev_step_box = {320, 950, 40, 40};
    static Rectangle go_to_begin = {260, 950, 40, 40};
    static Rectangle go_to_end = {500, 950, 40, 40};
    static bool pause = false;

    static float speedSliderValue = 0.0f; // Giá trị thanh trượt (0.0 đến 1.0)
    static float animationSpeed = 1.0f;   // Tốc độ hoạt ảnh (1x đến 10x)
    const int speedBarWidth = 200;
    const int speedBarHeight = 10;
    const int speedBarX = 1500;
    const int speedBarY = 970;
    const int speedKnobRadius = 10;
    static Rectangle menu = {0, 600, 40, 300};
    static bool menuActive = false;

    static Rectangle inputButton = {50, 600, 200, 50};
    static Rectangle searchButton = {50, 650, 200, 50};
    static Rectangle removeButton = {50, 700, 200, 50};
    static Rectangle createButton = {50, 750, 200, 50};
    static Rectangle fileButton = {50, 800, 200, 50};
    static Rectangle RandomButton = {50, 850, 200, 50};
    static bool inputActiveButton = false;
    static bool searchActiveButton = false;
    static bool removeActiveButton = false;
    static bool createActiveButton = false;
    static bool fileActiveButton = false;
    static bool RandomActiveButton = false;
    static GuiWindowFileDialogState fileDialogState; // Trạng thái của file dialog
    static bool fileDialogInitialized = false;       // Để kiểm tra xem dialog đã được khở
    static Rectangle pseudoCodeBox = {1880, 600, 40, 300};
    static bool pseudoCodeActive = false;
    if (CheckButton_hash(pseudoCodeBox, "PSEUDO CODE"))
    {
        pseudoCodeActive = !pseudoCodeActive;
    }
    if (pseudoCodeActive)
    {
        DrawRectangleGradientV(pseudoCodeBox.x, pseudoCodeBox.y, pseudoCodeBox.width, pseudoCodeBox.height, SKYBLUE, DARKBLUE);
        static Texture2D codetexture = LoadTexture("res/right arrow.png");
        Texture2D *texture = &codetexture;
        Rectangle sourceRect = {0, 0, (float)texture->width, (float)texture->height};
        Rectangle destRect = {1880, 730, 40, 40};
        DrawTexturePro(*texture, sourceRect, destRect, {0, 0}, 0.0f, WHITE);
    }
    else
    {
        DrawRectangleGradientV(pseudoCodeBox.x, pseudoCodeBox.y, pseudoCodeBox.width, pseudoCodeBox.height, SKYBLUE, DARKBLUE);
        static Texture2D codetexture = LoadTexture("res/left arrow.png");
        Texture2D *texture = &codetexture;
        Rectangle sourceRect = {0, 0, (float)texture->width, (float)texture->height};
        Rectangle destRect = {1880, 730, 40, 40};
        DrawTexturePro(*texture, sourceRect, destRect, {0, 0}, 0.0f, WHITE);
    }
    if (!fileDialogInitialized)
    {
        fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
        fileDialogInitialized = true;
    }
    if (CheckButton_hash(menu, "menu"))
    {
        menuActive = !menuActive;
    }
    if (menuActive)
    {
        DrawRectangleGradientV(menu.x, menu.y, menu.width, menu.height, SKYBLUE, DARKBLUE);
        static Texture2D menutexture = LoadTexture("res/left arrow.png");
        Texture2D *texture = &menutexture;
        Rectangle sourceRect = {0, 0, (float)texture->width, (float)texture->height};
        Rectangle destRect = {0, 730, 40, 40};
        DrawTexturePro(*texture, sourceRect, destRect, {0, 0}, 0.0f, WHITE);
    }
    else
    {
        DrawRectangleGradientV(menu.x, menu.y, menu.width, menu.height, SKYBLUE, DARKBLUE);
        static Texture2D menutexture = LoadTexture("res/right arrow.png");
        Texture2D *texture = &menutexture;
        Rectangle sourceRect = {0, 0, (float)texture->width, (float)texture->height};
        Rectangle destRect = {0, 730, 40, 40};
        DrawTexturePro(*texture, sourceRect, destRect, {0, 0}, 0.0f, WHITE);
    }
    Vector2 mousePoint = GetMousePosition();
    if (!(hashtablestep < hashTableHistory.size() - 1) && CheckButton_trie(pause_box,
                                                                           "replay"))
    {
        pause = false;
        hashtablestep = 0;
    }
    else
    {
        if ((!pause && hashtablestep < hashTableHistory.size() - 1 && CheckButton_trie(pause_box, "pause")) ||
            (pause && hashtablestep < hashTableHistory.size() - 1 && CheckButton_trie(pause_box, "play")))
        {
            // Toggle pause state
            pause = !pause;
        }
    }
    if (CheckButton_trie(prev_step_box, "previous step"))
    {
        if (hashtablestep > 0)
        {
            hashtablestep = max(hashtablestep - 10, 0);
            pause = true; // Pause the animation when going to the previous step
        }
    }

    if (CheckButton_trie(next_step_box, "next step"))
    {
        if (hashtablestep < int(hashTableHistory.size()) - 1)
        {
            hashtablestep = min(hashtablestep + 10, int(hashTableHistory.size()) - 1);
            pause = true; // Pause the animation when going to the next step
        }
    }

    if (CheckButton_trie(go_to_begin, "go to begin") && hashtablestep > 0)
    {
        hashtablestep = 0;
    }
    if (CheckButton_trie(go_to_end, "go to end") && hashtablestep < int(hashTableHistory.size()) - 1)
    {
        hashtablestep = int(hashTableHistory.size()) - 1;
    }
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
    {
        if (mousePoint.x >= barX && mousePoint.x <= barX + barWidth &&
            mousePoint.y >= barY - 10 && mousePoint.y <= barY + barHeight + 10)
        {
            sliderValue = (float)(mousePoint.x - barX) / (float)barWidth;
            hashtablestep = (int)(sliderValue * (hashTableHistory.size() - 1));
        }
        if (mousePoint.x >= speedBarX && mousePoint.x <= speedBarX + speedBarWidth &&
            mousePoint.y >= speedBarY - 10 && mousePoint.y <= speedBarY + speedBarHeight + 10)
        {
            speedSliderValue = (float)(mousePoint.x - speedBarX) / (float)speedBarWidth;
            animationSpeed = 1.0f + speedSliderValue * 9.0f; // Tốc độ từ 1x đến 10x
        }
    }
    // Vẽ thanh speed bar
    DrawRectangle(speedBarX, speedBarY, speedBarWidth, speedBarHeight, LIGHTGRAY);
    DrawRectangleGradientH(speedBarX, speedBarY, (int)(speedBarWidth * speedSliderValue), speedBarHeight, SKYBLUE, DARKBLUE);
    DrawCircle(speedBarX + (int)(speedBarWidth * speedSliderValue), speedBarY + speedBarHeight / 2, speedKnobRadius, DARKBLUE);

    // Hiển thị tốc độ hiện tại bên cạnh thanh speed bar
    int currentSpeed = static_cast<int>(1 + speedSliderValue * 9); // Tốc độ từ 1x đến 10x
    std::string speedText = std::to_string(currentSpeed) + "x";
    DrawTextEx(GetFont(), speedText.c_str(), {float(speedBarX + speedBarWidth + 20), float(speedBarY) - 10}, GetFont().baseSize, 1, BLACK);

    sliderValue = (float)hashtablestep / (float)(hashTableHistory.size() - 1);
    DrawRectangle(barX, barY, barWidth, barHeight, LIGHTGRAY);
    DrawRectangleGradientH(barX, barY, (int)(barWidth * sliderValue), barHeight, SKYBLUE, DARKBLUE);
    DrawCircle(barX + (int)(sliderValue * barWidth), barY + barHeight / 2, knobRadius, DARKBLUE);
    // Chọn chế độ băm
    if (CheckButton_hash(linearButton, "Linear Probing"))
    {
        mode = LINEAR;
        std::fill(hashTable.begin(), hashTable.end(), HashNode());
    }
    if (CheckButton_hash(quadraticButton, "Quadratic Probing"))
    {
        mode = QUADRATIC;
        std::fill(hashTable.begin(), hashTable.end(), HashNode());
    }
    if (CheckButton_hash(doubleHashingButton, "Double Hashing"))
    {
        mode = DOUBLE_HASHING;
        std::fill(hashTable.begin(), hashTable.end(), HashNode());
    }
    if (CheckButton_hash(chainingButton, "Chaining"))
    {
        mode = CHAINING;
        std::fill(hashTable.begin(), hashTable.end(), HashNode());
    }
    if (menuActive)
    {
        if (CheckButton_hash(inputButton, "Insert"))
            inputActiveButton = !inputActiveButton;
        if (CheckButton_hash(removeButton, "Remove"))
            removeActiveButton = !removeActiveButton;
        if (CheckButton_hash(searchButton, "Search"))
            searchActiveButton = !searchActiveButton;
        if (CheckButton_hash(createButton, "Create"))
            createActiveButton = !createActiveButton;
        if (CheckButton_hash(fileButton, "File"))
        {
            fileDialogState.windowActive = !fileDialogState.windowActive; // Kích hoạt file dialog
        }
        if (fileDialogState.windowActive)
        {
            GuiWindowFileDialog(&fileDialogState);

            if (fileDialogState.SelectFilePressed)
            {
                // Xử lý file được chọn
                std::string selectedFilePath = std::string(fileDialogState.dirPathText) + "\\" + std::string(fileDialogState.fileNameText);
                std::cout << "Selected file: " << selectedFilePath << std::endl;

                std::ifstream inputFile(selectedFilePath);
                if (inputFile.is_open())
                {
                    std::string line;
                    hashTable.clear();                       // Xóa bảng băm hiện tại
                    hashTable.resize(tableSize, HashNode()); // Tạo bảng băm mới với kích thước đã chỉ định

                    while (std::getline(inputFile, line))
                    {
                        std::stringstream ss(line); // Sử dụng stringstream để tách các số trong dòng
                        int value;
                        while (ss >> value) // Đọc từng số nguyên từ dòng
                        {
                            inputHashTable(hashTable, hashTableHistory, value, value, mode); // Thêm giá trị vào bảng băm
                        }
                    }
                    inputFile.close();
                    hashTableHistory.clear();    // Xóa lịch sử bảng băm
                    hashtablestep = 0;           // Đặt lại chỉ số lịch sử
                    found = false;               // Đặt lại trạng thái tìm kiếm
                    foundIndex = -1;             // Đặt lại chỉ số tìm kiếm
                    showNotFoundMessage = false; // Đặt lại thông báo không tìm thấy
                }
                else
                {
                    std::cerr << "Failed to open file: " << selectedFilePath << std::endl;
                }

                fileDialogState.SelectFilePressed = false; // Reset trạng thái
            }
        }
        if (CheckButton_hash(RandomButton, "Random"))
        {
            hashTable.clear();     // Xóa bảng băm hiện tại
            resetcolor(hashTable); // Đặt lại màu sắc cho bảng băm
            hashTableHistory.clear();
            hashtablestep = 0;
            found = false;
            foundIndex = -1;
            showNotFoundMessage = false;

            // Random kích thước bảng băm mới
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> sizeDist(10, (mode != CHAINING) ? 100 : 23); // Kích thước từ 10 đến 100
            tableSize = sizeDist(gen);
            hashTable.resize(tableSize, HashNode());
            std::fill(hashTable.begin(), hashTable.end(), HashNode());

            // Random giá trị để thêm vào bảng băm
            std::uniform_int_distribution<> valueDist(1, 100);      // Giá trị từ 1 đến 1000
            int numValues = std::min(sizeDist(gen) / 2, tableSize); // Số lượng giá trị không vượt quá kích thước bảng
            if (numValues == tableSize)
                numValues /= 2;
            cout << tableSize << " " << numValues << endl;
            for (int i = 0; i < numValues; ++i)
            {
                int randomValue = valueDist(gen);
                inputHashTable(hashTable, hashTableHistory, randomValue, randomValue, mode);
            }
            hashTableHistory.clear(); // Xóa lịch sử bảng băm
            hashtablestep = 0;        // Đặt lại chỉ số lịch sử
        }
        if (inputActiveButton)
        {
            DrawBoxes_hash(inputBox, inputBuffer, framecounter, inputActive);
            HandleInput_hash(inputBox, inputBuffer, inputIndex, inputActive);
        }
        if (removeActiveButton)
        {
            DrawBoxes_hash(removeBox, removeBuffer, framecounter, removeActive);
            HandleInput_hash(removeBox, removeBuffer, removeIndex, removeActive);
        }
        if (searchActiveButton)
        {
            DrawBoxes_hash(searchBox, searchBuffer, framecounter, searchActive);
            HandleInput_hash(searchBox, searchBuffer, searchIndex, searchActive);
        }
        // Vẽ bảng băm
        // Xử lý nhập liệu
        if (createActiveButton)
        {
            DrawBoxes_hash(sizeBox, sizeBuffer, framecounter, sizeActive);
            HandleInput_hash(sizeBox, sizeBuffer, sizeIndex, sizeActive);
        }

        // Thay đổi kích thước bảng băm
        if (IsKeyPressed(KEY_ENTER) && sizeActive && createActiveButton)
        {
            int newSize = std::stoi(sizeBuffer);
            if (newSize > 0)
            {
                tableSize = newSize;
                hashTable.resize(tableSize, HashNode());
                std::fill(hashTable.begin(), hashTable.end(), HashNode());
            }
            sizeIndex = 0;
            sizeBuffer[0] = '\0';
        }
        if (IsKeyPressed(KEY_ENTER))
        {
            // Thêm giá trị vào bảng băm
            if (inputActive && inputActiveButton)
            {
                resetcolor(hashTable); // Đặt lại màu sắc cho bảng băm
                hashTableHistory.clear();
                hashtablestep = 0;
                int inputNumber = std::stoi(inputBuffer);
                int hashIndex = inputNumber % tableSize;
                inputHashTable(hashTable, hashTableHistory, inputNumber, inputNumber, mode); // Thêm giá trị vào bảng băm
                found = true;
                foundIndex = hashIndex;
                showNotFoundMessage = false;
                inputIndex = 0;
                inputBuffer[0] = '\0';
            }

            // Tìm kiếm giá trị trong bảng băm
            else if (searchActive && searchActiveButton)
            {
                resetcolor(hashTable); // Đặt lại màu sắc cho bảng băm
                hashTableHistory.clear();
                hashtablestep = 0;
                int searchNumber = std::stoi(searchBuffer);
                found = searchHashTable(hashTable, hashTableHistory, searchNumber, mode, foundIndex); // Tìm kiếm giá trị trong bảng băm
                showNotFoundMessage = !found;
                searchIndex = 0;
                searchBuffer[0] = '\0';
            }

            // Xóa giá trị khỏi bảng băm
            else if (removeActive && removeActiveButton)
            {
                resetcolor(hashTable); // Đặt lại màu sắc cho bảng băm
                hashTableHistory.clear();
                hashtablestep = 0;
                int removeNumber = std::stoi(removeBuffer);
                found = removeHashTable(hashTable, hashTableHistory, removeNumber, mode); // Xóa giá trị khỏi bảng băm
                showNotFoundMessage = !found;
                removeIndex = 0;
                removeBuffer[0] = '\0';
            }
        }
    }
    // Hiển thị chế độ hiện tại
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
    case CHAINING:
        modeText = "Current Mode: Chaining";
        break;
    }
    // DrawText(modeText, 50, 100, 20, RED);
    DrawTextEx(GetFont(), modeText, {50, 140}, GetFont().baseSize, 1, RED);
    // Thêm, tìm kiếm, hoặc xóa giá trị

    if (hashtablestep >= 0 && hashtablestep < hashTableHistory.size())
    {
        const auto &currentState = hashTableHistory[hashtablestep];
        DrawHashTable(currentState.hashTable, foundIndex, found); // Vẽ bảng băm
        if (pseudoCodeActive)
            DrawPseudoCode(mode, currentState.operation, currentState.highlightedLine); // Vẽ pseudocode
        if (!pause)
            hashtablestep += animationSpeed; // Tăng chỉ số lịch sử theo tốc độ
    }
    else
    {
        DrawHashTable(hashTable, foundIndex, found); // Vẽ bảng băm
    }

    framecounter++;
}
void DrawBoxes_hash(const Rectangle &Box, const char *Buffer, const int &framesCounter, const bool &isActive)
{
    DrawRectangleRec(Box, LIGHTGRAY);
    // DrawText(Buffer, Box.x + 10, Box.y + 15, 20, BLACK);
    DrawTextEx(GetFont(), Buffer, {Box.x + GetFont().baseSize / 2, Box.y + 15}, GetFont().baseSize, 1, BLACK);
    int MAX_INPUT_CHARS = 10;
    if (((framesCounter / 20) % 2) == 0)
    {
        if (isActive && strlen(Buffer) < MAX_INPUT_CHARS)
            DrawText("_", Box.x + GetFont().baseSize / 2 + MeasureText(Buffer, GetFont().baseSize), Box.y + 15, GetFont().baseSize, BLACK);
    }
}

void DrawPseudoCode(HashingMode mode, const std::string &operation, int highlightedLine)
{
    std::vector<std::string> pseudoCode;

    // Xác định pseudocode dựa trên thao tác và chế độ băm
    if (operation == "Insert")
    {
        if (mode == LINEAR)
        {
            pseudoCode = {
                "Insert_LinearProbing: ",
                "index = key % size; ",
                "while not empty: ",
                "   index = (index + 1) % size; ",
                "set value"};
        }
        else if (mode == QUADRATIC)
        {
            pseudoCode = {
                "Insert_QuadraticProbing: ",
                "index = key % size; ",
                "i = 0; ",
                "while not empty: ",
                "   i++; ",
                "   index = (index + i^2) % size; ",
                "set value"};
        }
        else if (mode == DOUBLE_HASHING)
        {
            pseudoCode = {
                "Insert_DoubleHashing: ",
                "index = key % size; ",
                "step = secondaryHash(key, size); ",
                "while not empty: ",
                "   index = (index + step) % size; ",
                "set value"};
        }
        else if (mode == CHAINING)
        {
            pseudoCode = {
                "Insert_Chaining: ",
                "index = key % size; ",
                "if empty: ",
                "   create list; ",
                "append value"};
        }
    }
    else if (operation == "Search")
    {
        if (mode == LINEAR)
        {
            pseudoCode = {
                "Search_LinearProbing: ",
                "index = key % size; ",
                "while not empty: ",
                "   if match: ",
                "       return index; ",
                "   index = (index + 1) % size; ",
                "return -1"};
        }
        else if (mode == QUADRATIC)
        {
            pseudoCode = {
                "Search_QuadraticProbing: ",
                "index = key % size; ",
                "i = 0; ",
                "while not empty: ",
                "   if match: ",
                "       return index; ",
                "   i++; ",
                "   index = (index + i^2) % size; ",
                "return -1"};
        }
        else if (mode == DOUBLE_HASHING)
        {
            pseudoCode = {
                "Search_DoubleHashing: ",
                "index = key % size; ",
                "step = secondaryHash(key, size); ",
                "while not empty: ",
                "   if match: ",
                "       return index; ",
                "   index = (index + step) % size; ",
                "return -1"};
        }
        else if (mode == CHAINING)
        {
            pseudoCode = {
                "Search_Chaining: ",
                "index = key % size; ",
                "if not empty: ",
                "   for node in list: ",
                "       if match: ",
                "       return index; ",
                "return -1"};
        }
    }
    else if (operation == "Remove")
    {
        if (mode == LINEAR)
        {
            pseudoCode = {
                "Remove_LinearProbing: ",
                "index = key % size; ",
                "while not empty: ",
                "   if match: ",
                "       set empty; ",
                "       return true; ",
                "   index = (index + 1) % size; ",
                "return false"};
        }
        else if (mode == QUADRATIC)
        {
            pseudoCode = {
                "Remove_QuadraticProbing: ",
                "index = key % size; ",
                "i = 0; ",
                "while not empty: ",
                "   if match: ",
                "       set empty; ",
                "       return true; ",
                "   i++; ",
                "   index = (index + i^2) % size; ",
                "return false"};
        }
        else if (mode == DOUBLE_HASHING)
        {
            pseudoCode = {
                "Remove_DoubleHashing: ",
                "index = key % size; ",
                "step = secondaryHash(key, size); ",
                "while not empty: ",
                "   if match: ",
                "       set empty; ",
                "       return true; ",
                "   index = (index + step) % size; ",
                "return false"};
        }
        else if (mode == CHAINING)
        {
            pseudoCode = {
                "Remove_Chaining: ",
                "index = key % size; ",
                "if not empty: ",
                "for node in list: ",
                "   if match: ",
                "       remove node; ",
                "       return true; ",
                "return false"};
        }
    }
    DrawRectangle(1320, 600, 550, 300, LIGHTGRAY); // Vẽ nền cho hộp thoại
    // Vẽ pseudocode trên màn hình
    int x = 1320, y = 600;                   // Vị trí bắt đầu vẽ pseudocode
    int lineHeight = GetFont().baseSize + 2; // Chiều cao mỗi dòng
    for (size_t i = 0; i < pseudoCode.size(); ++i)
    {
        Color textColor = (i + 1 == highlightedLine) ? RED : BLACK; // Highlight dòng hiện tại
        DrawTextEx(GetFont(), pseudoCode[i].c_str(), {float(x), float(y + i * lineHeight)}, GetFont().baseSize, 1, textColor);
    }
}