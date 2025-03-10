// linked_list.h
#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <iostream>

struct Node {
    int data;
    Node* next;
    Node(int val) : data(val), next(nullptr) {}
};

struct LinkedList {
    Node* head;
    LinkedList();
    ~LinkedList();
    void insert(int value);
    void remove(int value);
    void display() const;
    void clear();
};

#endif // LINKED_LIST_H