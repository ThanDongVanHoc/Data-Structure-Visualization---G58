#include "trie.h"
#include <algorithm>
#include <cmath>
#include "raylib.h"
#include <thread>
#include "renderer.h"
#include <fstream>
using namespace std;

Trie::Trie() : cur(0)
{
    root = new Node();
}

std::vector<std::string> insertPseudocode = {
    "function insert(root, word):",
    "    node = root",
    "    for character in word:",
    "        if node.child[character] is NULL:",
    "            node.child[character] = new Node()",
    "        node = node.child[character]",
    "        node.count += 1",
    "    node.isEndOfWord = true"};

std::vector<std::string> deletePseudocode = {
    "function delete(root, word, depth):",
    "    if root is NULL:",
    "        return NULL",
    "    if depth == length of word:",
    "        if root.isEndOfWord:",
    "            root.isEndOfWord = false",
    "        if isEmpty(root):",
    "            delete root",
    "            root = NULL",
    "        return root",
    "    index = word[depth]",
    "    root.child[index] = delete(root.child[index], word, depth + 1)",
    "    if isEmpty(root) and root.isEndOfWord is false:",
    "        delete root",
    "        root = NULL",
    "    return root"};

std::vector<std::string> searchPseudocode = {
    "function search(root, word):",
    "    node = root",
    "    for character in word:",
    "        if node.child[character] is NULL:",
    "            return false",
    "        node = node.child[character]",
    "    return node is not NULL and node.isEndOfWord"};

void DrawPseudocode(const std::vector<std::string> &pseudocode, int currentStep, int screenWidth, int screenHeight)
{
    int rectWidth = 470;
    int rectHeight = pseudocode.size() * 20 + 20;
    int rectX = screenWidth - rectWidth - 10;
    int rectY = screenHeight - rectHeight - 10;

    DrawRectangle(rectX, rectY, rectWidth, rectHeight, LIGHTGRAY);
    DrawRectangleLines(rectX, rectY, rectWidth, rectHeight, BLACK);

    for (int i = 0; i < int(pseudocode.size()); ++i)
    {
        DrawText(pseudocode[i].c_str(), rectX + 10, rectY + 10 + i * 20, 20, (i == currentStep) ? RED : BLACK);
    }
}

bool Trie::check(Node *node)
{
    if (!node)
        return true;
    if (node->targetX != node->x || node->targetY != node->y)
        return false;
    for (int i = 0; i < 52; i++)
    {
        if (node->child[i] && !check(node->child[i]))
            return false;
    }
    return true;
}

void Trie::animateNode(Node *node, int step, const std::vector<std::string> &pseudocode, bool delay)
{
    DrawTrie(root, GetScreenWidth(), GetScreenHeight());
    DrawPseudocode(pseudocode, step, GetScreenWidth(), GetScreenHeight());
    EndDrawing();
    if (delay)
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    BeginDrawing();
    ClearBackground(RAYWHITE);
}

void Trie::add_string(std::string s)
{
    Node *p = root;
    p->cnt++;
    animateNode(root, 1, insertPseudocode, 1);
    for (auto f : s)
    {
        p->color = YELLOW;
        animateNode(root, 2, insertPseudocode, 1);
        p->color = BLACK;
        int c = (f >= 'a' && f <= 'z') ? f - 'a' : f - 'A' + 26;
        if (!p->child[c])
        {
            animateNode(root, 3, insertPseudocode, 1);
            p->child[c] = new Node();
            p = p->child[c];
            p->character = f;
            p->cnt++;
            float currentX = 40;
            float horizontalSpacing = 80;
            float verticalSpacing = 80;
            compute_positions(root, 1, currentX, horizontalSpacing, verticalSpacing);
            float stiffness = 3.5f;
            float damping = 0.75f;
            int step = 0;
            while (step <= 50)
            {
                double dt = 0.1;
                updateSpringAnimation(root, stiffness, damping, dt, 1.0f);
                animateNode(root, 4, insertPseudocode, 0);
                step++;
            }
        }
        else
        {
            p = p->child[c];
            p->character = f;
            p->cnt++;
        }
        animateNode(root, 5, insertPseudocode, 1);
    }
    p->color = YELLOW;
    animateNode(root, 7, insertPseudocode, 1);
    p->color = BLACK;
    p->exist++;
}

bool Trie::delete_string_recursive(Node *&p, std::string &s, int i)
{
    if (i != (int)s.size())
    {
        int c = (s[i] >= 'a' && s[i] <= 'z') ? s[i] - 'a' : s[i] - 'A' + 26;
        bool isChildDeleted = delete_string_recursive(p->child[c], s, i + 1);
        if (isChildDeleted)
            p->child[c] = NULL;
    }
    else
    {
        p->exist--;
    }
    p->color = YELLOW;
    if (p != root)
    {
        p->cnt--;
        if (p->cnt == 0)
        {
            delete p;
            p = nullptr;
            float currentX = 40;
            float horizontalSpacing = 80;
            float verticalSpacing = 80;
            compute_positions(root, 1, currentX, horizontalSpacing, verticalSpacing);
            float stiffness = 3.5f;
            float damping = 0.75f;
            int step = 0;
            while (step <= 50)
            {
                updateSpringAnimation(root, stiffness, damping, 0.1, 1.0f);
                step++;
            }
            return true;
        }
    }
    else
    {
        p->cnt--;
    }
    p->color = BLACK;
    return false;
}

void Trie::delete_string(std::string s)
{
    if (!find_string(s))
        return;
    delete_string_recursive(root, s, 0);
}

bool Trie::find_string_red(std::string s)
{
    Node *p = root;
    for (auto f : s)
    {
        p->color = RED;
        int c = (f >= 'a' && f <= 'z') ? f - 'a' : f - 'A' + 26;
        if (!p->child[c])
            return false;
        p = p->child[c];
    }
    p->color = RED;
    return (p->exist != 0);
}

bool Trie::find_string(std::string s)
{
    Node *p = root;
    for (auto f : s)
    {
        p->color = YELLOW;
        p->color = BLACK;
        int c = (f >= 'a' && f <= 'z') ? f - 'a' : f - 'A' + 26;
        if (!p->child[c])
            return false;
        p = p->child[c];
    }
    p->color = YELLOW;
    p->color = BLACK;
    return (p->exist != 0);
}

void Trie::updateSpringAnimation(Node *node, float stiffness, float damping, float dt, float speedMultiplier)
{
    dt *= speedMultiplier;
    float ax = (node->targetX - node->x) * stiffness;
    float ay = (node->targetY - node->y) * stiffness;
    node->velocityX = (node->velocityX + ax * dt) * damping;
    node->velocityY = (node->velocityY + ay * dt) * damping;
    node->x += node->velocityX * dt;
    node->y += node->velocityY * dt;
    for (int i = 0; i < 52; i++)
    {
        if (node->child[i])
            updateSpringAnimation(node->child[i], stiffness, damping, dt, speedMultiplier);
    }
}

void Trie::compute_positions(Node *node, int depth, float &currentX, float horizontalSpacing, float verticalSpacing)
{
    if (!node)
        return;
    node->targetY = depth * verticalSpacing;

    int numChildren = 0;
    for (int i = 0; i < 52; ++i) // Thay đổi kích thước mảng
    {
        if (node->child[i] != nullptr)
        {
            compute_positions(node->child[i], depth + 1, currentX, horizontalSpacing, verticalSpacing);
            numChildren++;
        }
    }

    if (numChildren == 0)
    {
        node->targetX = currentX;
        currentX += horizontalSpacing;
    }
    else
    {
        int start = -1;
        int finish = -1;
        for (int i = 0; i < 52; i++) // Thay đổi kích thước mảng
        {
            if (node->child[i])
            {
                start = i;
                break;
            }
        }
        for (int i = 51; i >= 0; i--) // Thay đổi kích thước mảng
        {
            if (node->child[i])
            {
                finish = i;
                break;
            }
        }
        if (start != -1 && finish != -1)
            node->targetX = (node->child[start]->targetX + node->child[finish]->targetX) / 2.0f;
    }
}

bool is_valid_input(const std::string &s)
{
    return std::all_of(s.begin(), s.end(), ::isalpha);
}

void Trie::load_from_file(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }
    std::string line;
    while (std::getline(file, line))
    {
        if (is_valid_input(line))
            add_string(line);
    }
    file.close();
}
