// #include "linked_list.h"
// #include <cstdio>
// #include <cmath>
// #include <cstdlib>
// #include <thread>
// #include <chrono>
// #include <iostream>

// // Các hằng số cho kích thước node, khoảng cách, và khung hình
// #define NODE_WIDTH 50
// #define NODE_HEIGHT 50
// // Tăng GAP từ 30 lên 50 để đảm bảo phần đầu mũi tên không bị che.
// #define GAP 50

// #define SCREEN_WIDTH 1920
// #define SCREEN_HEIGHT 1080

// // Các hằng số vật lý cho animation
// #define SETTLE_DISTANCE 1.0f
// #define SPRING_CONSTANT 0.1f
// #define DAMPING 0.4f
// #define MAX_VELOCITY 20.0f

// // Thời gian delay cho hiệu ứng highlight (mili giây)
// #define HIGHLIGHT_DELAY 100

// // Các màu mặc định và màu highlight
// #define NODE_DEFAULT_COLOR LIGHTGRAY
// #define HIGHLIGHT_COLOR YELLOW
// #define DELETE_HIGHLIGHT_COLOR RED

// // =======================
// // Node implementation
// // =======================
// Node::Node(int v)
//     : value(v), next(nullptr),
//       x(0), y(0), targetX(0), targetY(0),
//       vx(0), vy(0) {
//     color = NODE_DEFAULT_COLOR;
//     edge = LLEdge();
// }

// // =======================
// // LinkedList implementation
// // =======================
// LinkedList::LinkedList() 
//     : head(nullptr), isPaused(false), isAnimating(false),
//       isDeleting(false), currentDeleteNode(nullptr), 
//       deleteTargetValue(-1), deleteFound(false),
//       deleteAnimationDelay(30), currentDeleteDelay(0),
//       delayedLLEdgeIndex(-1), firstdelay(0),
//       currentFrameIndex(0), recordingFrames(false), 
//       playbackMode(false), currentOperationName("") {
//     // Save initial empty state
//     saveState();
// }

// LinkedList::~LinkedList() {
//     Node* curr = head;
//     while (curr) {
//         Node* temp = curr->next;
//         delete curr;
//         curr = temp;
//     }
    
//     // Clean up any temporary nodes
//     for (Node* node : tempNodes) {
//         delete node;
//     }
//     tempNodes.clear();
// }

// void LinkedList::togglePause() {
//     isPaused = !isPaused;
// }

// void LinkedList::resumeAnimation() {
//     isPaused = false;
// }

// bool LinkedList::isAnimationPaused() const {
//     return isPaused;
// }

// // Hàm updateTargets() tính toán vị trí của các node sao cho danh sách được căn giữa.
// // Tính tổng chiều rộng: totalWidth = count * NODE_WIDTH + (count - 1) * GAP.
// void LinkedList::updateTargets() {
//     int count = 0;
//     Node* curr = head;
//     while (curr) {
//         count++;
//         curr = curr->next;
//     }
//     if (count == 0) return;
//     float totalWidth = count * NODE_WIDTH + (count - 1) * GAP;
//     float startX = (SCREEN_WIDTH - totalWidth) / 2.0f;
//     float startY = (SCREEN_HEIGHT - NODE_HEIGHT) / 2.0f;
    
//     curr = head;
//     int index = 0;
//     while (curr) {
//         curr->targetX = startX + index * (NODE_WIDTH + GAP);
//         curr->targetY = startY;
//         index++;
//         curr = curr->next;
//     }
// }

// // Update edges after node positions are updated
// void LinkedList::updateEdges() {
//     Node* curr = head;
//     int index = 0;
    
//     while (curr && curr->next) {
//         float midY = curr->y + NODE_HEIGHT / 2.0f;
        
//         // Set edge properties
//         curr->edge.startPos = {curr->x + NODE_WIDTH, midY};
//         curr->edge.endPos = {curr->x + NODE_WIDTH + 30, midY};  // 30 is edge length
//         curr->edge.arrowPos = {curr->edge.endPos.x + 10, midY};
//         curr->edge.visible = (index != delayedLLEdgeIndex);
        
//         index++;
//         curr = curr->next;
//     }
// }

// // Hàm updateAnimation() cập nhật vị trí hiện tại của mỗi node dựa trên hiệu ứng spring/damping.
// void LinkedList::updateAnimation() {
//     if (isPaused || playbackMode) return;
    
//     // Handle delete animation
//     if (isDeleting) {
//         if (currentDeleteDelay > 0) {
//             currentDeleteDelay--;
//             return;
//         }
        
//         if (currentDeleteNode) {
//             // Reset previous node color if it's not the found node
//             if (!deleteFound || currentDeleteNode->value != deleteTargetValue) {
//                 currentDeleteNode->color = NODE_DEFAULT_COLOR;
//             }
//             currentDeleteNode = currentDeleteNode->next;
//             if (currentDeleteNode) {
//                 currentDeleteNode->color = HIGHLIGHT_COLOR;
//                 if (currentDeleteNode->value == deleteTargetValue) {
//                     currentDeleteNode->color = DELETE_HIGHLIGHT_COLOR;
//                     deleteFound = true;
//                     captureFrame("Found node to delete: " + std::to_string(deleteTargetValue));
//                 }
//                 currentDeleteDelay = deleteAnimationDelay; // Reset delay counter
//             } else {
//                 // End of traversal
//                 if (deleteFound) {
//                     // Capture frame before actual deletion
//                     captureFrame("About to delete node: " + std::to_string(deleteTargetValue));
                    
//                     // Save the value being deleted
//                     deletedValues.push_back(deleteTargetValue);
                    
//                     // Actually delete the node
//                     if (head->value == deleteTargetValue) {
//                         Node* temp = head;
//                         head = head->next;
//                         delete temp;
//                     } else {
//                         Node* curr = head;
//                         while (curr->next && curr->next->value != deleteTargetValue) {
//                             curr = curr->next;
//                         }
//                         if (curr->next) {
//                             Node* temp = curr->next;
//                             curr->next = temp->next;
//                             delete temp;
//                         }
//                     }
//                     updateTargets();
//                     captureFrame("Node deleted: " + std::to_string(deleteTargetValue));
                    
//                     // Save state after deletion completes
//                     saveState();
//                 } else {
//                     captureFrame("Node not found: " + std::to_string(deleteTargetValue));
//                 }
//                 isDeleting = false;
//                 deleteFound = false;
//                 deleteTargetValue = -1;
//                 currentDeleteDelay = 0;
//             }
//         }
//     }

//     // Update node positions as before
//     Node* curr = head;
//     while (curr) {
//         float dx = curr->targetX - curr->x;
//         float dy = curr->targetY - curr->y;
//         if (fabs(dx) < SETTLE_DISTANCE && fabs(dy) < SETTLE_DISTANCE &&
//             fabs(curr->vx) < 0.1f && fabs(curr->vy) < 0.1f) {
//             curr->x = curr->targetX;
//             curr->y = curr->targetY;
//             curr->vx = 0;
//             curr->vy = 0;
//         } else {
//             float forceX = dx * SPRING_CONSTANT;
//             float forceY = dy * SPRING_CONSTANT;
//             curr->vx = (curr->vx + forceX) * DAMPING;
//             curr->vy = (curr->vy + forceY) * DAMPING;
//             float speed = sqrt(curr->vx * curr->vx + curr->vy * curr->vy);
//             if (speed > MAX_VELOCITY) {
//                 curr->vx = (curr->vx / speed) * MAX_VELOCITY;
//                 curr->vy = (curr->vy / speed) * MAX_VELOCITY;
//             }
//             curr->x += curr->vx;
//             curr->y += curr->vy;
//         }
//         curr = curr->next;
//     }
    
//     // Update edges after node positions
//     updateEdges();
    
//     // Record a frame if we're in recording mode
//     if (recordingFrames) {
//         // Make sure we capture frames even if nothing is moving
//         static int frameCounter = 0;
//         if (frameCounter++ % 3 == 0) {  // Every 3rd frame
//             captureFrame("Animation frame " + std::to_string(frameCounter));
//         }
        
//         // Ensure we capture a frame when nodes are settled
//         bool allSettled = areAllNodesSettled();
//         static bool wasSettled = false;
        
//         if (allSettled && !wasSettled) {
//             captureFrame("All nodes settled");
//         }
        
//         wasSettled = allSettled;
//     }
// }

// // Hàm visualize() vẽ danh sách: ta chia làm 2 bước để đảm bảo mũi tên (đặc biệt là phần triangle) được vẽ sau các node.
// // Bước 1: Vẽ đường nối giữa các node.
// // Bước 2: Vẽ các node (hình chữ nhật và text) và sau đó vẽ phần mũi tên (triangle) lên trên.
// void LinkedList::visualize() {
//     if (!head || (isPaused && isAnimating && !playbackMode)) return;
    
//     const int FIXED_LLEdge_LENGTH = 30; // Độ dài cố định của cạnh nối giữa các node.
    
//     // Vẽ node đầu tiên
//     DrawRectangle((int)head->x, (int)head->y, NODE_WIDTH, NODE_HEIGHT, head->color);
//     {
//         char text[16];
//         sprintf(text, "%d", head->value);
//         int textWidth = MeasureText(text, 20);
//         DrawText(text, (int)head->x + (NODE_WIDTH - textWidth) / 2, (int)head->y + (NODE_HEIGHT / 2 - 10), 20, BLACK);
//     }
    
//     Node* curr = head;
//     int currentIndex = 0;
    
//     // Các biến tạm để lưu tọa độ cạnh bị delay
//     float delayedStartX = 0, delayedStartY = 0, delayedEndX = 0, delayedEndY = 0;
    
//     // Duyệt qua danh sách, vẽ cạnh và các node tiếp theo
//     while (curr->next) {
//         Node* next = curr->next;
//         int arrowLineStart = (int)curr->x + NODE_WIDTH;
//         int arrowLineEnd = arrowLineStart + FIXED_LLEdge_LENGTH;
//         int midY = (int)curr->y + NODE_HEIGHT / 2;
        
//         // Nếu đây là cạnh được đánh dấu delay (tương ứng với index insertAfter),
//         // lưu tọa độ và bỏ qua việc vẽ ngay lúc này.
//         if (currentIndex == delayedLLEdgeIndex) {
//             delayedStartX = curr->x + NODE_WIDTH;
//             delayedStartY = midY;
//             delayedEndX = delayedStartX + FIXED_LLEdge_LENGTH;
//             delayedEndY = midY;
//         } else {
//             DrawLine(arrowLineStart, midY, arrowLineEnd, midY, BLACK);
//             DrawTriangle(
//                 (Vector2){(float)arrowLineEnd, (float)(midY - 5)},
//                 (Vector2){(float)arrowLineEnd, (float)(midY + 5)},
//                 (Vector2){(float)arrowLineEnd + 10, (float)midY},
//                 BLACK
//             );
//         }
        
//         // Vẽ node tiếp theo
//         DrawRectangle((int)next->x, (int)next->y, NODE_WIDTH, NODE_HEIGHT, next->color);
//         {
//             char text[16];
//             sprintf(text, "%d", next->value);
//             int textWidth = MeasureText(text, 20);
//             DrawText(text, (int)next->x + (NODE_WIDTH - textWidth) / 2, (int)next->y + (NODE_HEIGHT / 2 - 10), 20, BLACK);
//         }
        
//         currentIndex++;
//         curr = next;
//     }
    
//     // Sau khi vẽ hết các cạnh khác, vẽ cạnh bị delay nếu có.
//     if (delayedLLEdgeIndex != -1) {
//         if(firstdelay == 0){
//             std::cout << "time to delay" << '\n';
//             firstdelay = 1;
//         } 
//         //std::this_thread::sleep_for(std::chrono::milliseconds(5000));
//         std::cout << "DELAYING" << " " << delayedLLEdgeIndex << '\n';
//         //return;
//         DrawLine((int)delayedStartX, (int)delayedStartY, (int)delayedEndX, (int)delayedEndY, BLACK);
//         DrawTriangle(
//             (Vector2){delayedEndX, delayedEndY - 5},
//             (Vector2){delayedEndX, delayedEndY + 5},
//             (Vector2){delayedEndX + 10, delayedEndY},
//             BLACK
//         );
//         // Reset lại delayedLLEdgeIndex để tránh vẽ lại trong lần visualize tiếp theo
//         delayedLLEdgeIndex = -1;
//         firstdelay = 0;
//     }
// }

// // Frame recording/playback methods
// void LinkedList::resetAnimationState() {
//     // Reset animation and playback flags
//     playbackMode = false;
//     recordingFrames = false;
//     isAnimating = false;
    
//     // Don't clear the frames here - we'll do that in startRecordingFrames
//     currentFrameIndex = 0;
    
//     std::cout << "Animation state reset" << std::endl;
// }

// void LinkedList::startRecordingFrames(const std::string& operationName) {
//     // First ensure we're not in playback mode
//     resetAnimationState();
    
//     // Now start a new recording session
//     operationFrames.clear();
//     currentFrameIndex = 0;
//     recordingFrames = true;
//     currentOperationName = operationName;
    
//     std::cout << "Started recording frames for: " << operationName << std::endl;
//     captureFrame("Operation start: " + operationName);
// }

// void LinkedList::captureFrame(const std::string& description) {
//     if (!recordingFrames) {
//         // Don't capture if we're not in recording mode
//         return;
//     }
    
//     AnimationFrame frame;
//     frame.description = description;
    
//     // Add all nodes to the frame, including nodes to be deleted
//     Node* curr = head;
//     while (curr) {
//         frame.nodeX.push_back(curr->x);
//         frame.nodeY.push_back(curr->y);
//         frame.values.push_back(curr->value);
//         frame.colors.push_back(curr->color);
//         frame.isDeleted.push_back(false); // Not deleted yet
        
//         if (curr->next) {
//             frame.edges.push_back(curr->edge);
//         }
        
//         curr = curr->next;
//     }
    
//     // Also add nodes that are about to be deleted but highlighted
//     if (isDeleting && deleteFound && currentDeleteNode && 
//         currentDeleteNode->value == deleteTargetValue) {
//         // Mark this node as "to be deleted" in the next frame
//         for (size_t i = 0; i < frame.values.size(); i++) {
//             if (frame.values[i] == deleteTargetValue) {
//                 // Mark as deleted but still show in this frame
//                 // The next frame will actually remove it
//                 frame.colors[i] = DELETE_HIGHLIGHT_COLOR;
//             }
//         }
//     }
    
//     operationFrames.push_back(frame);
//     currentFrameIndex = operationFrames.size() - 1;
    
//     std::cout << "Captured frame: " << description << " (Frame #" << operationFrames.size() << ")" << std::endl;
// }

// // This function will fully reconstruct the list from a frame during playback
// void LinkedList::reconstructListFromFrame(int frameIndex) {
//     if (frameIndex < 0 || frameIndex >= operationFrames.size()) {
//         return;
//     }
    
//     // Clear temporary nodes from previous reconstruction
//     for (Node* node : tempNodes) {
//         delete node;
//     }
//     tempNodes.clear();
    
//     const AnimationFrame& frame = operationFrames[frameIndex];
    
//     // If we're in playback mode and this is a delete operation
//     // We may need to reconstruct the list structure
//     if (playbackMode) {
//         // If we're at an earlier frame than when deletion occurred,
//         // we need to temporarily recreate the deleted nodes
//         std::vector<Node*> nodes; // Will hold all nodes in order
        
//         // First, add all existing nodes
//         Node* curr = head;
//         while (curr) {
//             nodes.push_back(curr);
//             curr = curr->next;
//         }
        
//         // Update the positions of existing nodes
//         for (size_t i = 0; i < nodes.size() && i < frame.values.size(); i++) {
//             nodes[i]->x = frame.nodeX[i];
//             nodes[i]->y = frame.nodeY[i];
//             nodes[i]->color = frame.colors[i];
            
//             if (i < nodes.size() - 1 && i < frame.edges.size()) {
//                 nodes[i]->edge = frame.edges[i];
//             }
//         }
        
//         // Check if we need to add back deleted nodes for this frame
//         if (nodes.size() < frame.values.size()) {
//             // We're missing some nodes that were deleted
//             for (size_t i = nodes.size(); i < frame.values.size(); i++) {
//                 // Create temporary nodes for the deleted values
//                 Node* tempNode = new Node(frame.values[i]);
//                 tempNode->x = frame.nodeX[i];
//                 tempNode->y = frame.nodeY[i];
//                 tempNode->color = frame.colors[i];
                
//                 // Add to temporary node list for cleanup later
//                 tempNodes.push_back(tempNode);
                
//                 // Find where to insert this node
//                 if (i == 0) {
//                     // This should become the new head
//                     tempNode->next = head;
//                     head = tempNode;
//                 } else if (i < nodes.size()) {
//                     // Insert in the middle
//                     nodes[i-1]->next = tempNode;
//                     tempNode->next = nodes[i];
//                 } else {
//                     // Insert at the end
//                     if (nodes.empty()) {
//                         head = tempNode;
//                     } else {
//                         nodes.back()->next = tempNode;
//                     }
//                 }
                
//                 // Add to our nodes list
//                 nodes.push_back(tempNode);
//             }
//         }
        
//         // Re-establish all edge connections
//         for (size_t i = 0; i < nodes.size() - 1 && i < frame.edges.size(); i++) {
//             nodes[i]->edge = frame.edges[i];
//         }
//     } else {
//         // Normal behavior if not in playback mode
//         Node* curr = head;
//         int i = 0;
        
//         while (curr && i < frame.values.size()) {
//             curr->x = frame.nodeX[i];
//             curr->y = frame.nodeY[i];
//             curr->color = frame.colors[i];
            
//             // Apply edge properties if there's a next node
//             if (curr->next && i < frame.edges.size()) {
//                 curr->edge = frame.edges[i];
//             }
            
//             i++;
//             curr = curr->next;
//         }
//     }
// }

// // Replace restoreFrame with this new reconstruction method
// void LinkedList::restoreFrame(int frameIndex) {
//     if (operationFrames.empty() || frameIndex < 0 || frameIndex >= operationFrames.size()) {
//         return;
//     }
    
//     reconstructListFromFrame(frameIndex);
// }

// void LinkedList::stopRecordingFrames() {
//     if (!recordingFrames) {
//         std::cout << "Warning: Stopping recording when not in recording mode" << std::endl;
//         return;
//     }
    
//     recordingFrames = false;
//     captureFrame("Operation complete: " + currentOperationName);
//     playbackMode = true;
    
//     std::cout << "Stopped recording frames. Total frames: " << operationFrames.size() << std::endl;
// }

// // Modified operation methods to handle frame recording

// // Hàm addHead(): tạo node mới với giá trị cho trước, xuất hiện từ bên trái (x = -NODE_WIDTH) và sau đó update layout.
// void LinkedList::addHead(int value) {
//     resetAnimationState();  // Reset animation state first
//     startRecordingFrames("Add Head: " + std::to_string(value));
    
//     Node* newNode = new Node(value);
//     newNode->x = -NODE_WIDTH;
//     newNode->y = (SCREEN_HEIGHT - NODE_HEIGHT) / 2.0f;
//     newNode->next = head;
//     head = newNode;
//     updateTargets();
//     updateEdges();
    
//     // Save state for undo
//     saveState();
    
//     // End recording after the animation completes (handled externally)
//     // ... animation happens in updateAnimation() ...
// }

// // Hàm addTail(): tạo node mới xuất hiện từ bên phải (x = SCREEN_WIDTH) và update layout.
// void LinkedList::addTail(int value) {
//     resetAnimationState();  // Reset animation state first
//     startRecordingFrames("Add Tail: " + std::to_string(value));
    
//     Node* newNode = new Node(value);
//     newNode->x = SCREEN_WIDTH;
//     newNode->y = (SCREEN_HEIGHT - NODE_HEIGHT) / 2.0f;
//     if (!head) {
//         head = newNode;
//     } else {
//         Node* curr = head;
//         while (curr->next)
//             curr = curr->next;
//         curr->next = newNode;
//     }
//     updateTargets();
//     updateEdges();
    
//     // Save state for undo
//     saveState();
    
//     // End recording after the animation completes (handled externally)
// }

// // Hàm insertAfter(): duyệt đến node tại chỉ số index (0-indexed), sau đó tạo node mới xuất hiện từ phía trên (y = -NODE_HEIGHT)
// // và chèn sau node đó.
// void LinkedList::insertAfter(int index, int value) {
//     resetAnimationState();  // Reset animation state first
//     startRecordingFrames("Insert After Node " + std::to_string(index) + ": " + std::to_string(value));
    
//     // Duyệt đến node tại vị trí index (0-indexed)
//     Node* curr = head;
//     int pos = 0;
//     while (curr && pos < index) {
//         curr = curr->next;
//         pos++;
//     }
//     if (!curr) {
//         stopRecordingFrames();
//         return; // Nếu index không hợp lệ, thoát
//     }
    
//     // Capture traversal frames
//     captureFrame("Found insertion point at node " + std::to_string(index));
    
//     // Bước 2: Tạo node mới, xuất hiện từ phía trên
//     Node* newNode = new Node(value);
//     newNode->x = curr->targetX;   // Bắt đầu cùng vị trí x của node hiện tại
//     newNode->y = -NODE_HEIGHT;    // Xuất hiện từ trên
//     newNode->next = curr->next;
    
//     // Bước 3: Nối node mới vào danh sách
//     curr->next = newNode;
//     captureFrame("Created new node with value " + std::to_string(value));
    
//     // Bước 4: Đánh dấu cạnh cần delay (cạnh từ node curr đến node mới)
//     delayedLLEdgeIndex = index;
//     curr->edge.visible = false;
    
//     // Cập nhật lại layout
//     updateTargets();
//     updateEdges();
//     captureFrame("Updating layout...");
    
//     // Save state for undo
//     saveState();
    
//     // End recording after the animation completes (handled externally)
// }

// // Hàm deleteValue(): trước khi xóa, thực hiện highlight traversal để tìm node cần xóa.
// bool LinkedList::deleteValue(int value) {
//     if (!head) return false;
    
//     resetAnimationState();  // Reset animation state first
//     startRecordingFrames("Delete Value: " + std::to_string(value));
    
//     // Initialize delete animation
//     isDeleting = true;
//     currentDeleteNode = head;
//     deleteTargetValue = value;
//     deleteFound = false;
//     currentDeleteNode->color = HIGHLIGHT_COLOR;
//     captureFrame("Starting search for value " + std::to_string(value));
    
//     // Note: We'll need to save state after the animation completes
//     // This will be handled in updateAnimation() when deletion finishes
    
//     return true;
// }

// // Hàm animateInsertAfterTraversal(): duyệt danh sách highlight dần các node từ đầu cho đến node có chỉ số index.
// void LinkedList::animateInsertAfterTraversal(int index) {
//     resetAnimationState();  // Reset animation state first
//     startRecordingFrames("Traversing to node " + std::to_string(index));
    
//     Node* curr = head;
//     int pos = 0;
//     while (curr) {
//         Color original = curr->color;
//         curr->color = HIGHLIGHT_COLOR;
//         captureFrame("Visiting node at position " + std::to_string(pos));
        
//         BeginDrawing();
//         ClearBackground(RAYWHITE);
//         visualize();
//         EndDrawing();
//         std::this_thread::sleep_for(std::chrono::milliseconds(HIGHLIGHT_DELAY));
        
//         curr->color = original;
//         captureFrame("Finished visiting node at position " + std::to_string(pos));
        
//         if (pos == index) {
//             captureFrame("Found target node at position " + std::to_string(index));
//             break;
//         }
//         pos++;
//         curr = curr->next;
//     }
    
//     stopRecordingFrames();
// }

// // Helper to save current state to undo stack
// void LinkedList::saveState() {
//     ListState state;
//     Node* curr = head;
    
//     // Collect all node values in order
//     while (curr) {
//         state.values.push_back(curr->value);
//         curr = curr->next;
//     }
    
//     // Add to undo stack
//     undoStack.push(state);
    
//     // Clear redo stack when a new operation is performed
//     while (!redoStack.empty()) {
//         redoStack.pop();
//     }
// }

// // Helper to load a state into the linked list
// void LinkedList::loadState(const ListState& state) {
//     // Delete current list
//     while (head) {
//         Node* temp = head;
//         head = head->next;
//         delete temp;
//     }
    
//     // Recreate list from state values
//     for (size_t i = 0; i < state.values.size(); i++) {
//         if (i == 0) {
//             head = new Node(state.values[i]);
//             head->x = (SCREEN_WIDTH - NODE_WIDTH) / 2.0f;
//             head->y = (SCREEN_HEIGHT - NODE_HEIGHT) / 2.0f;
//         } else {
//             Node* newNode = new Node(state.values[i]);
//             newNode->x = SCREEN_WIDTH;
//             newNode->y = (SCREEN_HEIGHT - NODE_HEIGHT) / 2.0f;
            
//             Node* curr = head;
//             while (curr->next) {
//                 curr = curr->next;
//             }
//             curr->next = newNode;
//         }
//     }
    
//     // Update positions with animation
//     updateTargets();
// }

// bool LinkedList::undo() {
//     if (undoStack.size() <= 1) {
//         return false; // Nothing to undo (keep initial state)
//     }
    
//     // Save current state to redo stack
//     redoStack.push(undoStack.top());
    
//     // Remove current state
//     undoStack.pop();
    
//     // Load previous state
//     loadState(undoStack.top());
    
//     return true;
// }

// bool LinkedList::redo() {
//     if (redoStack.empty()) {
//         return false; // Nothing to redo
//     }
    
//     // Get state to redo
//     ListState state = redoStack.top();
//     redoStack.pop();
    
//     // Add current state to undo stack without disturbing the stack
//     // (we don't call saveState() because that would clear the redo stack)
//     undoStack.push(state);
    
//     // Load the state
//     loadState(state);
    
//     return true;
// }

// bool LinkedList::canUndo() const {
//     return undoStack.size() > 1; // Keep at least initial state
// }

// bool LinkedList::canRedo() const {
//     return !redoStack.empty();
// }

// void LinkedList::drawProgressBar(Rectangle barRect) {
//     if (operationFrames.empty()) {
//         // Draw empty progress bar with message
//         DrawRectangleRec(barRect, LIGHTGRAY);
//         DrawRectangleLinesEx(barRect, 2, DARKGRAY);
//         DrawText("No animation frames recorded", barRect.x + 10, barRect.y + 5, 20, DARKGRAY);
//         return;
//     }
    
//     // Draw background with gradient for better visibility
//     DrawRectangleGradientH(barRect.x, barRect.y, barRect.width, barRect.height, 
//                           LIGHTGRAY, ColorAlpha(BLUE, 0.3f));
    
//     // Draw border
//     DrawRectangleLinesEx(barRect, 2, DARKGRAY);
    
//     // Calculate progress indicator position
//     float progress = (operationFrames.size() <= 1) ? 0 : 
//                     (float)currentFrameIndex / (operationFrames.size() - 1);
//     float indicatorPos = barRect.x + progress * barRect.width;
    
//     // Draw progress indicator
//     Rectangle indicator = {
//         indicatorPos - 5, barRect.y - 5,
//         10, barRect.height + 10
//     };
//     DrawRectangleRec(indicator, MAROON);
    
//     // Draw frame counter
//     char frameText[32];
//     sprintf(frameText, "Frame: %d/%d", currentFrameIndex + 1, (int)operationFrames.size());
//     DrawText(frameText, barRect.x, barRect.y - 25, 20, BLACK);
    
//     // Draw operation name with background for better visibility
//     int nameWidth = MeasureText(currentOperationName.c_str(), 20);
//     DrawRectangle(barRect.x + barRect.width - nameWidth - 10, 
//                  barRect.y - 25, nameWidth + 10, 25, ColorAlpha(WHITE, 0.7f));
//     DrawText(currentOperationName.c_str(), 
//             barRect.x + barRect.width - nameWidth - 5, 
//             barRect.y - 25, 20, DARKBLUE);
    
//     // Draw current frame description with background
//     if (!operationFrames.empty() && currentFrameIndex < operationFrames.size()) {
//         std::string desc = operationFrames[currentFrameIndex].description;
//         int descWidth = MeasureText(desc.c_str(), 20);
//         DrawRectangle(barRect.x, barRect.y + barRect.height + 10, 
//                      descWidth + 10, 25, ColorAlpha(WHITE, 0.7f));
//         DrawText(desc.c_str(), barRect.x + 5, 
//                 barRect.y + barRect.height + 10, 20, DARKGRAY);
//     }
// }

// bool LinkedList::areAllNodesSettled() const {
//     Node* curr = head;
//     while (curr) {
//         float dx = curr->targetX - curr->x;
//         float dy = curr->targetY - curr->y;
        
//         if (fabs(dx) > SETTLE_DISTANCE || fabs(dy) > SETTLE_DISTANCE || 
//             fabs(curr->vx) > 0.1f || fabs(curr->vy) > 0.1f) {
//             return false;
//         }
//         curr = curr->next;
//     }
//     return true;
// }

// void LinkedList::jumpToFrame(int frameIndex) {
//     if (!playbackMode || operationFrames.empty()) return;
    
//     if (frameIndex >= 0 && frameIndex < operationFrames.size()) {
//         currentFrameIndex = frameIndex;
//         reconstructListFromFrame(currentFrameIndex);
//     }
// }

// std::string LinkedList::getCurrentFrameDescription() const {
//     if (operationFrames.empty() || currentFrameIndex >= operationFrames.size()) {
//         return "";
//     }
//     return operationFrames[currentFrameIndex].description;
// }
