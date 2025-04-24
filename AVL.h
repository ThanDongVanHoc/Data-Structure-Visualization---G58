#ifndef AVL_H
#define AVL_H

#include <iostream>
#include <algorithm>
#include <cmath>
#include "raylib.h"
#include "Constants.h"
#include <vector>
#include<map>
// Add near the top after other includes
extern Font customFont;

// Structure to serialize AVL node state
struct SerializableAVLNode {
    int data;
    int height;
    int leftChildIndex;   // Index of left child in array (-1 if null)
    int rightChildIndex;  // Index of right child in array (-1 if null)
    float x, y;           // Current position
    float targetX, targetY; // Target position
    float vx, vy;         // Current velocity
    Color color;          // Current color
};

// Structure to store highlighted edge during traversal
struct EdgeHighlight {
    int sourceIndex;
    int targetIndex;
    Color color;
};

// Structure to store AVL tree state for animation/history
struct AVLTreeState {
    std::vector<SerializableAVLNode> nodes;
    int rootIndex;  // Index of the root node (-1 if empty tree)
    std::vector<EdgeHighlight> highlightedEdges;  // Edges being highlighted
    std::vector<int> traversalOrder;  // Store the traversal order for visualization
    std::vector<std::string> pseudoCode;  // Added for pseudocode steps
    int highlightedLine;  // Index of the line to highlight (-1 for none)
    std::string operationName;  // Name of the current operation
};

// Cấu trúc của một nút trong cây AVL (bao gồm thông tin render và vật lý)
struct AVLNode {
    int data;
    int height;
    AVLNode* left;
    AVLNode* right;
    float x, y;             // Vị trí hiện tại (để vẽ)
    float targetX, targetY; // Vị trí đích (sau khi tính layout)
    float vx, vy;           // Vận tốc dùng cho vật lý chuyển động
    Color color;            // Màu sắc của node (dùng để highlight)

    AVLNode(int k) :
        data(k), height(1), left(nullptr), right(nullptr),
        x(0), y(0), targetX(0), targetY(0), vx(0), vy(0),
        color(NODE_DEFAULT_COLOR) // mặc định là màu xanh lá
    {}
};

class AVL {
public:
    AVLNode* foundNode;  // Chỉ giữ lại node tìm thấy cho highlight
    AVLNode* root;
    
    // Animation control variables
    bool isPaused;
    bool isAnimating;
    int currentAnimationStep;
    
    // Undo/Redo history
    std::vector<AVLTreeState> historyStates;
    int currentHistoryPosition;

    // Add traversal order tracking
    std::vector<int> traversalOrder;
    std::string traversalType;  // "inorder", "preorder", or "postorder"

    // Pseudocode tracking
    std::vector<std::string> currentPseudoCode;
    int currentHighlightedLine;
    std::string currentOperation;

public:
    // Move animationStates to public section
    std::vector<AVLTreeState> animationStates;
    
    // Constructor
    AVL() : root(nullptr), foundNode(nullptr), 
            isPaused(false), isAnimating(false), 
            currentAnimationStep(0), currentHistoryPosition(-1),
            traversalType(""), currentHighlightedLine(-1),
            currentOperation("") {
        // Initialize history with empty state
        historyStates.push_back(captureCurrentState());
        currentHistoryPosition = 0;
    }

    // Các hàm cơ bản của cây AVL
    int getHeight(AVLNode* node);
    int getBalanceFactor(AVLNode* node);
    AVLNode* RotateLeft(AVLNode* node);
    AVLNode* RotateRight(AVLNode* node);
    void self_balance(AVLNode*& node);
    void preOrder(AVLNode* node);

    // Các hàm phục vụ tính layout (target positions)
    void computeTargets(AVLNode* node, int depth, float &xCounter);
    void updateTargets();

    // Hàm animate: áp dụng vật lý cho chuyển động (mượt)
    void animateNodes(AVLNode* node);

    // Hàm vẽ cây AVL theo đệ quy
    void drawTree(AVLNode* node);
    void draw() {
        // // Vẽ gradient background từ trên xuống dưới
        // for (int y = 0; y < 1080; y++) {
        //     float factor = (float)y / 1080.0f;
            
        //     Color currentColor = {
        //         (unsigned char)(BACKGROUND_COLOR_TOP.r + (BACKGROUND_COLOR_BOTTOM.r - BACKGROUND_COLOR_TOP.r) * factor),
        //         (unsigned char)(BACKGROUND_COLOR_TOP.g + (BACKGROUND_COLOR_BOTTOM.g - BACKGROUND_COLOR_TOP.g) * factor),
        //         (unsigned char)(BACKGROUND_COLOR_TOP.b + (BACKGROUND_COLOR_BOTTOM.b - BACKGROUND_COLOR_TOP.b) * factor),
        //         255
        //     };
            
        //     DrawLine(0, y, 1920, y, currentColor);
        // }
        
        // Vẽ cây
        drawTree(root);
    }

    // Public method: chèn phần tử và cập nhật layout
    // Nếu cây rỗng, root sẽ được tạo với vị trí ban đầu ở giữa màn hình (960,540)
    void insert(int key) {
        if (root == nullptr)
            insertHelper(root, key, 960, 540);
        else
            // Khi chèn, vị trí ban đầu của node mới được gán bằng vị trí của node cha.
            insertHelper(root, key, root->x, root->y);
        updateTargets();
        // Sau khi animation, reset màu của toàn bộ node về mặc định
        resetColors(root);
        addStateToHistory();
    }
    AVLNode* getRoot() { return root; }

    // Thêm các phương thức mới
    void deleteKey(int key);
    bool find(int key);
    void clearFoundNode() { 
        foundNode = nullptr; 
    }

    // Animation control methods
    void togglePause() { isPaused = !isPaused; }
    void resumeAnimation() { isPaused = false; }
    bool isAnimationPaused() const { return isPaused; }
    bool isAnimationComplete() const;
    void clearAnimationStates() { animationStates.clear(); currentAnimationStep = 0; }
    
    // State management for animation and history
    AVLTreeState captureCurrentState();
    void restoreFromState(const AVLTreeState& state);
    void visualizeState(const AVLTreeState& state);
    
    // History navigation
    void addStateToHistory();
    bool canUndo() const { return currentHistoryPosition > 0; }
    bool canRedo() const { return currentHistoryPosition < historyStates.size() - 1; }
    const AVLTreeState& undo();
    const AVLTreeState& redo();
    const AVLTreeState& getCurrentHistoryState() const;
    
    // Process animation frames
    std::vector<AVLTreeState> processOperationAnimation();

    // Tree traversal methods
    void inorderTraversal();
    void preorderTraversal();
    void postorderTraversal();

    // Add method to clear traversal order
    void clearTraversalOrder() {
        traversalOrder.clear();
        traversalType = "";
    }

    // Pseudocode methods
    void setPseudoCodeInsert(int value);
    void setPseudoCodeDelete(int value);
    void setPseudoCodeSearch(int value);
    void setPseudoCodeInorder();
    void setPseudoCodePreorder();
    void setPseudoCodePostorder();
    void updatePseudoCodeHighlight(int line);

    // Add these new methods
    void createEmpty();  // Clear the tree and make it empty
    void createRandom(int size);  // Create a tree with random values

    bool showPseudoCode;

private:
    // Hàm insertHelper: thêm node mới với thông tin vị trí của node cha.
    void insertHelper(AVLNode*& node, int key, float parentX, float parentY);

    // Hàm đệ quy để reset màu của tất cả các node về màu mặc định
    void resetColors(AVLNode* node) {
        if (node == nullptr) return;
        node->color = NODE_DEFAULT_COLOR;
        resetColors(node->left);
        resetColors(node->right);
    }

    AVLNode* deleteHelper(AVLNode*& node, int key);
    AVLNode* findMin(AVLNode* node);
    AVLNode* findHelper(AVLNode* node, int key);

    // Helper functions for traversal
    void inorderHelper(AVLNode* node);
    void preorderHelper(AVLNode* node);
    void postorderHelper(AVLNode* node);

    // Helper for serializing and deserializing tree structure
    void serializeNode(AVLNode* node, std::vector<SerializableAVLNode>& nodes, std::map<AVLNode*, int>& nodeToIndex);
    AVLNode* deserializeNode(const SerializableAVLNode& nodeData, std::vector<AVLNode*>& nodeList);

    // Add helper for edge highlighting
    void highlightEdge(AVLNode* from, AVLNode* to, Color color, int frames);
};

#endif // AVL_H
