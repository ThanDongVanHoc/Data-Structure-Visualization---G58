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
#define NODE_WIDTH 50
#define NODE_HEIGHT 50
// Tăng GAP từ 50 lên 70 để đảm bảo phần đầu mũi tên không bị che.
#define GAP 100

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

// Các hằng số vật lý cho animation
#define SETTLE_DISTANCE 1.0f
#define SPRING_CONSTANT 0.1f
#define DAMPING 0.4f
#define MAX_VELOCITY 20.0f

// Thời gian delay cho hiệu ứng highlight (mili giây)
#define HIGHLIGHT_DELAY 100

// Các màu mặc định và màu highlight
#define NODE_DEFAULT_COLOR LIGHTGRAY
#define HIGHLIGHT_COLOR YELLOW
#define DELETE_HIGHLIGHT_COLOR RED

// Tốc độ animation của cạnh
#define EDGE_ANIMATION_SPEED 0.05f

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

void LinkedList::togglePause() {
    isPaused = !isPaused;
}

void LinkedList::resumeAnimation() {
    isPaused = false;
}

bool LinkedList::isAnimationPaused() const {
    return isPaused;
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
    float totalWidth = count * NODE_WIDTH + (count - 1) * GAP;
    float startX = (SCREEN_WIDTH - totalWidth) / 2.0f;
    float startY = (SCREEN_HEIGHT - NODE_HEIGHT) / 2.0f;
    
    curr = head;
    int index = 0;
    while (curr) {
        curr->targetX = startX + index * (NODE_WIDTH + GAP);
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

// Hàm updateAnimation() cập nhật vị trí hiện tại của mỗi node và edge animations
void LinkedList::updateAnimation() {
    if (isPaused) return;
    
    // Handle delete animation
    if (isDeleting) {
        if (currentDeleteDelay > 0) {
            currentDeleteDelay--;
            return;
        }
        
        if (currentDeleteNode) {
            // Reset previous node color if it's not the found node
            if (!deleteFound || currentDeleteNode->value != deleteTargetValue) {
                currentDeleteNode->color = NODE_DEFAULT_COLOR;
            }
            currentDeleteNode = currentDeleteNode->next;
            if (currentDeleteNode) {
                currentDeleteNode->color = HIGHLIGHT_COLOR;
                if (currentDeleteNode->value == deleteTargetValue) {
                    currentDeleteNode->color = DELETE_HIGHLIGHT_COLOR;
                    deleteFound = true;
                }
                currentDeleteDelay = deleteAnimationDelay; // Reset delay counter
            } else {
                // End of traversal
                if (deleteFound) {
                    // Actually delete the node
                    if (head->value == deleteTargetValue) {
                        Node* temp = head;
                        head = head->next;
                        delete temp;
                    } else {
                        Node* curr = head;
                        while (curr->next && curr->next->value != deleteTargetValue) {
                            curr = curr->next;
                        }
                        if (curr->next) {
                            Node* temp = curr->next;
                            curr->next = temp->next;
                            delete temp;
                        }
                    }
                    updateTargets();
                }
                isDeleting = false;
                deleteFound = false;
                deleteTargetValue = -1;
                currentDeleteDelay = 0;
            }
        }
    }

    // Update node positions with physics animation
    Node* curr = head;
    while (curr) {
        float dx = curr->targetX - curr->x;
        float dy = curr->targetY - curr->y;
        if (fabs(dx) < SETTLE_DISTANCE && fabs(dy) < SETTLE_DISTANCE &&
            fabs(curr->vx) < 0.1f && fabs(curr->vy) < 0.1f) {
            curr->x = curr->targetX;
            curr->y = curr->targetY;
            curr->vx = 0;
            curr->vy = 0;
        } else {
            float forceX = dx * SPRING_CONSTANT;
            float forceY = dy * SPRING_CONSTANT;
            curr->vx = (curr->vx + forceX) * DAMPING;
            curr->vy = (curr->vy + forceY) * DAMPING;
            float speed = sqrt(curr->vx * curr->vx + curr->vy * curr->vy);
            if (speed > MAX_VELOCITY) {
                curr->vx = (curr->vx / speed) * MAX_VELOCITY;
                curr->vy = (curr->vy / speed) * MAX_VELOCITY;
            }
            curr->x += curr->vx;
            curr->y += curr->vy;
        }
        curr = curr->next;
    }
    
    // Check if lastInsertedNode has reached its target position
    if (lastInsertedNode) {
        float dx = lastInsertedNode->targetX - lastInsertedNode->x;
        float dy = lastInsertedNode->targetY - lastInsertedNode->y;
        
        // If node has settled at target position, animate its outgoing edge
        if (fabs(dx) < SETTLE_DISTANCE && fabs(dy) < SETTLE_DISTANCE) {
            currentHighlightedLine = 1;
            if(currentOperation == "Insert After"){
                currentHighlightedLine = 5;
            }
            
            int flag = 0;
            for (auto& edge : edges) {
                if (edge.startNode == lastInsertedNode && edge.isAnimating) {
                    edge.animationProgress += EDGE_ANIMATION_SPEED;
                    if (edge.animationProgress >= 1.0f) {
                        edge.animationProgress = 1.0f;
                        edge.isAnimating = false;
                        lastInsertedNode = nullptr; // Reset once animation is complete
                    }
                    flag = 1;
                    break;
                }
            } 
            if(flag == 0){
               currentHighlightedLine = 2;
               if(currentOperation == "Insert After"){
                    //for(int i = 0; i < 30; i++){
                    currentHighlightedLine = 5;
               }
            }
        }
    }
    
    // Update pseudocode highlight based on current animation state
    //updatePseudoCodeHighlight();
    
    // After all updates are complete, capture the current state
    animationStates.push_back(captureCurrentState());
}

// Hàm visualize() vẽ danh sách với node và mũi tên theo animation state
void LinkedList::visualize() {
    if (!head) return;
    
    // Draw nodes first
    Node* curr = head;
    while (curr) {
        DrawRectangle((int)curr->x, (int)curr->y, NODE_WIDTH, NODE_HEIGHT, curr->color);
        
        // Draw node value
        char text[16];
        sprintf(text, "%d", curr->value);
        int textWidth = MeasureText(text, 20);
        DrawText(text, (int)curr->x + (NODE_WIDTH - textWidth) / 2, (int)curr->y + (NODE_HEIGHT / 2 - 10), 20, BLACK);
        
        curr = curr->next;
    }
    
    // Then draw edges with animation
    for (const auto& edge : edges) {
        if (edge.animationProgress > 0) {
            // Start position is right side of start node
            float startX = edge.startNode->x + NODE_WIDTH;
            float startY = edge.startNode->y + NODE_HEIGHT / 2;
            
            // End position is left side of end node with an offset to prevent overlap
            float endX = edge.endNode->x;
            float endY = edge.endNode->y + NODE_HEIGHT / 2;
            
            // Leave space before the end node (30 pixels) for arrow visibility
            float arrowOffset = 30;
            
            // Calculate the effective endpoint with offset
            float effectiveEndX = endX - arrowOffset;
            
            // Calculate total distance and current distance
            float totalDistance = effectiveEndX - startX;
            float currentDistance = totalDistance * edge.animationProgress;
            
            // Calculate current endpoint
            float currentEndX = startX + currentDistance;
            
            // Draw the main line with increased thickness
            DrawLineEx(
                (Vector2){startX, startY},
                (Vector2){currentEndX, startY},
                3.0f, // thicker line
                BLACK
            );
            
            // Draw arrow head as a proper triangle
            if (edge.animationProgress > 0.9f) {
                // Define the three vertices of the triangle arrow head
                Vector2 v1 = { currentEndX, startY - 10 };       // Top vertex
                Vector2 v2 = { currentEndX, startY + 10 };       // Bottom vertex
                Vector2 v3 = { currentEndX + 10, startY };       // Tip vertex (pointing right)
                
                // Draw the triangle with proper vertex ordering
                DrawTriangle(v1, v2, v3, RED);
            }
        }
    }
    
    // // Draw pseudocode with highlighting if available
    // if (!currentPseudoCode.empty()) {
    //     int startX = 50;
    //     int startY = 50;
    //     int lineHeight = 30;
        
    //     // Draw operation name
    //     DrawTextEx(customFont, currentOperation.c_str(), (Vector2){startX, startY - lineHeight}, 24, 1, BLACK);
        
    //     // Draw each line of pseudocode
    //     for (size_t i = 0; i < currentPseudoCode.size(); i++) {
    //         Color textColor = (i == currentHighlightedLine) ? RED : BLACK;
    //         DrawTextEx(customFont, currentPseudoCode[i].c_str(), (Vector2){startX, startY + i * lineHeight}, 20, 1, textColor);
    //     }
    // }
}

// Helper function to set pseudocode for addHead operation
void LinkedList::setPseudoCodeAddHead(int value) {
    currentOperation = "Add Head";
    currentPseudoCode.clear();
    
    char valueLine[50];
    sprintf(valueLine, "Node node = new Node(%d)", value);
    
    currentPseudoCode.push_back(valueLine);
    currentPseudoCode.push_back("node.next = head");
    currentPseudoCode.push_back("head = node");
    
    currentHighlightedLine = 0; // Start at first line
}

// Helper function to set pseudocode for addTail operation
void LinkedList::setPseudoCodeAddTail(int value) {
    currentOperation = "Add Tail";
    currentPseudoCode.clear();
    
    char valueLine[50];
    sprintf(valueLine, "Node node = new Node(%d)", value);
    
    currentPseudoCode.push_back(valueLine);
    currentPseudoCode.push_back("tail.next = node");
    currentPseudoCode.push_back("tail = node");
    currentHighlightedLine = 0; // Start at first line
}

// Helper function to set pseudocode for insertAfter operation
void LinkedList::setPseudoCodeInsertAfter(int index, int value) {
    currentOperation = "Insert After";
    currentPseudoCode.clear();
    
    char valueLine[50];
    sprintf(valueLine, "Node newNode = new Node(%d)", value);
    
    currentPseudoCode.push_back("Node curr = head");
    currentPseudoCode.push_back("for (int pos = 0; pos < index; pos++)");
    currentPseudoCode.push_back("    curr = curr.next");
    currentPseudoCode.push_back("// Found node at index");
    currentPseudoCode.push_back(valueLine);
    currentPseudoCode.push_back("newNode.next = curr.next, curr.next = newNode");

    currentHighlightedLine = 0; // Start at first line
}

// Helper function to set pseudocode for insertAfter operation
void LinkedList::setPseudoCodeRemoveAtIdx() {
    currentOperation = "Remove At Index";
    currentPseudoCode.clear();
    currentPseudoCode.push_back("// Check if list is empty");
    currentPseudoCode.push_back("Node* curr = head");
    currentPseudoCode.push_back("for (int pos = 0; pos < index-1; pos++)");
    currentPseudoCode.push_back("    curr = curr.next");
    currentPseudoCode.push_back("Node* temp = curr.next, curr.next = temp.next // Node to delete & bypass it");
    currentPseudoCode.push_back("delete temp // Free memory");
    
    currentHighlightedLine = 0; // Start at first line
}


void LinkedList::setPseudoCodeSearch() {
    currentOperation = "Search Value";
    currentPseudoCode.clear();
    currentPseudoCode.push_back("if empty, return NOT_FOUND");
    currentPseudoCode.push_back("index = 0, tmp = head");
    currentPseudoCode.push_back("while (tmp.item != v && index < list.size())");
    currentPseudoCode.push_back("  index++, tmp = tmp.next");
    currentPseudoCode.push_back("return index < list.size()");
    
    currentHighlightedLine = 0; // Start at first line
}

// Helper function to set pseudocode for delete operation
void LinkedList::setPseudoCodeDelete(int value) {
    currentOperation = "Delete Node";
    currentPseudoCode.clear();
    
    char valueLine[50];
    sprintf(valueLine, "// Delete node with value %d", value);
    
    currentPseudoCode.push_back(valueLine);
    currentPseudoCode.push_back("if (head == NULL) return");
    currentPseudoCode.push_back("Node* curr = head");
    currentPseudoCode.push_back("while (curr->next && curr->next->value != value)");
    currentPseudoCode.push_back("   curr = curr->next");
    currentPseudoCode.push_back("curr->next = curr->next->next // Bypass the node to delete");
    currentPseudoCode.push_back("delete curr->next // Free memory");
    
    currentHighlightedLine = 0; // Start at first line
}

// Update the highlighted pseudocode line based on animation state
void LinkedList::updatePseudoCodeHighlight() {
    if (currentPseudoCode.empty()) return;
    
    // Check if we're adding a new head
    if (currentOperation == "Add Head" && lastInsertedNode && lastInsertedNode == head) {
        // If node is moving to target - highlight line 1
        float dx = lastInsertedNode->targetX - lastInsertedNode->x;
        float dy = lastInsertedNode->targetY - lastInsertedNode->y;
        
        if (fabs(dx) > SETTLE_DISTANCE || fabs(dy) > SETTLE_DISTANCE) {
            currentHighlightedLine = 0; // "Node node = new Node(value)"
        }
        // If node is close to target but edge is animating - highlight line 2
        else if (areNodesSettled()) {
            // Check if the edge from head is still animating
            for (auto& edge : edges) {
                if (edge.startNode == head && edge.isAnimating) {
                    currentHighlightedLine = 1; // "node.next = head"
                    std::cout << "Edge animating from head" << '\n';
                    return;
                }
            }
            // If no edge is animating, highlight the last line
            currentHighlightedLine = 2; // "head = node"
        }
    }
    // Check if we're adding a tail
    else if (currentOperation == "Add Tail" && TailNode) {
        // If node is moving to target - highlight line 1
        float dx = TailNode->targetX - TailNode->x;
        float dy = TailNode->targetY - TailNode->y;
        
        if (fabs(dx) > SETTLE_DISTANCE || fabs(dy) > SETTLE_DISTANCE) {
            currentHighlightedLine = 0; // "Node node = new Node(value)"
        }
        // If node is close to target but edge is animating - highlight line 2
        else if (areNodesSettled()) {
            // Check if the edge to TailNode is still animating
            bool foundAnimatingEdge = false;
            for (auto& edge : edges) {
                if(edge.endNode == TailNode){
                    std::cout << "corresponding edge found: " << edge.endNode->value << " - " << TailNode->value << '\n';
                    std::cout << "Animation progress: " << edge.animationProgress << '\n';
                    
                    // Check animation progress instead of just isAnimating flag
                    if (edge.animationProgress < 1.0f) {
                        currentHighlightedLine = 1; // "tail.next = node"
                        std::cout << "Edge animating to tail" << '\n';
                        foundAnimatingEdge = true;
                        break;
                    }
                }
            }
            
            // If no edge is animating, highlight the last line
            if (!foundAnimatingEdge) {
                currentHighlightedLine = 2; // "tail = node"
            }
        }
    }
    // Check if we're inserting after a specific index
    else if (currentOperation == "Insert After") {
        // Logic for insertAfter operation
        // Determine if we're in traversal phase, node creation, or linking phase
        if (lastInsertedNode) {
            Node* curr = head;
            int index = 0;
            while (curr && curr->next != lastInsertedNode) {
                curr = curr->next;
                index++;
            }
            
            if (curr && curr->next == lastInsertedNode) {
                // Check if node is moving to position or edge is being created
                float dx = lastInsertedNode->targetX - lastInsertedNode->x;
                float dy = lastInsertedNode->targetY - lastInsertedNode->y;
                
                if (fabs(dx) > SETTLE_DISTANCE || fabs(dy) > SETTLE_DISTANCE) {
                    currentHighlightedLine = 5; // "Node node = new Node(value)"
                } else {
                    for (auto& edge : edges) {
                        if ((edge.startNode == curr && edge.endNode == lastInsertedNode) || 
                            (edge.startNode == lastInsertedNode)) {
                            
                            if (edge.isAnimating && edge.startNode == lastInsertedNode) {
                                currentHighlightedLine = 6; // "node.next = current.next"
                                return;
                            } else if (edge.isAnimating && edge.endNode == lastInsertedNode) {
                                currentHighlightedLine = 7; // "current.next = node"
                                return;
                            }
                        }
                    }
                    currentHighlightedLine = 7; // Final step
                }
            } else {
                // Still in traversal
                currentHighlightedLine = 2; // "while position < index:"
            }
        } else {
            // Initial traversal state
            currentHighlightedLine = 0; // "Node current = head"
        }
    }
    // Check if we're in delete operation
    else if (currentOperation == "Delete Node") {
        if (isDeleting) {
            if (currentDeleteNode) {
                if (currentDeleteNode == head) {
                    currentHighlightedLine = 1; // "if head.value == targetValue:"
                } else {
                    currentHighlightedLine = 7; // "while current.next && current.next.value != targetValue:"
                }
            } else if (deleteFound) {
                if (head && head->value == deleteTargetValue) {
                    currentHighlightedLine = 3; // "head = head.next"
                } else {
                    currentHighlightedLine = 11; // "current.next = temp.next"
                }
            }
        } else {
            // Deletion completed
            currentHighlightedLine = 12; // Last step
        }
    }
}

// Hàm addHead(): tạo node mới với giá trị cho trước và update layout
void LinkedList::addHead(int value) {
    // Set up pseudocode for this operation
    setPseudoCodeAddHead(value);
    
    Node* newNode = new Node(value);
    newNode->x = -NODE_WIDTH;
    newNode->y = (SCREEN_HEIGHT - NODE_HEIGHT) / 2.0f;
    
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
    newNode->y = (SCREEN_HEIGHT - NODE_HEIGHT) / 2.0f;
    
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
    newNode->y = -NODE_HEIGHT;
    
    // Handle next pointers
    newNode->next = curr->next;
    curr->next = newNode;
    
    // Track the node before the newly inserted node
    lastInsertedNode = curr;
    
    // Update layout
    updateTargets();
}

// Hàm animateDeleteTraversal(): duyệt danh sách highlight dần các node từ đầu cho đến node có giá trị cần xóa
void LinkedList::animateDeleteTraversal(int value) {
    Node* curr = head;    
    // Clear any previous animation states
    animationStates.clear();
    
    // First capture the initial state before any highlighting
    animationStates.push_back(captureCurrentState());
    currentHighlightedLine = 0;
    for(int i = 0; i < 60; i++)
        animationStates.push_back(captureCurrentState());
    currentHighlightedLine = 1;
        for(int i = 0; i < 60; i++)
            animationStates.push_back(captureCurrentState());
    currentHighlightedLine = 2;
        for(int i = 0; i < 60; i++)
            animationStates.push_back(captureCurrentState());
        
    while (curr) {
        // Save original color
        Color original = curr->color;
        
        // Highlight current node
        curr->color = HIGHLIGHT_COLOR;
        currentHighlightedLine = 3;
        // Capture the state with this node highlighted
        animationStates.push_back(captureCurrentState());
        
        // Add multiple frames to create a pause effect
        for (int i = 0; i < 60; i++) {
            animationStates.push_back(captureCurrentState());
        }
        
        // Check if this is the node to be deleted
        if (curr->value == value) {
            // Extra highlight for target node with more visible color
            curr->color = DELETE_HIGHLIGHT_COLOR;
            currentHighlightedLine = 5;
            animationStates.push_back(captureCurrentState());
            // Add more frames for emphasis on the target node
            for (int i = 0; i < 120; i++) {
                animationStates.push_back(captureCurrentState());
            }
            
            // Don't reset the color - leave it highlighted in red
            break;
        }
        currentHighlightedLine = 4;
        for(int i = 0; i < 60; i++)
            animationStates.push_back(captureCurrentState());
        // Reset to original color before moving to next node
        curr->color = original;
        animationStates.push_back(captureCurrentState());
        
        curr = curr->next;
    }
    
    currentHighlightedLine = 6;
    // Add a few final frames showing the end state
    for (int i = 0; i < 60; i++) {
        animationStates.push_back(captureCurrentState());
    }

}

// Hàm animateInsertAfterTraversal(): duyệt danh sách highlight dần các node từ đầu cho đến node có chỉ số index
void LinkedList::animateInsertAfterTraversal(int index) {
    Node* curr = head;
    int pos = 0;
    
    // Clear any previous animation states
    animationStates.clear();
    
    // First capture the initial state before any highlighting
    currentHighlightedLine = 0;
    for(int i = 0; i < 60; i++)
        animationStates.push_back(captureCurrentState());

    
    while (curr) {
        // Save original color
        Color original = curr->color;
        
        // Highlight current node
        curr->color = HIGHLIGHT_COLOR;
        
        // Capture the state with this node highlighted
    
        // Add multiple frames to create a pause effect
        currentHighlightedLine = 1;
        for (int i = 0; i < 60; i++) {
            animationStates.push_back(captureCurrentState());
        }
        
        
        if (pos == index) {
            // Extra highlight for target node with more visible color
            curr->color = RED;
            currentHighlightedLine = 3;
            animationStates.push_back(captureCurrentState());
            
            // Add more frames for emphasis on the target node
            for (int i = 0; i < 120; i++) { // Increase to 120 frames for longer emphasis
                animationStates.push_back(captureCurrentState());
            }
            
            // Reset color back to original
            curr->color = original;
            //animationStates.push_back(captureCurrentState());
            break;
        }
        currentHighlightedLine = 2;
        for(int i = 0; i < 60; i++)
            animationStates.push_back(captureCurrentState());
        
        // Reset to original color before moving to next node
        curr->color = original;
        animationStates.push_back(captureCurrentState());
        
        pos++;
        curr = curr->next;
    }
    
    // // Add a few final frames showing the end state
    // for (int i = 0; i < 60; i++) {
    //     animationStates.push_back(captureCurrentState());
    // }
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


// Capture current state of the linked list with simplified structure
LinkedListFrameState LinkedList::captureCurrentState() {
    LinkedListFrameState state;
    std::map<Node*, int> nodeToIndex;
    
    // First pass: serialize all nodes with positions and colors
    Node* curr = head;
    int index = 0;
    while (curr) {
        SerializableNode node;
        node.value = curr->value;
        node.x = curr->x;
        node.y = curr->y;
        node.color = curr->color;
        node.nextNodeIndex = -1; // Will be set in second pass
        
        nodeToIndex[curr] = index++;
        state.nodes.push_back(node);
        
        curr = curr->next;
    }
    
    // Second pass: set next indices
    curr = head;
    index = 0;
    while (curr) {
        if (curr->next && nodeToIndex.find(curr->next) != nodeToIndex.end()) {
            state.nodes[index].nextNodeIndex = nodeToIndex[curr->next];
        }
        index++;
        curr = curr->next;
    }
    
    // Serialize edges
    for (const auto& edge : edges) {
        if (nodeToIndex.find(edge.startNode) != nodeToIndex.end() && 
            nodeToIndex.find(edge.endNode) != nodeToIndex.end()) {
            
            SerializableEdge serialEdge;
            serialEdge.startNodeIndex = nodeToIndex[edge.startNode];
            serialEdge.endNodeIndex = nodeToIndex[edge.endNode];
            serialEdge.animationProgress = edge.animationProgress;
            serialEdge.isAnimating = edge.isAnimating;
            
            state.edges.push_back(serialEdge);
        }
    }
    
    // Add pseudocode and highlighted line information
    state.pseudoCode = currentPseudoCode;
    state.highlightedLine = currentHighlightedLine;
    state.operationName = currentOperation;
    
    return state;
}

// Visualize a specific saved state
void LinkedList::visualizeState(const LinkedListFrameState& state) {
    // Draw nodes
    for (size_t i = 0; i < state.nodes.size(); i++) {
        const auto& node = state.nodes[i];
        DrawRectangle((int)node.x, (int)node.y, NODE_WIDTH, NODE_HEIGHT, node.color);
        // Draw node value
        char text[16];
        sprintf(text, "%d", node.value);
        int textWidth = MeasureText(text, 20);
        DrawText(text, (int)node.x + (NODE_WIDTH - textWidth) / 2, 
                (int)node.y + (NODE_HEIGHT / 2 - 10), 20, BLACK);
    }
    
    // Draw edges
    for (const auto& edge : state.edges) {
        if (edge.animationProgress > 0 && 
            edge.startNodeIndex < state.nodes.size() && 
            edge.endNodeIndex < state.nodes.size()) {
            
            const auto& startNode = state.nodes[edge.startNodeIndex];
            const auto& endNode = state.nodes[edge.endNodeIndex];
            
            // Start position is right side of start node
            float startX = startNode.x + NODE_WIDTH;
            float startY = startNode.y + NODE_HEIGHT / 2;
            
            // End position is left side of end node
            float endX = endNode.x;
            float endY = endNode.y + NODE_HEIGHT / 2;
            
            // Leave space before the end node for arrow visibility
            float arrowOffset = 30;
            
            // Calculate the effective endpoint with offset
            float effectiveEndX = endX - arrowOffset;
            
            // Calculate total distance and current distance
            float totalDistance = effectiveEndX - startX;
            float currentDistance = totalDistance * edge.animationProgress;
            
            // Calculate current endpoint
            float currentEndX = startX + currentDistance;
            
            // Draw the main line
            DrawLineEx(
                (Vector2){startX, startY},
                (Vector2){currentEndX, startY},
                3.0f,
                BLACK
            );
            
            // Draw arrow head
            if (edge.animationProgress > 0.9f) {
                // Define the three vertices of the triangle arrow head
                Vector2 v1 = { currentEndX, startY - 10 };       // Top vertex
                Vector2 v2 = { currentEndX, startY + 10 };       // Bottom vertex
                Vector2 v3 = { currentEndX + 10, startY };       // Tip vertex (pointing right)
                
                // Draw the triangle with proper vertex ordering
                DrawTriangle(v1, v2, v3, RED);
            }
        }
    }
    
    // // Draw pseudocode with highlighting if available
    // if (!state.pseudoCode.empty()) {
    //     int startX = 50;
    //     int startY = 50;
    //     int lineHeight = 30;
        
    //     // Draw operation name
    //     DrawTextEx(customFont, state.operationName.c_str(), (Vector2){startX, startY - lineHeight}, 24, 1, BLACK);
        
    //     // Draw each line of pseudocode
    //     for (size_t i = 0; i < state.pseudoCode.size(); i++) {
    //         Color textColor = (i == state.highlightedLine) ? RED : BLACK;
    //         DrawTextEx(customFont, state.pseudoCode[i].c_str(), (Vector2){startX, startY + i * lineHeight}, 20, 1, textColor);
    //     }
    // }
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
        for (size_t i = 0; i < state.pseudoCode.size(); i++) {
            Color textColor = (i == state.highlightedLine) ? RED : BLACK;
            DrawTextEx(customFont, state.pseudoCode[i].c_str(), (Vector2){startX, startY + i * lineHeight}, 20, 1, textColor);
        }
    }
}




// Process the entire animation for a query and store states
std::vector<LinkedListFrameState> LinkedList::processQueryAnimation() {
    std::vector<LinkedListFrameState> states;
    
    // Keep updating animation until it's complete
    while (!isAnimationComplete()) {
        updateAnimation();
        states.push_back(captureCurrentState());
    }
    if(currentOperation == "Add Head"){
        currentHighlightedLine = 2; // Highlight the last line of pseudocode
        // std::cout << "ok here" << '\n';
        states.push_back(captureCurrentState());
    }else if(currentOperation == "Add Tail"){
        currentHighlightedLine = 2; // Highlight the last line of pseudocode
        // std::cout << "ok here" << '\n';
        states.push_back(captureCurrentState());
    }else if(currentOperation == "Insert After"){
        currentHighlightedLine = 6;
        states.push_back(captureCurrentState());
    }else if(currentOperation == "Search Value"){
        currentHighlightedLine = 4;
        states.push_back(captureCurrentState());
        // No pseudocode highlighting for search as per request
        states.push_back(captureCurrentState());
    }else if(currentOperation == "Remove At Index"){
        // No pseudocode highlighting for remove as per request
        currentHighlightedLine = 5;
        states.push_back(captureCurrentState());
    }
    else if(currentOperation == "Delete Node"){
        currentHighlightedLine = 6; // Highlight the last line of pseudocode
        states.push_back(captureCurrentState());
    }
    // Add the final state to history for undo/redo
    addStateToHistory();
    
    return states;
}

// Hàm searchValue(): duyệt danh sách và highlight dần các node từ đầu đến node có giá trị cần tìm
void LinkedList::searchValue(int value) {
    // Set the current operation name
    currentOperation = "Search Value";

    
    Node* curr = head;
    
    // Clear any previous animation states
    animationStates.clear();
    
    // First capture the initial state before any highlighting
    currentHighlightedLine = 0;
    for(int i = 0; i < 30; i++)
        animationStates.push_back(captureCurrentState());
    currentHighlightedLine = 1;
    for(int i = 0; i < 60; i++){
        animationStates.push_back(captureCurrentState());
    }
    bool found = false;
    int position = 0;
    
    
    while (curr) {
        // Save original color
        Color original = curr->color;
        
        // Highlight current node
        curr->color = HIGHLIGHT_COLOR;
        
        // // Capture the state with this node highlighted
        // animationStates.push_back(captureCurrentState());
        
        // // Add multiple frames to create a pause effect
        // for (int i = 0; i < 60; i++) {
        //     animationStates.push_back(captureCurrentState());
        // }
        
        // Check if this is the node we're looking for
        if (curr->value == value) {
            // Extra highlight for target node with more visible color
            curr->color = GREEN; // Use green for found node
            
            // Set found flag
            found = true;
            currentHighlightedLine = 4;
            
            // Capture the state with the node highlighted in green
            for(int i = 0; i < 60; i++)
                animationStates.push_back(captureCurrentState());
            animationStates.push_back(captureCurrentState());
            
            // Add more frames for emphasis on the found node
            for (int i = 0; i < 120; i++) {
                animationStates.push_back(captureCurrentState());
            }
            
            break;
        }
        currentHighlightedLine = 2;
        for(int i = 0; i < 60; i++){
            animationStates.push_back(captureCurrentState());
        }
        
        // Reset to original color before moving to next node
        curr->color = original;
        currentHighlightedLine = 3;
        for(int i = 0; i < 60; i++){
            animationStates.push_back(captureCurrentState());
        }
        //animationStates.push_back(captureCurrentState());
        
        curr = curr->next;
        position++;
    }
    currentHighlightedLine = 4;
    // If value was not found, show some indication
    if (!found) {
        // Add frames showing "not found" state
        for (int i = 0; i < 60; i++) {
            animationStates.push_back(captureCurrentState());
        }
    }
    
    // Add a few final frames showing the end state
    for (int i = 0; i < 60; i++) {
        animationStates.push_back(captureCurrentState());
    }
    curr = head;
    while(curr){
        curr->color = NODE_DEFAULT_COLOR;
        curr = curr->next; 
    }
}

// Hàm removeAtIndex(): duyệt danh sách và highlight dần các node từ đầu cho đến node tại vị trí index và xóa node đó
void LinkedList::removeAtIndexTraversal(int index) {
    // Set the current operation name
    currentOperation = "Remove At Index";
    
    // Set up empty pseudocode (as requested by the user)
    //currentPseudoCode.clear();
    //currentHighlightedLine = -1;
    
    // First animate the traversal to the node at the specified index
    Node* curr = head;
    int position = 0;
    
    // Clear any previous animation states
    animationStates.clear();
    
    
    // First capture the initial state before any highlighting
    currentHighlightedLine = 0;
    for(int i = 0; i < 60; i++)
        animationStates.push_back(captureCurrentState());
    currentHighlightedLine = 1;
    for(int i = 0; i < 60; i++)
        animationStates.push_back(captureCurrentState());
    // If index is 0, we're deleting the head
    if (index == 0) {
        if (head) {
            // Highlight head node to be deleted
            head->color = DELETE_HIGHLIGHT_COLOR;
            
            // Capture the state with head highlighted
            //animationStates.push_back(captureCurrentState());
            currentHighlightedLine = 4;
            // Add frames for emphasis
            for (int i = 0; i < 60; i++) {
                animationStates.push_back(captureCurrentState());
            }
            
            // // Actually delete the head
            // Node* temp = head;
            // head = head->next;
            // delete temp;
            
            // Update layout after deletion
            //updateTargets();
            
            // Return for head deletion case
            currentHighlightedLine = 5;
            return;
        }
    }
    
    // Find the node before the one to be deleted
    while (curr && position < index-1) {
        // Save original color
        Color original = curr->color;
        
        // Highlight current node
        curr->color = HIGHLIGHT_COLOR;
        
        // Capture the state with this node highlighted
        currentHighlightedLine = 2;
        for(int i = 0; i < 60; i++)
            animationStates.push_back(captureCurrentState());
        
        // // Add pause effect
        // for (int i = 0; i < 30; i++) {
        //     animationStates.push_back(captureCurrentState());
        // }
        
        // Reset to original color before moving to next node
        curr->color = original;
        //animationStates.push_back(captureCurrentState());
        
        currentHighlightedLine = 3;
        for(int i = 0; i < 60; i++){
            animationStates.push_back(captureCurrentState());
        }
        curr = curr->next;
        position++;
    }
    
    // If we found the node before the target position
    if (curr && curr->next) {
        // Highlight current node (node before the one to delete)
        curr->color = HIGHLIGHT_COLOR;
        animationStates.push_back(captureCurrentState());
        currentHighlightedLine = 4;
        for (int i = 0; i < 60; i++) {
            animationStates.push_back(captureCurrentState());
        }
        currentHighlightedLine = 4;
        for(int i = 0; i < 60; i++) {
            animationStates.push_back(captureCurrentState());
        }
        // Highlight the node to be deleted
        curr->next->color = DELETE_HIGHLIGHT_COLOR;
        animationStates.push_back(captureCurrentState());
        currentHighlightedLine = 4;
        for (int i = 0; i < 60; i++) {
            animationStates.push_back(captureCurrentState());
        }

        
        // Actually delete the node
        // Node* temp = curr->next;
        // curr->next = temp->next;
        // delete temp;
        
        // Reset current node color
        curr->color = NODE_DEFAULT_COLOR;
        
        // Update layout after deletion
        updateTargets();
    } else {
        // Index out of range, show some indication
        for (int i = 0; i < 60; i++) {
            animationStates.push_back(captureCurrentState());
        }
    }
    currentHighlightedLine = 5;
}

// Hàm removeAtIndex(): duyệt danh sách và highlight dần các node từ đầu cho đến node tại vị trí index và xóa node đó
void LinkedList::removeAtIndex(int index) {
    // Set the current operation name
    currentOperation = "Remove At Index";
    // First animate the traversal to the node at the specified index
    Node* curr = head;
    int position = 0;
    
    // Clear any previous animation states
    
    
    // If index is 0, we're deleting the head
    if (index == 0) {
        if (head) {
            Node* temp = head;
            head = head->next;
            
            delete temp;
            
            // Update layout after deletion
            updateTargets();
            return;
        }
    }
    
    // Find the node before the one to be deleted
    while (curr && position < index-1) {
        
        curr = curr->next;
        position++;
    }
    
    // If we found the node before the target position
    if (curr && curr->next) {
        currentHighlightedLine = 5;
        // Actually delete the node
        Node* temp = curr->next;
        curr->next = temp->next;
        delete temp;
        
        // Reset current node color
        curr->color = NODE_DEFAULT_COLOR;
        
        // Update layout after deletion
        updateTargets();
    }
}

// Clear animation states
void LinkedList::clearAnimationStates() {
    animationStates.clear();
}

// Check if current animation is complete
bool LinkedList::isAnimationComplete() {
    // Check if all nodes have reached their targets
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
    
    // Check if any edge is still animating
    for (const auto& edge : edges) {
        if (edge.isAnimating && edge.animationProgress < 1.0f) {
            return false;
        }
    }
    
    // Check if we're still in the middle of a delete operation
    if (isDeleting) {
        return false;
    }
    
    return true;
}

// Undo/Redo Functions

// Add current state to history
void LinkedList::addStateToHistory() {
    // If we're not at the end of history, remove future states
    if (currentHistoryPosition < historyStates.size() - 1) {
        historyStates.erase(historyStates.begin() + currentHistoryPosition + 1, 
                           historyStates.end());
    }
    
    // Add current state to history
    historyStates.push_back(captureCurrentState());
    currentHistoryPosition = historyStates.size() - 1;
}

bool LinkedList::canUndo() const {
    return currentHistoryPosition > 0;
}

bool LinkedList::canRedo() const {
    return currentHistoryPosition < historyStates.size() - 1;
}

// Restore the linked list structure from a saved state
void LinkedList::restoreFromState(const LinkedListFrameState& state) {
    // Clear the current list
    while (head) {
        Node* temp = head;
        head = head->next;
        delete temp;
    }
    head = nullptr;
    
    // Clear the edges
    edges.clear();
    lastInsertedNode = nullptr; // Reset lastInsertedNode to avoid dangling pointer
    
    // If the state is empty, we're done
    if (state.nodes.empty()) {
        return;
    }
    
    // First pass: create all nodes
    std::vector<Node*> nodeList(state.nodes.size(), nullptr);
    for (size_t i = 0; i < state.nodes.size(); i++) {
        const auto& nodeState = state.nodes[i];
        Node* newNode = new Node(nodeState.value);
        newNode->x = nodeState.x;
        newNode->y = nodeState.y;
        newNode->targetX = nodeState.x;  // Set target to current position
        newNode->targetY = nodeState.y;
        newNode->vx = 0;
        newNode->vy = 0;
        newNode->color = nodeState.color;
        
        nodeList[i] = newNode;
    }
    
    // Second pass: link nodes
    for (size_t i = 0; i < state.nodes.size(); i++) {
        const auto& nodeState = state.nodes[i];
        if (nodeState.nextNodeIndex >= 0 && nodeState.nextNodeIndex < nodeList.size()) {
            nodeList[i]->next = nodeList[nodeState.nextNodeIndex];
        } else {
            nodeList[i]->next = nullptr;
        }
    }
    
    // Set head to first node
    if (!nodeList.empty()) {
        head = nodeList[0];
    }
    
    // Recreate edges
    for (const auto& edgeState : state.edges) {
        if (edgeState.startNodeIndex >= 0 && edgeState.startNodeIndex < nodeList.size() &&
            edgeState.endNodeIndex >= 0 && edgeState.endNodeIndex < nodeList.size()) {
            ListEdge newEdge(nodeList[edgeState.startNodeIndex], nodeList[edgeState.endNodeIndex]);
            newEdge.animationProgress = edgeState.animationProgress;
            newEdge.isAnimating = edgeState.isAnimating;
            edges.push_back(newEdge);
        }
    }
    
    // Restore pseudocode state
    currentPseudoCode = state.pseudoCode;
    currentHighlightedLine = state.highlightedLine;
    currentOperation = state.operationName;
    
    // Ensure layout is refreshed
    updateTargets();
}

const LinkedListFrameState& LinkedList::undo() {
    if (canUndo()) {
        currentHistoryPosition--;
        // Restore the linked list to the previous state
        restoreFromState(historyStates[currentHistoryPosition]);
    }
    return getCurrentHistoryState();
}

const LinkedListFrameState& LinkedList::redo() {
    if (canRedo()) {
        currentHistoryPosition++;
        // Restore the linked list to the next state
        restoreFromState(historyStates[currentHistoryPosition]);
    }
    return getCurrentHistoryState();
}

const LinkedListFrameState& LinkedList::getCurrentHistoryState() const {
    if (historyStates.empty()) {
        static LinkedListFrameState emptyState;
        return emptyState;
    }
    return historyStates[currentHistoryPosition];
}

void LinkedList::clearLinkedList(){
    if(head == nullptr) return;
    Node * curr = head;
    while(curr){
        Node * temp = curr;
        curr = curr -> next;
        delete temp;
    }
    delete curr;
}

void LinkedList::createEmpty() {
    // Clear existing list
    while (head) {
        Node* temp = head;
        head = head->next;
        delete temp;
    }
    edges.clear();
    lastInsertedNode = nullptr;
    TailNode = nullptr;
    
    updateTargets();
    addStateToHistory();
}

void LinkedList::createRandom(int size) {
    createEmpty();
    
    // Calculate initial positions
    float startX = (SCREEN_WIDTH - (size-1)*GAP - NODE_WIDTH) / 2.0f;
    float startY = (SCREEN_HEIGHT - NODE_HEIGHT) / 2.0f;

    // Generate random numbers and add to list
    for(int i = 0; i < size; i++) {
        int value = GetRandomValue(1, 100);
        Node* newNode = new Node(value);
        
        // Set position directly to target
        newNode->x = startX + i * (NODE_WIDTH + GAP);
        newNode->y = startY;
        newNode->targetX = newNode->x;
        newNode->targetY = newNode->y;
        newNode->vx = 0;
        newNode->vy = 0;

        // Add to list
        if (!head) {
            head = newNode;
        } else {
            Node* curr = head;
            while (curr->next) curr = curr->next;
            curr->next = newNode;
            
            // Create edge with completed animation
            ListEdge edge(curr, newNode);
            edge.animationProgress = 1.0f;
            edge.isAnimating = false;
            edges.push_back(edge);
        }
    }

    addStateToHistory();
}

void LinkedList::createRandomSorted(int size) {
    createEmpty();
    
    // Calculate initial positions
    float startX = (SCREEN_WIDTH - (size-1)*GAP - NODE_WIDTH) / 2.0f;
    float startY = (SCREEN_HEIGHT - NODE_HEIGHT) / 2.0f;
    
    // Generate sorted numbers
    std::vector<int> numbers;
    int current = 1;
    for(int i = 0; i < size; i++) {
        current += GetRandomValue(1, 5);
        numbers.push_back(current);
    }

    // Add numbers to list with direct positioning
    for(int i = 0; i < size; i++) {
        Node* newNode = new Node(numbers[i]);
        
        // Set position directly to target
        newNode->x = startX + i * (NODE_WIDTH + GAP);
        newNode->y = startY;
        newNode->targetX = newNode->x;
        newNode->targetY = newNode->y;
        newNode->vx = 0;
        newNode->vy = 0;

        // Add to list
        if (!head) {
            head = newNode;
        } else {
            Node* curr = head;
            while (curr->next) curr = curr->next;
            curr->next = newNode;
            
            // Create edge with completed animation
            ListEdge edge(curr, newNode);
            edge.animationProgress = 1.0f;
            edge.isAnimating = false;
            edges.push_back(edge);
        }
    }

    addStateToHistory();
}

