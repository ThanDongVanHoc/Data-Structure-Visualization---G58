#ifndef TRIE_H
#define TRIE_H

#include <string>
#include <vector>
#include <raylib.h>
struct Trie
{
    struct Node
    {
        Node *child[52]; // Thay đổi kích thước mảng
        int exist, cnt;
        float x, y; // Coordinates for visualization
        Color color;
        std::string character;
        float targetX, targetY;     // Vị trí mới được tính từ computePositions
        float velocityX, velocityY; // Vận tốc dùng cho spring animation
        Node()
        {
            for (int i = 0; i < 52; i++) // Thay đổi kích thước mảng
                child[i] = NULL;
            exist = cnt = 0;
            x = y = 0;
            character = " ";
            color = BLACK;
        }
    };
    Trie();
    int cur;
    Node *root;
    void updateSpringAnimation(Node *node, float stiffness, float damping, float dt, float speedMultiplier);
    void add_string(std::string s);
    bool delete_string_recursive(Node *&p, std::string &s, int i);
    void delete_string(std::string s);
    bool find_string(std::string s);
    void compute_positions(Node *node, int depth, float &currentX, float horizontalSpacing, float verticalSpacing);
    void visualize(Node *node, int depth, int &x, std::vector<std::vector<Node *>> &levels);
    bool check(Node *node);
    bool find_string_red(std::string s);
    void load_from_file(const std::string &filename);
};
bool is_valid_input(const std::string &s);
void DrawPseudocode(const std::vector<std::string> &pseudocode, int currentStep, int screenWidth, int screenHeight);
#endif // TRIE_H