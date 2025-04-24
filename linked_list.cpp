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
    
    // Draw edges first (so they appear behind nodes)
    for (const auto& edge : edges) {
        if (edge.animationProgress > 0) {
            // Start position is right side of start node
            float startX = edge.startNode->x + NODE_RADIUS;
            float startY = edge.startNode->y;
            
            // End position is left side of end node with an offset to prevent overlap
            float endX = edge.endNode->x - NODE_RADIUS;
            float endY = edge.endNode->y;
            
            // Leave space before the end node (20 pixels) for arrow visibility
            float arrowOffset = 20;
            
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
                (Color){52, 73, 94, 255} // Dark slate for edges
            );
            
            // Draw arrow head as a proper triangle
            if (edge.animationProgress > 0.9f) {
                // Define the three vertices of the triangle arrow head
                Vector2 v1 = { currentEndX, startY - 10 };       // Top vertex
                Vector2 v2 = { currentEndX, startY + 10 };       // Bottom vertex
                Vector2 v3 = { currentEndX + 15, startY };       // Tip vertex (pointing right)
                
                // Draw the triangle with proper vertex ordering
                DrawTriangle(v1, v2, v3, (Color){231, 76, 60, 255}); // Red arrow
            }
        }
    }
    
    // Draw nodes
    Node* curr = head;
    while (curr) {
        // Draw shadow for 3D effect
        DrawCircle((int)curr->x + SHADOW_OFFSET, (int)curr->y + SHADOW_OFFSET, 
                   NODE_RADIUS, (Color){0, 0, 0, 60});
        
        // Draw node with gradient for more appealing look
        DrawCircleGradient(
            (int)curr->x, (int)curr->y, 
            NODE_RADIUS, 
            curr->color,
            ColorBrightness(curr->color, 0.7f)
        );
        
        // Draw outline
        DrawCircleLines((int)curr->x, (int)curr->y, NODE_RADIUS, NODE_BORDER_COLOR);
        
        // Draw node value with better positioning
        char text[16];
        sprintf(text, "%d", curr->value);
        int textWidth = MeasureText(text, 20);
        DrawText(text, (int)curr->x - textWidth/2, (int)curr->y - 10, 20, WHITE);
        
        curr = curr->next;
    }
    
    // Then draw edges with animation
    for (const auto& edge : edges) {
        if (edge.animationProgress > 0) {
            // Start position is right side of start node
            float startX = edge.startNode->x + NODE_RADIUS;
            float startY = edge.startNode->y;
            
            // End position is left side of end node with an offset to prevent overlap
            float endX = edge.endNode->x - NODE_RADIUS;
            float endY = edge.endNode->y;
            
            // Leave space before the end node (20 pixels) for arrow visibility
            float arrowOffset = 20;
            
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
                (Color){52, 73, 94, 255} // Dark slate for edges
            );
            
            // Draw arrow head as a proper triangle
            if (edge.animationProgress > 0.9f) {
                // Define the three vertices of the triangle arrow head
                Vector2 v1 = { currentEndX, startY - 10 };       // Top vertex
                Vector2 v2 = { currentEndX, startY + 10 };       // Bottom vertex
                Vector2 v3 = { currentEndX + 15, startY };       // Tip vertex (pointing right)
                
                // Draw the triangle with proper vertex ordering
                DrawTriangle(v1, v2, v3, (Color){231, 76, 60, 255}); // Red arrow
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


