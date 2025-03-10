// renderer.h
#ifndef RENDERER_H
#define RENDERER_H

#include "linked_list.h"
#include "raylib.h"
#include <vector>

struct Renderer {
    Renderer();
    // Linked_list
    void renderLinkedList(LinkedList* list);
    void renderAVL(); // Placeholder for AVL tree rendering
};

#endif // RENDERER_H