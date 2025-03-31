#ifndef SHORTEST_PATH_H
#define SHORTEST_PATH_H

#include "raylib.h"
#include <vector>
#include <utility> // Để sử dụng std::pair
#include <bits/stdc++.h>
using namespace std;
struct Node
{
    Vector2 position;
    Color color;
    Vector2 velocity;
    string text;
    Node() : position({0, 0}), color(BLACK), velocity({0, 0}), text("INF") {}
};

struct Edge
{
    int start;
    int end;
    float weight; // Thêm thuộc tính trọng số
    Color color;
    Edge(int s, int e, float w) : start(s), end(e), weight(w), color(BLACK) {};
};
extern std::vector<std::vector<Edge>> adj; // Declare adj as a vector of vectors of Edge
float EuclideanDistance(Vector2 a, Vector2 b);
void InitializeGraph(std::vector<Node> &nodes, std::vector<Edge> &edges, int screenWidth, int screenHeight);
void UpdateGraph(std::vector<Node> &nodes, std::vector<Edge> &edges, int &selectedNode, float C_rep, float c_spring, float L, float timeStep, float damping, int physicsIterations, float nodeRadius, int screenWidth, int screenHeight);
void RenderGraph(const std::vector<Node> &nodes, const std::vector<Edge> &edges, int selectedNode, float nodeRadius);
void rendershortestpath(int screenWidth, int screenHeight);
void DrawBoxes_graph(Rectangle Box, const char *Buffer, int framesCounter, bool isActive);
void HandleInput_graph(Rectangle box, char *buffer, int &index, bool &isActive);
void drawArrowLine_graph(Vector2 start, Vector2 end, float thickness, Color color);
void DrawCenteredText(const std::string &text, Vector2 position, int fontSize, Color color);
#endif // SHORTEST_PATH_H