#include "mst.h"
#include <iostream>
#include <algorithm>
#include <thread>
#include <chrono>
#include <cmath>

// Màn hình và một số hằng số định vị.
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

// Các hằng số cho danh sách cạnh (Edge List).
#define EDGE_LIST_START_X 20      // Vị trí bắt đầu vẽ Edge List.
#define EDGE_LIST_START_Y 570
#define EDGE_LIST_GAP 30          // Khoảng cách giữa các cạnh trong một cột.
#define FIXED_EDGE_LIST_LENGTH 120 // Độ dài đoạn thẳng biểu diễn cạnh trong Edge List.
#define EDGE_LIST_COLUMN_GAP 300  // Wider gap between columns
#define EDGE_LIST_ITEMS_PER_COLUMN 13 // Approximate number of edges per column (adjusts automatically)

// Enhanced constants for graph visualization
#define VERTEX_RADIUS 26          // Increased radius for better visibility
#define VERTEX_INNER_RADIUS 24    // Inner radius for gradient effect
#define VERTEX_SHADOW_OFFSET 3    // Shadow offset for 3D effect
#define WEIGHT_OFFSET 20          // Increased offset for weight labels
#define EDGE_GRAPH_THICKNESS 2.5f // Slightly thicker edges
#define EDGE_GRAPH_HIGHLIGHT_THICKNESS 5.0f // Thicker highlight
#define WEIGHT_BUBBLE_PADDING 8   // Padding for weight label background

// Define color constants with bolder, more vibrant colors
#define VERTEX_FILL_COLOR (Color){0, 119, 255, 255}      // Brighter blue
#define VERTEX_BORDER_COLOR (Color){0, 84, 180, 255}     // Darker blue border
#define VERTEX_HIGHLIGHT_COLOR (Color){255, 191, 0, 255} // Brighter gold
#define VERTEX_TEXT_COLOR (Color){255, 255, 255, 255}    // White text
#define EDGE_DEFAULT_COLOR (Color){80, 95, 120, 255}     // Darker gray for better visibility
#define WEIGHT_BG_COLOR (Color){245, 245, 245, 240}      // Clearer white background

// UI colors with more saturation
#define UI_PRIMARY (Color){63, 134, 255, 255}          // Vibrant blue
#define UI_PRIMARY_DARK (Color){32, 84, 219, 255}      // Darker blue
#define UI_ACCENT (Color){255, 76, 76, 255}            // Vibrant red
#define UI_SUCCESS (Color){46, 204, 64, 255}           // Vibrant green
#define UI_WARNING (Color){255, 173, 15, 255}          // Vibrant yellow

// Các hằng số cho Edge List drawing.
#define EDGE_THICKNESS 6.0f       // Độ dày cạnh trong Edge List khi highlight.
#define EDGE_NORMAL_THICKNESS 2.0f // Độ dày mặc định.

// Các hằng số animation.
#define COLOR_ANIM_DURATION 800  // Tăng từ 500 lên 800ms
#define POS_ANIM_FRAMES 45      // Tăng từ 30 lên 45 frames
#define FRAME_DELAY_MS 30       // Tăng từ 20 lên 30ms

// Các hằng số cho DSU.
#define DSU_START_X 20
#define DSU_START_Y 20
#define DSU_CELL_HEIGHT 25

///////////////////////////////////////////////////////////
// MST methods implementation

MST::MST(int n, int m) : n(n), m(m) {
    parent.resize(n + 1);
    for (int i = 0; i <= n; i++) {
        parent[i] = i;
    }
    highlightVertex1 = -1;
    highlightVertex2 = -1;
    
    // Initialize physics interaction properties
    selectedVertex = -1;
    C_rep = 1000000.0f;     // Repulsive force constant
    c_spring = 5.0f;        // Spring force constant
    L = 225.0f;             // Ideal spring length
    timeStep = 0.05f;       // Physics timestep
    damping = 0.7f;         // Velocity damping
    physicsIterations = 3;  // Physics iteration per frame
    vertexRadius = VERTEX_RADIUS;
    fixedPositionMode = false; // NEW: Initialize in physics mode by default
    
    // Initialize animation tracking variables
    currentFrame = 0;
    isPlayingAnimation = false;
    isPaused = false;
    frameSkip = 1;
    isDraggingProgressBar = false;
    showPseudoCode = false; // NEW: Start with pseudocode hidden
}

void MST::addEdge(int u, int v, int w) {
    Edge e;
    e.u = u; e.v = v; e.w = w;
    e.color = DARKGRAY; // Thay đổi màu mặc định sang DARKGRAY
    // Tính vị trí đỉnh cho đồ thị (đặt các đỉnh trên vòng tròn ở nửa bên phải).
    int centerX = SCREEN_WIDTH * 3 / 4;
    int centerY = SCREEN_HEIGHT / 2;
    int radius = 300;
    float angleU = 2 * PI * (u - 1) / n;
    float angleV = 2 * PI * (v - 1) / n;
    e.posU = { centerX + radius * cos(angleU), centerY + radius * sin(angleU) };
    e.posV = { centerX + radius * cos(angleV), centerY + radius * sin(angleV) };
    
    // Place new edges off-screen initially
    e.currentPos = { (float)EDGE_LIST_START_X, SCREEN_HEIGHT + 100.0f };
    e.targetPos = e.currentPos; // Initially target position is the same as current
    
    edges.push_back(e);
}

void MST::sortEdges() {
    std::sort(edges.begin(), edges.end(), [](const Edge &a, const Edge &b) {
        return a.w < b.w;
    });
    
    // Get the position of the title to position edges below it
    float edgeListStartY = DSU_START_Y + (n + 3) * DSU_CELL_HEIGHT + 50;
    // Add spacing for the title
    float edgeListContentY = edgeListStartY + 35; // Space below the title
    
    // Calculate max items per column based on screen height
    int itemsPerColumn = (SCREEN_HEIGHT - edgeListContentY - 50) / EDGE_LIST_GAP;
    if (itemsPerColumn <= 0) itemsPerColumn = EDGE_LIST_ITEMS_PER_COLUMN; // Fallback
    
    // Update targetPos for each edge in a column-first layout
    for (int i = 0; i < edges.size(); i++) {
        int col = i / itemsPerColumn;
        int row = i % itemsPerColumn;
        
        edges[i].targetPos = { 
            (float)(EDGE_LIST_START_X + col * EDGE_LIST_COLUMN_GAP), 
            (float)(edgeListContentY + row * EDGE_LIST_GAP) 
        };
    }
}

int MST::findParent(int u) {
    if (u == parent[u]) return u;
    parent[u] = findParent(parent[u]);
    return parent[u];
}

void MST::unionVertices(int u, int v) {
    int pu = findParent(u);
    int pv = findParent(v);
    parent[pu] = pv;
}

Color MST::interpolateColor(Color start, Color end, float t) {
    Color result;
    result.r = start.r + (int)((end.r - start.r) * t);
    result.g = start.g + (int)((end.g - start.g) * t);
    result.b = start.b + (int)((end.b - start.b) * t);
    result.a = start.a + (int)((end.a - start.a) * t);
    return result;
}

void MST::animateEdgeColor(Edge &e, Color start, Color end, int durationMs) {
    int frames = durationMs / FRAME_DELAY_MS;
    for (int f = 0; f <= frames; f++) {
        float t = (float)f / frames;
        e.color = interpolateColor(start, end, t);
    }
}

// ADDED: Set pseudocode for Kruskal's algorithm
void MST::setPseudoCodeKruskal() {
    currentOperation = "Kruskal's MST Algorithm";
    currentPseudoCode.clear();
    
    // Use the actual pseudocode with empty lines in between for clearer display
    currentPseudoCode.push_back("Sort E edges by increasing weight");
    currentPseudoCode.push_back("T = {}");
    currentPseudoCode.push_back("for (i = 0; i < edgeList.length; i++)");
    currentPseudoCode.push_back("  if adding e = edgelist[i] does not form a cycle");
    currentPseudoCode.push_back("    add e to T");
    currentPseudoCode.push_back("  else ignore e");
    currentPseudoCode.push_back("MST = T");
    
    currentHighlightedLine = 0;
}

// ADDED: Update highlighted line
void MST::updatePseudoCodeHighlight(int line) {
    currentHighlightedLine = line;
    // Add current highlighted line to the frame recording
    pseudoCodeHighlightFrames.push_back(line);
}

// ADDED: Update the drawPseudoCode method with a unique, elegant button design
void MST::drawPseudoCode() {
    // Create a stylish pill-shaped button with gradient and shadow
    float buttonX = 1650;
    float buttonY = 1000;
    if(showPseudoCode == true) buttonY = 700;
    float buttonWidth = 220;
    float buttonHeight = 40;
    
    // Draw shadow for floating effect
    DrawRectangleRounded(
        (Rectangle){buttonX + 3, buttonY + 3, buttonWidth, buttonHeight},
        0.5, 8, (Color){20, 20, 20, 50}
    );
    
    // Draw main button background - gradient
    Color startColor = showPseudoCode ? (Color){142, 68, 173, 240} : (Color){52, 152, 219, 240};
    Color endColor = showPseudoCode ? (Color){155, 89, 182, 240} : (Color){41, 128, 185, 240};
    
    // Create a pill shape with two half-circles and a rectangle
    float radius = buttonHeight / 2;
    
    // Left half-circle
    DrawCircleGradient(
        buttonX + radius, buttonY + radius,
        radius,
        startColor, endColor
    );
    
    // Right half-circle
    DrawCircleGradient(
        buttonX + buttonWidth - radius, buttonY + radius,
        radius,
        endColor, startColor
    );
    
    // Center rectangle with gradient
    DrawRectangleGradientH(
        buttonX + radius, buttonY,
        buttonWidth - buttonHeight, buttonHeight,
        startColor, endColor
    );
    
    // Draw outline
    DrawRing(
        (Vector2){buttonX + radius, buttonY + radius}, 
        radius - 1, radius, 0, 180, 36, 
        showPseudoCode ? (Color){142, 68, 173, 255} : (Color){41, 128, 185, 255}
    );
    DrawRing(
        (Vector2){buttonX + buttonWidth - radius, buttonY + radius}, 
        radius - 1, radius, 180, 360, 36, 
        showPseudoCode ? (Color){142, 68, 173, 255} : (Color){41, 128, 185, 255}
    );
    DrawRectangle(
        buttonX + radius, buttonY - 1,
        buttonWidth - buttonHeight, 2,
        showPseudoCode ? (Color){142, 68, 173, 255} : (Color){41, 128, 185, 255}
    );
    DrawRectangle(
        buttonX + radius, buttonY + buttonHeight - 1,
        buttonWidth - buttonHeight, 2,
        showPseudoCode ? (Color){142, 68, 173, 255} : (Color){41, 128, 185, 255}
    );
    
    // Draw icon instead of text
    if (showPseudoCode) {
        // Eye icon with slash (hide)
        DrawCircleLines(buttonX + 50, buttonY + radius, 10, WHITE);
        DrawLine(buttonX + 38, buttonY + radius - 12, buttonX + 62, buttonY + radius + 12, WHITE);
    } else {
        // Eye icon (show)
        DrawCircleLines(buttonX + 50, buttonY + radius, 10, WHITE);
        DrawCircle(buttonX + 50, buttonY + radius, 4, WHITE);
    }
    
    // Draw text with shadow effect for depth
    const char* buttonText = showPseudoCode ? "Hide Algorithm" : "Show Algorithm";
    DrawText(buttonText, buttonX + 80 + 1, buttonY + 13 + 1, 18, (Color){0, 0, 0, 120});
    DrawText(buttonText, buttonX + 80, buttonY + 13, 18, WHITE);
    
    // Only draw pseudocode if toggle is on
    if (showPseudoCode && !currentPseudoCode.empty()) {
        int startX = 1400;
        int startY = 800;
        int lineHeight = 30;
        int paddingX = 20;
        int paddingY = 15;
        
        // Calculate box dimensions based on content
        int boxWidth = 0;
        for (const auto& line : currentPseudoCode) {
            int width = MeasureText(line.c_str(), 20) + 2 * paddingX;
            boxWidth = std::max(boxWidth, width);
        }
        
        // Make sure the box is wide enough for the title too
        int titleWidth = MeasureText(currentOperation.c_str(), 24) + 2 * paddingX;
        boxWidth = std::max(boxWidth, titleWidth);
        
        int boxHeight = currentPseudoCode.size() * lineHeight + 2 * paddingY + lineHeight; // Extra line for title
        
        // Draw background box with drop shadow
        DrawRectangle(startX - paddingX + 5, startY - lineHeight - paddingY + 5, 
                     boxWidth, boxHeight, (Color){40, 40, 40, 100}); // Shadow
        
        DrawRectangleRounded(
            (Rectangle){startX - paddingX, startY - lineHeight - paddingY, boxWidth, boxHeight}, 
            0.1, 8, (Color){240, 240, 240, 240}
        );
        
        DrawRectangleRoundedLines(
            (Rectangle){startX - paddingX, startY - lineHeight - paddingY, boxWidth, boxHeight}, 
            0.1, 8, (Color){100, 100, 100, 200}
        );
        
        // Draw operation name (title)
        DrawTextEx(customFont, currentOperation.c_str(), (Vector2){startX, startY - lineHeight}, 24, 1, BLACK);
        
        // Draw each line of pseudocode
        for (size_t i = 0; i < currentPseudoCode.size(); i++) {
            Color textColor = (i == currentHighlightedLine) ? RED : BLACK;
            DrawTextEx(customFont, currentPseudoCode[i].c_str(), (Vector2){startX, startY + i * lineHeight}, 20, 1, textColor);
        }
    }
}

// NEW: Add handler for pseudocode toggle button
void MST::handlePseudoCodeToggle() {
    Vector2 mousePos = GetMousePosition();
    
    float buttonX = 1650;
    float buttonY = 1000;
    if(showPseudoCode == true) buttonY = 700;
    float buttonWidth = 220;
    float buttonHeight = 40;
    
    // Check for collision with the pill-shaped button
    bool collision = false;
    float radius = buttonHeight / 2;
    
    // Check left circle
    if (CheckCollisionPointCircle(mousePos, (Vector2){buttonX + radius, buttonY + radius}, radius)) {
        collision = true;
    }
    // Check right circle
    else if (CheckCollisionPointCircle(mousePos, (Vector2){buttonX + buttonWidth - radius, buttonY + radius}, radius)) {
        collision = true;
    }
    // Check middle rectangle
    else if (CheckCollisionPointRec(mousePos, (Rectangle){buttonX + radius, buttonY, buttonWidth - buttonHeight, buttonHeight})) {
        collision = true;
    }
    
    if (collision && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        showPseudoCode = !showPseudoCode;
    }
}

// ADDED: Step backward to previous highlighted line
void MST::stepBackward() {
    if (!pseudoCodeHighlightFrames.empty() && currentFrame > 0) {
        fixedPositionMode = true;
        // Get current highlighted line
        int currentHighlightedLine = pseudoCodeHighlightFrames[currentFrame];
        
        // First find the previous different highlighted line
        int previousLine = -1;
        int targetStep = currentFrame;
        
        while (targetStep >= 0) {
            if (pseudoCodeHighlightFrames[targetStep] != currentHighlightedLine) {
                previousLine = pseudoCodeHighlightFrames[targetStep];
                break;
            }
            targetStep--;
        }
        
        // Jump to the last frame of the previous line
        currentFrame = std::max(0, targetStep);
        
        // Temporarily force fixed position mode to prevent physics effects
        bool wasFixedMode = fixedPositionMode;
        fixedPositionMode = true;
        
        // Update current state from stored animation frame
        if (currentFrame < animationFrames.size()) {
            edges = animationFrames[currentFrame];
            parent = parentFrames[currentFrame];
            
            if (currentFrame < vertexPositionFrames.size()) {
                // Set vertex positions directly without animation/physics
                vertexPositions = vertexPositionFrames[currentFrame];
                
                // Reset all velocities to zero to prevent bounce effect
                for (int i = 0; i < velocities.size(); i++) {
                    velocities[i] = {0, 0};
                }
            }
            
            // Update edge positions to exactly match vertex positions
            for (auto &edge : edges) {
                edge.posU = vertexPositions[edge.u];
                edge.posV = vertexPositions[edge.v];
            }
            
            if (highlightFrames[currentFrame].first != -1) {
                highlightVertex1 = highlightFrames[currentFrame].first;
                highlightVertex2 = highlightFrames[currentFrame].second;
            } else {
                highlightVertex1 = highlightVertex2 = -1;
            }
            
            // Update current highlighted line
            this->currentHighlightedLine = pseudoCodeHighlightFrames[currentFrame];
        }
        
        isPaused = true; // Pause playback when stepping manually
        
        // Optional: Restore the original mode setting after a short delay
        // For now, we'll keep it in fixed mode to prevent physics from taking over
        // fixedPositionMode = wasFixedMode;
    }

}

// ADDED: Step forward to next highlighted line
void MST::stepForward() {
    
    if (!pseudoCodeHighlightFrames.empty() && currentFrame < pseudoCodeHighlightFrames.size() - 1) {
        fixedPositionMode = true;
        // Get current highlighted line

        int currentHighlightedLine = pseudoCodeHighlightFrames[currentFrame];
        
        // First find the next different highlighted line
        int nextLine = -1;
        int firstFrameOfNextLine = -1;
        int targetStep = currentFrame;
        
        // Find the first occurrence of a different highlighted line
        while (targetStep < pseudoCodeHighlightFrames.size()) {
            if (pseudoCodeHighlightFrames[targetStep] != currentHighlightedLine) {
                nextLine = pseudoCodeHighlightFrames[targetStep];
                firstFrameOfNextLine = targetStep;
                break;
            }
            targetStep++;
        }
        
        if (nextLine != -1) {
            // Now find the last frame with this new highlighted line
            int lastFrameOfNextLine = firstFrameOfNextLine;
            targetStep = firstFrameOfNextLine;
            
            while (targetStep < pseudoCodeHighlightFrames.size()) {
                if (pseudoCodeHighlightFrames[targetStep] == nextLine) {
                    lastFrameOfNextLine = targetStep;
                } else {
                    // Found a different highlight, stop here
                    break;
                }
                targetStep++;
            }
            
            // Jump to the last frame of the next line
            currentFrame = lastFrameOfNextLine;
        } else {
            // If no next line, go to the last frame
            currentFrame = pseudoCodeHighlightFrames.size() - 1;
        }
        
        // Temporarily force fixed position mode to prevent physics effects
        bool wasFixedMode = fixedPositionMode;
        fixedPositionMode = true;
        
        // Update current state from stored animation frame
        if (currentFrame < animationFrames.size()) {
            edges = animationFrames[currentFrame];
            // Update edge positions to exactly match vertex positions
            for (auto &edge : edges) {
                edge.posU = vertexPositions[edge.u];
                edge.posV = vertexPositions[edge.v];
            }
            
            if (highlightFrames[currentFrame].first != -1) {
                highlightVertex1 = highlightFrames[currentFrame].first;
                highlightVertex2 = highlightFrames[currentFrame].second;
            } else {
                highlightVertex1 = highlightVertex2 = -1;
            }
            
            // Update current highlighted line
            this->currentHighlightedLine = pseudoCodeHighlightFrames[currentFrame];
        }
        
        isPaused = true; // Pause playback when stepping manually
    }
}

void MST::runKruskal() {
    // Initialize pseudocode
    
    setPseudoCodeKruskal();
    
    // Clear any previous animation frames
    animationFrames.clear();
    parentFrames.clear();
    highlightFrames.clear();
    vertexPositionFrames.clear();
    pseudoCodeHighlightFrames.clear();
    currentFrame = 0;
    
    // Highlight "Sort E edges by increasing weight" line
    updatePseudoCodeHighlight(0);
    
    // Capture initial state before sorting
    for(int i = 0; i < 30; i++){
        animationFrames.push_back(edges);
        parentFrames.push_back(parent);
        highlightFrames.push_back({-1, -1});
        vertexPositionFrames.push_back(vertexPositions);
        pseudoCodeHighlightFrames.push_back(currentHighlightedLine);
    }
    
    // Sort edges
    sortEdges();
    
    // Capture the animation of edges moving to their sorted positions
    int frames = POS_ANIM_FRAMES;
    for (int f = 0; f <= frames; f++) {
        for (auto &e : edges) {
            // Use smooth easing for better animation
            float progress = (float)f / frames;
            float easedProgress = progress * progress * (3.0f - 2.0f * progress); // Smoothstep easing
            
            // Gradually move edges to their target positions with easing
            e.currentPos.x = e.currentPos.x + (e.targetPos.x - e.currentPos.x) * easedProgress;
            e.currentPos.y = e.currentPos.y + (e.targetPos.y - e.currentPos.y) * easedProgress;
            
        }
        
        // Capture each frame of the sorting animation
        animationFrames.push_back(edges);
        parentFrames.push_back(parent);
        highlightFrames.push_back({-1, -1});
        vertexPositionFrames.push_back(vertexPositions);
        pseudoCodeHighlightFrames.push_back(currentHighlightedLine);
    }
    
    int number_mst_edges = 0;
    int totalWeight = 0;
    
    // Start automatic animation playback
    isPlayingAnimation = true;
    //isPaused = false;
    
    // Highlight "T = {}" line
    updatePseudoCodeHighlight(1);
    
    // Capture initial state after sorting
    for(int i = 0; i < 60; i++){
        animationFrames.push_back(edges);
        parentFrames.push_back(parent);
        highlightFrames.push_back({-1, -1});
        vertexPositionFrames.push_back(vertexPositions);
        pseudoCodeHighlightFrames.push_back(currentHighlightedLine);
    }
    
    for (int i = 0; i < edges.size(); i++) {
        Edge &e = edges[i];

        highlightVertex1 = e.u;
        highlightVertex2 = e.v;
        
        // Highlight "if adding e = edgelist[i] does not form a cycle" line
        updatePseudoCodeHighlight(2);
        
        // Capture frame with highlighted vertices
        for(int j = 0; j < 60; j++){
            animationFrames.push_back(edges);
            parentFrames.push_back(parent);
            highlightFrames.push_back({e.u, e.v});
            vertexPositionFrames.push_back(vertexPositions);
            pseudoCodeHighlightFrames.push_back(currentHighlightedLine);
        }

        // Đổi sang màu cam khi xét
        e.color = ORANGE;
        updatePseudoCodeHighlight(3);
        // Capture frame after changing color
        for(int j = 0; j < 60; j++){
            animationFrames.push_back(edges);
            parentFrames.push_back(parent);
            highlightFrames.push_back({e.u, e.v});
            vertexPositionFrames.push_back(vertexPositions);
            pseudoCodeHighlightFrames.push_back(currentHighlightedLine);
        }

        int pu = findParent(e.u);
        int pv = findParent(e.v);
        
        if (pu == pv) {
            // Nếu bị loại, fade out
            updatePseudoCodeHighlight(5);
            for (int f = 0; f <= COLOR_ANIM_DURATION/FRAME_DELAY_MS; f++) {
                // Calculate fade progress (0.0 to 1.0)
                float fadeProgress = (float)f / (COLOR_ANIM_DURATION/FRAME_DELAY_MS);
                
                // Gradually decrease alpha from original color to transparent
                Color fadeColor = e.color;
                fadeColor.a = (unsigned char)(255 * (1.0f - fadeProgress));
                e.color = fadeColor;
                
                animationFrames.push_back(edges);
                parentFrames.push_back(parent);
                highlightFrames.push_back({e.u, e.v});
                vertexPositionFrames.push_back(vertexPositions);
                pseudoCodeHighlightFrames.push_back(currentHighlightedLine);
            }
            
            // Finally set to completely transparent
            e.color = (Color){0, 0, 0, 0};
            
            for(int j = 0; j < 120; j++){
                animationFrames.push_back(edges);
                parentFrames.push_back(parent);
                highlightFrames.push_back({-1, -1});
                vertexPositionFrames.push_back(vertexPositions);
                pseudoCodeHighlightFrames.push_back(currentHighlightedLine);
            }
        } else {
            // Highlight "add e to T" line
            updatePseudoCodeHighlight(4);
            
            unionVertices(e.u, e.v);
            number_mst_edges++;
            totalWeight += e.w;
            
            // Capture frame after union
            animationFrames.push_back(edges);
            parentFrames.push_back(parent);
            highlightFrames.push_back({e.u, e.v});
            vertexPositionFrames.push_back(vertexPositions);
            pseudoCodeHighlightFrames.push_back(currentHighlightedLine);
            
            // Change color without animation drawing
            e.color = GREEN;
            
            // Capture frames during color change - without actual drawing
            for (int f = 0; f <= COLOR_ANIM_DURATION/FRAME_DELAY_MS; f++) {
                animationFrames.push_back(edges);
                parentFrames.push_back(parent);
                highlightFrames.push_back({e.u, e.v});
                vertexPositionFrames.push_back(vertexPositions);
                pseudoCodeHighlightFrames.push_back(currentHighlightedLine);
            }
            for(int j = 0; j < 120; j++){
                animationFrames.push_back(edges);
                parentFrames.push_back(parent);
                highlightFrames.push_back({-1, -1});
                vertexPositionFrames.push_back(vertexPositions);
                pseudoCodeHighlightFrames.push_back(currentHighlightedLine);
            }
            mstEdges.push_back(e);
        }
        
        // Go back to the loop line
        //updatePseudoCodeHighlight(2);
        
        highlightVertex1 = -1;
        highlightVertex2 = -1;
        
        // Capture frame after clearing highlights
        animationFrames.push_back(edges);
        parentFrames.push_back(parent);
        highlightFrames.push_back({-1, -1});
        vertexPositionFrames.push_back(vertexPositions);
        pseudoCodeHighlightFrames.push_back(currentHighlightedLine);
    }
    
    // Highlight "MST = T" line
    updatePseudoCodeHighlight(6);
    
    std::cout << "Total weight of MST: " << totalWeight << std::endl;
    
    // After animation is complete, make sure to show final state
    for(int j = 0; j < 120; j++){
       animationFrames.push_back(edges);
        parentFrames.push_back(parent);
        highlightFrames.push_back({-1, -1});
        vertexPositionFrames.push_back(vertexPositions);
        pseudoCodeHighlightFrames.push_back(currentHighlightedLine);
    }
}

///////////////////////////////////////////////////////////
// Visualization methods

// Sửa lại hàm vẽ DSU theo cột dọc
void MST::drawDSU() {
    // Vẽ tiêu đề cho DSU
    DrawText("Disjoint Set Union", DSU_START_X, DSU_START_Y, 25, DARKGRAY);
    
    // Vẽ bảng DSU
    for (int i = 1; i <= n; i++) {
        int y = DSU_START_Y + 40 + (i-1) * DSU_CELL_HEIGHT;
        Color colText = (i == highlightVertex1 || i == highlightVertex2) ? ORANGE : BLACK;
        std::string s = "pa[" + std::to_string(i) + "] = " + std::to_string(parent[i]);
        DrawText(s.c_str(), DSU_START_X, y, 20, colText);
    }
}

// Vẽ danh sách cạnh (Edge List) dựa trên currentPos.
void MST::drawEdgeList() {
    // Calculate title position based on DSU size
    float edgeListStartY = DSU_START_Y + (n + 3) * DSU_CELL_HEIGHT + 50;
    
    // Vẽ tiêu đề Edge List 
    DrawText("Edge List (sorted by weight)", DSU_START_X, edgeListStartY, 25, DARKGRAY);
    
    // Draw each edge (they're now positioned in rows)
    for (int i = 0; i < edges.size(); i++) {
        if (edges[i].color.a == 0) continue;
        
        // Sử dụng currentPos cho animation
        Vector2 lineStart = edges[i].currentPos;
        Vector2 lineEnd = { lineStart.x + FIXED_EDGE_LIST_LENGTH, lineStart.y };
        
        float thickness = (edges[i].color.r == ORANGE.r || edges[i].color.r == GREEN.r) ? 
                         EDGE_THICKNESS : EDGE_NORMAL_THICKNESS;
        DrawLineEx(lineStart, lineEnd, thickness, edges[i].color);
        std::string s = std::to_string(edges[i].u) + "-" + std::to_string(edges[i].v) +
                        " (" + std::to_string(edges[i].w) + ")";
        DrawText(s.c_str(), lineEnd.x + 10, lineStart.y - 5, 20, edges[i].color);
    }
}

// Vẽ đồ thị: các đỉnh trên vòng tròn và các cạnh được vẽ không nối từ tâm đến tâm mà từ viền đỉnh.
void MST::drawGraph() {
    // Draw mode toggle button first
    drawModeToggle();
    
    // Draw pseudocode
    drawPseudoCode();
    
    // First draw edges to ensure they appear behind vertices
    for (auto &e : edges) {
        if (e.color.a == 0) continue;
        
        // Calculate direction vector from posU to posV
        Vector2 dir = { e.posV.x - e.posU.x, e.posV.y - e.posU.y };
        float len = sqrt(dir.x * dir.x + dir.y * dir.y);
        if (len == 0) continue;
        
        // Normalize direction vector
        dir.x /= len; dir.y /= len;
        
        // Calculate start and end points, leaving space for vertex
        Vector2 start = { e.posU.x + dir.x * VERTEX_RADIUS, e.posU.y + dir.y * VERTEX_RADIUS };
        Vector2 end = { e.posV.x - dir.x * VERTEX_RADIUS, e.posV.y - dir.y * VERTEX_RADIUS };
        
        // Use enhanced thickness based on edge state
        float thickness = (e.color.r == ORANGE.r || e.color.r == GREEN.r) ? 
                         EDGE_GRAPH_HIGHLIGHT_THICKNESS : EDGE_GRAPH_THICKNESS;
        
        // Draw edge with slight transparency for better look
        Color edgeColor = e.color;
        if (edgeColor.r == DARKGRAY.r && edgeColor.g == DARKGRAY.g && edgeColor.b == DARKGRAY.b) {
            edgeColor = EDGE_DEFAULT_COLOR; // Use custom gray for better visibility
        }
        
        // Draw edge with transparency gradient for depth effect
        Color startColor = edgeColor;
        Color endColor = edgeColor;
        startColor.a = 220;
        endColor.a = 220;
        
        DrawLineEx(start, end, thickness, edgeColor);
        
        // Calculate midpoint for weight label
        Vector2 mid = { (start.x + end.x) / 2, (start.y + end.y) / 2 };
        
        // Calculate perpendicular direction for weight offset
        Vector2 perp = { -dir.y, dir.x };
        Vector2 weightPos = { mid.x + perp.x * WEIGHT_OFFSET, mid.y + perp.y * WEIGHT_OFFSET };
        
        // Prepare weight text with better visibility
        std::string weightStr = std::to_string(e.w);
        int textWidth = MeasureText(weightStr.c_str(), 20);
        int textHeight = 20;
        
        // Draw background bubble for weight text
        DrawRectangleRounded(
            (Rectangle){
                weightPos.x - textWidth/2 - WEIGHT_BUBBLE_PADDING/2,
                weightPos.y - textHeight/2 - WEIGHT_BUBBLE_PADDING/2,
                textWidth + WEIGHT_BUBBLE_PADDING,
                textHeight + WEIGHT_BUBBLE_PADDING
            },
            0.5f, 8, WEIGHT_BG_COLOR
        );
        
        // Draw weight text with slight shadow for better visibility
        DrawText(weightStr.c_str(), weightPos.x - textWidth/2 + 1, weightPos.y - textHeight/2 + 1, 20, (Color){100, 100, 100, 128});
        DrawText(weightStr.c_str(), weightPos.x - textWidth/2, weightPos.y - textHeight/2, 20, edgeColor);
    }
    
    // Draw vertices with enhanced styling
    for (int i = 1; i <= n; i++) {
        Vector2 pos = vertexPositions[i];
        
        // Determine vertex color based on highlight state
        Color vertexColor = VERTEX_FILL_COLOR;
        Color textColor = VERTEX_TEXT_COLOR;
        
        if (i == highlightVertex1 || i == highlightVertex2) {
            vertexColor = VERTEX_HIGHLIGHT_COLOR;
        }
        
        // Draw shadow for 3D effect
        DrawCircle(pos.x + VERTEX_SHADOW_OFFSET, pos.y + VERTEX_SHADOW_OFFSET, 
                   VERTEX_RADIUS, (Color){0, 0, 0, 60});
        
        // Draw outer ring
        DrawCircle(pos.x, pos.y, VERTEX_RADIUS, VERTEX_BORDER_COLOR);
        
        // Draw main vertex with gradient for 3D effect
        DrawCircleGradient(
            pos.x, pos.y, 
            VERTEX_INNER_RADIUS, 
            vertexColor, 
            ColorBrightness(vertexColor, 0.7f)
        );
        
        // Draw vertex value with improved text positioning
        std::string s = std::to_string(i);
        int textWidth = MeasureText(s.c_str(), 22); // Slightly larger font
        
        // Draw text with shadow for better visibility
        DrawText(s.c_str(), pos.x - textWidth/2 + 1, pos.y - 11 + 1, 22, (Color){0, 0, 0, 128});
        DrawText(s.c_str(), pos.x - textWidth/2, pos.y - 11, 22, textColor);
    }
}

// NEW: Draw mode toggle button separately
void MST::drawModeToggle() {
    float buttonX = 1750;
    float buttonY = 120;
    Rectangle modeToggleButton = {buttonX, buttonY, 150, 50}; // Larger button
    
    DrawRectangleRounded(modeToggleButton, 0.3, 6, 
                        fixedPositionMode ? (Color){155, 89, 182, 230} : (Color){52, 152, 219, 230});
    DrawRectangleRoundedLines(modeToggleButton, 0.3, 6, 
                             fixedPositionMode ? (Color){142, 68, 173, 255} : (Color){41, 128, 185, 255});
    
    const char* modeText = fixedPositionMode ? "Fixed Position" : "Physics Mode";
    DrawText(modeText, modeToggleButton.x + 5, modeToggleButton.y + 15, 20, WHITE);
}

// NEW: Handle mode toggle button interaction separately
void MST::handleModeToggleInteraction() {
    Vector2 mousePos = GetMousePosition();
    
    float buttonX = 1750;
    float buttonY = 120;
    Rectangle modeToggleButton = {buttonX, buttonY, 150, 50}; // Match size with drawModeToggle
    
    if (CheckCollisionPointRec(mousePos, modeToggleButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        toggleFixedPositionMode();
    }
}

// Modify drawProgressBar to include stepping buttons
void MST::drawProgressBar() {
    if (animationFrames.empty()) return;
    
    // Increase progress bar size
    float progressWidth = 500; // Increased from 300
    float progressHeight = 25; // Increased from 20
    float progressX = (1920 - progressWidth) / 2;
    float progressY = 1080 - 100; // Position at the bottom with some margin
    
    // Update progress bar rectangle for interaction
    progressBarRect = (Rectangle){progressX, progressY, progressWidth, progressHeight};
    
    // Draw scrubber track with better appearance - add shadows for depth
    DrawRectangleRounded(
        (Rectangle){progressX - 3, progressY - 3, progressWidth + 6, progressHeight + 6}, 
        0.5, 8, (Color){40, 40, 40, 100} // Shadow
    );
    
    // Draw track background with gradient effect
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
    float progress = (float)currentFrame / (animationFrames.size() - 1);
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
        TextFormat("Frame: %d / %d", currentFrame, (int)animationFrames.size()-1),
        progressX, progressY - 30, 22, (Color){50, 50, 50, 255}
    );
    
    // LARGER BUTTONS: Play/pause button
    Rectangle playPauseButton = {progressX - 70, progressY - 10, 50, 45}; // Much larger
    DrawRectangleRounded(playPauseButton, 0.3, 6, 
                        isPaused ? (Color){52, 152, 219, 230} : (Color){46, 204, 113, 230});
    DrawRectangleRoundedLines(playPauseButton, 0.3, 6, 
                             isPaused ? (Color){41, 128, 185, 255} : (Color){39, 174, 96, 255});
    
    // Draw play/pause icon - larger
    if (isPaused) {
        DrawTriangle(
            (Vector2){playPauseButton.x + 13, playPauseButton.y + 8},
            (Vector2){playPauseButton.x + 13, playPauseButton.y + 37},
            (Vector2){playPauseButton.x + 40, playPauseButton.y + 22},
            WHITE
        );
    } else {
        DrawRectangle(playPauseButton.x + 15, playPauseButton.y + 8, 8, 29, WHITE);
        DrawRectangle(playPauseButton.x + 28, playPauseButton.y + 8, 8, 29, WHITE);
    }
    
    // LARGER BUTTONS: Speed controls
    Rectangle speedDecButton = {progressX + progressWidth + 20, progressY - 10, 45, 45}; // Larger
    Rectangle speedIncButton = {progressX + progressWidth + 70, progressY - 10, 45, 45}; // Larger
    
    DrawRectangleRounded(speedDecButton, 0.3, 6, (Color){192, 57, 43, 230});
    DrawRectangleRoundedLines(speedDecButton, 0.3, 6,  (Color){150, 40, 30, 255});
    
    DrawRectangleRounded(speedIncButton, 0.3, 6, (Color){39, 174, 96, 230});
    DrawRectangleRoundedLines(speedIncButton, 0.3, 6,  (Color){33, 148, 83, 255});
    
    // Larger text on buttons
    DrawText("-", speedDecButton.x + 18, speedDecButton.y + 10, 25, WHITE);
    DrawText("+", speedIncButton.x + 16, speedIncButton.y + 10, 25, WHITE);
    
    DrawText(TextFormat("Speed: %dx", frameSkip), 
            speedDecButton.x, speedDecButton.y - 30, 20, DARKGRAY);
    
    // LARGER BUTTONS: Step navigation buttons
    Rectangle backwardButton = {progressX - 130, progressY - 10, 55, 45}; // Much larger
    Rectangle forwardButton = {progressX - 195, progressY - 10, 55, 45}; // Much larger
    
    Color backwardColor = (currentFrame > 0) ? 
                      (Color){41, 128, 185, 230} : (Color){150, 150, 150, 150};
    Color forwardColor = (currentFrame < animationFrames.size() - 1) ? 
                      (Color){41, 128, 185, 230} : (Color){150, 150, 150, 150};
    
    DrawRectangleRounded(backwardButton, 0.3, 6, backwardColor);
    DrawRectangleRoundedLines(backwardButton, 0.3, 6, 
                           (currentFrame > 0) ? (Color){25, 80, 130, 255} : (Color){100, 100, 100, 150});
    
    DrawRectangleRounded(forwardButton, 0.3, 6, forwardColor);
    DrawRectangleRoundedLines(forwardButton, 0.3, 6, 
                           (currentFrame < animationFrames.size() - 1) ? (Color){25, 80, 130, 255} : (Color){100, 100, 100, 150});
    
    // Larger text on buttons
    DrawText("Next", backwardButton.x + 10, backwardButton.y + 12, 20, WHITE);
    DrawText("Prev", forwardButton.x + 10, forwardButton.y + 12, 20, WHITE);
    
    DrawText("Step By Code Line", backwardButton.x - 30, backwardButton.y - 30, 20, DARKGRAY);
    
    // Draw pseudocode
    drawPseudoCode();
}

void MST::handleProgressBarInteraction() {
    if (animationFrames.empty()) return;
    
    Vector2 mousePos = GetMousePosition();
    
    // Update button coordinates to match the new sizes in drawProgressBar
    float progressX = progressBarRect.x;
    float progressY = progressBarRect.y;
    float progressWidth = progressBarRect.width;
    
    // Check for play/pause button click - updated size
    Rectangle playPauseButton = {progressX - 70, progressY - 10, 50, 45};
    if (CheckCollisionPointRec(mousePos, playPauseButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        isPaused = !isPaused;
        isPlayingAnimation = !isPaused;
        return;
    }
    
    // Updated button sizes for interaction detection
    Rectangle forwardButton = {progressX - 130, progressY - 10, 55, 45};
    Rectangle backwardButton = {progressX - 195, progressY - 10, 55, 45};
    
    if (CheckCollisionPointRec(mousePos, backwardButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        stepBackward();
        return;
    }
    
    if (CheckCollisionPointRec(mousePos, forwardButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        stepForward();
        return;
    }
    
    // Check for speed control clicks - updated sizes
    Rectangle speedDecButton = {progressX + progressWidth + 20, progressY - 10, 45, 45};
    Rectangle speedIncButton = {progressX + progressWidth + 70, progressY - 10, 45, 45};
    
    if (CheckCollisionPointRec(mousePos, speedDecButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (frameSkip > 1) frameSkip--;
        return;
    }
    
    if (CheckCollisionPointRec(mousePos, speedIncButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (frameSkip < 10) frameSkip++;
        return;
    }
    
    // Handle mouse press on progress bar
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && 
        CheckCollisionPointRec(mousePos, (Rectangle){
            progressBarRect.x - 10, // Make the hit area a bit larger
            progressBarRect.y - 10,
            progressBarRect.width + 20,
            progressBarRect.height + 20
        })) {
        isDraggingProgressBar = true;
    }
    
    // Handle mouse drag on progress bar
    if (isDraggingProgressBar) {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            // Calculate new position based on mouse position
            float progress = (mousePos.x - progressBarRect.x) / progressBarRect.width;
            
            // Clamp progress between 0 and 1
            progress = progress < 0 ? 0 : (progress > 1 ? 1 : progress);
            
            // Calculate new frame position
            currentFrame = (int)(progress * (animationFrames.size() - 1));
            
            // Ensure currentFrame is within valid range
            currentFrame = currentFrame < 0 ? 0 : 
                         (currentFrame >= animationFrames.size() ? 
                          animationFrames.size() - 1 : currentFrame);
            
            // Update current state from stored animation frame
            if (currentFrame < animationFrames.size()) {
                edges = animationFrames[currentFrame];
                parent = parentFrames[currentFrame];
                
                // NEW: Restore vertex positions from saved frame
                if (currentFrame < vertexPositionFrames.size()) {
                    vertexPositions = vertexPositionFrames[currentFrame];
                }
                
                // Update edge positions to match restored vertex positions
                for (auto &edge : edges) {
                    edge.posU = vertexPositions[edge.u];
                    edge.posV = vertexPositions[edge.v];
                }
                
                if (highlightFrames[currentFrame].first != -1) {
                    highlightVertex1 = highlightFrames[currentFrame].first;
                    highlightVertex2 = highlightFrames[currentFrame].second;
                } else {
                    highlightVertex1 = highlightVertex2 = -1;
                }
                
                // FIXED: Update the highlighted pseudocode line from the stored frame
                if (currentFrame < pseudoCodeHighlightFrames.size()) {
                    currentHighlightedLine = pseudoCodeHighlightFrames[currentFrame];
                }
            }
            
            // Pause animation while scrubbing
            isPaused = true;
            isPlayingAnimation = false;
        } else {
            // Release drag when mouse button is released
            isDraggingProgressBar = false;
        }
    }
    
    // Handle automatic animation playback
    if (isPlayingAnimation && !isPaused) {
        currentFrame += frameSkip;
        if (currentFrame >= animationFrames.size()) {
            currentFrame = animationFrames.size() - 1;
            isPlayingAnimation = false;
        }
        
        // Update current state from stored animation frame
        if (currentFrame < animationFrames.size()) {
            edges = animationFrames[currentFrame];
            parent = parentFrames[currentFrame];
            
            // NEW: Restore vertex positions from saved frame
            if (currentFrame < vertexPositionFrames.size()) {
                vertexPositions = vertexPositionFrames[currentFrame];
            }
            
            // Update edge positions based on current vertex positions
            for (auto &edge : edges) {
                edge.posU = vertexPositions[edge.u];
                edge.posV = vertexPositions[edge.v];
            }
            
            if (highlightFrames[currentFrame].first != -1) {
                highlightVertex1 = highlightFrames[currentFrame].first;
                highlightVertex2 = highlightFrames[currentFrame].second;
            } else {
                highlightVertex1 = highlightVertex2 = -1;
            }
            
            // FIXED: Update the highlighted pseudocode line from the stored frame
            if (currentFrame < pseudoCodeHighlightFrames.size()) {
                currentHighlightedLine = pseudoCodeHighlightFrames[currentFrame];
            }
        }
    }
}

// Helper function for distance calculation
float MST::euclideanDistance(Vector2 a, Vector2 b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return sqrt(dx*dx + dy*dy);
}

// Modify the handleMouseInteraction method to add bounds checking
void MST::handleMouseInteraction() {
    Vector2 mousePos = GetMousePosition();
    
    // Check if vertexPositions is properly initialized
    if (vertexPositions.size() <= n) {
        // If not initialized, initialize now
        initializeVertexPositions();
        return;
    }
    
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        for (int i = 1; i <= n; i++) {
            // Safe access with bounds check
            if (i < vertexPositions.size() && 
                euclideanDistance(mousePos, vertexPositions[i]) <= vertexRadius * 1.5f) {
                selectedVertex = i;
                break;
            }
        }
    }
    
    if (selectedVertex != -1 && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        // Safe access with bounds check
        if (selectedVertex < vertexPositions.size()) {
            vertexPositions[selectedVertex] = mousePos;
            
            // Make sure velocities vector is also properly sized
            if (selectedVertex < velocities.size()) {
                velocities[selectedVertex] = {0, 0};
            }
            
            // Update positions of edges connected to this vertex
            for (auto &edge : edges) {
                if (edge.u == selectedVertex) {
                    edge.posU = mousePos;
                }
                if (edge.v == selectedVertex) {
                    edge.posV = mousePos;
                }
            }
        }
    }
    
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        selectedVertex = -1;
    }
}

// Update vertex positions using force-directed layout algorithm - modified for fixed position mode
void MST::updateInteractiveGraph(int screenWidth, int screenHeight) {
    // Handle mouse interactions first
    handleMouseInteraction();
    
    // Skip physics calculations in fixed position mode
    if (fixedPositionMode) {
        // Still update edge positions to match vertex positions
        for (auto &edge : edges) {
            edge.posU = vertexPositions[edge.u];
            edge.posV = vertexPositions[edge.v];
        }
        return;
    }
    
    // Calculate physics for all vertices when in physics mode
    for (int iter = 0; iter < physicsIterations; iter++) {
        std::vector<Vector2> forces(n + 1, {0, 0});
        
        // Calculate repulsive forces between all vertices
        for (int i = 1; i <= n; i++) {
            for (int j = i + 1; j <= n; j++) {
                Vector2 delta = {
                    vertexPositions[i].x - vertexPositions[j].x,
                    vertexPositions[i].y - vertexPositions[j].y
                };
                
                float dist = euclideanDistance(vertexPositions[i], vertexPositions[j]);
                if (dist < 0.1f) dist = 0.1f;
                
                float repForce = C_rep / (dist * dist);
                Vector2 dir = {delta.x / dist, delta.y / dist};
                Vector2 forceRep = {dir.x * repForce, dir.y * repForce};
                
                forces[i].x += forceRep.x;
                forces[i].y += forceRep.y;
                forces[j].x -= forceRep.x;
                forces[j].y -= forceRep.y;
            }
        }
        
        // Calculate attractive forces along edges
        for (const auto &edge : edges) {
            int u = edge.u;
            int v = edge.v;
            
            Vector2 delta = {
                vertexPositions[u].x - vertexPositions[v].x,
                vertexPositions[u].y - vertexPositions[v].y
            };
            
            float dist = euclideanDistance(vertexPositions[u], vertexPositions[v]);
            if (dist < 0.1f) dist = 0.1f;
            
            float displacement = dist - L;
            float attractiveForce = c_spring * displacement;
            Vector2 dir = {delta.x / dist, delta.y / dist};
            Vector2 forceAtt = {dir.x * attractiveForce, dir.y * attractiveForce};
            
            forces[u].x -= forceAtt.x;
            forces[u].y -= forceAtt.y;
            forces[v].x += forceAtt.x;
            forces[v].y += forceAtt.y;
        }
        
        // Apply forces to update velocities and positions
        for (int i = 1; i <= n; i++) {
            // Skip physics update only for the vertex being dragged
            if (i == selectedVertex) continue;
            
            // Update velocity with force and damping
            velocities[i].x = (velocities[i].x + forces[i].x * timeStep) * damping;
            velocities[i].y = (velocities[i].y + forces[i].y * timeStep) * damping;
            
            // Update position
            vertexPositions[i].x += velocities[i].x * timeStep;
            vertexPositions[i].y += velocities[i].y * timeStep;
            
            // Constrain to screen boundaries
            if (vertexPositions[i].x < vertexRadius)
                vertexPositions[i].x = vertexRadius;
            if (vertexPositions[i].y < vertexRadius)
                vertexPositions[i].y = vertexRadius;
            if (vertexPositions[i].x > screenWidth - vertexRadius)
                vertexPositions[i].x = screenWidth - vertexRadius;
            if (vertexPositions[i].y > screenHeight - vertexRadius)
                vertexPositions[i].y = screenHeight - vertexRadius;
        }
            
        // Update edge positions after all vertex positions are updated
        for (auto &edge : edges) {
            edge.posU = vertexPositions[edge.u];
            edge.posV = vertexPositions[edge.v];
        }
    }
}

// Replace the existing updateVertexPositions with our new interactive version
void MST::updateVertexPositions() {
    // Handle mode toggle interaction regardless of animation state
    handleModeToggleInteraction();
    
    // Handle pseudocode toggle button interaction
    handlePseudoCodeToggle();
    
    updateInteractiveGraph(1920, 1080); // Use screen dimensions
    
    // Handle progress bar interaction when animation frames are available
    if (!animationFrames.empty()) {
        handleProgressBarInteraction();
    }
}

void MST::initializeVertexPositions() {
    vertexPositions.clear();
    targetPositions.clear();
    velocities.clear();
    
    int centerX = SCREEN_WIDTH /2;
    int centerY = SCREEN_HEIGHT /3;
    int radius = 300;

    // Initialize vertex positions in a circular arrangement
    vertexPositions.push_back({0, 0}); // Dummy vertex at index 0 (not used)
    velocities.push_back({0, 0});
    
    // Place vertices directly on the circle
    for (int i = 1; i <= n; i++) {
        float angle = 2 * PI * (i - 1) / n;
        Vector2 circlePos = {
            (float)(centerX + radius * cos(angle)),
            (float)(centerY + radius * sin(angle))
        };
        
        vertexPositions.push_back(circlePos);
        velocities.push_back({0, 0});
        targetPositions.push_back(circlePos);
    }
    
    // Update edge positions to match vertex positions
    for (auto &edge : edges) {
        edge.posU = vertexPositions[edge.u];
        edge.posV = vertexPositions[edge.v];
    }
}

// NEW: Method to toggle fixed position mode
void MST::toggleFixedPositionMode() {
    fixedPositionMode = !fixedPositionMode;
}
