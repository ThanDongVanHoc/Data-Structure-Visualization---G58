#include "linked_list.h"
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <iostream>
#include <map>

// Define the customFont variable here
Font customFont;

// Các hằng số cho kích thước node, khoảng cách, và khung hình
#define NODE_RADIUS 30  // Changed from NODE_WIDTH/HEIGHT to NODE_RADIUS
// Tăng GAP từ 50 lên 70 để đảm bảo phần đầu mũi tên không bị che.
#define GAP 120  // Increased for better spacing between circular nodes

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

// Các hằng số vật lý cho animation
#define SETTLE_DISTANCE 1.0f
#define SPRING_CONSTANT 0.1f
#define DAMPING 0.4f
#define MAX_VELOCITY 20.0f

// Thời gian delay cho hiệu ứng highlight (mili giây)
#define HIGHLIGHT_DELAY 100

// Updated color scheme
#define NODE_DEFAULT_COLOR (Color){52, 152, 219, 255}  // Nice blue
#define NODE_BORDER_COLOR (Color){41, 128, 185, 255}   // Darker blue for border
#define HIGHLIGHT_COLOR (Color){241, 196, 15, 255}     // Brighter yellow
#define DELETE_HIGHLIGHT_COLOR (Color){231, 76, 60, 255} // Vivid red

// Tốc độ animation của cạnh
#define EDGE_ANIMATION_SPEED 0.05f

// Added shadow offset
#define SHADOW_OFFSET 4

// =======================
// Node implementation
// =======================
Node::Node(int v)
    : value(v), next(nullptr),
      x(0), y(0), targetX(0), targetY(0),
      vx(0), vy(0) {
    color = NODE_DEFAULT_COLOR;
}

// =======================
// ListEdge implementation
// =======================
ListEdge::ListEdge(Node* start, Node* end)
    : startNode(start), endNode(end),
      animationProgress(0.0f), isAnimating(true) {}

// =======================
// LinkedList implementation
// =======================
LinkedList::LinkedList() 
    : head(nullptr), isPaused(false), isAnimating(false),
      isDeleting(false), currentDeleteNode(nullptr), 
      deleteTargetValue(-1), deleteFound(false),
      deleteAnimationDelay(30), currentDeleteDelay(0),
      lastInsertedNode(nullptr),
      currentHistoryPosition(-1),
      currentHighlightedLine(-1) { 
    
    // Initialize history with empty state
    historyStates.push_back(captureCurrentState());
    currentHistoryPosition = 0;
}

LinkedList::~LinkedList() {
    Node* curr = head;
    while (curr) {
        Node* temp = curr->next;
        delete curr;
        curr = temp;
    }
    // Clear the edges vector
    edges.clear();
}



// Check if all nodes are settled at their target positions
bool LinkedList::areNodesSettled() {
    Node* curr = head;
    while (curr) {
        float dx = curr->targetX - curr->x;
        float dy = curr->targetY - curr->y;
        if (fabs(dx) > SETTLE_DISTANCE || fabs(dy) > SETTLE_DISTANCE ||
            fabs(curr->vx) > 0.1f || fabs(curr->vy) > 0.1f) {
            return false;
        }
        curr = curr->next;
    }
    return true;
}
// Hàm updateTargets() tính toán vị trí của các node sao cho danh sách được căn giữa.
void LinkedList::updateTargets() {
    int count = 0;
    Node* curr = head;
    while (curr) {
        count++;
        curr = curr->next;
    }
    if (count == 0) return;
    
    // Adjusted for node radius instead of width
    float totalWidth = count * (2 * NODE_RADIUS) + (count - 1) * GAP;
    float startX = (SCREEN_WIDTH - totalWidth) / 2.0f + NODE_RADIUS;
    float startY = (SCREEN_HEIGHT - NODE_RADIUS) / 2.0f;
    
    curr = head;
    int index = 0;
    while (curr) {
        curr->targetX = startX + index * (2 * NODE_RADIUS + GAP);
        curr->targetY = startY;
        index++;
        curr = curr->next;
    }
    
    // Rebuild edge list when targets are updated
    updateEdges();
}

// Update edge information based on node status
void LinkedList::updateEdges() {
    // Don't clear existing edges, only update or add as needed
    
    // Find and update existing edges, or create new ones
    Node* curr = head;
    
    // Track which edges have been updated
    std::vector<bool> edgeUpdated(edges.size(), false);
    int edgeIndex = 0;
    
    while (curr && curr->next) {
        bool edgeExists = false;
        
        // Check if this edge already exists
        for (size_t i = 0; i < edges.size(); i++) {
            if (edges[i].startNode == curr && edges[i].endNode == curr->next) {
                edgeUpdated[i] = true;
                edgeExists = true;
                
                // Keep animation running if this is an edge from the last inserted node
                if (curr == lastInsertedNode && edges[i].animationProgress < 1.0f) {
                    edges[i].isAnimating = true;
                }
                
                break;
            }
        }
        
        // If edge doesn't exist, create a new one
        if (!edgeExists) {
            ListEdge newEdge(curr, curr->next);
            
            // Only animate if this is an edge from a newly inserted node
            if (curr == lastInsertedNode) {
                newEdge.animationProgress = 0.0f;
                newEdge.isAnimating = true;
            } else {
                // For existing nodes, edge should be fully visible
                newEdge.animationProgress = 1.0f;
                newEdge.isAnimating = false;
            }
            
            edges.push_back(newEdge);
        }
        
        curr = curr->next;
        edgeIndex++;
    }
    
    // Remove any edges that are no longer valid (e.g., after deletion)
    for (int i = edges.size() - 1; i >= 0; i--) {
        if (i < edgeUpdated.size() && !edgeUpdated[i]) {
            edges.erase(edges.begin() + i);
        }
    }
}

// Helper to create edge with animation
void LinkedList::createEdgeWithAnimation(Node* start, Node* end) {
    if (start && end) {
        ListEdge newEdge(start, end);
        edges.push_back(newEdge);
    }
}


// Hàm addHead(): tạo node mới với giá trị cho trước và update layout
void LinkedList::addHead(int value) {
    // Set up pseudocode for this operation
    setPseudoCodeAddHead(value);
    
    Node* newNode = new Node(value);
    newNode->x = -NODE_RADIUS;
    newNode->y = (SCREEN_HEIGHT - NODE_RADIUS) / 2.0f;
    
    // Add to the beginning of the list
    newNode->next = head;
    head = newNode;
    
    // Track this as the newly inserted node
    lastInsertedNode = newNode;
    
    updateTargets();
}

// Hàm addTail(): tạo node mới xuất hiện từ bên phải và update layout
void LinkedList::addTail(int value) {
    // Set up pseudocode for this operation
    setPseudoCodeAddTail(value);
    
    Node* newNode = new Node(value);
    newNode->x = SCREEN_WIDTH;
    newNode->y = (SCREEN_HEIGHT - NODE_RADIUS) / 2.0f;
    
    TailNode = newNode; // Initialize TailNode if head is null
    if (!head) {
        head = newNode;
    } else {
        Node* curr = head;
        while (curr->next)
            curr = curr->next;
        curr->next = newNode;
        
        // Track the node before the newly added tail
        lastInsertedNode = curr;
    }
    
    updateTargets();
}

// Hàm insertAfter(): chèn node sau node tại vị trí index
void LinkedList::insertAfter(int index, int value) {
    // Set up pseudocode for this operation
    //setPseudoCodeInsertAfter(index, value);
    currentHighlightedLine = 4;
    Node* curr = head;
    int pos = 0;
    while (curr && pos < index) {
        curr = curr->next;
        pos++;
    }
    if (!curr) return; // Nếu index không hợp lệ, thoát
    
    // Tạo node mới, xuất hiện từ phía trên
    Node* newNode = new Node(value);
    currentHighlightedLine = 4;
    newNode->x = curr->targetX;
    newNode->y = -NODE_RADIUS;
    
    // Handle next pointers
    newNode->next = curr->next;
    curr->next = newNode;
    
    // Track the node before the newly inserted node
    lastInsertedNode = curr;
    
    // Update layout
    updateTargets();
}



// Hàm deleteValue(): trước khi xóa, thực hiện highlight traversal để tìm node cần xóa
void LinkedList::deleteValue(int value) {
    // Set up pseudocode for this operation
    //setPseudoCodeDelete(value);
    
    if (!head) return;
    
    if (head->value == value) {
        Node* temp = head;
        head = head->next;
        delete temp;
    } else {
        Node* curr = head;
        while (curr->next && curr->next->value != value) {
            curr = curr->next;
        }
        if (curr->next) {
            Node* temp = curr->next;
            curr->next = temp->next;
            delete temp;
        }
    }
        
    // Update layout after deletion
    updateTargets();
}


