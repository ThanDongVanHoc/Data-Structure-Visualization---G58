#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include "raylib.h"
#include <string>
#include <vector>
#include <map>

// Only declare the external variable, don't define it here
extern Font customFont;

// Cấu trúc Node cho LinkedList, bao gồm các thuộc tính hỗ trợ animation.
struct Node {
    int value;
    Node* next;
    // Vị trí hiện tại và target (cho animation)
    float x, y;
    float targetX, targetY;
    // Vận tốc hiện tại
    float vx, vy;
    // Màu hiển thị (có thể dùng để highlight)
    Color color;

    Node(int v);
};

// Edge structure to store and animate edges
struct ListEdge {
    Node* startNode;
    Node* endNode;
    float animationProgress; // 0.0 to 1.0
    bool isAnimating;
    
    ListEdge(Node* start, Node* end);
};

// Define serializable structures for storing animation states
struct SerializableNode {
    int value;
    float x, y;
    Color color;
    int nextNodeIndex; // Index of the next node in the array (-1 if null)
};

struct SerializableEdge {
    int startNodeIndex;
    int endNodeIndex;
    float animationProgress;
    bool isAnimating;
};

// Simple state container for animation frames
struct LinkedListFrameState {
    std::vector<SerializableNode> nodes;
    std::vector<SerializableEdge> edges;
    std::vector<std::string> pseudoCode;  // Added for pseudocode steps
    int highlightedLine;  // Index of the line to highlight (-1 for none)
    std::string operationName;  // Name of the current operation (e.g., "Add Head", "Delete")
};

// Lớp LinkedList với các chức năng thao tác và visualization.
class LinkedList {
public:
    Node* head;

    LinkedList();
    ~LinkedList();

    // Các thao tác trên danh sách:
    void addHead(int value);              // Thêm node vào đầu (xuất hiện từ bên trái)
    void addTail(int value);              // Thêm node vào cuối (xuất hiện từ bên phải)
    void insertAfter(int index, int value); // Chèn node sau node tại vị trí index (xuất hiện từ phía trên)
    void deleteValue(int value);          // Xóa node theo giá trị, có hiệu ứng highlight traversal
    
    // New operations
    void searchValue(int value);          // Tìm kiếm node theo giá trị, có hiệu ứng highlight traversal
    void removeAtIndexTraversal(int index);        // Xóa node tại vị trí index, có hiệu ứng highlight traversal
    void removeAtIndex(int index);        // Xóa node tại vị trí index, không có hiệu ứng highlight traversal
    // Các hàm hỗ trợ animation & layout:
    void updateTargets();     // Tính toán vị trí target của các node sao cho danh sách căn giữa khung hình (1920x1080)
    void updateAnimation();   // Cập nhật vị trí các node dựa trên hiệu ứng spring/damping
    void visualize();         // Vẽ danh sách (bao gồm các node và mũi tên nối)
    void visualizeState(const LinkedListFrameState& state); // Vẽ danh sách từ state đã lưu

    // Hàm hỗ trợ highlight cho thao tác insertAfter: highlight dần từ head đến node có chỉ số index.
    void animateInsertAfterTraversal(int index);

    // Thêm các biến và phương thức điều khiển animation
    bool isPaused;
    bool isAnimating;
    void togglePause();
    void resumeAnimation();
    bool isAnimationPaused() const;

    // Thêm hàm để lấy ra trạng thái hiện tại và kiểm tra animation hoàn thành
    LinkedListFrameState captureCurrentState();
    bool isAnimationComplete();
    
    // Hàm xử lý toàn bộ animation cho một query và lưu các trạng thái
    std::vector<LinkedListFrameState> processQueryAnimation();
    
    // Clear animation history
    void clearAnimationStates();
    
    // Store animation states
    std::vector<LinkedListFrameState> animationStates;

    // Undo/Redo functionality
    void addStateToHistory();
    bool canUndo() const;
    bool canRedo() const;
    const LinkedListFrameState& undo();
    const LinkedListFrameState& redo();
    const LinkedListFrameState& getCurrentHistoryState() const;

    // Restore linked list from a saved state
    void restoreFromState(const LinkedListFrameState& state);

    // Undo/Redo history
    std::vector<LinkedListFrameState> historyStates;
    int currentHistoryPosition;

    // New method declaration
    void animateDeleteTraversal(int value);

    // Helper functions for setting pseudocode based on operation
    void setPseudoCodeAddHead(int value);
    void setPseudoCodeAddTail(int value);
    void setPseudoCodeInsertAfter(int index, int value);
    void setPseudoCodeDelete(int value);
    void setPseudoCodeRemoveAtIdx();
    void setPseudoCodeSearch();

    // New initialization methods
    void createEmpty();                     // Create empty list
    void createRandom(int size);            // Create random list
    void createRandomSorted(int size);      // Create sorted random list

    void clearLinkedList(); // Clear the linked list and reset animation states

    bool showPseudoCode;
    // Progress bar drawing methods

    //FOR PROGRESS BAR
private:
    // Store edges for visualization and animation
    std::vector<ListEdge> edges;
    
    // Track newly inserted nodes
    Node* lastInsertedNode;
    Node* TailNode;
    
    // Check if nodes are close enough to their target positions to start edge animations
    bool areNodesSettled();
    
    // Update edge positions based on node positions
    void updateEdges();
    
    // Helper to create edges for insert operations
    void createEdgeWithAnimation(Node* start, Node* end);
    
    // Hàm nội bộ hỗ trợ highlight trong quá trình delete.
    int delayedEdgeIndex = -1;
    bool firstdelay = 0;

    // Variables for delete animation
    bool isDeleting;
    Node* currentDeleteNode;
    int deleteTargetValue;
    bool deleteFound;

    int deleteAnimationDelay;
    int currentDeleteDelay;
    
    // Edge animation constants
    const float EDGE_ANIMATION_SPEED = 0.05f;

    // Animation state tracking for pseudocode highlighting
    std::vector<std::string> currentPseudoCode;
    int currentHighlightedLine;
    std::string currentOperation;
    
    
    
    // Update highlighted pseudocode line based on animation state
    void updatePseudoCodeHighlight();
};

#endif // LINKED_LIST_H