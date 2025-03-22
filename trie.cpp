#include "trie.h"
#include <algorithm>
#include <cmath>
#include "raylib.h" // Include raylib for drawing functions
#include <thread>   // Include thread for sleep
#include "renderer.h"
#include <fstream> // Include fstream for file operations
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
        if (i == currentStep)
        {
            DrawText(pseudocode[i].c_str(), rectX + 10, rectY + 10 + i * 20, 20, RED);
        }
        else
        {
            DrawText(pseudocode[i].c_str(), rectX + 10, rectY + 10 + i * 20, 20, BLACK);
        }
    }
}
bool Trie::check(Node *node)
{
    if (node == NULL)
        return true;
    if (node->targetX != node->x || node->targetY != node->y)
        return false;
    bool res = true;
    for (int i = 0; i < 52; i++) // Thay đổi kích thước mảng
    {
        if (node->child[i] != NULL)
            res = min(res, check(node->child[i]));
    }
    return res;
}
void Trie::add_string(std::string s)
{
    Node *p = root;
    p->cnt++;
    DrawTrie(root, GetScreenWidth(), GetScreenHeight());
    DrawPseudocode(insertPseudocode, 1, GetScreenWidth(), GetScreenHeight());
    EndDrawing();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    BeginDrawing();
    ClearBackground(RAYWHITE);
    for (auto f : s)
    {
        p->color = YELLOW;
        DrawTrie(root, GetScreenWidth(), GetScreenHeight());
        DrawPseudocode(insertPseudocode, 2, GetScreenWidth(), GetScreenHeight());
        EndDrawing();
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        BeginDrawing();
        ClearBackground(RAYWHITE);
        p->color = BLACK;
        int c;
        if (f >= 'a' && f <= 'z')
            c = f - 'a';
        else if (f >= 'A' && f <= 'Z')
            c = f - 'A' + 26; // Adjust index for uppercase letters

        if (p->child[c] == NULL)
        {
            DrawTrie(root, GetScreenWidth(), GetScreenHeight());
            DrawPseudocode(insertPseudocode, 3, GetScreenWidth(), GetScreenHeight());
            EndDrawing();
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            BeginDrawing();
            ClearBackground(RAYWHITE);
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
            while (!check(root) && step <= 50)
            {
                double dt = 0.1;
                updateSpringAnimation(root, stiffness, damping, dt, 1.0f);
                DrawTrie(root, GetScreenWidth(), GetScreenHeight());
                DrawPseudocode(insertPseudocode, 4, GetScreenWidth(), GetScreenHeight());
                EndDrawing();
                BeginDrawing();
                ClearBackground(RAYWHITE);
                step++;
            }
        }
        else
        {
            p = p->child[c];
            p->character = f;
            p->cnt++;
        }
        DrawTrie(root, GetScreenWidth(), GetScreenHeight());
        DrawPseudocode(insertPseudocode, 5, GetScreenWidth(), GetScreenHeight());
        EndDrawing();
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        BeginDrawing();
        ClearBackground(RAYWHITE);
    }
    p->color = YELLOW;
    DrawTrie(root, GetScreenWidth(), GetScreenHeight());
    DrawPseudocode(insertPseudocode, 7, GetScreenWidth(), GetScreenHeight());
    EndDrawing();
    std::this_thread::sleep_for(std::chrono::milliseconds(300)); // Delay for animation
    BeginDrawing();
    ClearBackground(RAYWHITE);
    p->color = BLACK;
    p->exist++;
}

bool Trie::delete_string_recursive(Node *&p, std::string &s, int i)
{

    if (i != (int)s.size())
    {
        int c;
        if (s[i] >= 'a' && s[i] <= 'z')
            c = s[i] - 'a';
        else if (s[i] >= 'A' && s[i] <= 'Z')
            c = s[i] - 'A' + 26; // Điều chỉnh chỉ số cho chữ cái in hoa

        bool isChildDeleted = delete_string_recursive(p->child[c], s, i + 1);
        if (isChildDeleted)
            p->child[c] = NULL;
    }
    else
    {
        p->exist--;
    }
    p->color = YELLOW;
    DrawTrie(root, GetScreenWidth(), GetScreenHeight());
    EndDrawing();
    std::this_thread::sleep_for(std::chrono::milliseconds(300)); // Delay for animation
    BeginDrawing();
    ClearBackground(RAYWHITE);
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
            float stiffness = 3.5f; // Tăng độ cứng: chuyển từ 0.2f sang 1.0f để animation nhanh hơn
            float damping = 0.75f;  // Điều chỉnh damping: có thể thử với 0.9f để ổn định chuyển động
            int step = 0;
            while (step <= 50)
            {
                double dt = 0.1;
                updateSpringAnimation(root, stiffness, damping, dt, 1.0f);
                DrawTrie(root, GetScreenWidth(), GetScreenHeight());
                EndDrawing();
                BeginDrawing();
                ClearBackground(RAYWHITE);
                step++;
            }
            return true;
        }
    }
    else
        p->cnt--;
    p->color = BLACK;
    return false;
}

void Trie::delete_string(std::string s)
{
    if (!find_string(s))
        return;
    delete_string_recursive(root, s, 0);
    return;
}
bool Trie::find_string_red(std::string s)
{
    Node *p = root;
    for (auto f : s)
    {
        p->color = RED;
        DrawTrie(root, GetScreenWidth(), GetScreenHeight());
        EndDrawing();
        std::this_thread::sleep_for(std::chrono::milliseconds(300)); // Delay for animation
        BeginDrawing();
        ClearBackground(RAYWHITE);
        int c;
        if (f >= 'a' && f <= 'z')
            c = f - 'a';
        else if (f >= 'A' && f <= 'Z')
            c = f - 'A' + 26; // Điều chỉnh chỉ số cho chữ cái in hoa

        if (p->child[c] == NULL)
            return false;
        p = p->child[c];
    }
    p->color = RED;
    DrawTrie(root, GetScreenWidth(), GetScreenHeight());
    EndDrawing();
    std::this_thread::sleep_for(std::chrono::milliseconds(300)); // Delay for animation
    BeginDrawing();
    ClearBackground(RAYWHITE);
    return (p->exist != 0);
}
bool Trie::find_string(std::string s)
{
    Node *p = root;
    for (auto f : s)
    {
        p->color = YELLOW;
        DrawTrie(root, GetScreenWidth(), GetScreenHeight());
        EndDrawing();
        std::this_thread::sleep_for(std::chrono::milliseconds(300)); // Delay for animation
        BeginDrawing();
        ClearBackground(RAYWHITE);
        p->color = BLACK;
        int c;
        if (f >= 'a' && f <= 'z')
            c = f - 'a';
        else if (f >= 'A' && f <= 'Z')
            c = f - 'A' + 26; // Điều chỉnh chỉ số cho chữ cái in hoa

        if (p->child[c] == NULL)
            return false;
        p = p->child[c];
    }
    p->color = YELLOW;
    DrawTrie(root, GetScreenWidth(), GetScreenHeight());
    EndDrawing();
    std::this_thread::sleep_for(std::chrono::milliseconds(300)); // Delay for animation
    BeginDrawing();
    ClearBackground(RAYWHITE);
    p->color = BLACK;
    return (p->exist != 0);
}
void Trie::updateSpringAnimation(Node *node, float stiffness, float damping, float dt, float speedMultiplier)
{
    dt *= speedMultiplier; // Tăng tốc bằng cách nhân dt với hệ số tốc độ

    // Tính gia tốc dựa trên hiệu số giữa target và vị trí hiện tại
    float ax = (node->targetX - node->x) * stiffness;
    float ay = (node->targetY - node->y) * stiffness;

    // Cập nhật vận tốc và áp dụng damping
    node->velocityX = (node->velocityX + ax * dt) * damping;
    node->velocityY = (node->velocityY + ay * dt) * damping;

    // Cập nhật vị trí hiện tại
    node->x += node->velocityX * dt;
    node->y += node->velocityY * dt;

    // Cập nhật cho các node con
    for (int i = 0; i < 52; i++)
    {
        if (node->child[i] != NULL)
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

void Trie::visualize(Node *node, int depth, int &x, std::vector<std::vector<Node *>> &levels)
{
    if (node == nullptr)
        return;

    float currentX = 40.0f;
    float horizontalSpacing = 80.0f;
    float verticalSpacing = 80.0f;
    compute_positions(node, 0, currentX, horizontalSpacing, verticalSpacing);

    if (levels.size() <= depth)
        levels.push_back(std::vector<Node *>());

    for (int i = 0; i < 52; ++i) // Thay đổi kích thước mảng
    {
        if (node->child[i] != nullptr)
        {
            visualize(node->child[i], depth + 1, x, levels);
        }
    }

    node->x = x++;
    node->y = depth;
    levels[depth].push_back(node);
}

bool is_valid_input(const std::string &s)
{
    for (char c : s)
    {
        if (!isalpha(c))
        {
            return false;
        }
    }
    return true;
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
        {
            add_string(line);
        }
    }

    file.close();
}
