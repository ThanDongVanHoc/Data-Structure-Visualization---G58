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
