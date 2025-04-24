#include "AVL.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include "Constants.h"
#include <map>
using namespace std;

// Remove duplicate physics constants
namespace {
    // NOTE: All physics constants are now in Constants.h
}

// --- Các hàm helper cho animation --- //
namespace {

    // Kiểm tra vị trí đã ổn định (cho các node: so sánh vị trí hiện tại và target)
    bool IsSettled(AVLNode* node, float tolerance) {
        if (node == nullptr) return true;
        
        // Kiểm tra khoảng cách đến target
        float dx = fabs(node->targetX - node->x);
        float dy = fabs(node->targetY - node->y);
        
        // Kiểm tra cả vận tốc hiện tại
        if ((dx > tolerance || dy > tolerance) || 
            (fabs(node->vx) > SETTLE_VELOCITY || fabs(node->vy) > SETTLE_VELOCITY))
            return false;
            
        return IsSettled(node->left, tolerance) && IsSettled(node->right, tolerance);
    }

    // Cập nhật hàm AnimateUntilSettled để thêm nhiều frame hơn và lưu vào animationStates
    void AnimateUntilSettled(AVL &tree, float tolerance, int maxFrames) {
        int frames = 0;
        cout << "Bắt đầu animation cho đến khi settled...\n";
        while (frames < maxFrames && !IsSettled(tree.getRoot(), tolerance)) {
            if (WindowShouldClose()) break;
            
            // Update physics for all nodes
            tree.animateNodes(tree.getRoot());
            
            // Capture current state for animation timeline
            tree.animationStates.push_back(tree.captureCurrentState());
            
            // BeginDrawing();
            //     tree.draw();
            // EndDrawing();
            
            frames++;
        }
        
        // Capture final settled state
        tree.animationStates.push_back(tree.captureCurrentState());
        cout << "Kết thúc animation sau " << frames << " frames.\n";
    }

    // Cập nhật hàm AnimateHighlight để thêm nhiều frame hơn
    void AnimateHighlight(AVL &tree, AVLNode* node, Color targetColor, float alpha, int maxFrames) {
        Color originalColor = node->color;
        
        for (int frame = 0; frame < maxFrames; frame++) {
            // Calculate interpolated color for this frame
            float progress = (float)frame / maxFrames;
            node->color.r = (unsigned char)(originalColor.r + (targetColor.r - originalColor.r) * progress);
            node->color.g = (unsigned char)(originalColor.g + (targetColor.g - originalColor.g) * progress);
            node->color.b = (unsigned char)(originalColor.b + (targetColor.b - originalColor.b) * progress);
            node->color.a = (unsigned char)(originalColor.a + (targetColor.a - originalColor.a) * progress);
            
            // Capture the state for this frame
            tree.animationStates.push_back(tree.captureCurrentState());
            
            // BeginDrawing();
            //     tree.draw();
            // EndDrawing();
        }
        
        // Set final color
        node->color = targetColor;
        
        // Capture final state
        tree.animationStates.push_back(tree.captureCurrentState());
    }
    
    // Hàm điều chỉnh target positions để căn giữa cây trong màn hình 1920x1080
    void CenterTargets(AVLNode* node, float offsetX, float offsetY) {
        if (node == nullptr) return;
        node->targetX += offsetX;
        node->targetY += offsetY;
        CenterTargets(node->left, offsetX, offsetY);
        CenterTargets(node->right, offsetX, offsetY);
    }
    
} // namespace

// --- Các hàm của lớp AVL --- //

int AVL::getHeight(AVLNode* node) {
    return (node == nullptr) ? 0 : node->height;
}

int AVL::getBalanceFactor(AVLNode* node) {
    return (node == nullptr) ? 0 : getHeight(node->left) - getHeight(node->right);
}

AVLNode* AVL::RotateLeft(AVLNode* node) {
    cout << "RotateLeft on node " << node->data << "\n";
    AVLNode* newRoot = node->right;
    AVLNode* temp = newRoot->left;
    newRoot->left = node;
    node->right = temp;
    node->height = 1 + max(getHeight(node->left), getHeight(node->right));
    newRoot->height = 1 + max(getHeight(newRoot->left), getHeight(newRoot->right));
    return newRoot;
}

AVLNode* AVL::RotateRight(AVLNode* node) {
    cout << "RotateRight on node " << node->data << "\n";
    AVLNode* newRoot = node->left;
    AVLNode* temp = newRoot->right;
    newRoot->right = node;
    node->left = temp;
    node->height = 1 + max(getHeight(node->left), getHeight(node->right));
    newRoot->height = 1 + max(getHeight(newRoot->left), getHeight(newRoot->right));
    return newRoot;
}

// Thêm hiệu ứng khi rotateLeft/rotateRight với nhiều frame hơn
void AVL::self_balance(AVLNode*& node) {
    // Fade in highlight cho node hiện tại sang màu HIGHLIGHT_ROTATE
    AnimateHighlight(*this, node, HIGHLIGHT_ROTATE, 0.1f, 20);
    
    int balance = getBalanceFactor(node);
    cout << "self_balance: Node " << node->data << " có balance factor = " << balance << "\n";
    
    // Add intermediate state showing balance factor
    animationStates.push_back(captureCurrentState());
    
    if (balance > 1) { // Trường hợp trái nặng
        cout << "Trường hợp trái nặng tại node " << node->data << "\n";
        
        // Update pseudocode for Case 1 or 2
        if (getBalanceFactor(node->left) >= 0) {
            // Case 1: Single right rotation
            updatePseudoCodeHighlight(3);
        } else {
            // Case 2: Left-Right rotation
            updatePseudoCodeHighlight(5);
        }
        
        // Add intermediate state highlighting left-heavy case
        animationStates.push_back(captureCurrentState());
        
        if (getBalanceFactor(node->left) < 0) { // Left-Right
            cout << "Left-Right case tại node " << node->data << ": Xoay trái tại con " << node->left->data << "\n";
            
            // Add intermediate state before left rotation
            animationStates.push_back(captureCurrentState());
            
            AnimateHighlight(*this, node->left, HIGHLIGHT_ROTATE, 0.1f, 20);
            node->left = RotateLeft(node->left);
            
            // Add intermediate state after inner rotation
            animationStates.push_back(captureCurrentState());
            
            updateTargets();
            
            // Add intermediate state after targets update
            animationStates.push_back(captureCurrentState());
            
            AnimateUntilSettled(*this, 0.5f, 10000);
            AnimateHighlight(*this, node->left, NODE_DEFAULT_COLOR, 0.1f, 20);
        }
        
        // Add state before the final rotation
        animationStates.push_back(captureCurrentState());
        
        node = RotateRight(node);
        
        // Add state right after structural change
        animationStates.push_back(captureCurrentState());
        
        updateTargets();
        
        // Add state after target positions are updated
        animationStates.push_back(captureCurrentState());
        
        AnimateUntilSettled(*this, 0.5f, 10000);
        if (node->right != nullptr) {
            AnimateHighlight(*this, node->right, NODE_DEFAULT_COLOR, 0.1f, 20);
        }
    } else if (balance < -1) { // Trường hợp phải nặng
        cout << "Trường hợp phải nặng tại node " << node->data << "\n";
        
        // Update pseudocode for Case 3 or 4
        if (getBalanceFactor(node->right) <= 0) {
            // Case 3: Single left rotation
            updatePseudoCodeHighlight(7);
        } else {
            // Case 4: Right-Left rotation
            updatePseudoCodeHighlight(9);
        }
        
        // Add intermediate state highlighting right-heavy case
        animationStates.push_back(captureCurrentState());
        
        if (getBalanceFactor(node->right) > 0) { // Right-Left
            cout << "Right-Left case tại node " << node->data << ": Xoay phải tại con " << node->right->data << "\n";
            
            // Add intermediate state before right rotation
            animationStates.push_back(captureCurrentState());
            
            AnimateHighlight(*this, node->right, HIGHLIGHT_ROTATE, 0.1f, 20);
            node->right = RotateRight(node->right);
            
            // Add intermediate state after inner rotation
            animationStates.push_back(captureCurrentState());
            
            updateTargets();
            
            // Add intermediate state after targets update
            animationStates.push_back(captureCurrentState());
            
            AnimateUntilSettled(*this, 0.5f, 10000);
            AnimateHighlight(*this, node->right, NODE_DEFAULT_COLOR, 0.1f, 20);
        }
        
        // Add state before the final rotation
        animationStates.push_back(captureCurrentState());
        
        node = RotateLeft(node);
        
        // Add state right after structural change
        animationStates.push_back(captureCurrentState());
        
        updateTargets();
        
        // Add state after target positions are updated
        animationStates.push_back(captureCurrentState());
        
        AnimateUntilSettled(*this, 0.5f, 10000);
        if (node->left != nullptr) {
            AnimateHighlight(*this, node->left, NODE_DEFAULT_COLOR, 0.1f, 20);
        }
    } else {
        // Tree is balanced - highlight the final line
        updatePseudoCodeHighlight(11);
    }
    
    // Add a state for each height update
    animationStates.push_back(captureCurrentState());
    
    AnimateHighlight(*this, node, NODE_DEFAULT_COLOR, 0.1f, 20);
    
    // Add final state after recoloring
    animationStates.push_back(captureCurrentState());
}

// Cập nhật insertHelper để sử dụng các màu sáng hơn khi thêm node
void AVL::insertHelper(AVLNode*& node, int key, float parentX, float parentY) {
    // Update pseudocode highlight to first line
    updatePseudoCodeHighlight(0);
    
    if (node == nullptr) {
        node = new AVLNode(key);
        // Gán vị trí ban đầu cho node mới dựa trên vị trí của node cha
        node->x = parentX;
        node->y = parentY;
        AnimateHighlight(*this, node, HIGHLIGHT_INSERT, 0.1f, 20);  // Sử dụng màu highlight_insert
        cout << "Inserted node " << key << " tại vị trí thích hợp\n";
        updateTargets();
        
        // Capture animation state after insertion
        animationStates.push_back(captureCurrentState());
        
        AnimateUntilSettled(*this, 0.5f, 10000);
        AnimateHighlight(*this, node, NODE_DEFAULT_COLOR, 0.1f, 20);
        
        // Capture state after node is settled
        animationStates.push_back(captureCurrentState());
        return;
    }
    
    // Highlight node hiện tại (đang duyệt) với màu highlight_search
    AnimateHighlight(*this, node, HIGHLIGHT_SEARCH, 0.1f, 20);
    
    // Capture state when node is highlighted
    animationStates.push_back(captureCurrentState());
    
    if (key < node->data) {
        cout << "Chèn " << key << " vào bên trái của node " << node->data << "\n";
        insertHelper(node->left, key, node->x, node->y);
        updateTargets();
        
        // Capture state after targets are updated
        animationStates.push_back(captureCurrentState());
        
        AnimateUntilSettled(*this, 0.5f, 10000);
    } else if (key > node->data) {
        cout << "Chèn " << key << " vào bên phải của node " << node->data << "\n";
        insertHelper(node->right, key, node->x, node->y);
        updateTargets();
        
        // Capture state after targets are updated
        animationStates.push_back(captureCurrentState());
        
        AnimateUntilSettled(*this, 0.5f, 10000);
    } else {
        cout << "Duplicate key " << key << " bị bỏ qua.\n";
        updateTargets();
        AnimateUntilSettled(*this, 0.5f, 10000);
        AnimateHighlight(*this, node, NODE_DEFAULT_COLOR, 0.1f, 20);
        return;
    }
    
    node->height = 1 + max(getHeight(node->left), getHeight(node->right));
    AnimateHighlight(*this, node, NODE_DEFAULT_COLOR, 0.1f, 20);
    
    // Capture state before self-balancing
    animationStates.push_back(captureCurrentState());
    
    // Update pseudocode highlight to check balance
    updatePseudoCodeHighlight(1);
    
    self_balance(node);
    
    // Capture state after self-balancing
    updatePseudoCodeHighlight(11); // Tree is now balanced
    animationStates.push_back(captureCurrentState());
}

void AVL::preOrder(AVLNode* node) {
    if (node != nullptr) {
        cout << node->data << " ";
        preOrder(node->left);
        preOrder(node->right);
    }
}

// Duyệt in-order để tính toán target positions cho các node, với margin ban đầu là 50.
void AVL::computeTargets(AVLNode* node, int depth, float &xCounter) {
    if (node == nullptr) return;
    computeTargets(node->left, depth + 1, xCounter);
    node->targetX = xCounter * HORIZONTAL_SPACING + 50;
    node->targetY = depth * VERTICAL_SPACING + 50;
    cout << "computeTargets: Node " << node->data << " tại depth " << depth 
         << " -> targetX: " << node->targetX << ", targetY: " << node->targetY << "\n";
    xCounter += 1.0f;
    computeTargets(node->right, depth + 1, xCounter);
}

// Hàm updateTargets cập nhật layout và căn giữa toàn bộ cây
// Trong hàm updateTargets() của AVL.cpp, điều chỉnh vị trí cây
void AVL::updateTargets() {
    cout << "Updating layout targets...\n";
    float xCounter = 0.0f;
    computeTargets(root, 0, xCounter);

    // Tính offset để căn giữa theo chiều ngang:
    float offsetX = 960 - ((100 + (xCounter - 1) * HORIZONTAL_SPACING) / 2);

    // Tính offset theo chiều dọc: điều chỉnh để cây nằm cao hơn
    // Thay vì căn giữa theo chiều dọc, giảm 150px để kéo cây lên trên
    int h = (root != nullptr) ? root->height : 0;
    float offsetY = 540 - ((100 + (h - 1) * VERTICAL_SPACING) / 2) - 150; 

    // Áp dụng offset cho tất cả các node target
    CenterTargets(root, offsetX, offsetY);
}


// Add more frames during animation in AnimateNodes
void AVL::animateNodes(AVLNode* node) {
    if (node == nullptr) return;
    
    float dx = node->targetX - node->x;
    float dy = node->targetY - node->y;
    
    // Add a frame every few physics updates if nodes are still moving significantly
    static int frameCounter = 0;
    bool significantMovement = fabs(dx) > 2.0f || fabs(dy) > 2.0f || 
                              fabs(node->vx) > 1.0f || fabs(node->vy) > 1.0f;
                              
    // Nếu node đã gần với target và vận tốc thấp, gắn nó vào target
    if (fabs(dx) < SETTLE_DISTANCE && fabs(dy) < SETTLE_DISTANCE &&
        fabs(node->vx) < SETTLE_VELOCITY && fabs(node->vy) < SETTLE_VELOCITY) {
        node->x = node->targetX;
        node->y = node->targetY;
        node->vx = 0;
        node->vy = 0;
    } else {
        // Tính lực kéo về target
        float forceX = dx * SPRING_CONSTANT;
        float forceY = dy * SPRING_CONSTANT;
        
        // Cập nhật vận tốc với lực cản (damping)
        node->vx = (node->vx + forceX) * DAMPING;
        node->vy = (node->vy + forceY) * DAMPING;
        
        // Giới hạn vận tốc tối đa
        float speed = sqrt(node->vx * node->vx + node->vy * node->vy);
        if (speed > MAX_VELOCITY) {
            node->vx = (node->vx / speed) * MAX_VELOCITY;
            node->vy = (node->vy / speed) * MAX_VELOCITY;
        }
        
        // Cập nhật vị trí
        node->x += node->vx;
        node->y += node->vy;
    }
    
    animateNodes(node->left);
    animateNodes(node->right);
}

// Update phương thức drawTree để cải thiện hiển thị text
// Update phương thức drawTree để loại bỏ hình vuông và làm text dễ đọc hơn
void AVL::drawTree(AVLNode* node) {
    if (node == nullptr) return;
    
    // Vẽ các đường nối với độ dày 2px và màu xám nhạt
    if (node->left != nullptr) {
        DrawLineEx(
            (Vector2){ node->x, node->y }, 
            (Vector2){ node->left->x, node->left->y }, 
            2.0f, 
            (Color){ 149, 165, 166, 255 }  // Màu xám (Silver)
        );
        drawTree(node->left);
    }
    
    if (node->right != nullptr) {
        DrawLineEx(
            (Vector2){ node->x, node->y }, 
            (Vector2){ node->right->x, node->right->y }, 
            2.0f, 
            (Color){ 149, 165, 166, 255 }  // Màu xám (Silver)
        );
        drawTree(node->right);
    }
    
    // Vẽ bóng đổ nhẹ cho node để tạo hiệu ứng 3D
    DrawCircle((int)node->x + 3, (int)node->y + 3, NODE_RADIUS, (Color){ 0, 0, 0, 50 });
    
    // Vẽ viền cho node
    DrawCircleLines((int)node->x, (int)node->y, NODE_RADIUS + 2, NODE_BORDER_COLOR);
    
    // Vẽ node với màu hiện tại
    DrawCircle((int)node->x, (int)node->y, NODE_RADIUS, node->color);
    
    // Vẽ giá trị của node trực tiếp, không có hộp text
    char textValue[10];
    sprintf(textValue, "%d", node->data);
    
    // Tính toán kích thước text để căn giữa
    int textWidth = MeasureText(textValue, 20);  // Tăng kích thước font lên 20
    
    // Vẽ text chính
    DrawText(textValue, (int)node->x - textWidth/2, (int)node->y - 10, 20, TEXT_COLOR);

    // Vẽ thông tin chiều cao của cây con trái và phải
    char leftHeight[5], rightHeight[5];
    sprintf(leftHeight, "%d", getHeight(node->left));
    sprintf(rightHeight, "%d", getHeight(node->right));
    
    // Vẽ chiều cao bên trái phía trên node - đậm và to hơn
    DrawText(leftHeight, 
        (int)node->x - NODE_RADIUS - 15, 
        (int)node->y - NODE_RADIUS - 20, 
        20,  // Tăng font size từ 16 lên 20
        (Color){ 0, 0, 0, 255 });  // Màu đen hoàn toàn
        
    // Vẽ chiều cao bên phải phía trên node - đậm và to hơn
    DrawText(rightHeight, 
        (int)node->x + NODE_RADIUS - 5, 
        (int)node->y - NODE_RADIUS - 20, 
        20,  // Tăng font size từ 16 lên 20
        (Color){ 0, 0, 0, 255 });  // Màu đen hoàn toàn
        
    // Vẽ giá trị node
    sprintf(textValue, "%d", node->data);
    textWidth = MeasureText(textValue, 20);
    DrawText(textValue, (int)node->x - textWidth/2, (int)node->y - 10, 20, TEXT_COLOR);
}

AVLNode* AVL::findMin(AVLNode* node) {
    AVLNode* current = node;
    while (current->left != nullptr) {
        AnimateHighlight(*this, current, HIGHLIGHT_FIND, 0.1f, 20);
        current = current->left;
    }
    return current;
}

// Add more frames during find operations
AVLNode* AVL::findHelper(AVLNode* node, int key) {
    // Highlight first line - "if this == null"
    updatePseudoCodeHighlight(1);
    
    if (node == nullptr) {
        // Highlight "return null" line
        updatePseudoCodeHighlight(2);
        
        // Capture state at leaf (key not found)
        animationStates.push_back(captureCurrentState());
        return nullptr;
    }
    
    // Capture state before checking node
    animationStates.push_back(captureCurrentState());
    
    // Highlight "else if this key == search value" line
    updatePseudoCodeHighlight(3);
    
    if (key == node->data) {
        // Highlight "return this" line
        updatePseudoCodeHighlight(4);
        
        // Capture state before highlighting found node
        animationStates.push_back(captureCurrentState());
        
        foundNode = node;
        
        // Capture state after found node is set
        animationStates.push_back(captureCurrentState());
        
        return node;
    }
    
    // Animate node highlight to show traversal
    AnimateHighlight(*this, node, HIGHLIGHT_FIND, 0.1f, 15);
    
    // Capture state with highlighted node
    animationStates.push_back(captureCurrentState());
    
    // Highlight appropriate direction
    if (key > node->data) {
        // Highlight "else if this key < search value" line
        updatePseudoCodeHighlight(5);
        
        // Highlight "search right" line
        updatePseudoCodeHighlight(6);
        
        // Capture state before going right
        animationStates.push_back(captureCurrentState());
        
        return findHelper(node->right, key);
    } else {
        // Highlight "else" line
        updatePseudoCodeHighlight(7);
        
        // Highlight "search left" line
        updatePseudoCodeHighlight(8);
        
        // Capture state before going left
        animationStates.push_back(captureCurrentState());
        
        return findHelper(node->left, key);
    }
}

// Add pseudocode for search operation
void AVL::setPseudoCodeSearch(int value) {
    currentOperation = "Search";
    currentPseudoCode.clear();
    
    char valueLine[50];
    sprintf(valueLine, "search %d", value);
    
    currentPseudoCode.push_back(valueLine);
    currentPseudoCode.push_back("if this == null");
    currentPseudoCode.push_back("  return null");
    currentPseudoCode.push_back("else if this key == search value");
    currentPseudoCode.push_back("  return this");
    currentPseudoCode.push_back("else if this key < search value");
    currentPseudoCode.push_back("  search right");
    currentPseudoCode.push_back("else");
    currentPseudoCode.push_back("  search left");
    
    currentHighlightedLine = 0;
}

// Update the find method to use pseudocode
bool AVL::find(int key) {
    // Set up pseudocode for search
    setPseudoCodeSearch(key);
    
    resetColors(root);
    foundNode = nullptr;
    
    AVLNode* result = findHelper(root, key);
    if (result != nullptr) {
        AnimateHighlight(*this, result, HIGHLIGHT_FIND, 0.1f, 20);
        return true;
    }
    
    // The actual history update happens in processOperationAnimation
    // which is called from main.cpp after this completes
    return false;
}

// Add more animation frames during delete operations
AVLNode* AVL::deleteHelper(AVLNode*& node, int key) {
    if (node == nullptr) {
        // Capture state at leaf node (key not found)
        animationStates.push_back(captureCurrentState());
        return nullptr;
    }
    
    // Capture state before highlighting node
    animationStates.push_back(captureCurrentState());
    
    AnimateHighlight(*this, node, HIGHLIGHT_DELETE, 0.1f, 20);
    
    // Capture state after highlighting node
    animationStates.push_back(captureCurrentState());
    
    if (key < node->data) {
        // Capture state before going left
        animationStates.push_back(captureCurrentState());
        
        node->left = deleteHelper(node->left, key);
        
        // Capture state after returning from recursive call
        animationStates.push_back(captureCurrentState());
    }
    else if (key > node->data) {
        // Capture state before going right
        animationStates.push_back(captureCurrentState());
        
        node->right = deleteHelper(node->right, key);
        
        // Capture state after returning from recursive call
        animationStates.push_back(captureCurrentState());
    }
    else { // Tìm thấy node cần xóa
        // Capture state when node is found
        animationStates.push_back(captureCurrentState());
        
        // Trường hợp 1: Node lá hoặc có 1 con
        if (node->left == nullptr) {
            AVLNode* temp = node->right;
            
            // Capture state before deletion
            animationStates.push_back(captureCurrentState());
            
            delete node;
            
            // Return early for leaf node
            return temp;
        }
        else if (node->right == nullptr) {
            AVLNode* temp = node->left;
            
            // Capture state before deletion
            animationStates.push_back(captureCurrentState());
            
            delete node;
            
            // Return early for node with one child
            return temp;
        }
        
        // Trường hợp 2: Node có 2 con - capture state before finding successor
        animationStates.push_back(captureCurrentState());
        
        AVLNode* temp = findMin(node->right);
        
        // Capture state after finding successor
        animationStates.push_back(captureCurrentState());
        
        // Show that we're replacing values
        AnimateHighlight(*this, node, HIGHLIGHT_DELETE, 0.1f, 10);
        AnimateHighlight(*this, temp, HIGHLIGHT_INSERT, 0.1f, 10);
        
        node->data = temp->data;
        
        // Capture state after value replacement
        animationStates.push_back(captureCurrentState());
        
        node->right = deleteHelper(node->right, temp->data);
        
        // Capture state after deleting successor
        animationStates.push_back(captureCurrentState());
    }
    
    // Cập nhật chiều cao
    node->height = 1 + max(getHeight(node->left), getHeight(node->right));
    
    // Capture state after height update
    animationStates.push_back(captureCurrentState());
    
    // Cân bằng lại cây
    self_balance(node);
    
    // Capture state after rebalancing
    animationStates.push_back(captureCurrentState());
    
    return node;
}

// Add pseudocode for delete operation
void AVL::setPseudoCodeDelete(int value) {
    currentOperation = "Delete";
    currentPseudoCode.clear();
    
    char valueLine[50];
    sprintf(valueLine, "remove %d", value);
    
    currentPseudoCode.push_back(valueLine);
    currentPseudoCode.push_back("check balance factor of this and its children");
    currentPseudoCode.push_back("");
    currentPseudoCode.push_back("  case1: this.rotateRight");
    currentPseudoCode.push_back("");
    currentPseudoCode.push_back("  case2: this.left.rotateLeft, this.rotateRight");
    currentPseudoCode.push_back("");
    currentPseudoCode.push_back("  case3: this.rotateLeft");
    currentPseudoCode.push_back("");
    currentPseudoCode.push_back("  case4: this.right.rotateRight, this.rotateLeft");
    currentPseudoCode.push_back("");
    currentPseudoCode.push_back("  this is balanced");
    
    currentHighlightedLine = 0;
}

void AVL::deleteKey(int key) {
    // Set up pseudocode for deletion
    setPseudoCodeDelete(key);
    
    // Perform deletion
    root = deleteHelper(root, key);
    updateTargets();
    AnimateUntilSettled(*this, 0.5f, 10000);
    resetColors(root);
    addStateToHistory();
}

// Check if animation is complete
bool AVL::isAnimationComplete() const {
    if (animationStates.empty()) return true;
    return currentAnimationStep >= animationStates.size() - 1;
}

// Capture current state of tree
AVLTreeState AVL::captureCurrentState() {
    AVLTreeState state;
    std::map<AVLNode*, int> nodeToIndex;
    
    state.rootIndex = -1; // Default to empty tree
    
    // Serialize tree structure if root exists
    if (root) {
        serializeNode(root, state.nodes, nodeToIndex);
        state.rootIndex = 0; // Root is always the first node
    }
    
    // Add traversal order to the state
    state.traversalOrder = traversalOrder;
    
    // Add pseudocode information
    state.pseudoCode = currentPseudoCode;
    state.highlightedLine = currentHighlightedLine;
    state.operationName = currentOperation;
    
    return state;
}

// Serialize a node and its children recursively
void AVL::serializeNode(AVLNode* node, std::vector<SerializableAVLNode>& nodes, std::map<AVLNode*, int>& nodeToIndex) {
    if (!node) return;
    
    // Create serializable node
    SerializableAVLNode serialNode;
    serialNode.data = node->data;
    serialNode.height = node->height;
    serialNode.x = node->x;
    serialNode.y = node->y;
    serialNode.targetX = node->targetX;
    serialNode.targetY = node->targetY;
    serialNode.vx = node->vx;
    serialNode.vy = node->vy;
    serialNode.color = node->color;
    serialNode.leftChildIndex = -1;  // Set default, will update later
    serialNode.rightChildIndex = -1; // Set default, will update later
    
    // Store this node's index
    int currentIndex = nodes.size();
    nodeToIndex[node] = currentIndex;
    nodes.push_back(serialNode);
    
    // Recursively serialize children
    if (node->left) {
        serializeNode(node->left, nodes, nodeToIndex);
        nodes[currentIndex].leftChildIndex = nodeToIndex[node->left];
    }
    
    if (node->right) {
        serializeNode(node->right, nodes, nodeToIndex);
        nodes[currentIndex].rightChildIndex = nodeToIndex[node->right];
    }
}

// Restore tree from saved state
void AVL::restoreFromState(const AVLTreeState& state) {
    // Clear current tree
    // (Proper memory cleanup would need a helper function to delete all nodes)
    // This is simplified for brevity
    // TODO: Add proper node deletion function
    root = nullptr;
    
    if (state.nodes.empty() || state.rootIndex == -1) {
        return; // Empty state
    }
    
    // Create all nodes first
    std::vector<AVLNode*> nodeList(state.nodes.size(), nullptr);
    for (size_t i = 0; i < state.nodes.size(); i++) {
        nodeList[i] = new AVLNode(state.nodes[i].data);
        nodeList[i]->height = state.nodes[i].height;
        nodeList[i]->x = state.nodes[i].x;
        nodeList[i]->y = state.nodes[i].y;
        nodeList[i]->targetX = state.nodes[i].targetX;
        nodeList[i]->targetY = state.nodes[i].targetY;
        nodeList[i]->vx = state.nodes[i].vx;
        nodeList[i]->vy = state.nodes[i].vy;
        nodeList[i]->color = state.nodes[i].color;
    }
    
    // Connect nodes
    for (size_t i = 0; i < state.nodes.size(); i++) {
        if (state.nodes[i].leftChildIndex != -1) {
            nodeList[i]->left = nodeList[state.nodes[i].leftChildIndex];
        }
        
        if (state.nodes[i].rightChildIndex != -1) {
            nodeList[i]->right = nodeList[state.nodes[i].rightChildIndex];
        }
    }
    
    // Set root
    if (state.rootIndex != -1) {
        root = nodeList[state.rootIndex];
    }
}

// Visualize a specific state
void AVL::visualizeState(const AVLTreeState& state) {
    // First draw gradient background as in the draw() method
    for (int y = 0; y < 1080; y++) {
        float factor = (float)y / 1080.0f;
        
        Color currentColor = {
            (unsigned char)(BACKGROUND_COLOR_TOP.r + (BACKGROUND_COLOR_BOTTOM.r - BACKGROUND_COLOR_TOP.r) * factor),
            (unsigned char)(BACKGROUND_COLOR_TOP.g + (BACKGROUND_COLOR_BOTTOM.g - BACKGROUND_COLOR_TOP.g) * factor),
            (unsigned char)(BACKGROUND_COLOR_TOP.b + (BACKGROUND_COLOR_BOTTOM.b - BACKGROUND_COLOR_TOP.b) * factor),
            255
        };
        
        DrawLine(0, y, 1920, y, currentColor);
    }
    
    // Build temporary tree structure to visualize
    if (state.nodes.empty() || state.rootIndex == -1) return;
    
    // Draw highlighted edges first (so they appear below nodes)
    for (const auto& edge : state.highlightedEdges) {
        if (edge.sourceIndex < state.nodes.size() && edge.targetIndex < state.nodes.size()) {
            const auto& source = state.nodes[edge.sourceIndex];
            const auto& target = state.nodes[edge.targetIndex];
            
            DrawLineEx(
                (Vector2){ source.x, source.y },
                (Vector2){ target.x, target.y },
                4.0f,  // Thicker line for highlight
                edge.color
            );
        }
    }
    
    // Draw nodes and connections
    for (size_t i = 0; i < state.nodes.size(); i++) {
        const auto& node = state.nodes[i];
        
        // Draw connections to children
        if (node.leftChildIndex != -1) {
            const auto& leftChild = state.nodes[node.leftChildIndex];
            DrawLineEx(
                (Vector2){ node.x, node.y }, 
                (Vector2){ leftChild.x, leftChild.y }, 
                2.0f, 
                (Color){ 149, 165, 166, 255 }  // Màu xám (Silver)
            );
        }
        
        if (node.rightChildIndex != -1) {
            const auto& rightChild = state.nodes[node.rightChildIndex];
            DrawLineEx(
                (Vector2){ node.x, node.y }, 
                (Vector2){ rightChild.x, rightChild.y }, 
                2.0f, 
                (Color){ 149, 165, 166, 255 }  // Màu xám (Silver)
            );
        }
        
        // Draw shadow
        DrawCircle((int)node.x + 3, (int)node.y + 3, NODE_RADIUS, (Color){ 0, 0, 0, 50 });
        
        // Draw node border
        DrawCircleLines((int)node.x, (int)node.y, NODE_RADIUS + 2, NODE_BORDER_COLOR);
        
        // Draw node with color
        DrawCircle((int)node.x, (int)node.y, NODE_RADIUS, node.color);
        
        // Draw node value
        char textValue[10];
        sprintf(textValue, "%d", node.data);
        int textWidth = MeasureText(textValue, 20);
        DrawText(textValue, (int)node.x - textWidth/2, (int)node.y - 10, 20, TEXT_COLOR);
        
        // Draw height values of left and right subtrees
        char leftHeight[5], rightHeight[5];
        int leftH = 0, rightH = 0;
        
        // Get heights from child indices
        if (node.leftChildIndex != -1) 
            leftH = state.nodes[node.leftChildIndex].height;
        if (node.rightChildIndex != -1)
            rightH = state.nodes[node.rightChildIndex].height;
            
        sprintf(leftHeight, "%d", leftH);
        sprintf(rightHeight, "%d", rightH);
        
        DrawText(leftHeight, 
            (int)node.x - NODE_RADIUS - 15, 
            (int)node.y - NODE_RADIUS - 20, 
            20,
            (Color){ 0, 0, 0, 255 });
            
        DrawText(rightHeight, 
            (int)node.x + NODE_RADIUS - 5, 
            (int)node.y - NODE_RADIUS - 20, 
            20,
            (Color){ 0, 0, 0, 255 });
    }

    // Display traversal order if available
    if (!state.traversalOrder.empty()) {
        // Display traversal type title
        std::string title;
        if (traversalType == "inorder") 
            title = "Inorder Traversal: ";
        else if (traversalType == "preorder")
            title = "Preorder Traversal: ";
        else if (traversalType == "postorder")
            title = "Postorder Traversal: ";
        else
            title = "Traversal Order: ";
            
        DrawText(title.c_str(), 50, 20, 24, (Color){40, 40, 40, 255});
        
        // Build the traversal order string
        std::string orderStr;
        for (size_t i = 0; i < state.traversalOrder.size(); i++) {
            if (i > 0) orderStr += " ==> ";
            orderStr += std::to_string(state.traversalOrder[i]);
        }
        
        // Draw traversal order with a background
        int textWidth = MeasureText(orderStr.c_str(), 24);
        DrawRectangle(50 + MeasureText(title.c_str(), 24), 15, textWidth + 20, 35, 
                     (Color){245, 245, 245, 220});
        DrawRectangleLines(50 + MeasureText(title.c_str(), 24), 15, textWidth + 20, 35, 
                         (Color){200, 200, 200, 255});
        DrawText(orderStr.c_str(), 50 + MeasureText(title.c_str(), 24) + 10, 20, 24, 
                (Color){20, 20, 20, 255});
    }

    // Create a stylish pill-shaped button with gradient and shadow
    float buttonX = 1650;
    float buttonY = 1000;
    if(showPseudoCode == true) buttonY = 500;
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
    if (showPseudoCode && !state.pseudoCode.empty()) {
        int startX = 1400;
        int startY = 600; // Changed from 800 to 600 to move pseudocode higher
        int lineHeight = 30;
        int paddingX = 20;
        int paddingY = 15;
        
        // Calculate box dimensions based on content
        int boxWidth = 0;
        for (const auto& line : state.pseudoCode) {
            int width = MeasureText(line.c_str(), 20) + 2 * paddingX;
            boxWidth = std::max(boxWidth, width);
        }
        
        // Make sure the box is wide enough for the title too
        int titleWidth = MeasureText(currentOperation.c_str(), 24) + 2 * paddingX;
        boxWidth = std::max(boxWidth, titleWidth);
        
        int boxHeight = state.pseudoCode.size() * lineHeight + 2 * paddingY + lineHeight; // Extra line for title
        
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

// Helper to highlight an edge during traversal
void AVL::highlightEdge(AVLNode* from, AVLNode* to, Color color, int frames) {
    if (!from || !to) return;
    
    // Create map to track node indices
    std::map<AVLNode*, int> nodeToIndex;
    AVLTreeState currentState = captureCurrentState();
    
    // Build nodeToIndex map from current state
    for (size_t i = 0; i < currentState.nodes.size(); i++) {
        // Find the node with matching data (simplified approach)
        if (currentState.nodes[i].data == from->data)
            nodeToIndex[from] = i;
        if (currentState.nodes[i].data == to->data)
            nodeToIndex[to] = i;
    }
    
    // Create edge highlight
    EdgeHighlight highlight;
    highlight.sourceIndex = nodeToIndex[from];
    highlight.targetIndex = nodeToIndex[to];
    highlight.color = color;
    
    // Add the highlight to current state
    currentState.highlightedEdges.push_back(highlight);
    
    // Save multiple frames with this highlight for animation
    for (int i = 0; i < frames; i++) {
        animationStates.push_back(currentState);
    }
}

// Add pseudocode for inorder traversal
void AVL::setPseudoCodeInorder() {
    currentOperation = "Inorder Traversal";
    currentPseudoCode.clear();
    
    currentPseudoCode.push_back("if this is null");
    currentPseudoCode.push_back("  return");
    currentPseudoCode.push_back("");
    currentPseudoCode.push_back("Inorder(left)");
    currentPseudoCode.push_back("");
    currentPseudoCode.push_back("visit this");
    currentPseudoCode.push_back("");
    currentPseudoCode.push_back("Inorder(right)");
    
    currentHighlightedLine = 0;
}

// Add pseudocode for preorder traversal
void AVL::setPseudoCodePreorder() {
    currentOperation = "Preorder Traversal";
    currentPseudoCode.clear();
    
    currentPseudoCode.push_back("if this is null");
    currentPseudoCode.push_back("  return");
    currentPseudoCode.push_back("");
    currentPseudoCode.push_back("visit this");
    currentPseudoCode.push_back("");
    currentPseudoCode.push_back("Preorder(left)");
    currentPseudoCode.push_back("");
    currentPseudoCode.push_back("Preorder(right)");
    
    currentHighlightedLine = 0;
}

// Add pseudocode for postorder traversal
void AVL::setPseudoCodePostorder() {
    currentOperation = "Postorder Traversal";
    currentPseudoCode.clear();
    
    currentPseudoCode.push_back("if this is null");
    currentPseudoCode.push_back("  return");
    currentPseudoCode.push_back("");
    currentPseudoCode.push_back("Postorder(left)");
    currentPseudoCode.push_back("");
    currentPseudoCode.push_back("Postorder(right)");
    currentPseudoCode.push_back("");
    currentPseudoCode.push_back("visit this");
    
    currentHighlightedLine = 0;
}

// INORDER TRAVERSAL (Left-Root-Right)
void AVL::inorderTraversal() {
    clearAnimationStates();
    clearTraversalOrder(); // Clear previous traversal order
    
    // Set traversal type and pseudocode
    traversalType = "inorder";
    setPseudoCodeInorder();
    
    // Capture initial state
    animationStates.push_back(captureCurrentState());
    
    // Perform traversal
    inorderHelper(root);
    
    // Reset all colors
    resetColors(root);
    
    // Capture final state
    animationStates.push_back(captureCurrentState());
    
    // Add to history
    addStateToHistory();
}

// Update inorderHelper to properly highlight leaf nodes
void AVL::inorderHelper(AVLNode* node) {
    // Check if node is null
    updatePseudoCodeHighlight(0);
    animationStates.push_back(captureCurrentState());
    
    if (node == nullptr) {
        updatePseudoCodeHighlight(1); // Highlight the return line
        animationStates.push_back(captureCurrentState());
        return;
    }
    
    // Explicitly highlight this node to show we're visiting it
    AnimateHighlight(*this, node, HIGHLIGHT_TRAVERSE, 0.1f, 10);
    animationStates.push_back(captureCurrentState());
    
    // Visit left subtree
    updatePseudoCodeHighlight(3);
    animationStates.push_back(captureCurrentState());
    
    if (node->left) {
        // Highlight edge to left child
        highlightEdge(node, node->left, HIGHLIGHT_TRAVERSE, 15);
        inorderHelper(node->left);
        highlightEdge(node->left, node, HIGHLIGHT_TRAVERSE, 15);
    } else {
        // Explicitly show we're checking a null child (left)
        updatePseudoCodeHighlight(0); // "if this is null"
        animationStates.push_back(captureCurrentState());
        updatePseudoCodeHighlight(1); // "return"
        animationStates.push_back(captureCurrentState());
    }
    
    // Visit current node - add to traversal order
    traversalOrder.push_back(node->data);
    
    // Highlight the "visit this" line
    updatePseudoCodeHighlight(5);
    animationStates.push_back(captureCurrentState());
    
    // Animate highlighting the current node
    AnimateHighlight(*this, node, HIGHLIGHT_TRAVERSE, 0.1f, 20);
    
    // Visit right subtree
    updatePseudoCodeHighlight(7);
    animationStates.push_back(captureCurrentState());
    
    if (node->right) {
        // Highlight edge to right child
        highlightEdge(node, node->right, HIGHLIGHT_TRAVERSE, 15);
        inorderHelper(node->right);
        highlightEdge(node->right, node, HIGHLIGHT_TRAVERSE, 15);
    } else {
        // Explicitly show we're checking a null child (right)
        updatePseudoCodeHighlight(0); // "if this is null"
        animationStates.push_back(captureCurrentState());
        updatePseudoCodeHighlight(1); // "return"
        animationStates.push_back(captureCurrentState());
    }
    
    // Restore node color
    AnimateHighlight(*this, node, NODE_DEFAULT_COLOR, 0.1f, 10);
}

// PREORDER TRAVERSAL (Root-Left-Right)
void AVL::preorderTraversal() {
    clearAnimationStates();
    clearTraversalOrder(); // Clear previous traversal order
    
    // Set traversal type and pseudocode
    traversalType = "preorder";
    setPseudoCodePreorder();
    
    // Capture initial state
    animationStates.push_back(captureCurrentState());
    
    // Perform traversal
    preorderHelper(root);
    
    // Reset all colors
    resetColors(root);
    
    // Capture final state
    animationStates.push_back(captureCurrentState());
    
    // Add to history
    addStateToHistory();
}

// Update preorderHelper to properly highlight leaf nodes
void AVL::preorderHelper(AVLNode* node) {
    // Check if node is null
    updatePseudoCodeHighlight(0);
    animationStates.push_back(captureCurrentState());
    
    if (node == nullptr) {
        updatePseudoCodeHighlight(1); // Highlight the return line
        animationStates.push_back(captureCurrentState());
        return;
    }
    
    // Explicitly highlight this node to show we're visiting it
    AnimateHighlight(*this, node, HIGHLIGHT_TRAVERSE, 0.1f, 10);
    animationStates.push_back(captureCurrentState());
    
    // Visit current node - add to traversal order
    traversalOrder.push_back(node->data);
    
    // Highlight the "visit this" line
    updatePseudoCodeHighlight(3);
    animationStates.push_back(captureCurrentState());
    
    // Animate highlighting the current node
    AnimateHighlight(*this, node, HIGHLIGHT_TRAVERSE, 0.1f, 20);
    
    // Visit left subtree
    updatePseudoCodeHighlight(5);
    animationStates.push_back(captureCurrentState());
    
    if (node->left) {
        // Highlight edge to left child
        highlightEdge(node, node->left, HIGHLIGHT_TRAVERSE, 15);
        preorderHelper(node->left);
        highlightEdge(node->left, node, HIGHLIGHT_TRAVERSE, 15);
    } else {
        // Explicitly show we're checking a null child (left)
        updatePseudoCodeHighlight(0); // "if this is null"
        animationStates.push_back(captureCurrentState());
        updatePseudoCodeHighlight(1); // "return"
        animationStates.push_back(captureCurrentState());
    }
    
    // Visit right subtree
    updatePseudoCodeHighlight(7);
    animationStates.push_back(captureCurrentState());
    
    if (node->right) {
        // Highlight edge to right child
        highlightEdge(node, node->right, HIGHLIGHT_TRAVERSE, 15);
        preorderHelper(node->right);
        highlightEdge(node->right, node, HIGHLIGHT_TRAVERSE, 15);
    } else {
        // Explicitly show we're checking a null child (right)
        updatePseudoCodeHighlight(0); // "if this is null"
        animationStates.push_back(captureCurrentState());
        updatePseudoCodeHighlight(1); // "return"
        animationStates.push_back(captureCurrentState());
    }
    
    // Restore node color
    AnimateHighlight(*this, node, NODE_DEFAULT_COLOR, 0.1f, 10);
}

// POSTORDER TRAVERSAL (Left-Right-Root)
void AVL::postorderTraversal() {
    clearAnimationStates();
    clearTraversalOrder(); // Clear previous traversal order
    
    // Set traversal type and pseudocode
    traversalType = "postorder";
    setPseudoCodePostorder();
    
    // Capture initial state
    animationStates.push_back(captureCurrentState());
    
    // Perform traversal
    postorderHelper(root);
    
    // Reset all colors
    resetColors(root);
    
    // Capture final state
    animationStates.push_back(captureCurrentState());
    
    // Add to history
    addStateToHistory();
}

// Update postorderHelper to properly highlight leaf nodes
void AVL::postorderHelper(AVLNode* node) {
    // Check if node is null
    updatePseudoCodeHighlight(0);
    animationStates.push_back(captureCurrentState());
    
    if (node == nullptr) {
        updatePseudoCodeHighlight(1); // Highlight the return line
        animationStates.push_back(captureCurrentState());
        return;
    }
    
    // Explicitly highlight this node to show we're visiting it
    AnimateHighlight(*this, node, HIGHLIGHT_TRAVERSE, 0.1f, 10);
    animationStates.push_back(captureCurrentState());
    
    // Visit left subtree
    updatePseudoCodeHighlight(3);
    animationStates.push_back(captureCurrentState());
    
    if (node->left) {
        // Highlight edge to left child
        highlightEdge(node, node->left, HIGHLIGHT_TRAVERSE, 15);
        postorderHelper(node->left);
        highlightEdge(node->left, node, HIGHLIGHT_TRAVERSE, 15);
    } else {
        // Explicitly show we're checking a null child (left)
        updatePseudoCodeHighlight(0); // "if this is null"
        animationStates.push_back(captureCurrentState());
        updatePseudoCodeHighlight(1); // "return"
        animationStates.push_back(captureCurrentState());
    }
    
    // Visit right subtree
    updatePseudoCodeHighlight(5);
    animationStates.push_back(captureCurrentState());
    
    if (node->right) {
        // Highlight edge to right child
        highlightEdge(node, node->right, HIGHLIGHT_TRAVERSE, 15);
        postorderHelper(node->right);
        highlightEdge(node->right, node, HIGHLIGHT_TRAVERSE, 15);
    } else {
        // Explicitly show we're checking a null child (right)
        updatePseudoCodeHighlight(0); // "if this is null"
        animationStates.push_back(captureCurrentState());
        updatePseudoCodeHighlight(1); // "return"
        animationStates.push_back(captureCurrentState());
    }
    
    // Visit current node last - add to traversal order
    traversalOrder.push_back(node->data);
    
    // Highlight the "visit this" line
    updatePseudoCodeHighlight(7);
    animationStates.push_back(captureCurrentState());
    
    // Animate highlighting the current node
    AnimateHighlight(*this, node, HIGHLIGHT_TRAVERSE, 0.1f, 20);
    
    // Restore node color
    AnimateHighlight(*this, node, NODE_DEFAULT_COLOR, 0.1f, 10);
}

// Add current state to history
void AVL::addStateToHistory() {
    // Remove future states if we're not at the end of history
    if (currentHistoryPosition < historyStates.size() - 1) {
        historyStates.erase(historyStates.begin() + currentHistoryPosition + 1, 
                           historyStates.end());
    }
    
    // Add current state to history
    AVLTreeState currentState = captureCurrentState();
    historyStates.push_back(currentState);
    currentHistoryPosition = historyStates.size() - 1;
    
    // Debug output
    std::cout << "Added state to history. Current position: " << currentHistoryPosition 
              << ", Total states: " << historyStates.size() << std::endl;
}

// Undo operation
const AVLTreeState& AVL::undo() {
    if (canUndo()) {
        std::cout << "Performing undo. Current position: " << currentHistoryPosition << std::endl;
        currentHistoryPosition--;
        
        // Clear any ongoing animation
        animationStates.clear();
        currentAnimationStep = 0;
        
        // Clear any existing traversal order
        clearTraversalOrder();
        
        // Restore the previous state
        const AVLTreeState& prevState = historyStates[currentHistoryPosition];
        restoreFromState(prevState);
        
        // Restore traversal order if it exists in the history state
        traversalOrder = prevState.traversalOrder;
        
        std::cout << "Undo complete. New position: " << currentHistoryPosition << std::endl;
    } else {
        std::cout << "Cannot undo - at earliest state" << std::endl;
    }
    return getCurrentHistoryState();
}

// Redo operation
const AVLTreeState& AVL::redo() {
    if (canRedo()) {
        std::cout << "Performing redo. Current position: " << currentHistoryPosition << std::endl;
        currentHistoryPosition++;
        
        // Clear any ongoing animation
        animationStates.clear();
        currentAnimationStep = 0;
        
        // Restore the next state
        const AVLTreeState& nextState = historyStates[currentHistoryPosition];
        restoreFromState(nextState);
        
        std::cout << "Redo complete. New position: " << currentHistoryPosition << std::endl;
    } else {
        std::cout << "Cannot redo - at latest state" << std::endl;
    }
    return getCurrentHistoryState();
}

// Get current history state
const AVLTreeState& AVL::getCurrentHistoryState() const {
    if (historyStates.empty()) {
        static AVLTreeState emptyState;
        return emptyState;
    }
    return historyStates[currentHistoryPosition];
}

// Process operation animation
std::vector<AVLTreeState> AVL::processOperationAnimation() {
    std::vector<AVLTreeState> states;
    
    // Store initial state
    states.push_back(captureCurrentState());
    
    // Add final state to history after operation completes
    addStateToHistory();
    
    return states;
}

// Add pseudocode for insert operation
void AVL::setPseudoCodeInsert(int value) {
    currentOperation = "Insert";
    currentPseudoCode.clear();
    
    char valueLine[50];
    sprintf(valueLine, "insert %d", value);
    
    currentPseudoCode.push_back(valueLine);
    currentPseudoCode.push_back("check balance factor of this and its children");
    currentPseudoCode.push_back("");
    currentPseudoCode.push_back("  case1: this.rotateRight");
    currentPseudoCode.push_back("");
    currentPseudoCode.push_back("  case2: this.left.rotateLeft, this.rotateRight");
    currentPseudoCode.push_back("");
    currentPseudoCode.push_back("  case3: this.rotateLeft");
    currentPseudoCode.push_back("");
    currentPseudoCode.push_back("  case4: this.right.rotateRight, this.rotateLeft");
    currentPseudoCode.push_back("");
    currentPseudoCode.push_back("  this is balanced");
    
    currentHighlightedLine = 0;
}

// Update highlighted line
void AVL::updatePseudoCodeHighlight(int line) {
    currentHighlightedLine = line;
    
    // Capture current state with updated highlight
    animationStates.push_back(captureCurrentState());
}

// Helper function for tree deletion (locally scoped)
namespace {
    void deleteTree(AVLNode* node) {
        if (node == nullptr) return;
        
        deleteTree(node->left);
        deleteTree(node->right);
        delete node;
    }
}

// Clear the tree and make it empty
void AVL::createEmpty() {
    // Delete all nodes
    deleteTree(root);
    root = nullptr;
    foundNode = nullptr;
    
    // Clear animation states and reset variables
    clearAnimationStates();
    traversalOrder.clear();
    currentOperation = "Create Empty Tree";
    
    // Add to history
    addStateToHistory();
}

// Create a tree with random values
void AVL::createRandom(int size) {
    // First clear the tree
    createEmpty();
    
    // Set operation name
    currentOperation = "Create Random Tree";
    
    // Generate unique random values (to avoid duplicates)
    std::vector<int> values;
    while (values.size() < size) {
        int value = GetRandomValue(1, 99);
        // Check if value already exists
        if (std::find(values.begin(), values.end(), value) == values.end()) {
            values.push_back(value);
        }
    }
    
    // Add values one by one
    for (int value : values) {
        setPseudoCodeInsert(value);
        insert(value);
    }
}