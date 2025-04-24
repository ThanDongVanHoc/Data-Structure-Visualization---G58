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
