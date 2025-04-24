#ifndef MST_H
#define MST_H

#include "raylib.h"
#include <vector>
#include <string>

// Add near the top after other includes
extern Font customFont;

// Cấu trúc lưu thông tin của cạnh.
struct Edge {
    int u, v, w;          // Đỉnh u, v và trọng số.
    Color color;          // Màu hiện tại của cạnh (sẽ animate).
    Vector2 posU, posV;   // Vị trí của đỉnh u, v trong đồ thị.
    // Dành cho danh sách cạnh (Edge List):
    Vector2 currentPos;   // Vị trí hiện tại của cạnh trong danh sách.
    Vector2 targetPos;    // Vị trí mục tiêu trong danh sách.
};

#define DSU_START_X 200    // Changed from 20 to move DSU table to center more
#define DSU_START_Y 20
#define DSU_CELL_HEIGHT 30 // Increased from 25
#define DSU_CELL_WIDTH 150 // New constant for wider cells

class MST {
public:
    int n, m;                     // Số đỉnh và số cạnh.
    std::vector<int> parent;      // DSU: mảng "index -> parent".
    std::vector<Edge> edges;      // Danh sách cạnh.
    std::vector<Edge> mstEdges;   // Các cạnh thuộc MST.

    // Các đỉnh được highlight trong DSU (đang được xét).
    int highlightVertex1;
    int highlightVertex2;

    // Physics interaction properties
    int selectedVertex;            // Index of vertex being dragged (-1 if none)
    float C_rep;                  // Repulsive force constant
    float c_spring;               // Spring force constant
    float L;                      // Ideal spring length
    float timeStep;               // Physics timestep
    float damping;                // Velocity damping
    int physicsIterations;        // Physics iteration per frame
    float vertexRadius;           // Radius of vertices for interaction
    bool fixedPositionMode;       // NEW: Flag for fixed position mode

    // Animation tracking variables
    std::vector<std::vector<Edge>> animationFrames; // Stores each frame of the animation
    std::vector<std::vector<int>> parentFrames;     // Stores the DSU parent array for each frame
    std::vector<std::pair<int, int>> highlightFrames; // Stores highlighted vertices for each frame
    std::vector<std::vector<Vector2>> vertexPositionFrames; // NEW: Stores vertex positions for each frame
    std::vector<int> pseudoCodeHighlightFrames;    // ADDED: Stores highlighted pseudocode line for each frame
    int currentFrame;                              // Current animation frame
    bool isPlayingAnimation;                       // Whether animation is playing
    bool isPaused;                                 // Whether animation is paused
    int frameSkip;                                 // Number of frames to skip (animation speed)

    // Pseudocode tracking
    std::vector<std::string> currentPseudoCode;    // ADDED: Current pseudocode lines
    int currentHighlightedLine;                    // ADDED: Currently highlighted line
    std::string currentOperation;                  // ADDED: Current operation name
    bool showPseudoCode;                           // NEW: Toggle for showing/hiding pseudocode

    // Progress bar interaction
    bool isDraggingProgressBar;                    // Whether progress bar is being dragged
    Rectangle progressBarRect;                     // Progress bar rectangle

    // Constructor: khởi tạo DSU với n đỉnh.
    MST(int n, int m);

    // Thêm cạnh mới.
    void addEdge(int u, int v, int w);

    // Sắp xếp cạnh theo trọng số và cập nhật vị trí trong danh sách cạnh.
    void sortEdges();

    // Các hàm DSU.
    int findParent(int u);
    void unionVertices(int u, int v);

    // Thuật toán Kruskal với animation màu:
    // Trước khi xét, 2 đỉnh được highlight; cạnh được animate chuyển từ BLACK → DodgerBlue (nhấp nháy) →
    // nếu được chọn chuyển sang Orange, nếu không thì fade-out.
    // Sau khi hoàn thành, chỉ giữ lại các cạnh thuộc MST.
    void runKruskal();

    // Các hàm vẽ visualization:
    // Vẽ DSU dưới dạng bảng 2 cột với ô border.
    void drawDSU();
    // Vẽ danh sách cạnh (Edge List) dựa trên currentPos.
    void drawEdgeList();
    // Vẽ đồ thị: các đỉnh trên vòng tròn, cạnh được vẽ từ viền đỉnh, trọng số cạnh được hiển thị bên ngoài.
    void drawGraph();
    void drawProgressBar();                        // NEW: Drawing progress bar

    // Animate di chuyển các cạnh trong danh sách từ currentPos đến targetPos.
    void animateEdgeListPositions();

    std::vector<Vector2> vertexPositions;  // Vị trí thực tế của các đỉnh
    std::vector<Vector2> targetPositions;  // Vị trí đích của các đỉnh
    std::vector<Vector2> velocities;       // Vận tốc của các đỉnh
    
    void initializeVertexPositions();      // Khởi tạo vị trí các đỉnh
    void updateVertexPositions();          // Cập nhật vị trí theo spring animation

    // Interaction methods
    void updateInteractiveGraph(int screenWidth, int screenHeight);
    void handleMouseInteraction();
    void handleProgressBarInteraction();    // NEW: Handle progress bar interaction
    void toggleFixedPositionMode();         // NEW: Toggle fixed position mode
    
    // NEW: Separate functions for mode toggle
    void drawModeToggle();                  // Draw the mode toggle button
    void handleModeToggleInteraction();     // Handle mode toggle button interaction

    // Add pseudocode methods
    void setPseudoCodeKruskal();                   // ADDED: Set pseudocode for Kruskal's algorithm
    void updatePseudoCodeHighlight(int line);      // ADDED: Update highlighted line
    void drawPseudoCode();                         // ADDED: Draw pseudocode with highlighting
    void handlePseudoCodeToggle();                 // NEW: Handle pseudocode toggle button

    // Add step navigation methods
    void stepBackward();                          // ADDED: Step backward to previous highlighted line
    void stepForward();                           // ADDED: Step forward to next highlighted line

    // Add helper function for text width measurement
    int textWidth(const char* text);

private:
    // Nội suy màu từ start đến end theo t (0 <= t <= 1).
    Color interpolateColor(Color start, Color end, float t);
    // Animate thay đổi màu của cạnh từ start sang end trong durationMs.
    void animateEdgeColor(Edge &e, Color start, Color end, int durationMs);
    
    // Helper function for distance calculation
    float euclideanDistance(Vector2 a, Vector2 b);
};

#endif // MST_H
