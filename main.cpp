#include "raylib.h"


#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#undef RAYGUI_IMPLEMENTATION            // Avoid including raygui implementation again

#define GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION
#include "gui_window_file_dialog.h"

#include "AVL.h"
#include <string>
#include <cstdlib>  // atoi
#include<linked_list.h>
#include <sstream>  // istringstream
#include<iostream>
#include "mst.h"
#include "ShortestPath.h"
#include "renderer.h"
#include "Effects.h" // Add this include
#include <vector>
#include <fstream>
#include "hashtable.h"
#include "trie.h"
#include "var.h"

using namespace std;

// Add function prototypes
void RunAVLVisualization();
void RunLinkedListVisualization();
void RunMSTVisualization();
void RunShortestPath();
void RunTrie();
void RunHashTable();

// Update color scheme for light theme
const Color MENU_BG = {255, 255, 255, 255};        // Pure white
const Color BUTTON_PRIMARY = {67, 134, 245, 255};   // Modern blue
const Color BUTTON_HOVER = {89, 156, 255, 255};     // Lighter blue
const Color BUTTON_TEXT = {255, 255, 255, 255};     // White
const Color TITLE_COLOR = {40, 44, 52, 255};        // Dark gray
const Color SUBTITLE_COLOR = {91, 94, 109, 255};    // Medium gray

// Add effects constants
const Color BUTTON_SHADOW = {0, 0, 0, 30};         // Soft shadow
const Color BUTTON_ACTIVE = {45, 112, 223, 255};   // Darker blue for click
const Color GLOW_COLOR = {67, 134, 245, 20};      // Very light blue glow

const int MENU_BUTTON_WIDTH = 300;
const int MENU_BUTTON_HEIGHT = 60;
const float BUTTON_ROUND = 0.2f;

bool isMenuActive = true;
bool windowInitialized = false;

Rectangle CreateBackButton() {
    return Rectangle{1750, 30, 120, 40}; // Move to top-right corner
}

// Material Design color palette
const Color MD_PRIMARY = {63, 81, 181, 255};    // Indigo 500
const Color MD_PRIMARY_DARK = {48, 63, 159, 255}; // Indigo 700
const Color MD_ACCENT = {255, 64, 129, 255};    // Pink A200
const Color MD_TEXT = {33, 33, 33, 255};        // Grey 900
const Color MD_TEXT_LIGHT = {158, 158, 158, 255}; // Grey 400
const Color MD_BACKGROUND = {250, 250, 250, 255}; // Grey 50
const Color MD_SURFACE = {255, 255, 255, 255};    // White
const Color MD_ERROR = {244, 67, 54, 255};      // Red 500

// Material elevation shadows
const Color SHADOW_COLOR = {0, 0, 0, 30};
const int SHADOW_OFFSET = 4;

// Button states and animations
const float HOVER_SCALE = 1.02f;
const float PRESS_SCALE = 0.98f;
const float ANIMATION_SPEED = 0.2f;

// Replace the template Clamp function with a specific float implementation
float Clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

void DrawBackButton(Rectangle backBtn) {
    Vector2 mousePoint = GetMousePosition();
    bool isHovered = CheckCollisionPointRec(mousePoint, backBtn);
    bool isPressed = isHovered && IsMouseButtonDown(MOUSE_LEFT_BUTTON);
    
    // Shadow
    DrawRectangleRounded(
        (Rectangle){
            backBtn.x + SHADOW_OFFSET,
            backBtn.y + SHADOW_OFFSET,
            backBtn.width,
            backBtn.height
        },
        BUTTON_ROUND, 0, SHADOW_COLOR
    );
    
    // Button
    Color btnColor = isPressed ? MD_PRIMARY_DARK :
                    (isHovered ? ColorBrightness(MD_PRIMARY, 0.1f) : MD_PRIMARY);
                    
    DrawRectangleRounded(backBtn, BUTTON_ROUND, 0, btnColor);
    
    // Icon and text
    DrawText("Back",
            backBtn.x + 45,
            backBtn.y + 10,
            20, MD_SURFACE);
}

void DrawMenuButton(Rectangle btn, const char* text, bool isHovered) {
    Vector2 mousePoint = GetMousePosition();
    bool isPressed = IsMouseButtonDown(MOUSE_LEFT_BUTTON) && 
                    CheckCollisionPointRec(mousePoint, btn);
    
    // Base button shadow
    DrawRectangleRounded(
        (Rectangle){
            btn.x + SHADOW_OFFSET, 
            btn.y + SHADOW_OFFSET,
            btn.width,
            btn.height
        },
        0.25f, 8, SHADOW_COLOR
    );

    // Button background with elevation
    Color btnColor = isPressed ? MD_PRIMARY_DARK : 
                    (isHovered ? ColorBrightness(MD_PRIMARY, 0.1f) : MD_PRIMARY);
    
    // Smooth animation scaling
    float scale = isPressed ? PRESS_SCALE : 
                 (isHovered ? HOVER_SCALE : 1.0f);
    
    Rectangle scaledBtn = {
        btn.x + (btn.width * (1 - scale)) / 2,
        btn.y + (btn.height * (1 - scale)) / 2,
        btn.width * scale,
        btn.height * scale
    };

    // Draw main button with ripple effect
    DrawRectangleRounded(scaledBtn, 0.25f, 8, btnColor);
    
    // Draw text with proper alignment
    int fontSize = 24;
    int textWidth = MeasureText(text, fontSize);
    int textX = scaledBtn.x + (scaledBtn.width - textWidth) / 2;
    int textY = scaledBtn.y + (scaledBtn.height - fontSize) / 2;
    
    // Text shadow for depth
    DrawText(text, textX + 1, textY + 1, fontSize, (Color){0, 0, 0, 50});
    DrawText(text, textX, textY, fontSize, MD_SURFACE);
}

void ShowMenu() {
    LoadFonts();
    if (!windowInitialized) {

        SetConfigFlags(FLAG_MSAA_4X_HINT);
        InitWindow(1920, 1080, "Data Structure Visualization");
        //SetConfigFlags(FLAG_FULLSCREEN_MODE);
        SetTargetFPS(120);
        windowInitialized = true;
    }

    ParticleSystem particleSystem(1920, 1080, 150);
    float deltaTime = 0.0f;

    const char* menuItems[] = {
        "AVL Tree",
        "Linked List",
        "Minimum Spanning Tree",
        "Shortest Path",
        "Trie",
        "Hash Table",
        "Exit"
    };
    int numItems = sizeof(menuItems) / sizeof(menuItems[0]);

    // Điều chỉnh startY lớn hơn để tránh chồng lấp với title
    int startY = (1080 - (numItems * (MENU_BUTTON_HEIGHT + 20))) / 2 + 100; // Thêm offset 100px
    std::vector<Rectangle> buttons;
    
    for (int i = 0; i < numItems; i++) {
        Rectangle btn = {
            (1920 - MENU_BUTTON_WIDTH) / 2,
            startY + i * (MENU_BUTTON_HEIGHT + 20),
            MENU_BUTTON_WIDTH,
            MENU_BUTTON_HEIGHT
        };
        buttons.push_back(btn);
    }

    while (!WindowShouldClose() && isMenuActive) {
        deltaTime = GetFrameTime();
        Vector2 mousePoint = GetMousePosition();

        // Update particles
        particleSystem.Update(deltaTime);

        // Add particles at mouse position when moving
        static Vector2 lastMouse = mousePoint;
        // Calculate vector length manually
        float dx = mousePoint.x - lastMouse.x;
        float dy = mousePoint.y - lastMouse.y;
        float distance = sqrt(dx*dx + dy*dy);
        
        if (distance > 2.0f) {
            particleSystem.CreateParticle(mousePoint);
            lastMouse = mousePoint;
        }

        BeginDrawing();
            ClearBackground(MD_BACKGROUND);
            
            // Draw animated background and particles
            particleSystem.DrawBackground(deltaTime);
            particleSystem.Draw();
            
            // Draw subtle grid pattern
            for (int i = 0; i < 1920; i += 40) {
                for (int j = 0;  j < 1080; j += 40) {
                    DrawPixel(i, j, (Color){0, 0, 0, 5});
                }
            }
            
            // Remove the white card background and shadow, keep only buttons and text
            const char* title = "Data Structure";
            const char* subtitle = "Visualization Platform";
            int titleWidth = MeasureText(title, 50);
            int subtitleWidth = MeasureText(subtitle, 30);
            
            DrawText(title, 
                    (1920 - titleWidth) / 2,
                    startY - 130, // Adjust position without card
                    50, MD_PRIMARY);
                    
            DrawText(subtitle,
                    (1920 - subtitleWidth) / 2,
                    startY - 70, // Adjust position without card
                    30, MD_TEXT_LIGHT);
            
            // Draw menu items with proper spacing and grouping
            for (int i = 0; i < numItems; i++) {
                bool isHovered = CheckCollisionPointRec(mousePoint, buttons[i]);
                DrawMenuButton(buttons[i], menuItems[i], isHovered);
                
                if (isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    switch (i) {
                        case 0: RunAVLVisualization(); break;
                        case 1: RunLinkedListVisualization(); break;
                        case 2: RunMSTVisualization(); break;
                        case 3: RunShortestPath(); break;
                        case 4: RunTrie(); break;
                        case 5: RunHashTable(); break;
                        case 6: 
                            isMenuActive = false;
                            CloseWindow();
                            return;
                    }
                }
            }
            
        EndDrawing();
    }
    UnloadFont(GetFont());
    CloseWindow();
    windowInitialized = false;
}

// Function to process a single line of input and update the graph
bool processInputLine(const std::string& line, MST& mst, int numVertices, std::string& errorMsg) {
    if (line.empty() || line[0] == '/') return false; // Skip empty lines and comments
    
    std::istringstream iss(line);
    std::vector<int> values;
    int val;
    
    while (iss >> val) {
        values.push_back(val);
    }
    
    if (values.size() == 1) {
        // Single vertex - no connections needed
        if (values[0] < 1 || values[0] > numVertices) {
            errorMsg = "Invalid vertex: " + std::to_string(values[0]);
            return false;
        }
        return true; // Valid vertex
    } else if (values.size() == 2) {
        // Two vertices - edge with weight 1
        int u = values[0];
        int v = values[1];
        
        if (u < 1 || u > numVertices || v < 1 || v > numVertices) {
            errorMsg = "Invalid vertices: " + std::to_string(u) + ", " + std::to_string(v);
            return false;
        }
        
        mst.addEdge(u, v, 1);
        return true;
    } else if (values.size() == 3) {
        // Three values - edge with specified weight
        int u = values[0];
        int v = values[1];
        int w = values[2];
        
        if (u < 1 || u > numVertices || v < 1 || v > numVertices) {
            errorMsg = "Invalid vertices: " + std::to_string(u) + ", " + std::to_string(v);
            return false;
        }
        
        mst.addEdge(u, v, w);
        return true;
    } else if (!values.empty()) {
        // Invalid format
        errorMsg = "Invalid format: needs to be 'u v w' or 'u v'";
        return false;
    }
    
    return false;
}


// Hàm vẽ hamburger icon với thiết kế Material Design và animation
void DrawHamburgerIconChPlay(Rectangle hamburgerIcon)
{
    static float animationProgress = 0.0f;
    static bool lastMenuState = false;
    static bool isMenuOpen = false;
    Vector2 mousePoint = GetMousePosition();
    bool isHovered = CheckCollisionPointRec(mousePoint, hamburgerIcon);
    
    // Handle animation progress based on menu state
    if (isMenuOpen != lastMenuState) {
        lastMenuState = isMenuOpen;
    }
    
    // Update animation smoothly
    if (isMenuOpen && animationProgress < 1.0f) {
        animationProgress += GetFrameTime() * 4.0f; // Speed of animation
        if (animationProgress > 1.0f) animationProgress = 1.0f;
    } else if (!isMenuOpen && animationProgress > 0.0f) {
        animationProgress -= GetFrameTime() * 4.0f;
        if (animationProgress < 0.0f) animationProgress = 0.0f;
    }
    
    // Background with elevation effect and hover state
    Color bgColor = isHovered ? 
        (IsMouseButtonDown(MOUSE_LEFT_BUTTON) ? ColorAlpha(MD_PRIMARY_DARK, 0.9f) : ColorAlpha(MD_PRIMARY, 0.8f)) : 
        ColorAlpha(LIGHTGRAY, 0.7f);
    
    // Draw shadow for elevation effect
    DrawRectangleRounded(
        (Rectangle){
            hamburgerIcon.x + 2,
            hamburgerIcon.y + 2,
            hamburgerIcon.width,
            hamburgerIcon.height
        },
        0.3f, 8, ColorAlpha(BLACK, 0.2f)
    );
    
    // Draw main button background
    DrawRectangleRounded(hamburgerIcon, 0.3f, 8, bgColor);
    
    // Calculate line dimensions
    float lineHeight = 3.0f;
    float lineWidth = hamburgerIcon.width - 16;
    float centerX = hamburgerIcon.x + hamburgerIcon.width / 2;
    float centerY = hamburgerIcon.y + hamburgerIcon.height / 2;
    
    // Hamburger-to-X animation
    Color lineColor = isHovered ? WHITE : DARKGRAY;
    
    // Top line animation (rotates to form part of the X)
    DrawLineEx(
        (Vector2){
            centerX - lineWidth/2 * (1 - animationProgress),
            centerY - 8 + 8 * animationProgress
        },
        (Vector2){
            centerX + lineWidth/2 * (1 - animationProgress) + lineWidth/2 * animationProgress,
            centerY - 8 + 8 * animationProgress + 8 * animationProgress
        },
        lineHeight,
        lineColor
    );
    
    // Middle line animation (fades out)
    DrawLineEx(
        (Vector2){centerX - lineWidth/2, centerY},
        (Vector2){centerX + lineWidth/2, centerY},
        lineHeight,
        ColorAlpha(lineColor, 1.0f - animationProgress)
    );
    
    // Bottom line animation (rotates to form part of the X)
    DrawLineEx(
        (Vector2){
            centerX - lineWidth/2 * (1 - animationProgress),
            centerY + 8 - 8 * animationProgress
        },
        (Vector2){
            centerX + lineWidth/2 * (1 - animationProgress) + lineWidth/2 * animationProgress,
            centerY + 8 - 8 * animationProgress - 8 * animationProgress
        },
        lineHeight,
        lineColor
    );
}

// Hàm vẽ sidebar chứa các button và vùng nhập liệu với thiết kế Material Design
void DrawSidebar(Rectangle sidebar)
{
    // Vẽ nền sidebar với hiệu ứng gradient và độ mờ
    DrawRectangleGradientV(
        sidebar.x, sidebar.y, 
        sidebar.width, sidebar.height,
        ColorAlpha(MD_PRIMARY_DARK, 0.95f),
        ColorAlpha(MD_PRIMARY, 0.90f)
    );
    
    // Add subtle pattern for texture
    for (int y = sidebar.y; y < sidebar.y + sidebar.height; y += 10) {
        for (int x = sidebar.x; x < sidebar.x + sidebar.width; x += 10) {
            if ((x + y) % 20 == 0) {
                DrawRectangle(x, y, 1, 1, ColorAlpha(WHITE, 0.03f));
            }
        }
    }
    
    // Draw decorative header
    DrawRectangleGradientH(
        sidebar.x, sidebar.y, 
        sidebar.width, 80,
        ColorAlpha(MD_PRIMARY_DARK, 0.9f),
        ColorAlpha(MD_ACCENT, 0.7f)
    );
    
    // Draw sidebar title
    DrawText("Controls Panel", sidebar.x + 20, sidebar.y + 20, 24, WHITE);
    DrawText("Configure and run algorithms", sidebar.x + 20, sidebar.y + 50, 16, ColorAlpha(WHITE, 0.7f));
    
    // Draw separator line
    DrawLineEx(
        (Vector2){sidebar.x + 15, sidebar.y + 90},
        (Vector2){sidebar.x + sidebar.width - 15, sidebar.y + 90},
        2,
        ColorAlpha(WHITE, 0.2f)
    );


}

void RunMSTVisualization() {
    Rectangle backBtn = CreateBackButton();
    const int screenWidth = 1920;
    const int screenHeight = 1080;
    
    int numVertices = 7; // Default value
    MST mst(numVertices, 0);
    mst.initializeVertexPositions(); // Explicitly initialize vertex positions right at the start
    
    // UI constants
    const float BUTTON_WIDTH = 140;
    const float BUTTON_HEIGHT = 40;
    const float CENTER_X = screenWidth / 2;
    const float TITLE_Y = 15;
    const float START_Y = TITLE_Y + 40;
    
    // Hamburger icon: góc trái dưới (padding 10 pixel)
    Rectangle hamburgerIcon = { 10, (float)screenHeight - 100, 40, 40 };
    bool isMenuOpen = false;

    // Sidebar: toàn bộ chiều cao, chiều rộng 300 pixel (bên trái)
    Rectangle sidebar = { 20 , 400, 350, (float)screenHeight };
    
    // Buttons
    //Rectangle runMSTButton = { CENTER_X - BUTTON_WIDTH/2, START_Y + 15, BUTTON_WIDTH, BUTTON_HEIGHT };
    Rectangle runMSTButton = { sidebar.x + 50, 500, 260, 40 };

    //Rectangle randomButton = { CENTER_X + 350, START_Y + 15, BUTTON_WIDTH, BUTTON_HEIGHT };
    Rectangle randomButton  = { sidebar.x + 50, runMSTButton.y + runMSTButton.height + 20, 130, 40 };

    // Scrollable input container
    //Rectangle vertexInputBox = { CENTER_X - 100, START_Y - 10, 80, 30 };   
    Rectangle vertexInputBox = { sidebar.x + 50, randomButton.y + randomButton.height + 20, 130, 40 };
    Rectangle fileButton = { vertexInputBox.x + vertexInputBox.width + 10, vertexInputBox.y, 120 , 40 };

    //Rectangle inputContainer = { CENTER_X - 300, START_Y + BUTTON_HEIGHT + 30, 600, 350 };
    
    Rectangle inputContainer  = { sidebar.x + 50, vertexInputBox.y + vertexInputBox.height + 20, 260, 350 };
    
    Rectangle edgeCountBox = { randomButton.x + 130 + 10, randomButton.y, 120, 40 };
    


    // Scrolling variables
    float scrollY = 0;
    float maxScroll = 0;
    float scrollBarHeight = 0;
    float scrollBarPos = 0;
    bool isDraggingScrollBar = false;
    
    // Input variables
    std::string inputText = "";
    std::string vertexCountInput = std::to_string(numVertices);
    std::string edgeCountInput = "";
    bool isEdgeCountTyping = false;
    bool isVertexInputTyping = false;
    bool isInputContainerActive = false;
    bool isVertexInput = false; // We'll start directly with the graph input
    
    std::vector<std::string> inputLines = {"", "// Enter graph data, one line per entry:", "// - Single number: vertex with no connections", "// - Two numbers: edge with weight 1", "// - Three numbers: u v w for edge with weight w", ""};
    int currentLine = inputLines.size() - 1;
    
    std::string errorMsg = "";
    float errorMsgTimer = 0;
    
    // Add cursor position tracking for text editing
    int cursorPosition = 0; // Position within the current line


    GuiWindowFileDialogState fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());

    SetTargetFPS(120);
    bool firstRun = true;
    while (!WindowShouldClose()) {
        Vector2 mousePoint = GetMousePosition();
        
        if (CheckCollisionPointRec(mousePoint, backBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            return;
        }
        if (CheckCollisionPointRec(mousePoint, hamburgerIcon) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            isMenuOpen = !isMenuOpen;
        }
        
        // Vertex count input box handling
        if (CheckCollisionPointRec(mousePoint, vertexInputBox) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            isVertexInputTyping = true;
            isInputContainerActive = false;
            isEdgeCountTyping = false;
            cursorPosition = vertexCountInput.length(); // Place cursor at the end
        }
        
        if (isVertexInputTyping) {
            int key = GetCharPressed();
            while (key > 0) {
                if ((key >= '0' && key <= '9') && vertexCountInput.length() < 5) {
                    // Insert character at cursor position instead of appending
                    vertexCountInput.insert(cursorPosition, 1, (char)key);
                    cursorPosition++; // Move cursor forward
                }
                key = GetCharPressed();
            }
            
            // Handle backspace - only delete if cursor isn't at the beginning
            if (IsKeyPressed(KEY_BACKSPACE) && !vertexCountInput.empty() && cursorPosition > 0) {
                vertexCountInput.erase(cursorPosition - 1, 1);
                cursorPosition--;
            }
            
            // Handle delete key - delete character after cursor
            if (IsKeyPressed(KEY_DELETE) && cursorPosition < vertexCountInput.length()) {
                vertexCountInput.erase(cursorPosition, 1);
            }
            
            // Handle left/right arrows for cursor movement
            if (IsKeyPressed(KEY_LEFT) && cursorPosition > 0) {
                cursorPosition--;
            }
            if (IsKeyPressed(KEY_RIGHT) && cursorPosition < vertexCountInput.length()) {
                cursorPosition++;
            }
            
            if (IsKeyPressed(KEY_ENTER)) {
                int newVertexCount = std::stoi(vertexCountInput.empty() ? "0" : vertexCountInput);
                if (newVertexCount >= 1) {
                    numVertices = newVertexCount;
                    mst = MST(numVertices, 0);
                    mst.initializeVertexPositions();
                    isVertexInputTyping = false;
                } else {
                    errorMsg = "Please enter a number between 3 and 15";
                    errorMsgTimer = 2.0f;
                }
            }
        }
        
        // Input container handling - enhanced with cursor positioning
        if (CheckCollisionPointRec(mousePoint, inputContainer) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            isInputContainerActive = true;
            isVertexInputTyping = false;
            isEdgeCountTyping = false;
            
            // Calculate which line was clicked
            float relativeY = mousePoint.y - inputContainer.y + scrollY;
            int clickedLineIndex = (int)(relativeY / 22);
            
            // Make sure the clicked line is valid
            if (clickedLineIndex >= 0 && clickedLineIndex < inputLines.size()) {
                currentLine = clickedLineIndex;
                
                // Calculate cursor position within the line based on X coordinate
                float lineStartX = inputContainer.x + 10;
                float relativeX = mousePoint.x - lineStartX;
                
                // Find the closest character position based on text measurement
                cursorPosition = 0;
                float accumulatedWidth = 0;
                for (size_t i = 0; i <= inputLines[currentLine].length(); i++) {
                    float charWidth = MeasureText(
                        (i > 0) ? inputLines[currentLine].substr(i-1, 1).c_str() : " ", 
                        20
                    );
                    
                    if (i > 0) {
                        accumulatedWidth += charWidth;
                    }
                    
                    // If we've gone past the clicked position or reached the end
                    if (accumulatedWidth >= relativeX || i == inputLines[currentLine].length()) {
                        cursorPosition = i;
                        break;
                    }
                }
            }
        }
        
        // Handle text input in the container with enhanced cursor position and clipboard
        if (isInputContainerActive) {
            int key = GetCharPressed();
            while (key > 0) {
                if ((key >= ' ' && key <= '~') && inputLines[currentLine].length() < 40) {
                    // Insert character at cursor position
                    inputLines[currentLine].insert(cursorPosition, 1, (char)key);
                    cursorPosition++;
                }
                key = GetCharPressed();
            }
            
            // Handle clipboard operations
            bool ctrlPressed = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
            
            // Paste from clipboard (Ctrl+V)
            if (ctrlPressed && IsKeyPressed(KEY_V)) {
                const char* clipText = GetClipboardText();
                if (clipText != nullptr && clipText[0] != '\0') {
                    std::string clipboardText = clipText;
                    
                    // Handle multiline paste by splitting at newlines
                    size_t pos = 0;
                    size_t lastPos = 0;
                    std::string delimiter = "\n";
                    bool isFirstLine = true;
                    
                    while ((pos = clipboardText.find(delimiter, lastPos)) != std::string::npos) {
                        std::string line = clipboardText.substr(lastPos, pos - lastPos);
                        
                        // Remove carriage returns if present
                        if (!line.empty() && line.back() == '\r') {
                            line.pop_back();
                        }
                        
                        if (isFirstLine) {
                            // For first line, insert at cursor position in current line
                            if (inputLines[currentLine].length() + line.length() <= 40) {
                                inputLines[currentLine].insert(cursorPosition, line);
                                cursorPosition += line.length();
                            } else {
                                // Truncate if too long
                                int allowedChars = 40 - inputLines[currentLine].length();
                                if (allowedChars > 0) {
                                    inputLines[currentLine].insert(cursorPosition, line.substr(0, allowedChars));
                                    cursorPosition += allowedChars;
                                }
                            }
                            isFirstLine = false;
                        } else {
                            // For subsequent lines, insert as new lines
                            currentLine++;
                            inputLines.insert(inputLines.begin() + currentLine, line.substr(0, std::min(line.length(), size_t(40))));
                            cursorPosition = std::min(line.length(), size_t(40));
                        }
                        
                        lastPos = pos + delimiter.length();
                    }
                    
                    // Handle the last line or single line case
                    std::string lastLine = clipboardText.substr(lastPos);
                    // Remove carriage returns if present
                    if (!lastLine.empty() && lastLine.back() == '\r') {
                        lastLine.pop_back();
                    }
                    
                    if (isFirstLine) {
                        // If this is the only line, insert at cursor
                        if (inputLines[currentLine].length() + lastLine.length() <= 40) {
                            inputLines[currentLine].insert(cursorPosition, lastLine);
                            cursorPosition += lastLine.length();
                        } else {
                            // Truncate if too long
                            int allowedChars = 40 - inputLines[currentLine].length();
                            if (allowedChars > 0) {
                                inputLines[currentLine].insert(cursorPosition, lastLine.substr(0, allowedChars));
                                cursorPosition += allowedChars;
                            }
                        }
                    } else if (!lastLine.empty()) {
                        // If we already processed lines, add this as a new line
                        currentLine++;
                        inputLines.insert(inputLines.begin() + currentLine, lastLine.substr(0, std::min(lastLine.length(), size_t(40))));
                        cursorPosition = std::min(lastLine.length(), size_t(40));
                    }
                    
                    // Update max scroll after adding new lines
                    maxScroll = std::max(0.0f, (inputLines.size() * 22) - inputContainer.height + 20);
                    
                    // Auto scroll to show current line
                    float lineY = currentLine * 22;
                    if (lineY < scrollY) {
                        scrollY = lineY;
                    } else if (lineY > scrollY + inputContainer.height - 22) {
                        scrollY = lineY - inputContainer.height + 22;
                    }
                    scrollY = Clamp(scrollY, 0, maxScroll);
                }
            }
            
            // Copy selected text to clipboard (Ctrl+C)
            if (ctrlPressed && IsKeyPressed(KEY_C)) {
                if (!inputLines[currentLine].empty()) {
                    // For now, just copy the current line
                    SetClipboardText(inputLines[currentLine].c_str());
                }
            }
            
            // Cut selected text (Ctrl+X)
            if (ctrlPressed && IsKeyPressed(KEY_X)) {
                if (!inputLines[currentLine].empty()) {
                    // Copy to clipboard then delete
                    SetClipboardText(inputLines[currentLine].c_str());
                    inputLines[currentLine] = "";
                    cursorPosition = 0;
                }
            }
            
            // Select all text (Ctrl+A) - just marking for future extension
            if (ctrlPressed && IsKeyPressed(KEY_A)) {
                // For now, just move cursor to end of line
                cursorPosition = inputLines[currentLine].length();
            }
            
            // Handle backspace - delete character before cursor
            if (IsKeyPressed(KEY_BACKSPACE)) {
                if (cursorPosition > 0) {
                    inputLines[currentLine].erase(cursorPosition - 1, 1);
                    cursorPosition--;
                } else if (currentLine > 0) {
                    // If at beginning of line and not the first line, 
                    // move to end of previous line
                    cursorPosition = inputLines[currentLine - 1].length();
                    inputLines[currentLine - 1] += inputLines[currentLine]; // Join lines
                    inputLines.erase(inputLines.begin() + currentLine);
                    currentLine--;
                }
            }
            
            // Handle delete key - delete character after cursor
            if (IsKeyPressed(KEY_DELETE)) {
                if (cursorPosition < inputLines[currentLine].length()) {
                    inputLines[currentLine].erase(cursorPosition, 1);
                } else if (currentLine < inputLines.size() - 1) {
                    // If at end of line and not the last line, join with next line
                    inputLines[currentLine] += inputLines[currentLine + 1];
                    inputLines.erase(inputLines.begin() + currentLine + 1);
                }
            }
            
            // Handle left/right arrow keys for cursor movement
            if (IsKeyPressed(KEY_LEFT)) {
                if (cursorPosition > 0) {
                    cursorPosition--;
                } else if (currentLine > 0) {
                    // Move to end of previous line
                    currentLine--;
                    cursorPosition = inputLines[currentLine].length();
                }
            }
            if (IsKeyPressed(KEY_RIGHT)) {
                if (cursorPosition < inputLines[currentLine].length()) {
                    cursorPosition++;
                } else if (currentLine < inputLines.size() - 1) {
                    // Move to beginning of next line
                    currentLine++;
                    cursorPosition = 0;
                }
            }
            
            // Handle up/down arrow keys for line navigation
            if (IsKeyPressed(KEY_UP) && currentLine > 0) {
                currentLine--;
                // Keep cursor at same horizontal position if possible
                cursorPosition = std::min(cursorPosition, (int)inputLines[currentLine].length());
            }
            if (IsKeyPressed(KEY_DOWN) && currentLine < inputLines.size() - 1) {
                currentLine++;
                // Keep cursor at same horizontal position if possible
                cursorPosition = std::min(cursorPosition, (int)inputLines[currentLine].length());
            }
            
            if (IsKeyPressed(KEY_ENTER)) {
                // Create a new line with content after the cursor
                std::string remainder = "";
                if (cursorPosition < inputLines[currentLine].length()) {
                    remainder = inputLines[currentLine].substr(cursorPosition);
                    inputLines[currentLine].erase(cursorPosition);
                }
                
                // Insert the new line after the current one
                inputLines.insert(inputLines.begin() + currentLine + 1, remainder);
                currentLine++;
                cursorPosition = 0;
                
                // Always recreate the graph from scratch with all valid lines
                mst = MST(numVertices, 0);
                
                // Force fixed position mode to prevent bouncing effects
                mst.initializeVertexPositions();
                
                bool validEdgesFound = false;
                // Process all lines of input to rebuild the graph
                for (size_t i = 0; i < inputLines.size(); i++) {
                    if (processInputLine(inputLines[i], mst, numVertices, errorMsg)) {
                        validEdgesFound = true;
                    }
                    
                    if (!errorMsg.empty()) {
                        errorMsg = "Error at line " + std::to_string(i+1) + ": " + errorMsg;
                        errorMsgTimer = 2.0f;
                        errorMsg = "";  // Clear for next iteration
                    }
                }
                
                maxScroll = std::max(0.0f, (inputLines.size() * 22) - inputContainer.height + 20);
                // Auto scroll to show current line
                float lineY = currentLine * 22;
                if (lineY < scrollY) {
                    // Line is above visible area
                    scrollY = lineY;
                } else if (lineY > scrollY + inputContainer.height - 22) {
                    // Line is below visible area
                    scrollY = lineY - inputContainer.height + 22;
                }
                scrollY = Clamp(scrollY, 0, maxScroll);
            }
            firstRun = false;
        }



        //file dialog-----------------------------------
           // Add file load button
           //Rectangle fileButton = { 500, 500, 200, 50 };
        if (CheckCollisionPointRec(GetMousePosition(), fileButton)) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                fileDialogState.windowActive = !fileDialogState.windowActive;
            }
        }
    
        // Handle file dialog
        if (fileDialogState.windowActive) {
            cout << "ok" << '\n';
            GuiWindowFileDialog(&fileDialogState);
    
            if (fileDialogState.SelectFilePressed) {
                std::string selectedFilePath = std::string(fileDialogState.dirPathText) + "\\" + 
                                             std::string(fileDialogState.fileNameText);
                
                std::ifstream inputFile(selectedFilePath);
                if (inputFile.is_open()) {
                    // Clear existing input lines
                    inputLines.clear();
                    inputLines.push_back("");
                    inputLines.push_back("// Loaded from file: " + std::string(fileDialogState.fileNameText));
                    inputLines.push_back("");
    
                    // Read file contents
                    std::string line;
                    while (std::getline(inputFile, line)) {
                        inputLines.push_back(line);
                    }
                    inputFile.close();
    
                    // Create new MST instance with loaded data
                    mst = MST(numVertices, 0);
                    mst.initializeVertexPositions();
    
                    // Process each line to build the graph
                    std::string errorMsg;
                    for (const auto& line : inputLines) {
                        processInputLine(line, mst, numVertices, errorMsg);
                        if (!errorMsg.empty()) {
                            // Handle error if needed
                            errorMsg = "";
                        }
                    }
    
                    // Update scroll position to see new content
                    scrollY = 0;
                    maxScroll = std::max(0.0f, (inputLines.size() * 22) - inputContainer.height + 20);
                }
    
                fileDialogState.SelectFilePressed = false;
            }
        }


        //file dialog-----------------------------------end
        
        // Edge count box for random graph generation
        if (CheckCollisionPointRec(mousePoint, edgeCountBox) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            isEdgeCountTyping = true;
            isInputContainerActive = false;
            isVertexInputTyping = false;
            edgeCountInput = "";
            cursorPosition = 0; // Reset cursor for edge count input
        }
        
        if (isEdgeCountTyping) {
            int key = GetCharPressed();
            while (key > 0) {
                if ((key >= '0' && key <= '9') && edgeCountInput.length() < 3) {
                    // Ensure cursorPosition is valid before inserting
                    cursorPosition = std::min(cursorPosition, (int)edgeCountInput.length());
                    edgeCountInput.insert(cursorPosition, 1, (char)key);
                    cursorPosition++;
                }
                key = GetCharPressed();
            }
            
            // Handle backspace and left/right arrows for cursor movement
            if (IsKeyPressed(KEY_BACKSPACE) && !edgeCountInput.empty() && cursorPosition > 0) {
                edgeCountInput.erase(cursorPosition - 1, 1);
                cursorPosition--;
            }
            if (IsKeyPressed(KEY_LEFT) && cursorPosition > 0) {
                cursorPosition--;
            }
            if (IsKeyPressed(KEY_RIGHT) && cursorPosition < edgeCountInput.length()) {
                cursorPosition++;
            }
            
            if (IsKeyPressed(KEY_ENTER)) {
                isEdgeCountTyping = false;
            }
        }

        // Handle scrolling in the input container - FIXED scroll handling
        if (CheckCollisionPointRec(mousePoint, inputContainer)) {
            float wheel = GetMouseWheelMove();
            if (wheel != 0) {
                // Adjust scrolling speed
                scrollY -= wheel * 40;
                
                // Limit scrolling
                maxScroll = std::max(0.0f, (inputLines.size() * 22) - inputContainer.height + 20);
                scrollY = Clamp(scrollY, 0, maxScroll);
            }
        }
        
        // FIX: Run MST button handling
        if (CheckCollisionPointRec(mousePoint, runMSTButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            // Create a fresh MST instance for algorithm execution
            // MST tempMST(numVertices, 0);
            // tempMST.initializeVertexPositions();
            isMenuOpen = false; // Close menu if open
            
            // // Process each line of input
            // bool validEdgesFound = false;
            // errorMsg = ""; // Clear any previous error message
            
            // for (size_t i = 0; i < inputLines.size(); i++) {
            //     std::string lineErrorMsg = "";
            //     if (processInputLine(inputLines[i], tempMST, numVertices, lineErrorMsg)) {
            //         validEdgesFound = true;
            //     }
                
            //     if (!lineErrorMsg.empty()) {
            //         errorMsg = "Error at line " + std::to_string(i+1) + ": " + lineErrorMsg;
            //         errorMsgTimer = 2.0f;
            //         break; // Stop at first error
            //     }
            // }
            
            // if (validEdgesFound && errorMsg.empty()) {
            //     // Run MST on the freshly created graph
            //     mst = tempMST;
            //     mst.setPseudoCodeKruskal();
            //     mst.runKruskal();
            //     mst.initializeVertexPositions();
            //     mst.updateVertexPositions();
                
            //     // Replace the old graph with the new one
            // } else if (!validEdgesFound) {
            //     errorMsg = "No valid edges to run MST on";
            //     errorMsgTimer = 2.0f;
            // }
            mst.setPseudoCodeKruskal();
            mst.runKruskal();
            //mst.initializeVertexPositions();
            //mst.updateVertexPositions();
        }
        
        // FIX: Random button handling
        if (CheckCollisionPointRec(mousePoint, randomButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || firstRun == true) {
            if (edgeCountInput.empty()  && firstRun == false) {
                errorMsg = "Please enter number of edges first!";
                errorMsgTimer = 2.0f;
            } else {
                int maxEdges = (numVertices * (numVertices - 1)) / 2;
                int edgeCount = maxEdges - 2; 
                if(edgeCountInput.length() > 0){
                    edgeCount = std::stoi(edgeCountInput);
                }

                if (edgeCount > maxEdges) {
                    errorMsg = "Max edges allowed: " + std::to_string(maxEdges);
                    errorMsgTimer = 2.0f;
                } else {
                    // Create new MST with random edges
                    mst = MST(numVertices, 0);
                    mst.initializeVertexPositions();
                    
                    // Clear input lines and generate new lines for the random graph
                    inputLines = {"", "// Random graph with " + std::to_string(edgeCount) + " edges:", ""};
                    currentLine = inputLines.size() - 1;
                    cursorPosition = 0;
                    
                    // Generate random edges
                    std::vector<std::pair<int,int>> usedPairs;
                    for (int i = 0; i < edgeCount; i++) {
                        int u, v, w;
                        bool found;
                        do {
                            u = GetRandomValue(1, numVertices);
                            v = GetRandomValue(1, numVertices);
                            found = false;
                            if (u != v) {
                                for (auto &p : usedPairs) {
                                    if ((p.first == u && p.second == v) || 
                                        (p.first == v && p.second == u)) {
                                        found = true;
                                        break;
                                    }
                                }
                                if (!found) {
                                    usedPairs.push_back({u, v});
                                    w = GetRandomValue(1, 20);
                                    mst.addEdge(u, v, w);
                                    // Add the edge to the input lines
                                    inputLines.push_back(std::to_string(u) + " " + std::to_string(v) + " " + std::to_string(w));
                                }
                            }
                        } while (found || u == v);
                    }
                    
                    // Add empty line at the end
                    inputLines.push_back("");
                    
                    // Update current line, scroll position and max scroll
                    maxScroll = std::max(0.0f, (inputLines.size() * 22) - inputContainer.height + 20);
                    scrollY = maxScroll; // Scroll to bottom
                }
            }
            firstRun = false;
        }

        // Update vertex positions with interactive physics
        mst.updateVertexPositions();
        
        if (errorMsgTimer > 0) {
            errorMsgTimer -= GetFrameTime();
        }
        
        // Calculate scroll bar parameters
        float containerHeight = inputContainer.height;
        float contentHeight = inputLines.size() * 22;
        scrollBarHeight = containerHeight * (containerHeight / contentHeight);
        if (scrollBarHeight > containerHeight) scrollBarHeight = containerHeight;
        scrollBarPos = (scrollY / maxScroll) * (containerHeight - scrollBarHeight);
        if (maxScroll <= 0) scrollBarPos = 0;
        
        // Drawing code
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawBackButton(backBtn);

            DrawHamburgerIconChPlay(hamburgerIcon);
            // Draw MST visualization
        
                mst.drawDSU();
                mst.drawEdgeList();
            
            mst.drawGraph();
            const char* title = "Minimum Spanning Tree Visualization";
            int titleWidth = MeasureText(title, 30);
            DrawText(title, CENTER_X - titleWidth/2, TITLE_Y, 30, DARKGRAY);
            if(isMenuOpen){
                DrawSidebar(sidebar);
                DrawHamburgerIconChPlay(hamburgerIcon);
            }
            if(isMenuOpen){
                
                // Draw vertex count input box with cursor
                Color vertexBoxColor = isVertexInputTyping ? SKYBLUE : WHITE;
                DrawRectangleRec(vertexInputBox, vertexBoxColor);
                DrawRectangleRoundedLines(vertexInputBox, 0.1f, 0, ColorAlpha(DARKGRAY, 0.5f));
                DrawText(vertexCountInput.c_str(), vertexInputBox.x + 10, vertexInputBox.y + 5, 20, BLACK);
                // Draw cursor for vertex input box
                if (isVertexInputTyping && ((int)(GetTime() * 2) % 2)) {
                    std::string textBeforeCursor = vertexCountInput.substr(0, cursorPosition);
                    int cursorX = vertexInputBox.x + 10 + MeasureText(textBeforeCursor.c_str(), 20);
                    DrawRectangle(cursorX, vertexInputBox.y + 5, 2, 20, BLACK);
                }
                else{
                    DrawText("Vertices:", vertexInputBox.x + 50, vertexInputBox.y + 5, 20, DARKGRAY);

                }
                
                // Draw Run MST button
                Color runMSTColor = CheckCollisionPointRec(mousePoint, runMSTButton) ? 
                                (Color){41, 128, 185, 255} : (Color){52, 152, 219, 255};
                DrawRectangleGradientH((int)runMSTButton.x, (int)runMSTButton.y, 
                                    (int)runMSTButton.width, (int)runMSTButton.height,
                                    runMSTColor, ColorBrightness(runMSTColor, 0.7f));
                DrawRectangleRoundedLines(runMSTButton, 0.2f, 0, (Color){41, 128, 185, 255});
                
                const char* buttonText = "Run MST";
                int btnTextWidth = MeasureText(buttonText, 20);
                DrawText(buttonText, 
                        (int)(runMSTButton.x + (runMSTButton.width - btnTextWidth)/2),
                        (int)(runMSTButton.y + 10), 20, WHITE);
                
                // Draw scrollable input container
                Color containerColor = isInputContainerActive ? ColorAlpha(SKYBLUE, 0.1f) : WHITE;
                DrawRectangleRounded(inputContainer, 0.1f, 0, containerColor);
                DrawRectangleRoundedLines(inputContainer, 0.1f, 0, ColorAlpha(DARKGRAY, 0.5f));
                
                // Scissor test to only draw text inside container
                BeginScissorMode(inputContainer.x, inputContainer.y, inputContainer.width, inputContainer.height);
                
                // Draw each line of text with cursor
                for (size_t i = 0; i < inputLines.size(); i++) {
                    float yPos = inputContainer.y + 10 + (i * 22) - scrollY;
                    
                    // Only draw visible lines
                    if (yPos >= inputContainer.y - 22 && yPos <= inputContainer.y + inputContainer.height) {
                        // Highlight current line
                        if (i == currentLine && isInputContainerActive) {
                            DrawRectangle(inputContainer.x + 5, yPos - 2, inputContainer.width - 30, 22, ColorAlpha(SKYBLUE, 0.2f));
                        }
                        
                        // Draw text with different colors based on prefix
                        if (inputLines[i].empty()) {
                            // Empty line
                        } else if (inputLines[i][0] == '/') {
                            // Comment line
                            DrawText(inputLines[i].c_str(), inputContainer.x + 10, yPos, 20, DARKGRAY);
                        } else {
                            // Normal input line
                            DrawText(inputLines[i].c_str(), inputContainer.x + 10, yPos, 20, BLACK);
                        }
                        
                        // Draw cursor on current line
                        if (i == currentLine && isInputContainerActive && ((int)(GetTime() * 2) % 2)) {
                            // Measure text width up to cursor position for proper placement
                            std::string textBeforeCursor = inputLines[i].substr(0, cursorPosition);
                            int cursorX = inputContainer.x + 10 + MeasureText(textBeforeCursor.c_str(), 20);
                            DrawRectangle(cursorX, yPos, 2, 20, BLACK);
                        }
                    }
                }
                
                EndScissorMode();
                
                // Draw scroll bar if needed
                if (contentHeight > containerHeight) {
                    DrawRectangle(
                        inputContainer.x + inputContainer.width - 20, 
                        inputContainer.y, 
                        20, 
                        inputContainer.height, 
                        ColorAlpha(LIGHTGRAY, 0.5f)
                    );
                    
                    DrawRectangle(
                        inputContainer.x + inputContainer.width - 18, 
                        inputContainer.y + scrollBarPos, 
                        16, 
                        scrollBarHeight, 
                        ColorAlpha(DARKGRAY, 0.5f)
                    );
                }
                
                // Draw random button and edge count input
                Color randomColor = CheckCollisionPointRec(mousePoint, randomButton) ? 
                                (Color){155, 89, 182, 255} : (Color){142, 68, 173, 255};
                DrawRectangleGradientH(randomButton.x, randomButton.y, 
                                    randomButton.width, randomButton.height,
                                    randomColor, ColorBrightness(randomColor, 0.7f));
                DrawRectangleRoundedLines(randomButton, 0.2f, 0, (Color){142, 68, 173, 255});
                DrawText("Random", randomButton.x + 20, randomButton.y + 10, 20, WHITE);
                
                // Draw edge count input box with cursor
                Color edgeBoxColor = isEdgeCountTyping ? SKYBLUE : WHITE;
                DrawRectangleRec(edgeCountBox, edgeBoxColor);
                DrawRectangleRoundedLines(edgeCountBox, 0.1f, 0, ColorAlpha(DARKGRAY, 0.5f));
                if (edgeCountInput.empty() && !isEdgeCountTyping) {
                    DrawText("# edges", edgeCountBox.x + 10, edgeCountBox.y + 10, 20, ColorAlpha(DARKGRAY, 0.5f));
                } else {
                    DrawText(edgeCountInput.c_str(), edgeCountBox.x + 10, edgeCountBox.y + 10, 20, BLACK);
                    
                    // Draw cursor for edge count input
                    if (isEdgeCountTyping && ((int)(GetTime() * 2) % 2)) {
                        std::string textBeforeCursor = edgeCountInput.substr(0, cursorPosition);
                        int cursorX = edgeCountBox.x + 10 + MeasureText(textBeforeCursor.c_str(), 20);
                        DrawRectangle(cursorX, edgeCountBox.y + 10, 2, 20, BLACK);
                    }
                }
                
                // Draw error message if any
                if (errorMsgTimer > 0) {
                    DrawText(errorMsg.c_str(), inputContainer.x, 
                            inputContainer.y + inputContainer.height + 10, 20, 
                            ColorAlpha(RED, errorMsgTimer/2.0f));
                }
                // DrawText("Load File", fileButton.x + 60, fileButton.y + 15, 20, WHITE);
                // if (CheckCollisionPointRec(GetMousePosition(), fileButton)) {
                //     DrawRectangleRounded(fileButton, 0.2f, 8, ColorBrightness(MD_PRIMARY, 0.2f));
                // } else {
                //     DrawRectangleRounded(fileButton, 0.2f, 8, MD_PRIMARY);
                // }
                // // Draw random button and edge count input
                Color fileColor = CheckCollisionPointRec(mousePoint, fileButton) ? 
                (Color){155, 89, 182, 255} : (Color){142, 68, 173, 255};
                DrawRectangleGradientH(fileButton.x, fileButton.y, 
                    fileButton.width, fileButton.height,
                    fileColor, ColorBrightness(fileColor, 0.7f));
                DrawRectangleRoundedLines(fileButton, 0.2f, 0, (Color){142, 68, 173, 255});
                DrawText("Load File", fileButton.x + 20, fileButton.y + 10, 20, WHITE);
            }
            
            
            // Draw progress bar if animation frames are available
            if (!mst.animationFrames.empty()) {
                mst.drawProgressBar();
            }
 
            
        EndDrawing();
    }
    
    CloseWindow();
}

void RunAVLVisualization() {
    Rectangle backBtn = CreateBackButton();
    
    AVL tree;
    int screenWidth = 1920;
    int screenHeight = 1080;
    const float BUTTON_WIDTH = 80;
    const float BUTTON_HEIGHT = 35;
    const float BUTTON_SPACING = 10;
    const float START_X = 800;
    const float START_Y = 30;
    // Existing buttons
    // Hamburger icon: góc trái dưới (padding 10 pixel)
    Rectangle hamburgerIcon = { 10, (float)screenHeight - 100, 40, 40 };
    bool isMenuOpen = false;
    float progressWidth = 500;
    float progressHeight = 10;
    float progressX = (screenWidth - progressWidth) / 2 - 100;
    float progressY = screenHeight - 100;
    
    // Sidebar: toàn bộ chiều cao, chiều rộng 300 pixel (bên trái)
    Rectangle sidebar = { 20 ,630, 290, (float)screenHeight };


    Rectangle insertButton = { sidebar.x + 5, sidebar.y + 100, BUTTON_WIDTH, BUTTON_HEIGHT };
    Rectangle deleteButton = { insertButton.x + BUTTON_WIDTH + BUTTON_SPACING, insertButton.y , BUTTON_WIDTH, BUTTON_HEIGHT };
    Rectangle findButton = { deleteButton.x + BUTTON_WIDTH + BUTTON_SPACING, deleteButton.y, BUTTON_WIDTH, BUTTON_HEIGHT };

    Rectangle insertBox = { insertButton.x, insertButton.y + BUTTON_HEIGHT + 5, BUTTON_WIDTH, BUTTON_HEIGHT };
    Rectangle deleteBox = { deleteButton.x, deleteButton.y + BUTTON_HEIGHT + 5, BUTTON_WIDTH, BUTTON_HEIGHT };
    Rectangle findBox = { findButton.x, findButton.y + BUTTON_HEIGHT + 5, BUTTON_WIDTH, BUTTON_HEIGHT };

    // Add new UI elements for the new features
    //Rectangle pauseResumeButton = { START_X + 4 * (BUTTON_WIDTH + BUTTON_SPACING), START_Y, BUTTON_WIDTH, BUTTON_HEIGHT };
    Rectangle pauseResumeButton = {progressX - 70, progressY - 10, 50, 45};

    // Add speed control buttons
    Rectangle speedDecreaseButton =  {progressX + progressWidth + 20, progressY - 10, 45, 45}; // Larger
    Rectangle speedIncreaseButton ={progressX + progressWidth + 70, progressY - 10, 45, 45}; // Larger
    
    // Add undo/redo buttons
    Rectangle undoButton = {speedIncreaseButton.x + 45 + 10, speedIncreaseButton.y , 45, 45 };
    Rectangle redoButton = {undoButton.x + 45 + 10, speedIncreaseButton.y, 45, 45 };


    // Add traversal buttons
    const float TRAVERSAL_START_X = 300; // Đặt ở bên trái màn hình
    const float TRAVERSAL_Y = 100;
    const float TRAVERSAL_BUTTON_WIDTH = 120;
    const float TRAVERSAL_BUTTON_HEIGHT = 40;
    const float TRAVERSAL_SPACING = 10;

    Rectangle inorderButton = { insertBox.x, insertBox.y + insertBox.height + 15, BUTTON_WIDTH, BUTTON_HEIGHT };
    Rectangle preorderButton = { deleteBox.x, deleteBox.y + deleteBox.height + 15, BUTTON_WIDTH, BUTTON_HEIGHT};
    Rectangle postorderButton = { findBox.x, findBox.y + findBox.height + 15, BUTTON_WIDTH, BUTTON_HEIGHT};

    // Add new buttons for backward/forward navigation
    Rectangle forwardButton = {progressX - 130, progressY - 10, 55, 45}; // Much larger
    Rectangle backwardButton = {progressX - 195, progressY - 10, 55, 45}; // Much larger
    // Add initialization buttons near top of screen
    Rectangle createEmptyBtn = { inorderButton.x, inorderButton.y + BUTTON_HEIGHT + BUTTON_SPACING, BUTTON_WIDTH, BUTTON_HEIGHT };
    Rectangle createRandomBtn = { preorderButton.x  , preorderButton.y + BUTTON_HEIGHT + BUTTON_SPACING, BUTTON_WIDTH, BUTTON_HEIGHT };
    Rectangle sizeInputBox = { createRandomBtn.x + createRandomBtn.width + BUTTON_SPACING, createRandomBtn.y, BUTTON_WIDTH, BUTTON_HEIGHT };
    std::string sizeInput = "5";  // Default size
    bool isSizeInputActive = false;

    bool isTyping = false;
    std::string inputText = "";
    std::string operation = "";
    Rectangle* activeTextBox = nullptr;
    
    // Variables for animation playback
    int currentStep = 0;
    std::vector<AVLTreeState> currentAnimation;
    bool isPlayingAnimation = false;
    int frameSkip = 1; // Animation speed control (higher means faster)
    
    // Variables for progress bar interaction
    bool isDraggingProgressBar = false;
    Rectangle progressBarRect = (Rectangle){progressX, progressY, progressWidth, progressHeight};

    // Add file dialog state
    GuiWindowFileDialogState fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
    
    // Add file load button next to search button
    Rectangle fileButton = { createEmptyBtn.x, createEmptyBtn.y + createEmptyBtn.height + 15, (postorderButton.x + postorderButton.width - inorderButton.x), BUTTON_HEIGHT };
    vector<int> file_data;
    SetTargetFPS(120);
    while (!WindowShouldClose()) {
        Vector2 mousePoint = GetMousePosition();
        
        if (CheckCollisionPointRec(mousePoint, backBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            return;
        }
        if(CheckCollisionPointRec(mousePoint, hamburgerIcon) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
            isMenuOpen = !isMenuOpen;
        }
        // Handle playback speed controls
        if (CheckCollisionPointRec(mousePoint, speedDecreaseButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (frameSkip > 1) frameSkip--;
        }
        
        if (CheckCollisionPointRec(mousePoint, speedIncreaseButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (frameSkip < 10) frameSkip++;
        }
        
        // Handle undo button
        if (CheckCollisionPointRec(mousePoint, undoButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (tree.canUndo()) {
                tree.undo();
                isPlayingAnimation = false;
                currentAnimation.clear();
            }
        }
        
        // Handle redo button
        if (CheckCollisionPointRec(mousePoint, redoButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (tree.canRedo()) {
                tree.redo();
                isPlayingAnimation = false;
                currentAnimation.clear();
            }
        }

        // Handle animation playback
        if (isPlayingAnimation && !isDraggingProgressBar) {
            currentStep += frameSkip;
            if (currentStep >= currentAnimation.size()) {
                currentStep = currentAnimation.size() - 1;
                isPlayingAnimation = false;
            }
        }

        // Handle pause/resume button
        if (CheckCollisionPointRec(mousePoint, pauseResumeButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            isPlayingAnimation = !isPlayingAnimation;
        }

        // Handle traversal buttons
        if (CheckCollisionPointRec(mousePoint, inorderButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            tree.inorderTraversal();
            currentAnimation = tree.animationStates;
            currentStep = 0;
            isPlayingAnimation = true;
        }
        
        if (CheckCollisionPointRec(mousePoint, preorderButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            tree.preorderTraversal();
            currentAnimation = tree.animationStates;
            currentStep = 0;
            isPlayingAnimation = true;
        }
        
        if (CheckCollisionPointRec(mousePoint, postorderButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            tree.postorderTraversal();
            currentAnimation = tree.animationStates;
            currentStep = 0;
            isPlayingAnimation = true;
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(mousePoint, insertButton)) {
                isTyping = true;
                operation = "insert";
                activeTextBox = &insertBox;
                inputText = "";
                currentAnimation.clear();
            }
            else if (CheckCollisionPointRec(mousePoint, deleteButton)) {
                isTyping = true;
                operation = "delete";
                activeTextBox = &deleteBox;
                inputText = "";
                currentAnimation.clear();
            }
            else if (CheckCollisionPointRec(mousePoint, findButton)) {
                isTyping = true;
                operation = "find";
                activeTextBox = &findBox;
                inputText = "";
                currentAnimation.clear();
            }
            else if (CheckCollisionPointRec(mousePoint, createEmptyBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                tree.createEmpty();
                currentAnimation.clear();
                currentStep = 0;
            }

            if (CheckCollisionPointRec(mousePoint, createRandomBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                int size = std::stoi(sizeInput.empty() ? "5" : sizeInput);
                if (size > 0) { // Limit size for performance
                    cout << "sz =-------------------------------------- " << size << '\n';
                    tree.createRandom(size);
                    currentAnimation = tree.animationStates;
                    currentStep = 0;
                    isPlayingAnimation = true;
                }
            }

            // Handle size input box
            if (CheckCollisionPointRec(mousePoint, sizeInputBox) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                isTyping = true;
                operation = "size";
                activeTextBox = &sizeInputBox;
                inputText = sizeInput;
            }
        }

        if (isTyping) {
            if (operation == "size") {
                int key = GetCharPressed();
                while (key > 0) {
                    if ((key >= '0' && key <= '9') && inputText.length() < 2) {
                        inputText.push_back((char)key);
                    }
                    key = GetCharPressed();
                }
                
                if (IsKeyPressed(KEY_BACKSPACE) && !inputText.empty()) {
                    inputText.pop_back();
                }
                
                if (IsKeyPressed(KEY_ENTER)) {
                    sizeInput = inputText;
                    isTyping = false;
                    operation = "";
                    activeTextBox = nullptr;
                }
            }
            else {
                int key = GetCharPressed();
                while (key > 0) {
                    if ((key >= '0' && key <= '9') && inputText.length() < 5)
                        inputText.push_back((char)key);
                    key = GetCharPressed();
                }
                if (IsKeyPressed(KEY_BACKSPACE) && !inputText.empty())
                    inputText.pop_back();
                if (IsKeyPressed(KEY_ENTER) && !inputText.empty()) {
                    int value = atoi(inputText.c_str());
                    
                    // Clear previous animation
                    tree.clearAnimationStates();
                    currentStep = 0;
                    
                    if (operation == "insert") {
                        tree.setPseudoCodeInsert(value);
                        tree.insert(value);
                        currentAnimation = tree.animationStates;
                    }
                    else if (operation == "delete") {
                        tree.deleteKey(value);
                        currentAnimation = tree.animationStates;
                    }
                    else if (operation == "find") {
                        tree.find(value);
                        currentAnimation = tree.animationStates;
                    }
                   
                    
                    // Start animation playback only if there are animation frames
                    if (!currentAnimation.empty()) {
                        isPlayingAnimation = true;
                    } else {
                        // If no animation frames were generated, just update the state
                        tree.addStateToHistory();
                    }
                    
                    isTyping = false;
                    operation = "";
                    activeTextBox = nullptr;
                    inputText = "";
                }
            }
        }

        if((int)file_data.size() > 0){
            // Clear previous animation
            tree.clearAnimationStates();
            currentStep = 0;
            int value = file_data.back();
            cout << value << '\n';
            file_data.pop_back();
            // tree.clearAnimationStates();

            tree.setPseudoCodeInsert(value);
            tree.insert(value);
            currentAnimation = tree.animationStates;
            currentStep = 0;
            isPlayingAnimation = true;
            //tree.historyStates.clear();
            
        }

        // Only animate nodes if we're NOT playing an animation AND the current animation is empty
        // This prevents node animation during playback which could cause double execution
        if (!isPlayingAnimation && currentAnimation.empty()) {
            //tree.animateNodes(tree.getRoot());
        }
        
        // Handle progress bar interaction - only when we have animation frames
        if (!currentAnimation.empty()) {
            
            // Handle mouse press on progress bar
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && 
                CheckCollisionPointRec(GetMousePosition(), progressBarRect)) {
                isDraggingProgressBar = true;
            }
            
            // Handle mouse drag on progress bar
            if (isDraggingProgressBar) {
                if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                    // Calculate new position based on mouse position
                    float mouseX = GetMousePosition().x;
                    float progress = (mouseX - progressX) / progressWidth;
                    
                    // Clamp progress between 0 and 1
                    progress = progress < 0 ? 0 : (progress > 1 ? 1 : progress);
                    
                    // Calculate new frame position
                    currentStep = (int)(progress * (currentAnimation.size() - 1));
                    
                    // Ensure currentStep is within valid range
                    currentStep = currentStep < 0 ? 0 : 
                                 (currentStep >= currentAnimation.size() ? 
                                  currentAnimation.size() - 1 : currentStep);
                    
                    // Pause animation while scrubbing
                    isPlayingAnimation = false;
                } else {
                    // Release drag when mouse button is released
                    isDraggingProgressBar = false;
                }
            }
        }

        // Handle backward button - jump to last frame of previous highlighted line
        if (CheckCollisionPointRec(mousePoint, backwardButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (!currentAnimation.empty() && currentStep > 0) {
                // Get current highlighted line
                int currentHighlightedLine = currentAnimation[currentStep].highlightedLine;
                
                // First find the previous different highlighted line
                int previousLine = -1;
                int targetStep = currentStep;
                
                while (targetStep >= 0) {
                    if (currentAnimation[targetStep].highlightedLine != currentHighlightedLine) {
                        previousLine = currentAnimation[targetStep].highlightedLine;
                        break;
                    }
                    targetStep--;
                }   
                // Jump to the last frame of the previous line
                currentStep = max(0, targetStep);
            } else {
                // If no previous line, go to the first frame
                currentStep = 0;
            }
            isPlayingAnimation = false; // Pause playback when stepping manually
        }
        
        // Handle forward button - jump to LAST frame of next highlighted line
        if (CheckCollisionPointRec(mousePoint, forwardButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (!currentAnimation.empty() && currentStep < currentAnimation.size() - 1) {
                // Get current highlighted line
                int currentHighlightedLine = currentAnimation[currentStep].highlightedLine;
                
                // First find the next different highlighted line
                int nextLine = -1;
                int firstFrameOfNextLine = -1;
                int targetStep = currentStep;
                
                // Find the first occurrence of a different highlighted line
                while (targetStep < currentAnimation.size()) {
                    if (currentAnimation[targetStep].highlightedLine != currentHighlightedLine) {
                        nextLine = currentAnimation[targetStep].highlightedLine;
                        firstFrameOfNextLine = targetStep;
                        break;
                    }
                    targetStep++;
                }
                
                if (nextLine != -1) {
                    // Now find the last frame with this new highlighted line
                    int lastFrameOfNextLine = firstFrameOfNextLine;
                    targetStep = firstFrameOfNextLine;
                    
                    while (targetStep < currentAnimation.size()) {
                        if (currentAnimation[targetStep].highlightedLine == nextLine) {
                            lastFrameOfNextLine = targetStep;
                        } else {
                            // Found a different highlight, stop here
                            break;
                        }
                        targetStep++;
                    }
                    
                    // Jump to the last frame of the next line
                    currentStep = lastFrameOfNextLine;
                } else {
                    // If no next line, go to the last frame
                    currentStep = currentAnimation.size() - 1;
                }
                
                isPlayingAnimation = false; // Pause playback when stepping manually
            }
        }

        
        float buttonX = 1650;
        float buttonY = 1000;
        if(tree.showPseudoCode == true) buttonY = 500;
        float buttonWidth = 220;
        float buttonHeight = 40;
        
        // Check for collision with the pill-shaped button
        bool collision = false;
        float radius = buttonHeight / 2;
        
        // Check left circle
        if (CheckCollisionPointCircle(mousePoint, (Vector2){buttonX + radius, buttonY + radius}, radius)) {
            collision = true;
        }
        // Check right circle
        else if (CheckCollisionPointCircle(mousePoint, (Vector2){buttonX + buttonWidth - radius, buttonY + radius}, radius)) {
            collision = true;
        }
        // Check middle rectangle
        else if (CheckCollisionPointRec(mousePoint, (Rectangle){buttonX + radius, buttonY, buttonWidth - buttonHeight, buttonHeight})) {
            collision = true;
        }
        
        if (collision && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            tree.showPseudoCode = !tree.showPseudoCode;
        }
    


        BeginDrawing();
            ClearBackground(RAYWHITE);
            
            // Visualization based on current state
            if (!currentAnimation.empty() && currentStep < currentAnimation.size()) {
                // If we have an animation in progress, show the current frame
                tree.visualizeState(currentAnimation[currentStep]);
            } else{
                // Otherwise show the current state of the tree
                tree.draw();
                //tree.visualizeState(currentAnimation[currentAnimation.size() - 1]);
            }
            DrawHamburgerIconChPlay(hamburgerIcon);

            if(isMenuOpen){
                DrawSidebar(sidebar);
                DrawHamburgerIconChPlay(hamburgerIcon);
            
                Color insertColor = CheckCollisionPointRec(mousePoint, insertButton) ? (Color){41, 128, 185, 255} : LIGHTGRAY;
                Color deleteColor = CheckCollisionPointRec(mousePoint, deleteButton) ? (Color){231, 76, 60, 255} : LIGHTGRAY;
                Color findColor = CheckCollisionPointRec(mousePoint, findButton) ? (Color){39, 174, 96, 255} : LIGHTGRAY;
            
                DrawRectangleRec(insertButton, insertColor);
                DrawRectangleRec(deleteButton, deleteColor);
                DrawRectangleRec(findButton, findColor);
                
                DrawText("Insert", (int)insertButton.x + 10, (int)insertButton.y + 8, 20, BLACK);
                DrawText("Delete", (int)deleteButton.x + 10, (int)deleteButton.y + 8, 20, BLACK);
                DrawText("Find", (int)findButton.x + 20, (int)findButton.y + 8, 20, BLACK);

                // Draw traversal buttons
                Color inorderColor = CheckCollisionPointRec(mousePoint, inorderButton) ? 
                                (Color){125, 95, 235, 255} : (Color){125, 95, 255, 200};
                Color preorderColor = CheckCollisionPointRec(mousePoint, preorderButton) ?
                            (Color){125, 95, 235, 255} : (Color){125, 95, 255, 200};
                Color postorderColor = CheckCollisionPointRec(mousePoint, postorderButton) ?
                            (Color){125, 95, 235, 255} : (Color){125, 95, 255, 200};

                DrawRectangleRounded(inorderButton, 0.2f, 8, inorderColor);
                DrawRectangleRounded(preorderButton, 0.2f, 8, preorderColor);
                DrawRectangleRounded(postorderButton, 0.2f, 8, postorderColor);

                DrawText("In", inorderButton.x + 10, inorderButton.y + 10, 20, WHITE);
                DrawText("Pre", preorderButton.x + 10, preorderButton.y + 10, 20, WHITE);
                DrawText("Post", postorderButton.x + 10, postorderButton.y + 10, 20, WHITE);

                // Draw traversal description
                DrawText("Tree Traversals", TRAVERSAL_START_X, TRAVERSAL_Y - 30, 24, DARKGRAY);

                // Draw initialization buttons
                Color initBtnColor = LIGHTGRAY;
                DrawRectangleRec(createEmptyBtn, CheckCollisionPointRec(mousePoint, createEmptyBtn) ? 
                                (Color){41, 128, 185, 255} : initBtnColor);
                DrawRectangleRec(createRandomBtn, CheckCollisionPointRec(mousePoint, createRandomBtn) ? 
                                (Color){41, 128, 185, 255} : initBtnColor);
                DrawText("Empty", createEmptyBtn.x + 15, createEmptyBtn.y + 8, 20, BLACK);
                DrawText("Random", createRandomBtn.x + 10, createRandomBtn.y + 8, 20, BLACK);

                // Draw size input box
                DrawRectangleRec(sizeInputBox, WHITE);
                DrawRectangleLines((int)sizeInputBox.x, (int)sizeInputBox.y, (int)sizeInputBox.width, (int)sizeInputBox.height, BLACK);
                if (operation == "size") {
                    DrawText(inputText.c_str(), (int)sizeInputBox.x + 5, (int)sizeInputBox.y + 8, 20, BLACK);
                } else {
                    DrawText(sizeInput.c_str(), (int)sizeInputBox.x + 5, (int)sizeInputBox.y + 8, 20, BLACK);
                }
                DrawText("Size:", sizeInputBox.x, sizeInputBox.y - 20, 16, DARKGRAY);
    
                if (operation == "insert") {
                    DrawRectangleRec(insertBox, WHITE);
                    DrawRectangleLines((int)insertBox.x, (int)insertBox.y, (int)insertBox.width, (int)insertBox.height, BLACK);
                    DrawText(inputText.c_str(), (int)insertBox.x + 5, (int)insertBox.y + 8, 20, BLACK);
                }
                else if (operation == "delete") {
                    DrawRectangleRec(deleteBox, WHITE);
                    DrawRectangleLines((int)deleteBox.x, (int)deleteBox.y, (int)deleteBox.width, (int)deleteBox.height, BLACK);
                    DrawText(inputText.c_str(), (int)deleteBox.x + 5, (int)deleteBox.y + 8, 20, BLACK);
                }
                else if (operation == "find") {
                    DrawRectangleRec(findBox, WHITE);
                    DrawRectangleLines((int)findBox.x, (int)findBox.y, (int)findBox.width, (int)findBox.height, BLACK);
                    DrawText(inputText.c_str(), (int)findBox.x + 5, (int)findBox.y + 8, 20, BLACK);
                }
                
                // Draw file button with matching style
                Color fileColor = CheckCollisionPointRec(mousePoint, fileButton) ? 
                                 (Color){41, 128, 185, 255} : LIGHTGRAY;
                DrawRectangleRec(fileButton, fileColor);
                DrawText("Load File", (int)fileButton.x + 20, (int)fileButton.y + 8, 20, BLACK);
                

            }    
            Color speedDecColor = CheckCollisionPointRec(mousePoint, speedDecreaseButton) ? 
                               (Color){41, 128, 185, 255} : LIGHTGRAY;
            Color speedIncColor = CheckCollisionPointRec(mousePoint, speedIncreaseButton) ?
                               (Color){41, 128, 185, 255} : LIGHTGRAY;
                               
            // DrawRectangleRec(speedDecreaseButton, speedDecColor);
            // DrawRectangleRec(speedIncreaseButton, speedIncColor);
            // DrawText("-", speedDecreaseButton.x + 20, speedDecreaseButton.y + 8, 20, BLACK);
            // DrawText("+", speedIncreaseButton.x + 20, speedIncreaseButton.y + 8, 20, BLACK);
            // DrawText(TextFormat("Speed: %d", frameSkip), 
            //         speedDecreaseButton.x, 
            //         speedDecreaseButton.y + BUTTON_HEIGHT + 5, 16, GRAY);

            // // Vẽ nút Pause/Resume với màu và text phù hợp
            // Color pauseResumeColor = CheckCollisionPointRec(mousePoint, pauseResumeButton) ? 
            //                        (Color){41, 128, 185, 255} : LIGHTGRAY;
            // DrawRectangleRec(pauseResumeButton, pauseResumeColor);
            // const char* buttonText = isPlayingAnimation ? "Pause" : "Play";
            // DrawText(buttonText, 
            //         pauseResumeButton.x + (pauseResumeButton.width - MeasureText(buttonText, 20))/2, 
            //         pauseResumeButton.y + 8, 20, BLACK);

            // Draw undo/redo buttons
            Color undoColor = tree.canUndo() ? 
                            (CheckCollisionPointRec(mousePoint, undoButton) ? 
                             (Color){41, 128, 185, 255} : GREEN) : 
                            RED; // Disabled color
                            
            Color redoColor = tree.canRedo() ? 
                            (CheckCollisionPointRec(mousePoint, redoButton) ? 
                             (Color){41, 128, 185, 255} : GREEN) : 
                            RED; // Disabled color
            
            // DrawRectangleRec(undoButton, undoColor);
            // DrawRectangleRec(redoButton, redoColor);
            
            // DrawText("Undo", undoButton.x + 2, undoButton.y + 8, 20, BLACK);
            // DrawText("Redo", redoButton.x + 2, redoButton.y + 8, 20, BLACK);
            
            DrawRectangleRounded(undoButton, 0.3, 6, undoColor);
            DrawRectangleRoundedLines(undoButton, 0.3, 6, 
                            BLACK);

            DrawRectangleRounded(redoButton, 0.3, 6, redoColor);
            DrawRectangleRoundedLines(redoButton, 0.3, 6, BLACK);

            // Larger text on buttons
            DrawText("Undo", undoButton.x + 2, undoButton.y + 12, 20, WHITE);
            DrawText("Redo", redoButton.x + 2, redoButton.y + 12, 20, WHITE);

            // Draw history position indicator
            if (!tree.historyStates.empty()) {
                DrawText(TextFormat("History: %d/%d", 
                       tree.currentHistoryPosition + 1, 
                       (int)tree.historyStates.size()),
                       undoButton.x, undoButton.y + BUTTON_HEIGHT + 10, 16, DARKGRAY);
            }

            // Draw enhanced progress bar with scrubbing indicator
            if (!currentAnimation.empty()) {
  
                
                

                // Draw scrubber track with better appearance - add shadows for depth
                DrawRectangleRounded(
                    (Rectangle){progressX - 3, progressY - 3, progressWidth + 6, progressHeight + 6}, 
                    0.5, 8, (Color){40, 40, 40, 100} // Shadow
                );

                DrawRectangleGradientH(
                    progressX, progressY, 
                    progressWidth, progressHeight, 
                    (Color){220, 220, 220, 255}, 
                    (Color){180, 180, 180, 255}
                );

                // Draw track outline
                DrawRectangleRoundedLines(
                    (Rectangle){progressX, progressY, progressWidth, progressHeight}, 
                    0.5, 8,  (Color){120, 120, 120, 200}
                );

                // Draw progress fill with gradient
                float progress = (float)currentStep / (currentAnimation.size() - 1);
                DrawRectangleGradientH(
                    progressX, progressY, 
                    progressWidth * progress, progressHeight,
                    (Color){66, 134, 244, 255}, // Bright blue
                    (Color){41, 128, 185, 255}  // Darker blue
                );

                // Draw scrubber handle with improved appearance
                float handleX = progressX + progressWidth * progress;
                float handleRadius = 15; // Increased from 12

                // Handle shadow
                DrawCircle(handleX + 2, progressY + progressHeight/2 + 2, handleRadius, (Color){40, 40, 40, 80});

                // Handle body
                DrawCircleGradient(
                    handleX, progressY + progressHeight/2, 
                    handleRadius,
                    isDraggingProgressBar ? (Color){255, 100, 100, 255} : (Color){255, 255, 255, 255},
                    isDraggingProgressBar ? (Color){200, 50, 50, 255} : (Color){220, 220, 220, 255}
                );

                // Handle outline
                DrawCircleLines(handleX, progressY + progressHeight/2, handleRadius, (Color){100, 100, 100, 200});

                // Draw frame counter with larger text
                DrawText(
                    TextFormat("Frame: %d / %d", currentStep, (int)currentAnimation.size()-1),
                    progressX, progressY - 30, 22, (Color){50, 50, 50, 255}
                );

                // LARGER BUTTONS: Play/pause button
                //Rectangle playPauseButton = {progressX - 70, progressY - 10, 50, 45}; // Much larger
                DrawRectangleRounded(pauseResumeButton, 0.3, 6, 
                    isPlayingAnimation ? (Color){52, 152, 219, 230} : (Color){46, 204, 113, 230});
                DrawRectangleRoundedLines(pauseResumeButton, 0.3, 6, 
                    isPlayingAnimation ? (Color){41, 128, 185, 255} : (Color){39, 174, 96, 255});

                // Draw play/pause icon - larger
                if (isPlayingAnimation) {
                    DrawTriangle(
                        (Vector2){pauseResumeButton.x + 13, pauseResumeButton.y + 8},
                        (Vector2){pauseResumeButton.x + 13, pauseResumeButton.y + 37},
                        (Vector2){pauseResumeButton.x + 40, pauseResumeButton.y + 22},
                        WHITE
                    );
                } else {
                    DrawRectangle(pauseResumeButton.x + 15, pauseResumeButton.y + 8, 8, 29, WHITE);
                    DrawRectangle(pauseResumeButton.x + 28, pauseResumeButton.y + 8, 8, 29, WHITE);
                }

                
                DrawRectangleRounded(speedDecreaseButton, 0.3, 6, (Color){192, 57, 43, 230});
                DrawRectangleRoundedLines(speedDecreaseButton, 0.3, 6,  (Color){150, 40, 30, 255});

                DrawRectangleRounded(speedIncreaseButton, 0.3, 6, (Color){39, 174, 96, 230});
                DrawRectangleRoundedLines(speedIncreaseButton, 0.3, 6,  (Color){33, 148, 83, 255});

                // Larger text on buttons
                DrawText("-", speedDecreaseButton.x + 18, speedDecreaseButton.y + 10, 25, WHITE);
                DrawText("+", speedIncreaseButton.x + 16, speedIncreaseButton.y + 10, 25, WHITE);

                DrawText(TextFormat("Speed: %dx", frameSkip), 
                speedDecreaseButton.x, speedDecreaseButton.y - 30, 20, DARKGRAY);

                // LARGER BUTTONS: Step navigation buttons


                Color backwardColor = (currentStep > 0) ? 
                                (Color){41, currentStep, 185, 230} : (Color){150, 150, 150, 150};
                Color forwardColor = (currentStep < currentAnimation.size() - 1) ? 
                                (Color){41, 128, 185, 230} : (Color){150, 150, 150, 150};

                DrawRectangleRounded(backwardButton, 0.3, 6, backwardColor);
                DrawRectangleRoundedLines(backwardButton, 0.3, 6, 
                                    (currentStep > 0) ? (Color){25, 80, 130, 255} : (Color){100, 100, 100, 150});

                DrawRectangleRounded(forwardButton, 0.3, 6, forwardColor);
                DrawRectangleRoundedLines(forwardButton, 0.3, 6, 
                                    (currentStep < currentAnimation.size() - 1) ? (Color){25, 80, 130, 255} : (Color){100, 100, 100, 150});

                // Larger text on buttons
                DrawText("Prev", backwardButton.x + 10, backwardButton.y + 12, 20, WHITE);
                DrawText("Next", forwardButton.x + 10, forwardButton.y + 12, 20, WHITE);

                DrawText("Step By Code Line", backwardButton.x - 30, backwardButton.y - 30, 20, DARKGRAY);
            }



            // Handle file dialog
            if (fileDialogState.windowActive) {
                cout << "inside" << '\n';
                GuiWindowFileDialog(&fileDialogState);
                

                if (fileDialogState.SelectFilePressed) {
                    std::string selectedFilePath = std::string(fileDialogState.dirPathText) + "\\" + 
                                                std::string(fileDialogState.fileNameText);
                    
                    std::ifstream inputFile(selectedFilePath);
                    if (inputFile.is_open()) {
                        // Clear existing list
                        //list.clearLinkedList(); // Reset list
                        //list.clearAnimationStates(); // Clear previous animation
                        std::string line;
                        while (std::getline(inputFile, line)) {
                            try {
                                // Skip empty lines and comments
                                if (line.empty() || line[0] == '#' || line[0] == '/') 
                                    continue;
                                    
                                int value = std::stoi(line);
                                cout << value << '\n';
                                file_data.push_back(value);
                            } catch (const std::exception& e) {
                                // Handle invalid input silently
                                continue;
                            }
                        }
                        inputFile.close();
                    }
                    fileDialogState.SelectFilePressed = false;
                }
            }
            
            DrawBackButton(backBtn);  // Make sure back button is drawn
            EndDrawing();
        //     // Add after other button click handlers:
            if (CheckCollisionPointRec(mousePoint, fileButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                fileDialogState.windowActive = !fileDialogState.windowActive;
                cout << "OK" << " " << fileDialogState.windowActive << '\n';
            }
        }
        CloseWindow();
    }
    
void RunLinkedListVisualization() {
    Rectangle backBtn = CreateBackButton();
    const int screenWidth = 1920;
    const int screenHeight = 1080;

    LinkedList list;
    
    int currentStep = 0;
    std::vector<LinkedListFrameState> currentAnimation;
    bool isPlayingAnimation = false;
    bool isShowingHistory = false;  // Flag to indicate if we're viewing a history state
    
    const float BUTTON_WIDTH = 120;
    const float BUTTON_HEIGHT = 35;
    const float BUTTON_SPACING = 10;
    
    // Existing buttons
    // Hamburger icon: góc trái dưới (padding 10 pixel)
    Rectangle hamburgerIcon = { 10, (float)screenHeight - 100, 40, 40 };
    bool isMenuOpen = false;
    float progressWidth = 500;
    float progressHeight = 10;
    float progressX = (screenWidth - progressWidth) / 2 - 100;
    float progressY = screenHeight - 100;
    
    // Sidebar: toàn bộ chiều cao, chiều rộng 300 pixel (bên trái)
    Rectangle sidebar = { 20 ,575, 400, (float)screenHeight };
    const float START_X = sidebar.x + 5;
    const float START_Y = sidebar.y + 100;

    Rectangle addHeadButton     = { sidebar.x + 5, sidebar.y + 100, BUTTON_WIDTH, BUTTON_HEIGHT };
    Rectangle addTailButton     = { START_X + (BUTTON_WIDTH + BUTTON_SPACING), START_Y, BUTTON_WIDTH, BUTTON_HEIGHT };
    Rectangle insertAfterButton = { START_X + 2 * (BUTTON_WIDTH + BUTTON_SPACING), START_Y, BUTTON_WIDTH, BUTTON_HEIGHT };
    Rectangle deleteButton      = { addHeadButton.x, addHeadButton.y + addHeadButton.height + 45, BUTTON_WIDTH, BUTTON_HEIGHT };

    // New buttons for Search and Remove At Index
    Rectangle searchButton      = { insertAfterButton.x, insertAfterButton.y + insertAfterButton.height + 45, BUTTON_WIDTH, BUTTON_HEIGHT };
    Rectangle removeAtIdxButton = { addTailButton.x, addTailButton.y + addTailButton.height + 45, BUTTON_WIDTH, BUTTON_HEIGHT };

    Rectangle addHeadBox   = { addHeadButton.x,    START_Y + BUTTON_HEIGHT + 5, BUTTON_WIDTH, BUTTON_HEIGHT };
    Rectangle addTailBox   = { addTailButton.x,    START_Y + BUTTON_HEIGHT + 5, BUTTON_WIDTH, BUTTON_HEIGHT };
    Rectangle deleteBox    = { deleteButton.x,     deleteButton.y + BUTTON_HEIGHT + 5, BUTTON_WIDTH, BUTTON_HEIGHT };
    float iaBoxWidth = (BUTTON_WIDTH - 10) / 2.0f;
    Rectangle insertAfterIdBox  = { insertAfterButton.x, START_Y + BUTTON_HEIGHT + 5, iaBoxWidth, BUTTON_HEIGHT };
    Rectangle insertAfterValBox = { insertAfterButton.x + iaBoxWidth + 10, START_Y + BUTTON_HEIGHT + 5, iaBoxWidth, BUTTON_HEIGHT };
    
    // New input boxes for Search and Remove At Index
    Rectangle searchBox    = { searchButton.x,      START_Y + 2 * BUTTON_HEIGHT + 50, BUTTON_WIDTH, BUTTON_HEIGHT };
    Rectangle removeAtIdxBox = { removeAtIdxButton.x, START_Y + 2 * BUTTON_HEIGHT + 50, BUTTON_WIDTH, BUTTON_HEIGHT };

    Rectangle pauseResumeButton = {progressX - 70, progressY - 10, 50, 45};
    

    // Add new buttons for backward/forward navigation
    Rectangle forwardButton = {progressX - 130, progressY - 10, 55, 45}; // Much larger
    Rectangle backwardButton = {progressX - 195, progressY - 10, 55, 45}; // Much larger

      // Add initialization buttons near top of screen
    Rectangle createEmptyBtn = { deleteButton.x, deleteButton.y + deleteButton.height + 45, BUTTON_WIDTH, BUTTON_HEIGHT };
    Rectangle createRandomBtn = { removeAtIdxButton.x, removeAtIdxButton.y + removeAtIdxButton.height + 45, BUTTON_WIDTH, BUTTON_HEIGHT };
    Rectangle createSortedBtn = { searchButton.x, searchButton.y + searchButton.height + 45, BUTTON_WIDTH, BUTTON_HEIGHT };
    Rectangle emptyBox   = { createEmptyBtn.x,    createEmptyBtn.y + BUTTON_HEIGHT + 5, BUTTON_WIDTH, BUTTON_HEIGHT };
    Rectangle randomBox   = { createRandomBtn.x,    createRandomBtn.y + BUTTON_HEIGHT + 5, BUTTON_WIDTH, BUTTON_HEIGHT };
    Rectangle sortBox    = { sortBox.x,     sortBox.y + BUTTON_HEIGHT + 5, BUTTON_WIDTH, BUTTON_HEIGHT };

    bool isTyping = false;
    std::string inputText = "";
    std::string operation = "";
    std::string insertAfterIdText = "";
    std::string insertAfterValText = "";
    bool insertAfterIdActive = true;

    Rectangle* activeTextBox = nullptr;
    
    // Add speed control for animation playback
    int frameSkip = 1; // Number of frames to skip (higher means faster playback)
    Rectangle speedDecreaseButton =  {progressX + progressWidth + 20, progressY - 10, 45, 45}; // Larger
    Rectangle speedIncreaseButton ={progressX + progressWidth + 70, progressY - 10, 45, 45}; // Larger
                    // LARGER BUTTONS: Speed contro

    // Add undo/redo buttons
    Rectangle undoButton = {speedIncreaseButton.x + 45 + 10, speedIncreaseButton.y , 45, 45 };
    Rectangle redoButton = {undoButton.x + 45 + 10, speedIncreaseButton.y, 45, 45 };


    // Add variables for progress bar interaction
    bool isDraggingProgressBar = false;
    Rectangle progressBarRect = (Rectangle){progressX, progressY, progressWidth, progressHeight};

    
    // Add file dialog state
    GuiWindowFileDialogState fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
    vector<int> file_data;
    // Add file load button next to search button
    Rectangle sizeInputBox = { createRandomBtn.x , createRandomBtn.y + BUTTON_HEIGHT + 5, BUTTON_WIDTH, BUTTON_HEIGHT };
    Rectangle fileButton = { createEmptyBtn.x, sizeInputBox.y + BUTTON_HEIGHT + 25, (insertAfterButton.x + BUTTON_WIDTH - addHeadButton.x), BUTTON_HEIGHT };

  
    std::string sizeInput = "5"; // Default size


    SetTargetFPS(120);
    while (!WindowShouldClose()) {
        Vector2 mousePoint = GetMousePosition();
        if(CheckCollisionPointRec(mousePoint, hamburgerIcon) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            isMenuOpen = !isMenuOpen;
        }
        if (CheckCollisionPointRec(mousePoint, backBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            return;
        }

        // Handle playback speed controls
        if (CheckCollisionPointRec(mousePoint, speedDecreaseButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (frameSkip > 1) frameSkip--;
        }
        
        if (CheckCollisionPointRec(mousePoint, speedIncreaseButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (frameSkip < 10) frameSkip++;
        }
        
        // Handle undo button
        if (CheckCollisionPointRec(mousePoint, undoButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (list.canUndo()) {
                list.undo(); // This now restores the list to the previous state
                isPlayingAnimation = false;
                currentAnimation.clear(); // Clear any current animation since we've changed state
                isShowingHistory = false; // We're now at a real list state, not just visualizing
            }
        }
        
        // Handle redo button
        if (CheckCollisionPointRec(mousePoint, redoButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (list.canRedo()) {
                list.redo(); // This now restores the list to the next state
                isPlayingAnimation = false;
                currentAnimation.clear(); // Clear any current animation since we've changed state
                isShowingHistory = false; // We're now at a real list state, not just visualizing
            }
        }

        // Handle animation playback
        if (isPlayingAnimation && !isDraggingProgressBar) {
            currentStep += frameSkip;
            if (currentStep >= currentAnimation.size()) {
                currentStep = currentAnimation.size() - 1;
                isPlayingAnimation = false;
            }
        }

        // Handle backward button - jump to last frame of previous highlighted line (already correct)
        if (CheckCollisionPointRec(mousePoint, backwardButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (!currentAnimation.empty() && currentStep > 0) {
                // Get current highlighted line
                int currentHighlightedLine = currentAnimation[currentStep].highlightedLine;
                
                // First find the previous different highlighted line
                int previousLine = -1;
                int targetStep = currentStep;
                
                while (targetStep >= 0) {
                    if (currentAnimation[targetStep].highlightedLine != currentHighlightedLine) {
                        previousLine = currentAnimation[targetStep].highlightedLine;
                        break;
                    }
                    targetStep--;
                }   
                    // Jump to the last frame of the previous line
                currentStep = max(0,targetStep);
            } else {
                    // If no previous line, go to the first frame
                currentStep = 0;
            }
                isPlayingAnimation = false; // Pause playback when stepping manually
        }
        
        // Handle forward button - MODIFIED to jump to LAST frame of next highlighted line
        if (CheckCollisionPointRec(mousePoint, forwardButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (!currentAnimation.empty() && currentStep < currentAnimation.size() - 1) {
                // Get current highlighted line
                int currentHighlightedLine = currentAnimation[currentStep].highlightedLine;
                
                // First find the next different highlighted line
                int nextLine = -1;
                int firstFrameOfNextLine = -1;
                int targetStep = currentStep;
                
                // Find the first occurrence of a different highlighted line
                while (targetStep < currentAnimation.size()) {
                    if (currentAnimation[targetStep].highlightedLine != currentHighlightedLine) {
                        nextLine = currentAnimation[targetStep].highlightedLine;
                        firstFrameOfNextLine = targetStep;
                        break;
                    }
                    targetStep++;
                }
                
                if (nextLine != -1) {
                    // Now find the last frame with this new highlighted line
                    int lastFrameOfNextLine = firstFrameOfNextLine;
                    targetStep = firstFrameOfNextLine;
                    
                    while (targetStep < currentAnimation.size()) {
                        if (currentAnimation[targetStep].highlightedLine == nextLine) {
                            lastFrameOfNextLine = targetStep;
                        } else {
                            // Found a different highlight, stop here
                            break;
                        }
                        targetStep++;
                    }
                    
                    // Jump to the last frame of the next line
                    currentStep = lastFrameOfNextLine;
                } else {
                    // If no next line, go to the last frame
                    currentStep = currentAnimation.size() - 1;
                }
                
                isPlayingAnimation = false; // Pause playback when stepping manually
            }
        }

        // Xử lý sự kiện click Pause/Resume
        if (CheckCollisionPointRec(mousePoint, pauseResumeButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            isPlayingAnimation = !isPlayingAnimation;
        }

        // Clear animations if new operation is performed
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && 
            (CheckCollisionPointRec(mousePoint, addHeadButton) || 
             CheckCollisionPointRec(mousePoint, addTailButton) ||
             CheckCollisionPointRec(mousePoint, deleteButton) ||
             CheckCollisionPointRec(mousePoint, insertAfterButton) ||
             CheckCollisionPointRec(mousePoint, searchButton) ||      // Add search button
             CheckCollisionPointRec(mousePoint, removeAtIdxButton))) { // Add remove at index button
             currentAnimation.clear();
             currentStep = 0;
             isShowingHistory = false;  // Exit history view mode when performing a new operation
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(mousePoint, addHeadButton)) {
                isTyping = true;
                operation = "addHead";
                activeTextBox = &addHeadBox;
                inputText = "";
            }
            else if (CheckCollisionPointRec(mousePoint, addTailButton)) {
                isTyping = true;
                operation = "addTail";
                activeTextBox = &addTailBox;
                inputText = "";
            }
            else if (CheckCollisionPointRec(mousePoint, deleteButton)) {
                isTyping = true;
                operation = "delete";
                activeTextBox = &deleteBox;
                inputText = "";
            }
            else if (CheckCollisionPointRec(mousePoint, insertAfterButton)) {
                isTyping = true;
                operation = "insertAfter";
                insertAfterIdText = "";
                insertAfterValText = "";
                insertAfterIdActive = true;
            }
            // Add handlers for new buttons
            else if (CheckCollisionPointRec(mousePoint, searchButton)) {
                isTyping = true;
                operation = "search";
                activeTextBox = &searchBox;
                inputText = "";
            }
            else if (CheckCollisionPointRec(mousePoint, removeAtIdxButton)) {
                isTyping = true;
                operation = "removeAtIdx";
                activeTextBox = &removeAtIdxBox;
                inputText = "";
            }
            else if (CheckCollisionPointRec(mousePoint, sizeInputBox) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                isTyping = true;
                operation = "size";
                inputText = sizeInput;
                activeTextBox = &sizeInputBox;  // Thêm dòng này để theo dõi textbox đang active
            }
            
        }
        // Handle file dialog
        if (fileDialogState.windowActive) {
            list.clearAnimationStates(); // Clear previous animation
            GuiWindowFileDialog(&fileDialogState);


            if (fileDialogState.SelectFilePressed) {
                std::string selectedFilePath = std::string(fileDialogState.dirPathText) + "\\" + 
                                             std::string(fileDialogState.fileNameText);
                
                std::ifstream inputFile(selectedFilePath);
                if (inputFile.is_open()) {
                    // Clear existing list
                    //list.clearLinkedList(); // Reset list
                    //list.clearAnimationStates(); // Clear previous animation
                    std::string line;
                    while (std::getline(inputFile, line)) {
                        try {
                            // Skip empty lines and comments
                            if (line.empty() || line[0] == '#' || line[0] == '/') 
                                continue;
                                
                            int value = std::stoi(line);
                            cout << value << '\n';
                            file_data.push_back(value);
                        } catch (const std::exception& e) {
                            // Handle invalid input silently
                            continue;
                        }
                    }
                    inputFile.close();
                }
                fileDialogState.SelectFilePressed = false;
            }
        }
        while((int)file_data.size() > 0){
            int value = file_data.back();
            file_data.pop_back();
            list.clearAnimationStates(); // Clear previous animation
            list.addTail(value);
            currentAnimation = list.processQueryAnimation();

            currentStep = 0;
            isPlayingAnimation = true;
            // Process the animation and store states
        }
        if (isTyping) {
            if (operation == "size") {
                cout << "sz" << '\n';
                int key = GetCharPressed();
                cout << key << '\n';
                while (key > 0) {
                    if ((key >= '0' && key <= '9') && inputText.length() < 2) {
                        inputText.push_back((char)key);
                    }
                    key = GetCharPressed();
                }
                
                if (IsKeyPressed(KEY_BACKSPACE) && !inputText.empty()) {
                    inputText.pop_back();
                }
                
                if (IsKeyPressed(KEY_ENTER)) {
                    //int size = std::stoi(inputText.empty() ? "0" : inputText);
                    //cout << "sz = " << size << '\n';
                    // isTyping = false;
                    // operation = "";
                    // inputText = "";
                    // activeTextBox = nullptr;  // Reset active textbox
                }
                sizeInput = inputText;

            }
            else if (operation != "insertAfter") {
                int key = GetCharPressed();
                while (key > 0) {
                    if ((key >= '0' && key <= '9') && inputText.length() < 5)
                        inputText.push_back((char)key);
                    key = GetCharPressed();
                }
                if (IsKeyPressed(KEY_BACKSPACE) && !inputText.empty())
                    inputText.pop_back();
                if (IsKeyPressed(KEY_ENTER) && !inputText.empty()) {
                    int value = atoi(inputText.c_str());
                    list.clearAnimationStates(); // Clear previous animation
                    
                    if (operation == "addHead") {
                        list.addHead(value);
                    }
                    else if (operation == "addTail") {
                        list.addTail(value);
                    }
                    else if (operation == "delete") {
                        //cout << "hi" << '\n';
                        list.clearAnimationStates();
                        list.setPseudoCodeDelete(value);
                        
                        // First animate the search traversal
                        list.animateDeleteTraversal(value);
                        
                        // Store the traversal animation frames
                        currentAnimation = list.animationStates;
                        
                        // Clear animation states before performing deletion
                        list.clearAnimationStates();
                        
                        // If we found the node, perform the actual deletion
                    
                        list.deleteValue(value);
                        
                        // Get the animation frames for node repositioning
                        std::vector<LinkedListFrameState> deletionFrames = list.processQueryAnimation();
                        
                        // Add the deletion frames to the current animation
                        currentAnimation.insert(currentAnimation.end(), deletionFrames.begin(), deletionFrames.end());
                        //cout << "size = " << currentAnimation.size() << '\n';
                    }
                    // Add handling for new operations
                    else if (operation == "search") {
                        list.setPseudoCodeSearch();
                        list.clearAnimationStates();
                        
                        // Perform the search animation
                        list.searchValue(value);
                        
                        // Store the animation frames
                        currentAnimation = list.animationStates;
                    }
                    else if (operation == "removeAtIdx") {
                        //cout << "hi" << '\n';
                        list.setPseudoCodeRemoveAtIdx();
                        list.clearAnimationStates();
                        
                        // First animate the search traversal
                        list.removeAtIndexTraversal(value);
                        
                        // Store the traversal animation frames
                        currentAnimation = list.animationStates;
                        
                        // Clear animation states before performing deletion
                        list.clearAnimationStates();
                        
                        // If we found the node, perform the actual deletion
                    
                        list.removeAtIndex(value);
                        
                        // Get the animation frames for node repositioning
                        std::vector<LinkedListFrameState> deletionFrames = list.processQueryAnimation();
                        
                        // Add the deletion frames to the current animation
                        currentAnimation.insert(currentAnimation.end(), deletionFrames.begin(), deletionFrames.end());
                        //cout << "size = " << currentAnimation.size() << '\n';
                    }
                    
                    if(operation != "delete" && operation != "search" && operation != "removeAtIdx") {
                        // Clear animation states before performing the operation
                        // list.clearAnimationStates();
                        
                        // Process the animation and store states
                        currentAnimation = list.processQueryAnimation();
                    }
                    currentStep = 0;
                    isPlayingAnimation = true;
                    
                    isTyping = false;
                    operation = "";
                    activeTextBox = nullptr;
                    inputText = "";
                }
            } else {
                if (insertAfterIdActive) {
                    int key = GetCharPressed();
                    while (key > 0) {
                        if ((key >= '0' && key <= '9') && insertAfterIdText.length() < 3)
                            insertAfterIdText.push_back((char)key);
                        key = GetCharPressed();
                    }
                    if (IsKeyPressed(KEY_BACKSPACE) && !insertAfterIdText.empty())
                        insertAfterIdText.pop_back();
                    if (IsKeyPressed(KEY_ENTER) && !insertAfterIdText.empty()) {
                        insertAfterIdActive = false;
                    }
                } else {
                    int key = GetCharPressed();
                    while (key > 0) {
                        if ((key >= '0' && key <= '9') && insertAfterValText.length() < 5)
                            insertAfterValText.push_back((char)key);
                        key = GetCharPressed();
                    }
                    if (IsKeyPressed(KEY_BACKSPACE) && !insertAfterValText.empty())
                        insertAfterValText.pop_back();
                    if (IsKeyPressed(KEY_ENTER) && !insertAfterValText.empty()) {
                        int id = atoi(insertAfterIdText.c_str());
                        int value = atoi(insertAfterValText.c_str());
                        list.setPseudoCodeInsertAfter(id, value);
                        list.clearAnimationStates();
                        list.animateInsertAfterTraversal(id);
                        
                        // Lưu frames từ quá trình traversal vào currentAnimation
                        currentAnimation = list.animationStates;
                        
                        // Xóa trạng thái animation trước khi thêm node mới
                        list.clearAnimationStates();
                        
                        // Thực hiện thêm node
                        list.insertAfter(id, value);
                        
                        // Xử lý animation cho việc thêm node và thêm vào kết quả hiện tại
                        std::vector<LinkedListFrameState> insertionFrames = list.processQueryAnimation();
                        currentAnimation.insert(currentAnimation.end(), insertionFrames.begin(), insertionFrames.end());
                        
                        currentStep = 0;
                        isPlayingAnimation = true;
                        
                        insertAfterIdText = "";
                        insertAfterValText = "";
                        insertAfterIdActive = true;
                        isTyping = false;
                        operation = "";
                    }
                }
            }
        }

        // Handle progress bar interaction
        if (!currentAnimation.empty()) {
            
            // Handle mouse press on progress bar
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && 
                CheckCollisionPointRec(GetMousePosition(),progressBarRect)) {
                isDraggingProgressBar = true;
            }
            
            // Handle mouse drag on progress bar
            if (isDraggingProgressBar) {
                if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                    // Calculate new position based on mouse position
                    float mouseX = GetMousePosition().x;
                    float progress = (mouseX - progressX) / progressWidth;
                    
                    // Clamp progress between 0 and 1
                    progress = progress < 0 ? 0 : (progress > 1 ? 1 : progress);
                    
                    // Calculate new frame position
                    currentStep = (int)(progress * (currentAnimation.size() - 1));
                    
                    // Ensure currentStep is within valid range
                    currentStep = currentStep < 0 ? 0 : 
                                 (currentStep >= currentAnimation.size() ? 
                                  currentAnimation.size() - 1 : currentStep);
                    
                    // Pause animation while scrubbing
                    isPlayingAnimation = false;
                } else {
                    // Release drag when mouse button is released
                    isDraggingProgressBar = false;
                }
            }
        }

        // Inside the main loop, after other button checks:
        if (CheckCollisionPointRec(mousePoint, createEmptyBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            list.createEmpty();
            currentAnimation.clear();
            currentStep = 0;
        }

        if (CheckCollisionPointRec(mousePoint, createRandomBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            int size = std::stoi(sizeInput);
            if(size > 0 && size <= 15) {
                list.createRandom(size);
                list.processQueryAnimation();
                currentAnimation.clear();
                currentStep = 0;
            }
        }

        if (CheckCollisionPointRec(mousePoint, createSortedBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            int size = std::stoi(sizeInput);
            if(size > 0 && size <= 15) {
                list.createRandomSorted(size);
                currentAnimation.clear();
                currentStep = 0;
            }
        }


        float buttonX = 1650;
        float buttonY = 1000;
        if(list.showPseudoCode == true) buttonY = 700;
            float buttonWidth = 220;
            float buttonHeight = 40;
            
            // Check for collision with the pill-shaped button
            bool collision = false;
            float radius = buttonHeight / 2;
            
            // Check left circle
            if (CheckCollisionPointCircle(mousePoint, (Vector2){buttonX + radius, buttonY + radius}, radius)) {
                collision = true;
            }
            // Check right circle
            else if (CheckCollisionPointCircle(mousePoint, (Vector2){buttonX + buttonWidth - radius, buttonY + radius}, radius)) {
                collision = true;
            }
            // Check middle rectangle
            else if (CheckCollisionPointRec(mousePoint, (Rectangle){buttonX + radius, buttonY, buttonWidth - buttonHeight, buttonHeight})) {
                collision = true;
            }
            
            if (collision && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                list.showPseudoCode = !list.showPseudoCode;
            }
        

        // Thêm phần xử lý cho size input vào vòng lặp chín
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawBackButton(backBtn);
            
            // Visualization based on current state
            if (!currentAnimation.empty() && currentStep < currentAnimation.size()) {
                // If we have an animation in progress, show the current frame
                list.visualizeState(currentAnimation[currentStep]);
            } else {
                // Otherwise show the current state of the list
                list.visualize();
            }

            Color addHeadColor     = CheckCollisionPointRec(mousePoint, addHeadButton) ? (Color){41, 128, 185, 255} : LIGHTGRAY;
            Color addTailColor     = CheckCollisionPointRec(mousePoint, addTailButton) ? (Color){39, 174, 96, 255} : LIGHTGRAY;
            Color deleteColor      = CheckCollisionPointRec(mousePoint, deleteButton) ? (Color){231, 76, 60, 255} : LIGHTGRAY;
            Color insertAfterColor = CheckCollisionPointRec(mousePoint, insertAfterButton) ? (Color){142, 68, 173, 255} : LIGHTGRAY;
            
            // Set colors for new buttons
            Color searchColor      = CheckCollisionPointRec(mousePoint, searchButton) ? (Color){41, 128, 185, 255} : LIGHTGRAY;
            Color removeAtIdxColor = CheckCollisionPointRec(mousePoint, removeAtIdxButton) ? (Color){231, 76, 60, 255} : LIGHTGRAY;
            DrawHamburgerIconChPlay(hamburgerIcon);
            if(isMenuOpen == true){
                // Draw sidebar
                DrawSidebar(sidebar);
                //DrawRectangleRounded(sidebar, 0.5, 8, (Color){41, 128, 185, 255});
                DrawHamburgerIconChPlay(hamburgerIcon);
                DrawRectangleRec(addHeadButton, addHeadColor);
                DrawRectangleRec(addTailButton, addTailColor);
                DrawRectangleRec(deleteButton, deleteColor);
                DrawRectangleRec(insertAfterButton, insertAfterColor);
                
                // Draw new buttons
                DrawRectangleRec(searchButton, searchColor);
                DrawRectangleRec(removeAtIdxButton, removeAtIdxColor);

                DrawText("Add Head", (int)addHeadButton.x + 10, (int)addHeadButton.y + 8, 20, BLACK);
                DrawText("Add Tail", (int)addTailButton.x + 10, (int)addTailButton.y + 8, 20, BLACK);
                DrawText("Delete", (int)deleteButton.x + 20, (int)deleteButton.y + 8, 20, BLACK);
                DrawText("Insert After", (int)insertAfterButton.x + 2, (int)insertAfterButton.y + 8, 18, BLACK);
                
                // Draw text on new buttons
                DrawText("Search", (int)searchButton.x + 20, (int)searchButton.y + 8, 20, BLACK);
                DrawText("Remove At", (int)removeAtIdxButton.x + 10, (int)removeAtIdxButton.y + 8, 18, BLACK);
                // Draw file button with matching style
                Color fileColor = CheckCollisionPointRec(mousePoint, fileButton) ? 
                                (Color){41, 128, 185, 255} : LIGHTGRAY;
                DrawRectangleRec(fileButton, fileColor);
                DrawText("Load File", (int)fileButton.x + 20, (int)fileButton.y + 8, 20, BLACK);

                // Inside drawing section:
                Color initBtnColor = LIGHTGRAY;
                
                // Draw initialization buttons
                DrawRectangleRec(createEmptyBtn, CheckCollisionPointRec(mousePoint, createEmptyBtn) ? 
                                (Color){41, 128, 185, 255} : initBtnColor);
                DrawRectangleRec(createRandomBtn, CheckCollisionPointRec(mousePoint, createRandomBtn) ? 
                                (Color){41, 128, 185, 255} : initBtnColor);
                DrawRectangleRec(createSortedBtn, CheckCollisionPointRec(mousePoint, createSortedBtn) ? 
                                (Color){41, 128, 185, 255} : initBtnColor);
                                
                DrawText("Empty", createEmptyBtn.x + 30, createEmptyBtn.y + 8, 20, BLACK);
                DrawText("Random", createRandomBtn.x + 25, createRandomBtn.y + 8, 20, BLACK);
                DrawText("Sorted", createSortedBtn.x + 30, createSortedBtn.y + 8, 20, BLACK);
                DrawRectangleRec(sizeInputBox, WHITE);
                DrawRectangleLines((int)sizeInputBox.x, (int)sizeInputBox.y, (int)sizeInputBox.width, (int)sizeInputBox.height, BLACK);
                if(operation == "size")
                    DrawText(inputText.c_str(), (int)sizeInputBox.x + 5, (int)sizeInputBox.y + 8, 20, BLACK);
                
            }   
            // Draw speed control buttons
            Color speedDecColor = CheckCollisionPointRec(mousePoint, speedDecreaseButton) ? 
                               (Color){41, 128, 185, 255} : LIGHTGRAY;
            Color speedIncColor = CheckCollisionPointRec(mousePoint, speedIncreaseButton) ?
                               (Color){41, 128, 185, 255} : LIGHTGRAY;
                               
            // DrawRectangleRec(speedDecreaseButton, speedDecColor);
            // DrawRectangleRec(speedIncreaseButton, speedIncColor);
            // DrawText("-", speedDecreaseButton.x + 20, speedDecreaseButton.y + 8, 20, BLACK);
            // DrawText("+", speedIncreaseButton.x + 20, speedIncreaseButton.y + 8, 20, BLACK);
            // DrawText(TextFormat("Speed: %d", frameSkip), 
            //         speedDecreaseButton.x, 
            //         speedDecreaseButton.y + BUTTON_HEIGHT + 5, 16, GRAY);

            // // Vẽ nút Pause/Resume với màu và text phù hợp
            // Color pauseResumeColor = CheckCollisionPointRec(mousePoint, pauseResumeButton) ? 
            //                        (Color){41, 128, 185, 255} : LIGHTGRAY;
            // DrawRectangleRec(pauseResumeButton, pauseResumeColor);
            // const char* buttonText = isPlayingAnimation ? "Pause" : "Play";
            // DrawText(buttonText, 
            //         pauseResumeButton.x + (pauseResumeButton.width - MeasureText(buttonText, 20))/2, 
            //         pauseResumeButton.y + 8, 20, BLACK);

            // Draw undo/redo buttons
            Color undoColor = list.canUndo() ? 
                            (CheckCollisionPointRec(mousePoint, undoButton) ? 
                             (Color){41, 128, 185, 255} : GREEN) : 
                            RED; // Disabled color
                            
            Color redoColor = list.canRedo() ? 
                            (CheckCollisionPointRec(mousePoint, redoButton) ? 
                             (Color){41, 128, 185, 255} : GREEN) : 
                            RED; // Disabled color
            
            // DrawRectangleRec(undoButton, undoColor);
            // DrawRectangleRec(redoButton, redoColor);
            
            // DrawText("Undo", undoButton.x + 2, undoButton.y + 8, 20, BLACK);
            // DrawText("Redo", redoButton.x + 2, redoButton.y + 8, 20, BLACK);
            
            DrawRectangleRounded(undoButton, 0.3, 6, undoColor);
            DrawRectangleRoundedLines(undoButton, 0.3, 6, 
                            BLACK);

            DrawRectangleRounded(redoButton, 0.3, 6, redoColor);
            DrawRectangleRoundedLines(redoButton, 0.3, 6, BLACK);

            // Larger text on buttons
            DrawText("Undo", undoButton.x + 2, undoButton.y + 12, 20, WHITE);
            DrawText("Redo", redoButton.x + 2, redoButton.y + 12, 20, WHITE);

            // Draw history position indicator
            if (!list.historyStates.empty()) {
                DrawText(TextFormat("History: %d/%d", 
                       list.currentHistoryPosition + 1, 
                       (int)list.historyStates.size()),
                       undoButton.x, undoButton.y + BUTTON_HEIGHT + 10, 16, DARKGRAY);
            }

            // Draw enhanced progress bar with scrubbing indicator
            if (!currentAnimation.empty()) {
  
                
                

                // Draw scrubber track with better appearance - add shadows for depth
                DrawRectangleRounded(
                    (Rectangle){progressX - 3, progressY - 3, progressWidth + 6, progressHeight + 6}, 
                    0.5, 8, (Color){40, 40, 40, 100} // Shadow
                );

                DrawRectangleGradientH(
                    progressX, progressY, 
                    progressWidth, progressHeight, 
                    (Color){220, 220, 220, 255}, 
                    (Color){180, 180, 180, 255}
                );

                // Draw track outline
                DrawRectangleRoundedLines(
                    (Rectangle){progressX, progressY, progressWidth, progressHeight}, 
                    0.5, 8,  (Color){120, 120, 120, 200}
                );

                // Draw progress fill with gradient
                float progress = (float)currentStep / (currentAnimation.size() - 1);
                DrawRectangleGradientH(
                    progressX, progressY, 
                    progressWidth * progress, progressHeight,
                    (Color){66, 134, 244, 255}, // Bright blue
                    (Color){41, 128, 185, 255}  // Darker blue
                );

                // Draw scrubber handle with improved appearance
                float handleX = progressX + progressWidth * progress;
                float handleRadius = 15; // Increased from 12

                // Handle shadow
                DrawCircle(handleX + 2, progressY + progressHeight/2 + 2, handleRadius, (Color){40, 40, 40, 80});

                // Handle body
                DrawCircleGradient(
                    handleX, progressY + progressHeight/2, 
                    handleRadius,
                    isDraggingProgressBar ? (Color){255, 100, 100, 255} : (Color){255, 255, 255, 255},
                    isDraggingProgressBar ? (Color){200, 50, 50, 255} : (Color){220, 220, 220, 255}
                );

                // Handle outline
                DrawCircleLines(handleX, progressY + progressHeight/2, handleRadius, (Color){100, 100, 100, 200});

                // Draw frame counter with larger text
                DrawText(
                    TextFormat("Frame: %d / %d", currentStep, (int)currentAnimation.size()-1),
                    progressX, progressY - 30, 22, (Color){50, 50, 50, 255}
                );

                // LARGER BUTTONS: Play/pause button
                //Rectangle playPauseButton = {progressX - 70, progressY - 10, 50, 45}; // Much larger
                DrawRectangleRounded(pauseResumeButton, 0.3, 6, 
                    isPlayingAnimation ? (Color){52, 152, 219, 230} : (Color){46, 204, 113, 230});
                DrawRectangleRoundedLines(pauseResumeButton, 0.3, 6, 
                    isPlayingAnimation ? (Color){41, 128, 185, 255} : (Color){39, 174, 96, 255});

                // Draw play/pause icon - larger
                if (isPlayingAnimation) {
                    DrawTriangle(
                        (Vector2){pauseResumeButton.x + 13, pauseResumeButton.y + 8},
                        (Vector2){pauseResumeButton.x + 13, pauseResumeButton.y + 37},
                        (Vector2){pauseResumeButton.x + 40, pauseResumeButton.y + 22},
                        WHITE
                    );
                } else {
                    DrawRectangle(pauseResumeButton.x + 15, pauseResumeButton.y + 8, 8, 29, WHITE);
                    DrawRectangle(pauseResumeButton.x + 28, pauseResumeButton.y + 8, 8, 29, WHITE);
                }

                
                DrawRectangleRounded(speedDecreaseButton, 0.3, 6, (Color){192, 57, 43, 230});
                DrawRectangleRoundedLines(speedDecreaseButton, 0.3, 6,  (Color){150, 40, 30, 255});

                DrawRectangleRounded(speedIncreaseButton, 0.3, 6, (Color){39, 174, 96, 230});
                DrawRectangleRoundedLines(speedIncreaseButton, 0.3, 6,  (Color){33, 148, 83, 255});

                // Larger text on buttons
                DrawText("-", speedDecreaseButton.x + 18, speedDecreaseButton.y + 10, 25, WHITE);
                DrawText("+", speedIncreaseButton.x + 16, speedIncreaseButton.y + 10, 25, WHITE);

                DrawText(TextFormat("Speed: %dx", frameSkip), 
                speedDecreaseButton.x, speedDecreaseButton.y - 30, 20, DARKGRAY);

                // LARGER BUTTONS: Step navigation buttons


                Color backwardColor = (currentStep > 0) ? 
                                (Color){41, currentStep, 185, 230} : (Color){150, 150, 150, 150};
                Color forwardColor = (currentStep < currentAnimation.size() - 1) ? 
                                (Color){41, 128, 185, 230} : (Color){150, 150, 150, 150};

                DrawRectangleRounded(backwardButton, 0.3, 6, backwardColor);
                DrawRectangleRoundedLines(backwardButton, 0.3, 6, 
                                    (currentStep > 0) ? (Color){25, 80, 130, 255} : (Color){100, 100, 100, 150});

                DrawRectangleRounded(forwardButton, 0.3, 6, forwardColor);
                DrawRectangleRoundedLines(forwardButton, 0.3, 6, 
                                    (currentStep < currentAnimation.size() - 1) ? (Color){25, 80, 130, 255} : (Color){100, 100, 100, 150});

                // Larger text on buttons
                DrawText("Prev", backwardButton.x + 10, backwardButton.y + 12, 20, WHITE);
                DrawText("Next", forwardButton.x + 10, forwardButton.y + 12, 20, WHITE);

                DrawText("Step By Code Line", backwardButton.x - 30, backwardButton.y - 30, 20, DARKGRAY);
            }

            // // Draw backward and forward buttons
            // Color backwardColor = (!currentAnimation.empty() && currentStep > 0) ? 
            //                   (CheckCollisionPointRec(mousePoint, backwardButton) ? 
            //                   (Color){41, 128, 185, 255} : LIGHTGRAY) : 
            //                   (Color){200, 200, 200, 128}; // Disabled color
                              
            // Color forwardColor = (!currentAnimation.empty() && currentStep < currentAnimation.size() - 1) ? 
            //                   (CheckCollisionPointRec(mousePoint, forwardButton) ? 
            //                   (Color){41, 128, 185, 255} : LIGHTGRAY) : 
            //                   (Color){200, 200, 200, 128}; // Disabled color
            
            // DrawRectangleRec(backwardButton, backwardColor);
            // DrawRectangleRec(forwardButton, forwardColor);
            
            // DrawText("◄◄", backwardButton.x + 20, backwardButton.y + 8, 20, BLACK); // Changed to double arrow
            // DrawText("►►", forwardButton.x + 20, forwardButton.y + 8, 20, BLACK);   // Changed to double arrow
            
            // // Update the label to reflect the new functionality
            // DrawText("Step By Code Line", backwardButton.x + 5, backwardButton.y - 20, 16, DARKGRAY);

            //list.drawProgressBar(currentStep);
            if (operation == "addHead") {
                DrawRectangleRec(addHeadBox, WHITE);
                DrawRectangleLines((int)addHeadBox.x, (int)addHeadBox.y, (int)addHeadBox.width, (int)addHeadBox.height, BLACK);
                DrawText(inputText.c_str(), (int)addHeadBox.x + 5, (int)addHeadBox.y + 8, 20, BLACK);
            }
            else if (operation == "addTail") {
                DrawRectangleRec(addTailBox, WHITE);
                DrawRectangleLines((int)addTailBox.x, (int)addTailBox.y, (int)addTailBox.width, (int)addTailBox.height, BLACK);
                DrawText(inputText.c_str(), (int)addTailBox.x + 5, (int)addTailBox.y + 8, 20, BLACK);
            }
            else if(operation == "delete") {
                DrawRectangleRec(deleteBox, WHITE);
                DrawRectangleLines((int)deleteBox.x, (int)deleteBox.y, (int)deleteBox.width, (int)deleteBox.height, BLACK);
                DrawText(inputText.c_str(), (int)deleteBox.x + 5, (int)deleteBox.y + 8, 20, BLACK);
            }
            else if (operation == "insertAfter") {
                DrawRectangleRec(insertAfterIdBox, WHITE);
                DrawRectangleLines((int)insertAfterIdBox.x, (int)insertAfterIdBox.y, (int)insertAfterIdBox.width, (int)insertAfterIdBox.height, BLACK);
                if (insertAfterIdText.empty())
                    DrawText("idx", (int)insertAfterIdBox.x + 5, (int)insertAfterIdBox.y + 8, 20, DARKGRAY);
                else
                    DrawText(insertAfterIdText.c_str(), (int)insertAfterIdBox.x + 5, (int)insertAfterIdBox.y + 8, 20, BLACK);

                DrawRectangleRec(insertAfterValBox, WHITE);
                DrawRectangleLines((int)insertAfterValBox.x, (int)insertAfterValBox.y, (int)insertAfterValBox.width, (int)insertAfterValBox.height, BLACK);
                if (insertAfterValText.empty())
                    DrawText("val", (int)insertAfterValBox.x + 5, (int)insertAfterValBox.y + 8, 20, DARKGRAY);
                else
                    DrawText(insertAfterValText.c_str(), (int)insertAfterValBox.x + 5, (int)insertAfterValBox.y + 8, 20, BLACK);
            }
            // Draw input boxes for new operations
            else if(operation == "search") {
                DrawRectangleRec(searchBox, WHITE);
                DrawRectangleLines((int)searchBox.x, (int)searchBox.y, (int)searchBox.width, (int)searchBox.height, BLACK);
                DrawText(inputText.c_str(), (int)searchBox.x + 5, (int)searchBox.y + 8, 20, BLACK);
            }
            else if(operation == "removeAtIdx") {
                DrawRectangleRec(removeAtIdxBox, WHITE);
                DrawRectangleLines((int)removeAtIdxBox.x, (int)removeAtIdxBox.y, (int)removeAtIdxBox.width, (int)removeAtIdxBox.height, BLACK);
                DrawText(inputText.c_str(), (int)removeAtIdxBox.x + 5, (int)removeAtIdxBox.y + 8, 20, BLACK);
            }
            


            // Draw size input box
            

            // // Update size input handling in typing section
            // if (operation == "size") {
            //     int key = GetCharPressed();
            //     while (key > 0) {
            //         if ((key >= '0' && key <= '9') && inputText.length() < 2)
            //             inputText.push_back((char)key);
            //         key = GetCharPressed();
            //     }
            //     if (IsKeyPressed(KEY_BACKSPACE) && !inputText.empty())
            //         inputText.pop_back();
            //     if (IsKeyPressed(KEY_ENTER)) {
            //         int size = std::stoi(inputText.empty() ? "0" : inputText);
            //         if(size > 0 && size <= 15) {
            //             sizeInput = inputText;
            //         }
            //         isTyping = false;
            //         operation = "";
            //         inputText = "";
            //     }
            // }
        EndDrawing();


        // Add after other button click handlers:
        if (CheckCollisionPointRec(mousePoint, fileButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            fileDialogState.windowActive = !fileDialogState.windowActive;
        }
    }
    CloseWindow();
}

void RunShortestPath(){
    const int ScreenWidth = 1920;
    const int ScreenHeight = 1080;

    Rectangle backBtn = CreateBackButton();
    LoadFonts();

    while (!WindowShouldClose())
    {
        Vector2 mousePoint = GetMousePosition();
        
        if (CheckCollisionPointRec(mousePoint, backBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            return;
        }
        rendershortestpath(ScreenWidth, ScreenHeight);
        DrawBackButton(backBtn);  // Make sure back button is drawn
    }
    UnloadFont(GetFont());
    return;
}

void RunTrie(){
    Rectangle backBtn = CreateBackButton();
    const int ScreenWidth = 1920;
    const int ScreenHeight = 1080;
    LoadFonts();
    while (!WindowShouldClose())
    {
        Vector2 mousePoint = GetMousePosition();
        
        if (CheckCollisionPointRec(mousePoint, backBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            return;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawBackButton(backBtn);  // Make sure back button is drawn
        RenderTrie();
        EndDrawing();
    }
    UnloadFont(GetFont());
    CloseWindow();
}

void RunHashTable(){
    Rectangle backBtn = CreateBackButton();
    const int ScreenWidth = 1920;
    const int ScreenHeight = 1080;
    LoadFonts();
    while (!WindowShouldClose())
    {
        Vector2 mousePoint = GetMousePosition();
        
        if (CheckCollisionPointRec(mousePoint, backBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            return;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawBackButton(backBtn);
        RenderHashTable();
        EndDrawing();
    }
    UnloadFonts();
}

int main() {
    ShowMenu();
    //RunShortestPath();
    return 0;
}

// #include "raylib.h"

// int main(void) {
//     // Bật MSAA nếu muốn có chất lượng chữ tốt hơn
//     SetConfigFlags(FLAG_MSAA_4X_HINT);
//     InitWindow(800, 600, "Đổi kiểu chữ trong Raylib");

//     // Tải font TTF với kích thước mong muốn
//     // Ví dụ: Tải font "customfont.ttf" với kích thước 32 và 0 là mặc định số ký tự
//     Font customFont = LoadFontEx("roboto.ttf", 32, 0, 0);

//     SetTargetFPS(60);
//     while (!WindowShouldClose()) {
//         BeginDrawing();
//         ClearBackground(RAYWHITE);

//         // Vẽ văn bản sử dụng font tùy chỉnh
//         DrawTextEx(customFont, "Hello, Raylib!", (Vector2){100, 100}, customFont.baseSize, 2, DARKBLUE);

//         EndDrawing();
//     }

//     // Giải phóng font sau khi dùng
//     UnloadFont(customFont);
//     CloseWindow();
//     return 0;
// }
