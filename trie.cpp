#include "raylib.h"
#include "raygui.h"
#include "gui_window_file_dialog.h"
#include "trie.h"
#include <algorithm>
#include <cmath>
#include <thread>
#include "renderer.h"
#include <fstream>
#include "var.h"
#include <bits/stdc++.h>
#include <random> // Thêm thư viện random
const int MAX_INPUT_CHARS = 20;
using namespace std;
const int nodeRadius = 20;
Trie::Trie() : cur(0)
{
    root = new Node();
    step = 0;
    states.clear();
    importance_states.clear();
    // Define pseudo code for input
    inputPseudoCode = {
        "1. Start at the root node.",
        "2. For each character in the string:",
        "3.    If the child node does not exist, create it.",
        "4.    Move to the child node.",
        "5. Mark the end of the string."};

    // Define pseudo code for remove
    removePseudoCode = {
        "1. Start at the root node.",
        "2. Traverse the string recursively.",
        "3.    If the node is not needed, delete it.",
        "4. Recompute positions after deletion."};

    // Define pseudo code for search
    searchPseudoCode = {
        "1.Start at the root node",
        "2.For each character in the string:",
        "3.   If the child node does not exist, return false",
        "4.If the end of the string is reached, check exist"};
    highlightedLine = -1;
    currentOperation = ""; // Set the current operation to "input"
}
// Hàm vẽ một đường thẳng có mũi tên (đầu mũi tên là tam giác đầy)
void drawArrowLine(Vector2 start, Vector2 end, float thickness, Color color)
{
    // Vẽ đường thẳng nối hai điểm
    DrawLineEx(start, end, thickness, color);
    // Tính vector chỉ phương từ start đến end
    Vector2 direction = {end.x - start.x, end.y - start.y};
    float len = sqrt(direction.x * direction.x + direction.y * direction.y);
    if (len == 0)
        return;
    direction.x /= len;
    direction.y /= len;

    float arrowLength = 10.0f; // Chiều dài của mũi tên
    float arrowWidth = 5.0f;   // Chiều rộng của mũi tên

    // Tạo vector vuông góc với hướng của đường thẳng
    Vector2 perp = {-direction.y, direction.x};

    // Tính các điểm của tam giác mũi tên
    Vector2 arrowTip = end;
    Vector2 arrowLeft = {end.x - direction.x * arrowLength + perp.x * arrowWidth,
                         end.y - direction.y * arrowLength + perp.y * arrowWidth};
    Vector2 arrowRight = {end.x - direction.x * arrowLength - perp.x * arrowWidth,
                          end.y - direction.y * arrowLength - perp.y * arrowWidth};

    // Vẽ tam giác đầy làm đầu mũi tên
    DrawTriangle(arrowTip, arrowRight, arrowLeft, color);
}
void DrawTrie(Trie::Node *node, const int &screenWidth, const int &screenHeight)
{
    if (!node)
        return;
    if (node->cnt <= 0)
        return;
    if (node->exist)
    {
        DrawCircle(node->x, node->y, nodeRadius, GREEN);
    }
    for (double thickness = 0.0; thickness < 3.0; thickness += 0.5)
        DrawCircleLinesV({node->x, node->y}, 20 + thickness, node->color);
    int valueTextWidth = MeasureText(node->character.c_str(), GetFont().baseSize);
    DrawTextEx(GetFont(), node->character.c_str(), {node->x - float(1.0 * valueTextWidth / 2.0), node->y - float(1.0 * GetFont().baseSize / 2.0)}, GetFont().baseSize, 1, BLUE);
    // Vẽ đường nối từ nút cha tới các nút con
    for (int i = 0; i < 52; i++)
    {
        if (node->child[i] != NULL)
        {
            Vector2 parentCenter = {node->x, node->y};
            Vector2 childCenter = {node->child[i]->x, node->child[i]->y};

            // Tính vector chỉ phương từ cha đến con
            Vector2 direction = {childCenter.x - parentCenter.x, childCenter.y - parentCenter.y};
            float len = sqrt(direction.x * direction.x + direction.y * direction.y);
            if (len != 0)
            {
                direction.x /= len;
                direction.y /= len;
            }

            // Tính điểm bắt đầu và kết thúc: cách tâm một khoảng bằng bán kính
            Vector2 startEdge = {parentCenter.x + direction.x * nodeRadius, parentCenter.y + direction.y * nodeRadius};
            Vector2 endEdge = {childCenter.x - direction.x * nodeRadius, childCenter.y - direction.y * nodeRadius};

            drawArrowLine(startEdge, endEdge, 2.0f, BLACK);
            DrawTrie(node->child[i], screenWidth, screenHeight);
        }
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

void Trie::add_string(std::string s)
{
    currentOperation = "input"; // Set the current operation to "input"
    Node *p = root;
    p->cnt++;
    // animateNode(root, 1, insertPseudocode, 1);
    for (int i = 0; i <= 20; i++)
        saveCurrentState(0);
    importance_states.push_back(states.size() - 1);
    for (auto f : s)
    {
        p->color = YELLOW;
        // animateNode(root, 2, insertPseudocode, 1);
        for (int i = 0; i <= 60; i++)
            saveCurrentState(1);
        importance_states.push_back(states.size() - 1);
        p->color = BLACK;
        // saveCurrentState();
        int c = (f >= 'a' && f <= 'z') ? f - 'a' : f - 'A' + 26;
        if (!p->child[c])
        {
            // animateNode(root, 3, insertPseudocode, 1);
            saveCurrentState(2);
            importance_states.push_back(states.size() - 1);
            p->child[c] = new Node();
            p = p->child[c];
            p->character = f;
            p->cnt++;
            float currentX = 40;
            float horizontalSpacing = 80;
            float verticalSpacing = 80;
            compute_positions(root, 1, currentX, horizontalSpacing, verticalSpacing);
            float stiffness = 4.0f;
            float damping = 0.75f;
            int step = 0;
            while (step <= 240)
            {
                double dt = 0.045f;
                updateSpringAnimation(root, stiffness, damping, dt, 1.0f);
                // animateNode(root, 4, insertPseudocode, 0);
                saveCurrentState(2);
                step++;
            }
            importance_states.push_back(states.size() - 1);
        }
        else
        {
            p = p->child[c];
            p->character = f;
            p->cnt++;
        }
        // animateNode(root, 5, insertPseudocode, 1);
        for (int i = 0; i <= 20; i++)
            saveCurrentState(3);
        importance_states.push_back(states.size() - 1);
    }

    p->color = YELLOW;
    // animateNode(root, 7, insertPseudocode, 1);
    for (int i = 0; i <= 60; i++)
        saveCurrentState(3);
    importance_states.push_back(states.size() - 1);
    p->color = BLACK;
    p->exist++;
    saveCurrentState(4);
    importance_states.push_back(states.size() - 1);
    return;
}

bool Trie::delete_string_recursive(Node *&p, std::string &s, int i)
{
    currentOperation = "remove"; // Set the current operation to "remove"
    p->color = YELLOW;
    // animateNode(root, 8, deletePseudocode, 1);
    for (int i = 0; i <= 60; i++)
        saveCurrentState(1);
    importance_states.push_back(states.size() - 1);
    p->color = BLACK;
    saveCurrentState(1);
    importance_states.push_back(states.size() - 1);
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
    // animateNode(root, 8, deletePseudocode, 1);
    for (int i = 0; i <= 60; i++)
        saveCurrentState(1);
    importance_states.push_back(states.size() - 1);
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
            float stiffness = 4.0f;
            float damping = 0.75f;
            int step = 0;
            // animateNode(root, 9, deletePseudocode, 1);
            for (int i = 0; i <= 20; i++)
                saveCurrentState(2);
            importance_states.push_back(states.size() - 1);

            while (step <= 240)
            {
                updateSpringAnimation(root, stiffness, damping, 0.045, 1.0f);
                // animateNode(root, 10, deletePseudocode, 0);
                saveCurrentState(3);
                step++;
            }
            importance_states.push_back(states.size() - 1);

            return true;
        }
    }
    else
    {
        p->cnt--;
    }
    p->color = BLACK;
    saveCurrentState(3);
    importance_states.push_back(states.size() - 1);

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
    currentOperation = "search"; // Set the current operation to "search"
    Node *p = root;
    for (int i = 0; i <= 20; i++)
        saveCurrentState(0);
    for (auto f : s)
    {
        p->color = RED;
        // animateNode(root, 6, searchPseudocode, 1);
        for (int i = 0; i <= 60; i++)
            saveCurrentState(1);
        importance_states.push_back(states.size() - 1);

        int c = (f >= 'a' && f <= 'z') ? f - 'a' : f - 'A' + 26;
        if (!p->child[c])
        {
            for (int i = 0; i <= 60; i++)
                saveCurrentState(2);
            return false;
        }
        p = p->child[c];
    }
    p->color = RED;
    // animateNode(root, 7, searchPseudocode, 1);
    for (int i = 0; i <= 60; i++)
        saveCurrentState(3);
    importance_states.push_back(states.size() - 1);

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

std::string generateRandomString(int maxLength)
{
    static const char alphabet[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> lengthDist(1, maxLength);
    std::uniform_int_distribution<int> charDist(0, sizeof(alphabet) - 2);

    int length = lengthDist(rng);
    std::string result;
    result.reserve(length);
    for (int i = 0; i < length; ++i)
    {
        result += alphabet[charDist(rng)];
    }
    return result;
}

void Trie::load_from_file(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
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
void resetcolor(Trie::Node *node)
{
    if (!node)
        return;
    if (node->cnt <= 0)
        return;
    node->color = BLACK;
    for (int i = 0; i < 52; i++)
    {
        if (node->child[i] != NULL)
            resetcolor(node->child[i]);
    }
    return;
}
void RenderTrie()
{
    DrawTextEx(GetFont(), "TRIE", {float(GetScreenWidth() / 2 - 25), 20}, GetFont().baseSize * 1.5, 1, BLUE);
    static Trie trie;
    static int framesCounter = 0;
    static std::vector<Trie> query = {trie.copyTrie()};
    static int step_query = 0;
    Rectangle inputBox = {1460, 600, 200, 50};
    Rectangle searchBox = {1460, 650, 200, 50};
    Rectangle removeBox = {1460, 700, 200, 50};
    Rectangle loadButton = {1670, 750, 200, 50};         // Add a button for loading the file
    Rectangle clearButton = {1670, 800, 200, 50};        // Add a button for clearing the Trie tree
    Rectangle randomBox = {1670, 850, 200, 50};          // Button for random
    Rectangle randomInputBox = {1460, 850, 200, 50};     // Input box for number of strings
    static char randomBuffer[MAX_INPUT_CHARS + 1] = {0}; // Buffer for random input
    static int randomIndex = 0;                          // Index for random buffer
    static bool randomActive = false;                    // Active state for random input
    static bool random_query = false;                    // Flag for random query
    static int inputIndex = 0;                           // Index for input buffer
    static int searchIndex = 0;                          // Index for search buffer
    static int removeIndex = 0;
    static char inputBuffer[MAX_INPUT_CHARS + 1] = {0};  // Buffer for input
    static char searchBuffer[MAX_INPUT_CHARS + 1] = {0}; // Buffer for search input
    static char removeBuffer[MAX_INPUT_CHARS + 1] = {0}; // Buffer for remove input
    static bool inputActive = false;
    static bool searchActive = false;
    static bool removeActive = false;
    // Seekbar variables
    static float sliderValue = 0.0f; // Normalized value (0.0 to 1.0)
    const int barWidth = 800;
    const int barHeight = 10;
    const int barX = 560;
    const int barY = 970;
    const int knobRadius = 10;
    static Rectangle Previous_query = {70, 950, 80, 50};
    static Rectangle Next_query = {450, 950, 80, 50};

    static Rectangle pause_box = {280, 950, 40, 40};
    static bool pause = false;
    static Rectangle next_step_box = {340, 950, 40, 40};
    static Rectangle prev_step_box = {220, 950, 40, 40};
    static Rectangle go_to_begin = {160, 950, 40, 40};
    static Rectangle go_to_end = {400, 950, 40, 40};

    static float speedSliderValue = 0.0f; // Giá trị thanh trượt (0.0 đến 1.0)
    static float animationSpeed = 1.0f;   // Tốc độ hoạt ảnh (1x đến 10x)
    const int speedBarWidth = 200;
    const int speedBarHeight = 10;
    const int speedBarX = 1500;
    const int speedBarY = 970;
    const int speedKnobRadius = 10;

    static Rectangle menu = {1880, 600, 40, 300};
    static bool menuActive = false;
    static GuiWindowFileDialogState fileDialogState; // Trạng thái của file dialog
    static bool fileDialogInitialized = false;       // Để kiểm tra xem dialog đã được khởi tạo chưa
    static Rectangle inputBotton = {1670, 600, 200, 50};
    static bool inputActiveBotton = false;
    static Rectangle searchBotton = {1670, 650, 200, 50};
    static bool searchActiveBotton = false;
    static Rectangle removeBotton = {1670, 700, 200, 50};
    static bool removeActiveBotton = false;

    static Rectangle pseudoCodeBox = {1880, 200, 40, 300};
    static bool DrawPseudocodeActive = false;
    if (CheckButton_trie(pseudoCodeBox, "Pseudocode"))
    {
        DrawPseudocodeActive = !DrawPseudocodeActive;
    }
    if (DrawPseudocodeActive)
    {
        DrawRectangleGradientV(pseudoCodeBox.x, pseudoCodeBox.y, pseudoCodeBox.width, pseudoCodeBox.height, SKYBLUE, DARKBLUE);
        static Texture2D codetexture = LoadTexture("res/right arrow.png");
        Texture2D *texture = &codetexture;
        Rectangle sourceRect = {0, 0, (float)texture->width, (float)texture->height};
        Rectangle destRect = {1880, 330, 40, 40};
        DrawTexturePro(*texture, sourceRect, destRect, {0, 0}, 0.0f, WHITE);
    }
    else
    {
        DrawRectangleGradientV(pseudoCodeBox.x, pseudoCodeBox.y, pseudoCodeBox.width, pseudoCodeBox.height, SKYBLUE, DARKBLUE);
        static Texture2D codetexture = LoadTexture("res/left arrow.png");
        Texture2D *texture = &codetexture;
        Rectangle sourceRect = {0, 0, (float)texture->width, (float)texture->height};
        Rectangle destRect = {1880, 330, 40, 40};
        DrawTexturePro(*texture, sourceRect, destRect, {0, 0}, 0.0f, WHITE);
    }
    if (CheckButton_trie(menu, "menu"))
    {
        menuActive = !menuActive;
    }
    if (menuActive)
    {
        DrawRectangleGradientV(menu.x, menu.y, menu.width, menu.height, SKYBLUE, DARKBLUE);
        static Texture2D menutexture = LoadTexture("res/right arrow.png");
        Texture2D *texture = &menutexture;
        Rectangle sourceRect = {0, 0, (float)texture->width, (float)texture->height};
        Rectangle destRect = {1880, 730, 40, 40};
        DrawTexturePro(*texture, sourceRect, destRect, {0, 0}, 0.0f, WHITE);
    }
    else
    {
        DrawRectangleGradientV(menu.x, menu.y, menu.width, menu.height, SKYBLUE, DARKBLUE);
        static Texture2D menutexture = LoadTexture("res/left arrow.png");
        Texture2D *texture = &menutexture;
        Rectangle sourceRect = {0, 0, (float)texture->width, (float)texture->height};
        Rectangle destRect = {1880, 730, 40, 40};
        DrawTexturePro(*texture, sourceRect, destRect, {0, 0}, 0.0f, WHITE);
    }
    if (!fileDialogInitialized)
    {
        fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
        fileDialogInitialized = true;
    }
    if (!(trie.step < trie.states.size() - 1) && CheckButton_trie(pause_box,
                                                                  "replay"))
    {
        pause = false;
        trie.step = 0;
    }
    else
    {
        if ((!pause && trie.step < trie.states.size() - 1 && CheckButton_trie(pause_box, "pause")) ||
            (pause && trie.step < trie.states.size() - 1 && CheckButton_trie(pause_box, "play")))
        {
            // Toggle pause state
            pause = !pause;
        }
    }
    if (CheckButton_trie(prev_step_box, "previous step"))
    {
        if (trie.step > 0 && !trie.importance_states.empty())
        {
            int newstep = lower_bound(trie.importance_states.begin(), trie.importance_states.end(), trie.step) - trie.importance_states.begin() - 1;
            if (newstep >= 0)
            {
                trie.step = trie.importance_states[newstep];
                pause = true; // Pause the animation when going to the previous step
            }
        }
    }
    if (CheckButton_trie(next_step_box, "next step"))
    {
        if (trie.step < int(trie.states.size()) - 1 && !trie.importance_states.empty())
        {
            int newstep = upper_bound(trie.importance_states.begin(), trie.importance_states.end(), trie.step) - trie.importance_states.begin();
            if (newstep < int(trie.importance_states.size()))
            {
                trie.step = trie.importance_states[newstep];
                pause = true; // Pause the animation when going to the next step
            }
        }
    }
    if (CheckButton_trie(go_to_begin, "go to begin") && trie.step > 0)
    {
        trie.step = 0;
        resetcolor(trie.root);
    }
    if (CheckButton_trie(go_to_end, "go to end") && trie.step < int(trie.states.size()) - 1)
    {
        trie.step = int(trie.states.size()) - 1;
        resetcolor(trie.root);
    }
    if (((!trie.root || trie.states.empty() || trie.step >= int(trie.states.size() - 1))) && menuActive)
    {
        if (CheckButton_trie(inputBotton, "Input"))
        {
            inputActiveBotton = !inputActiveBotton;
        }
        if (CheckButton_trie(searchBotton, "Search"))
        {
            searchActiveBotton = !searchActiveBotton;
        }
        if (CheckButton_trie(removeBotton, "Remove"))
        {
            removeActiveBotton = !removeActiveBotton;
        }
        if (inputActiveBotton)
        {

            DrawBoxes_trie(inputBox, inputBuffer, framesCounter, inputActive);
            HandleInput_trie(inputBox, inputBuffer, inputIndex, inputActive);
            if (!inputActive)
                DrawTextEx(GetFont(), "Enter string to add", {inputBox.x + 10, inputBox.y + 10}, 20, 1, RED);
        }
        if (searchActiveBotton)
        {
            DrawBoxes_trie(searchBox, searchBuffer, framesCounter, searchActive);
            HandleInput_trie(searchBox, searchBuffer, searchIndex, searchActive);
            if (!searchActive)
                DrawTextEx(GetFont(), "Enter string to search", {searchBox.x + 10, searchBox.y + 10}, 20, 1, RED);
        }
        if (removeActiveBotton)
        {
            DrawBoxes_trie(removeBox, removeBuffer, framesCounter, removeActive);
            HandleInput_trie(removeBox, removeBuffer, removeIndex, removeActive);
            if (!removeActive)
                DrawTextEx(GetFont(), "Enter string to remove", {removeBox.x + 10, removeBox.y + 10}, 20, 1, RED);
        }
        if (CheckButton_trie(loadButton, "Load from file"))
        {
            fileDialogState.windowActive = !fileDialogState.windowActive; // Kích hoạt file dialog
        }
        if (fileDialogState.windowActive)
        {
            GuiWindowFileDialog(&fileDialogState);

            if (fileDialogState.SelectFilePressed)
            {
                // Xử lý file được chọn
                std::string selectedFilePath = std::string(fileDialogState.dirPathText) + "\\" + std::string(fileDialogState.fileNameText);
                std::cout << "Selected file: " << selectedFilePath << std::endl;

                std::ifstream inputFile(selectedFilePath);
                if (inputFile.is_open())
                {
                    trie = Trie();                    // Reinitialize the Trie to clear it
                    query.clear();                    // Clear the query vector
                    query.push_back(trie.copyTrie()); // Add the initial state of the empty Trie
                    step_query = 0;                   // Reset the step_query to 0
                    trie.load_from_file(selectedFilePath);
                    float currentX = 40;
                    float horizontalSpacing = 80;
                    float verticalSpacing = 80;
                    trie.compute_positions(trie.root, 1, currentX, horizontalSpacing, verticalSpacing);
                }
                else
                {
                    std::cerr << "Failed to open file: " << selectedFilePath << std::endl;
                }

                fileDialogState.SelectFilePressed = false; // Reset trạng thái
            }
        }
        if (CheckButton_trie(clearButton, "Clear Trie"))
        {
            trie = Trie();                    // Reinitialize the Trie to clear it
            query.clear();                    // Clear the query vector
            query.push_back(trie.copyTrie()); // Add the initial state of the empty Trie
            step_query = 0;                   // Reset the step_query to 0
        }
        if (CheckButton_trie(randomBox, "Random"))
        {
            random_query = !random_query;
        }
        if (random_query)
        {
            DrawBoxes_trie(randomInputBox, randomBuffer, framesCounter, randomActive);
            HandleInput_trie(randomInputBox, randomBuffer, randomIndex, randomActive);
            if (!randomActive)
                DrawTextEx(GetFont(), "Enter number of strings", {randomInputBox.x + 10, randomInputBox.y + 10}, 20, 1, RED);
            if (IsKeyPressed(KEY_ENTER) && randomBuffer[0] != '\0')
            {
                std::string randomString(randomBuffer);
                int numStrings = std::stoi(randomString); // Convert input to integer
                if (numStrings > 0)
                {
                    trie = Trie();                    // Reinitialize the Trie to clear it
                    query.clear();                    // Clear the query vector
                    query.push_back(trie.copyTrie()); // Add the initial state of the empty Trie
                    step_query = 0;
                    for (int i = 0; i < numStrings; ++i)
                    {
                        std::string randomWord = generateRandomString(10); // Generate random string
                        trie.add_string(randomWord);                       // Add to Trie
                    }
                    float currentX = 40;
                    float horizontalSpacing = 80;
                    float verticalSpacing = 80;
                    trie.compute_positions(trie.root, 1, currentX, horizontalSpacing, verticalSpacing);

                    // Clear the random buffer
                    randomIndex = 0;
                    randomBuffer[0] = '\0';
                    randomActive = false; // Deactivate the input box
                }
            }
        }

        if (IsKeyPressed(KEY_ENTER))
        {
            if (inputActive && inputIndex != 0)
            {
                resetcolor(trie.root);
                std::string inputString(inputBuffer);
                if (is_valid_input(inputString))
                {
                    trie.states.clear(); // Clear the states vector when adding a new string
                    trie.step = 0;
                    trie.add_string(inputString);
                    // Clear the input buffer
                    inputIndex = 0;
                    inputBuffer[0] = '\0';
                    for (int i = query.size() - 1; i > step_query; i--)
                    {
                        query.pop_back();
                    }
                    query.push_back(trie.copyTrie());
                    step_query = query.size() - 1; // Update step_query to the last index
                }
            }
            else if (removeActive && removeIndex != 0)
            {
                resetcolor(trie.root);
                std::string removeString(removeBuffer);
                if (is_valid_input(removeString))
                {
                    trie.states.clear(); // Clear the states vector when removing a string
                    trie.step = 0;
                    trie.delete_string(removeString);
                    // Recompute positions after removing a string
                    float currentX = 40;
                    float horizontalSpacing = 80;
                    float verticalSpacing = 80;
                    trie.compute_positions(trie.root, 1, currentX, horizontalSpacing, verticalSpacing);

                    // Clear the remove buffer
                    removeIndex = 0;
                    removeBuffer[0] = '\0';
                    for (int i = query.size() - 1; i > step_query; i--)
                    {
                        query.pop_back();
                    }
                    query.push_back(trie.copyTrie());
                    step_query = query.size() - 1; // Update step_query to the last index
                }
            }
            else if (searchActive && searchIndex != 0)
            {
                resetcolor(trie.root);
                std::string searchString(searchBuffer);
                if (is_valid_input(searchString))
                {
                    trie.states.clear(); // Clear the states vector when searching for a string
                    trie.step = 0;
                    trie.find_string_red(searchString);
                    searchIndex = 0;
                    searchBuffer[0] = '\0';
                    for (int i = query.size() - 1; i > step_query; i--)
                    {
                        query.pop_back();
                    }
                    query.push_back(trie.copyTrie());
                    step_query = query.size() - 1; // Update step_query to the last index
                }
            }
        }
    }
    if (CheckButton_trie(Previous_query, "Prev"))
    {
        if (step_query > 0)
        {
            step_query--;
            trie = query[step_query].copyTrie();
            resetcolor(trie.root);
        }
    }
    if (CheckButton_trie(Next_query, "Next"))
    {
        if (step_query < query.size() - 1)
        {
            step_query++;
            trie = query[step_query].copyTrie();
            resetcolor(trie.root);
        }
    }
    Vector2 mousePoint = GetMousePosition();
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
    {
        if (mousePoint.x >= barX && mousePoint.x <= barX + barWidth &&
            mousePoint.y >= barY - 10 && mousePoint.y <= barY + barHeight + 10)
        {
            sliderValue = (float)(mousePoint.x - barX) / (float)barWidth;
            trie.step = (int)(sliderValue * (trie.states.size() - 1));
        }
        if (mousePoint.x >= speedBarX && mousePoint.x <= speedBarX + speedBarWidth &&
            mousePoint.y >= speedBarY - 10 && mousePoint.y <= speedBarY + speedBarHeight + 10)
        {
            speedSliderValue = (float)(mousePoint.x - speedBarX) / (float)speedBarWidth;
            animationSpeed = 1.0f + speedSliderValue * 9.0f; // Tốc độ từ 1x đến 10x
        }
    }
    // Vẽ thanh speed bar
    DrawRectangle(speedBarX, speedBarY, speedBarWidth, speedBarHeight, LIGHTGRAY);
    DrawRectangleGradientH(speedBarX, speedBarY, (int)(speedBarWidth * speedSliderValue), speedBarHeight, SKYBLUE, DARKBLUE);
    DrawCircle(speedBarX + (int)(speedBarWidth * speedSliderValue), speedBarY + speedBarHeight / 2, speedKnobRadius, DARKBLUE);

    // Hiển thị tốc độ hiện tại bên cạnh thanh speed bar
    int currentSpeed = static_cast<int>(1 + speedSliderValue * 9); // Tốc độ từ 1x đến 10x
    std::string speedText = std::to_string(currentSpeed) + "x";
    DrawTextEx(GetFont(), speedText.c_str(), {float(speedBarX + speedBarWidth + 20), float(speedBarY) - 10}, GetFont().baseSize, 1, BLACK);
    if (trie.step < int(trie.states.size()) && trie.step >= 0)
    {
        DrawTrie(trie.states[trie.step].root, GetScreenWidth(), GetScreenHeight());
        if (DrawPseudocodeActive)
        {
            if (trie.states[trie.step].currentOperation == "input")
            {
                DrawPseudoCode_trie(trie.states[trie.step].inputPseudoCode, trie.states[trie.step].highlightedLine);
            }
            else if (trie.states[trie.step].currentOperation == "remove")
            {
                DrawPseudoCode_trie(trie.states[trie.step].removePseudoCode, trie.states[trie.step].highlightedLine);
            }
            else if (trie.states[trie.step].currentOperation == "search")
            {
                DrawPseudoCode_trie(trie.states[trie.step].searchPseudoCode, trie.states[trie.step].highlightedLine);
            }
        }
        if (!pause)
            trie.step += animationSpeed;
    }
    else
    {
        DrawTrie(trie.root, GetScreenWidth(), GetScreenHeight());
        if (DrawPseudocodeActive)
        {
            if (trie.currentOperation == "input")
            {
                DrawPseudoCode_trie(trie.inputPseudoCode, trie.highlightedLine);
            }
            else if (trie.currentOperation == "remove")
            {
                DrawPseudoCode_trie(trie.removePseudoCode, trie.highlightedLine);
            }
            else if (trie.currentOperation == "search")
            {
                DrawPseudoCode_trie(trie.searchPseudoCode, trie.highlightedLine);
            }
        }
    }
    sliderValue = (float)trie.step / (float)(trie.states.size() - 1);
    DrawRectangle(barX, barY, barWidth, barHeight, LIGHTGRAY);
    DrawRectangleGradientH(barX, barY, (int)(barWidth * sliderValue), barHeight, SKYBLUE, DARKBLUE);
    DrawCircle(barX + (int)(sliderValue * barWidth), barY + barHeight / 2, knobRadius, DARKBLUE);
    framesCounter++;
    return;
}
void DrawBoxes_trie(const Rectangle &Box, const char *Buffer, const int &framesCounter, const bool &isActive)
{
    DrawRectangleRec(Box, LIGHTGRAY);
    // DrawText(Buffer, Box.x + 10, Box.y + 15, 20, BLACK);
    DrawTextEx(GetFont(), Buffer, {Box.x + GetFont().baseSize / 2, Box.y + 15}, GetFont().baseSize, 1, BLACK);
    int MAX_INPUT_CHARS = 10;
    if (((framesCounter / 20) % 2) == 0)
    {
        if (isActive && strlen(Buffer) < MAX_INPUT_CHARS)
            DrawText("_", Box.x + GetFont().baseSize / 2 + MeasureText(Buffer, GetFont().baseSize), Box.y + 15, GetFont().baseSize, BLACK);
    }
}

void HandleInput_trie(const Rectangle &box, char *buffer, int &index, bool &isActive)
{
    Vector2 mousePoint = GetMousePosition();
    if (CheckCollisionPointRec(mousePoint, box) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        isActive = true;
    }
    else if (!CheckCollisionPointRec(mousePoint, box) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        isActive = false;
    }

    if (isActive)
    {
        // Set the window's cursor to the I-Beam
        SetMouseCursor(MOUSE_CURSOR_IBEAM);

        // Get char pressed (unicode character) on the queue
        int key = GetCharPressed();

        // Check if more characters have been pressed on the same frame
        while (key > 0)
        {
            // NOTE: Only allow keys in range [32..125]
            if ((key >= 32) && (key <= 125) && (index < MAX_INPUT_CHARS))
            {
                buffer[index] = (char)key;
                buffer[index + 1] = '\0'; // Add null terminator at the end of the string.
                index++;
            }

            key = GetCharPressed(); // Check next character in the queue
        }

        if (IsKeyPressed(KEY_BACKSPACE))
        {
            index--;
            if (index < 0)
                index = 0;
            buffer[index] = '\0';
        }
    }
    else
    {
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    }
}

bool CheckButton_trie(Rectangle button, const char *text)
{
    static Texture2D pauseTexture = LoadTexture("res/pause.png");            // Load texture once
    static Texture2D playTexture = LoadTexture("res/play.png");              // Load texture once
    static Texture2D backwardTexture = LoadTexture("res/previous step.png"); // Load texture once
    static Texture2D beginTexture = LoadTexture("res/go to begin.png");      // Load texture once
    static Texture2D replayTexture = LoadTexture("res/replay.png");          // Load texture once
    static Texture2D forwardTexture = LoadTexture("res/next step.png");      // Load texture once
    static Texture2D endTexture = LoadTexture("res/go to end.png");          // Load texture once

    Vector2 mousePoint = GetMousePosition();
    bool mouseOnButton = CheckCollisionPointRec(mousePoint, button);

    if (mouseOnButton)
    {
        DrawRectangleRec(button, LIGHTGRAY);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            return true;
        }
    }
    else
    {
        DrawRectangleRec(button, WHITE);
    }

    // Draw the corresponding image based on the button text
    Texture2D *texture = nullptr;

    if (std::string(text) == "pause")
        texture = &pauseTexture;
    else if (std::string(text) == "play")
        texture = &playTexture;
    else if (std::string(text) == "previous step")
        texture = &backwardTexture;
    else if (std::string(text) == "go to begin")
        texture = &beginTexture;
    else if (std::string(text) == "replay")
        texture = &replayTexture;
    else if (std::string(text) == "next step")
        texture = &forwardTexture;
    else if (std::string(text) == "go to end")
        texture = &endTexture;
    else if (std::string(text) == "menu")
    {
        return false;
    }
    else if (std::string(text) == "PSEUDO CODE")
    {
        return false;
    }
    else
    {
        // Draw the text if no texture is found
        if (!mouseOnButton)
            DrawRectangleGradientH(button.x, button.y, button.width, button.height, SKYBLUE, DARKBLUE);
        DrawTextEx(GetFont(), text, {button.x + 10, button.y + 10}, GetFont().baseSize, 1, WHITE);
        return false; // No texture to draw, return false
    }
    if (texture != nullptr)
    {
        // Define the source rectangle (entire texture)
        Rectangle sourceRect = {0, 0, (float)texture->width, (float)texture->height};

        // Define the destination rectangle (scaled size and position)
        Rectangle destRect = {button.x, button.y, button.width, button.height};

        // Draw the resized texture
        DrawTexturePro(*texture, sourceRect, destRect, {0, 0}, 0.0f, WHITE);
    }

    return false;
}
void Trie::saveCurrentState(int lineToHighlight)
{
    // Sao chép trạng thái hiện tại của cây và thêm vào vector states
    this->highlightedLine = lineToHighlight; // Save the highlighted line
    states.push_back(this->copyTrie());
}
Trie Trie::copyTrie()
{
    Trie newTrie;
    newTrie.root = copyNode(this->root);
    newTrie.cur = this->cur;
    newTrie.highlightedLine = this->highlightedLine;
    newTrie.currentOperation = this->currentOperation;
    return newTrie;
}
Trie::Node *Trie::copyNode(Node *node)
{
    if (!node)
        return nullptr;

    Node *newNode = new Node();
    newNode->exist = node->exist;
    newNode->cnt = node->cnt;
    newNode->x = node->x;
    newNode->y = node->y;
    newNode->targetX = node->targetX;
    newNode->targetY = node->targetY;
    newNode->velocityX = node->velocityX;
    newNode->velocityY = node->velocityY;
    newNode->color = node->color;
    newNode->character = node->character;

    for (int i = 0; i < 52; i++)
    {
        if (node->child[i])
        {
            newNode->child[i] = copyNode(node->child[i]);
        }
    }

    return newNode;
}

void DrawPseudoCode_trie(const std::vector<std::string> &pseudoCode, int highlightedLine)
{
    int startX = 1320;                             // Starting X position
    int startY = 200;                              // Starting Y position
    int lineHeight = GetFont().baseSize + 5;       // Line height
    DrawRectangle(1320, 200, 550, 300, LIGHTGRAY); // Background for pseudo code

    for (size_t i = 0; i < pseudoCode.size(); i++)
    {
        Color color = (i == highlightedLine) ? RED : BLACK; // Highlight current line
        if (i == highlightedLine)
            DrawRectangle(startX, startY + i * lineHeight, 550, lineHeight, YELLOW); // Highlight background
        DrawTextEx(GetFont(), pseudoCode[i].c_str(), {float(startX), float(startY + i * lineHeight)}, GetFont().baseSize, 1, color);
    }
}