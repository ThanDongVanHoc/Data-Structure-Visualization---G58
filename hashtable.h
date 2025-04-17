#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <iostream>
#include <vector>
#include <list>
#include <string>
#include "raylib.h"
struct HashNode
{
    int value;
    bool isEmpty;
    Color color;
    HashNode *next; // Con trỏ đến nút tiếp theo trong danh sách liên kết (nếu có)
    HashNode() : value(0), isEmpty(true), color(BLACK), next(nullptr) {}
};
void RenderHashTable();
void HandleInput_hash(Rectangle box, char *buffer, int &index, bool &isActive);
bool CheckButton_hash(Rectangle button, const char *text);
std::vector<HashNode> CopyHashTable(const std::vector<HashNode> &originalHashTable);
HashNode *CopyLinkedList(const HashNode *head);
void DrawBoxes_hash(const Rectangle &Box, const char *Buffer, const int &framesCounter, const bool &isActive);
#endif // HASHTABLE_H