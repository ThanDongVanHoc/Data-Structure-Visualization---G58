#include "trie.h"
#include <algorithm>
#include <cmath>
#include "raylib.h" // Include raylib for drawing functions
#include <thread>   // Include thread for sleep
#include "renderer.h"
using namespace std;
Trie::Trie() : cur(0)
{
    root = new Node();
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
            c = f - 'A' + 26; // Adjust index for uppercase letters

        if (p->child[c] == NULL)
        {
            p->child[c] = new Node();
            // Recompute positions after adding a new node
            p = p->child[c];
            p->character = f;
            p->cnt++;
            float currentX = 40;
            float horizontalSpacing = 80;
            float verticalSpacing = 80;
            compute_positions(root, 1, currentX, horizontalSpacing, verticalSpacing);
            float stiffness = 3.5f; // Tăng độ cứng: chuyển từ 0.2f sang 1.0f để animation nhanh hơn
            float damping = 0.75f;  // Điều chỉnh damping: có thể thử với 0.9f để ổn định chuyển động
            double previousTime = GetTime();
            int step = 0;
            while (!check(root) && step <= 50)
            {
                double dt = 0.1;
                updateSpringAnimation(root, stiffness, damping, dt, 1.0f);
                DrawTrie(root, GetScreenWidth(), GetScreenHeight());
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
    }
    p->exist++;
}

bool Trie::delete_string_recursive(Node *p, std::string &s, int i)
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

    if (p != root)
    {
        p->cnt--;
        if (p->cnt == 0)
        {
            delete (p);
            return true;
        }
    }
    return false;
}

void Trie::delete_string(std::string s)
{
    if (!find_string(s))
        return;
    delete_string_recursive(root, s, 0);
}

bool Trie::find_string(std::string s)
{
    Node *p = root;
    for (auto f : s)
    {
        int c;
        if (f >= 'a' && f <= 'z')
            c = f - 'a';
        else if (f >= 'A' && f <= 'Z')
            c = f - 'A' + 26; // Điều chỉnh chỉ số cho chữ cái in hoa

        if (p->child[c] == NULL)
            return false;
        p = p->child[c];
    }
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