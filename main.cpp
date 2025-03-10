// main.cpp
#include "linked_list.h"
#include "renderer.h"
#include "raylib.h"
#include <string>

int main() {
    InitWindow(800, 600, "Data Structure Visualization");
    SetTargetFPS(60);

    LinkedList list;
    Renderer renderer;
    
    std::string inputText = ""; // Lưu số nhập vào
    int frameCount = 0;
    Rectangle insertButton = {320, 45, 80, 30}; // Vị trí và kích thước nút Insert

    while (!WindowShouldClose()) {
        frameCount++;
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        // // Bắt sự kiện nhập số
        // int key = GetCharPressed();
        // while (key > 0) {
        //     if (key >= '0' && key <= '9') {
        //         inputText += (char)key; // Thêm số vào chuỗi
        //     }
        //     key = GetCharPressed();
        // }

        // if (IsKeyPressed(KEY_BACKSPACE) && !inputText.empty()) {
        //     inputText.pop_back();
        // }

        // // Kiểm tra nếu nhấn Enter hoặc bấm nút Insert
        // bool insertPressed = IsKeyPressed(KEY_ENTER) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), insertButton));
        // if (insertPressed && !inputText.empty()) {
        //     int value = std::stoi(inputText);
        //     list.insert(value);
        //     inputText = ""; // Reset chuỗi nhập
        // }
        list.insert(2);
        list.insert(3);
        list.insert(4);
        list.insert(5);
        list.insert(1);
        renderer.renderLinkedList(&list);
        // DrawText("Enter number: ", 50, 50, 20, BLACK);
        // DrawRectangle(200, 45, 100, 30, LIGHTGRAY);
        // DrawText(inputText.c_str(), 210, 50, 20, BLACK);
        
        // // Hiển thị con trỏ nhấp nháy
        // if ((frameCount / 30) % 2 == 0) {
        //     DrawText("|", 200 + 10 * inputText.length(), 50, 20, BLACK);
        // }

        // Vẽ nút Insert
        DrawRectangleRec(insertButton, DARKGRAY);
        DrawText("Insert", 335, 52, 20, WHITE);
        
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
