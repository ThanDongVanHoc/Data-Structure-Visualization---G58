#ifndef SHORTEST_PATH_H
#define SHORTEST_PATH_H

#include "raylib.h"
#include <vector>
#include <utility> // Để sử dụng std::pair
#include <bits/stdc++.h>
using namespace std;

struct spnode
{
    Vector2 position;
    Color color;
    Vector2 velocity;
    string text;
    spnode() : position({0, 0}), color(BLACK), velocity({0, 0}), text("INF") {}
};
struct DrawAction
{
    std::string type; // Loại thao tác: "node", "edge", "text", "highlight", etc.
    int u, v;         // Các chỉ số liên quan (nút hoặc cạnh)
    Color color;
    std::string value; // Giá trị (nếu có)
    std::string text;  // Văn bản (nếu có)
    int lineNumber;    // Dòng code cần highlight (nếu type là "highlight")
};
struct spedge
{
    int start;
    int end;
    float weight; // Thêm thuộc tính trọng số
    Color color;
    spedge(int s, int e, float w) : start(s), end(e), weight(w), color(BLACK) {};
};
extern std::vector<std::vector<spedge>> adj; // Declare adj as a vector of vectors of spedge
float EuclideanDistance(Vector2 a, Vector2 b);
void InitializeGraph(std::vector<spnode> &spnodes, std::vector<spedge> &spedges, int screenWidth, int screenHeight);
void UpdateGraph(std::vector<spnode> &spnodes, std::vector<spedge> &spedges, int &selectedspnode, float C_rep, float c_spring, float L, float timeStep, float damping, int physicsIterations, float spnodeRadius, int screenWidth, int screenHeight);
void RenderGraph(const std::vector<spnode> &spnodes, const std::vector<spedge> &spedges, int selectedspnode, float spnodeRadius, bool isDirected);
void rendershortestpath(int screenWidth, int screenHeight);
void DrawBoxes_graph(Rectangle Box, const char *Buffer, int framesCounter, bool isActive);
void HandleInput_graph(Rectangle box, char *buffer, int &index, bool &isActive);
void drawArrowLine_graph(Vector2 start, Vector2 end, float thickness, Color color);
void DrawCenteredText(const std::string &text, Vector2 position, int fontSize, Color color);
bool CheckButton_graph(Rectangle button, const char *text);
void GenerateRandomConnectedGraph(std::vector<spnode> &spnodes, std::vector<spedge> &spedges, int maxNodes, int screenWidth, int screenHeight);
void HandleInputGraphUI(Rectangle inputGraphBox, bool &inputGraphActive, std::vector<spnode> &spnodes, std::vector<spedge> &spedges, int screenWidth, int screenHeight);
void ParseAdjMatrix(const std::vector<std::string> &lines, std::vector<spnode> &spnodes, std::vector<spedge> &spedges);
void ParseEdgeList(const std::vector<std::string> &lines, std::vector<spnode> &spnodes, std::vector<spedge> &spedges, bool &isDirected);
bool IsValidAdjMatrix(const std::vector<std::string> &lines);
bool IsValidEdgeList(const std::vector<std::string> &lines);
void DrawPseudoCode(const std::vector<std::string> &pseudoCode, int highlightedLine);
#endif // SHORTEST_PATH_H