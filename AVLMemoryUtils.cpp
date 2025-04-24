#include "AVL.h"

// Helper function to delete all nodes in an AVL tree
void deleteAllNodes(AVLNode* root) {
    if (!root) return;
    
    // Recursively delete children first
    deleteAllNodes(root->left);
    deleteAllNodes(root->right);
    
    // Then delete the current node
    delete root;
}

// Add proper node deletion to AVL::restoreFromState
void cleanupBeforeStateRestore(AVL& tree) {
    deleteAllNodes(tree.root);
    tree.root = nullptr;
}
