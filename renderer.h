// renderer.h
#ifndef RENDERER_H
#define RENDERER_H

#include "linked_list.h"
#include "trie.h"

class Renderer
{
public:
    Renderer();
    void renderLinkedList(LinkedList *list);
    void renderAVL();
    void RenderHashTable();
    void RenderTrie();
    // void DrawTrie(Trie::Node *node, int screenWidth, int screenHeight);
private:
};
void DrawTrie(Trie::Node *node, int screenWidth, int screenHeight);
#endif // RENDERER_H