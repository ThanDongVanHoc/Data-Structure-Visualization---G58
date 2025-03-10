// renderer.cpp
#include "renderer.h"

Renderer::Renderer() {}

void Renderer::renderLinkedList(LinkedList* list) {
    //BeginDrawing();
    //ClearBackground(RAYWHITE);
    Node* temp = list->head;
    int x = 100;
    int y = 200;
    while (temp) {
        DrawRectangle(x, y, 50, 50, BLUE);
        DrawText(std::to_string(temp->data).c_str(), x + 15, y + 15, 20, WHITE);
        if (temp->next) {
            DrawLine(x + 50, y + 25, x + 100, y + 25, BLACK);
        }
        x += 100;
        temp = temp->next;
    }
    //EndDrawing();
}

void Renderer::renderAVL() {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawText("AVL Tree Visualization", 300, 250, 20, BLACK);
    EndDrawing();
}
