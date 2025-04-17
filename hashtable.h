#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <iostream>
#include <vector>
#include <list>
#include <string>
#include "raylib.h"
enum HashingMode
{
    LINEAR,
    QUADRATIC,
    DOUBLE_HASHING,
    CHAINING
};
struct HashNode
{
    int value;
    bool isEmpty;
    Color color;
    HashNode *next; // Con trỏ đến nút tiếp theo trong danh sách liên kết (nếu có)
    HashNode() : value(0), isEmpty(true), color(BLACK), next(nullptr) {}
};
struct HashTableState
{
    std::vector<HashNode> hashTable; // Trạng thái bảng băm
    std::string operation;           // Tên thao tác: "Insert", "Search", "Remove"
    int highlightedLine;             // Dòng pseudocode được highlight
};
void RenderHashTable();
void HandleInput_hash(Rectangle box, char *buffer, int &index, bool &isActive);
bool CheckButton_hash(Rectangle button, const char *text);
std::vector<HashNode> CopyHashTable(const std::vector<HashNode> &originalHashTable);
HashNode *CopyLinkedList(const HashNode *head);
void DrawBoxes_hash(const Rectangle &Box, const char *Buffer, const int &framesCounter, const bool &isActive);
void DrawPseudoCode(HashingMode mode, const std::string &operation, int highlightedLine);
#endif // HASHTABLE_H