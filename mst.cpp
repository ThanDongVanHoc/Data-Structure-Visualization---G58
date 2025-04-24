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

////////////////////