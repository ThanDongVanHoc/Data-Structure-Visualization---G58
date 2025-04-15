#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#undef RAYGUI_IMPLEMENTATION // Avoid including raygui implementation again

#define GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION
#include "gui_window_file_dialog.h"
#include "ShortestPath.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <bits/stdc++.h>
#include "var.h"
#include "input_box.h"

using namespace std;
const int MAX_INPUT_CHARS = 10;
std::vector<std::vector<spedge>> adj;
vector<vector<Color>> color_spedge;
float total_step = 120;
typedef pair<int, int> ii;
const double M_PI = 3.14159265358979323846;

void drawArrowLine_graph(Vector2 start, Vector2 end, float thickness, Color color)
{
    // Vẽ đường thẳng nối hai điểm
    DrawLineEx(start, end, thickness, color);
    // Tính vector chỉ phương từ start đến end
    Vector2 direction = {end.x - start.x, end.y - start.y};
    float len = sqrt(direction.x * direction.x + direction.y * direction.y);
    if (len == 0)
        return;
    direction.x /= len;
    direction.y /= len;

    float arrowLength = 10.0f; // Chiều dài của mũi tên
    float arrowWidth = 5.0f;   // Chiều rộng của mũi tên

    // Tạo vector vuông góc với hướng của đường thẳng
    Vector2 perp = {-direction.y, direction.x};

    // Tính các điểm của tam giác mũi tên
    Vector2 arrowTip = end;
    Vector2 arrowLeft = {end.x - direction.x * arrowLength + perp.x * arrowWidth,
                         end.y - direction.y * arrowLength + perp.y * arrowWidth};
    Vector2 arrowRight = {end.x - direction.x * arrowLength - perp.x * arrowWidth,
                          end.y - direction.y * arrowLength - perp.y * arrowWidth};

    // Vẽ tam giác đầy làm đầu mũi tên
    DrawTriangle(arrowTip, arrowRight, arrowLeft, color);
}

bool animationedge(int u, int v, const std::vector<spnode> &spnodes, std::vector<spedge> &spedges, Color color, int currentstep)
{

    Vector2 parentCenter = {spnodes[u].position.x, spnodes[u].position.y};
    Vector2 childCenter = {spnodes[v].position.x, spnodes[v].position.y};

    // Tính vector chỉ phương từ cha đến con
    Vector2 direction = {childCenter.x - parentCenter.x, childCenter.y - parentCenter.y};
    float len = sqrt(direction.x * direction.x + direction.y * direction.y);
    if (len != 0)
    {
        direction.x /= len;
        direction.y /= len;
    }
    float spnodeRadius = 20.0f;
    // Tính điểm bắt đầu và kết thúc: cách tâm một khoảng bằng bán kính
    Vector2 startspedge = {parentCenter.x + direction.x * spnodeRadius, parentCenter.y + direction.y * spnodeRadius};
    Vector2 endspedge = {childCenter.x - direction.x * spnodeRadius, childCenter.y - direction.y * spnodeRadius};
    int thickness = 4.0f;
    float sus = 1.0f * (1 - 0.1f) / total_step;
    float step = 0.1f + sus * currentstep;
    if (step > 1.0f)
        return true;
    Vector2 direct = {startspedge.x + double((endspedge.x - startspedge.x) * step), startspedge.y + double((endspedge.y - startspedge.y) * step)};
    int selectedspnode = -1;
    DrawLineEx(startspedge, direct, thickness, color);
    // EndDrawing();
    return false;
}
/*
void animate(std::vector<spnode> &spnodes, std::vector<spedge> &spedges)
{
    float spnodeRadius = 20.0f;
    int selectedspnode = -1;
    RenderGraph(spnodes, spedges, selectedspnode, spnodeRadius);
    EndDrawing();
    BeginDrawing();
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Delay for animation
    // ClearBackground(RAYWHITE);
}*/
void Dijkstra(int start, vector<spnode> &spnodes, std::vector<spedge> &spedges, std::vector<DrawAction> &drawActions)
{
    std::vector<int> previous;
    std::vector<float> distances;
    previous.resize(spnodes.size() + 1);
    distances.resize(spnodes.size() + 1);
    int n = spnodes.size();
    long long INF = 1e9 + 7;
    distances.assign(n, INF);
    previous.assign(n, -1);
    distances[start] = 0;
    priority_queue<ii, vector<ii>, greater<ii>> pq;
    pq.push({0, start});
    spnodes[start].text = "source,0";
    for (int i = 0; i <= 20; i++)
        drawActions.push_back({"highlight", -1, -1, WHITE, "", "", 0}); // Highlight dòng 2
    while (!pq.empty())
    {
        for (int i = 0; i <= 20; i++)
            drawActions.push_back({"highlight", -1, -1, WHITE, "", "", 1}); // Highlight dòng 3
        auto k = pq.top();
        int currentDist = k.first;
        int u = k.second;
        pq.pop();

        if (currentDist > distances[u])
            continue;
        spnodes[u].color = RED;
        drawActions.push_back({"nodes", u, -1, spnodes[u].color, spnodes[u].text, "source", -1});
        for (int i = 0; i <= 20; i++)
            drawActions.push_back({"highlight", -1, -1, WHITE, "", "", 2});
        // animate(spnodes, spedges);
        // drawActions.push_back({"graph", -1, -1, RAYWHITE, "", ""});
        for (const auto &spedge : adj[u])
        {
            for (int i = 0; i <= 20; i++)
                drawActions.push_back({"highlight", -1, -1, WHITE, "", "", 3}); // Highlight dòng 6
            int v = spedge.end;
            if (v == u)
                v = spedge.start;
            int weight = spedge.weight;
            if (v == previous[u])
                continue;
            if (distances[u] + weight < distances[v])
            {
                for (int i = 0; i <= 20; i++)
                    drawActions.push_back({"highlight", -1, -1, WHITE, "", "", 4}); // Highlight dòng 7
                drawActions.push_back({"highlight", -1, -1, WHITE, "", "", 5});     // Highlight dòng 7
                for (int step = 0; step <= 120; step++)
                {
                    string text = std::to_string(int(step));
                    drawActions.push_back({"edges", u, v, YELLOW, "", text, -1});
                }
                color_spedge[u][v] = ORANGE;
                color_spedge[v][u] = ORANGE;

                drawActions.push_back({"coloredges", u, v, ORANGE, "", "", -1});
                spnodes[v].text = std::to_string(int(distances[u] + weight));
                drawActions.push_back({"nodes", v, -1, spnodes[v].color, spnodes[v].text, "", -1});

                if (previous[v] != -1)
                {
                    color_spedge[previous[v]][v] = GRAY;
                    color_spedge[v][previous[v]] = GRAY;
                    drawActions.push_back({"coloredges", previous[v], v, GRAY, "", "", -1});
                }
                spnodes[v].color = ORANGE;
                drawActions.push_back({"nodes", v, -1, spnodes[v].color, spnodes[v].text, "", -1});
                // animate(spnodes, spedges);
                // drawActions.push_back({"graph", -1, -1, RAYWHITE, "", ""});
                for (int i = 0; i <= 20; i++)
                    drawActions.push_back({"highlight", -1, -1, WHITE, "", "", 6});
                distances[v] = distances[u] + weight;
                previous[v] = u;
                pq.push({distances[v], v});
            }
            else if (distances[u] + weight > distances[v])
            {
                // animationedge(u, v, spnodes, spedges, YELLOW);
                for (int step = 0; step <= 120; step++)
                {
                    string text = std::to_string(int(step));
                    drawActions.push_back({"edges", u, v, YELLOW, "", text, -1});
                }
                color_spedge[u][v] = GRAY;
                color_spedge[v][u] = GRAY;
                drawActions.push_back({"coloredges", u, v, GRAY, "", "", -1});
                // animate(spnodes, spedges);
                // drawActions.push_back({"graph", -1, -1, RAYWHITE, "", ""});
            }
        }
    }
    for (int u = 0; u < int(spnodes.size()); u++)
    {
        drawActions.push_back({"highlight", -1, -1, WHITE, "", "", -1});
        if (distances[u] == INF)
        {
            for (auto &spedge : adj[u])
            {
                int v = spedge.end;
                if (v == u)
                    v = spedge.start;
                // animationedge(u, v, spnodes, spedges, YELLOW);
                for (int step = 0; step <= 120; step++)
                {
                    string text = std::to_string(int(step));
                    drawActions.push_back({"edges", u, v, YELLOW, "", text, -1});
                }
                color_spedge[spedge.start][spedge.end] = GRAY;
                color_spedge[spedge.end][spedge.start] = GRAY;
                drawActions.push_back({"coloredges", spedge.start, spedge.end, GRAY, "", "", -1});
                // animate(spnodes, spedges);
                // drawActions.push_back({"graph", -1, -1, RAYWHITE, "", ""});
            }
            spnodes[u].color = RED;
            drawActions.push_back({"nodes", u, -1, spnodes[u].color, spnodes[u].text, "", -1});
        }
    }
}
void resetcolor(std::vector<spnode> &spnodes)
{

    for (int i = 0; i < int(spnodes.size()); i++)
    {
        spnodes[i].color = BLACK;
        spnodes[i].text = "INF";
    }
    for (int i = 0; i < int(spnodes.size()); i++)
    {
        for (auto &spedge : adj[i])
        {
            color_spedge[spedge.start][spedge.end] = BLACK;
            color_spedge[spedge.end][spedge.start] = BLACK;
        }
    }
}
float EuclideanDistance(Vector2 a, Vector2 b)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return sqrtf(dx * dx + dy * dy);
}
void InitializeGraph(std::vector<spnode> &spnodes, std::vector<spedge> &spedges, int screenWidth, int screenHeight)
{
    const int numspnodes = 9;
    srand(static_cast<unsigned>(time(nullptr)));
    spnodes.resize(numspnodes);
    color_spedge.resize(numspnodes);
    for (int i = 0; i < numspnodes; i++)
    {
        color_spedge[i].resize(numspnodes, BLACK);
    }
    for (int i = 0; i < numspnodes; i++)
    {
        spnodes[i].position = {static_cast<float>(rand() % screenWidth), static_cast<float>(rand() % screenHeight)};
        spnodes[i].velocity = {0, 0};
    }
    spedges.push_back(spedge(0, 1, 8));
    spedges.push_back(spedge(0, 2, 12));
    spedges.push_back(spedge(1, 2, 13));
    spedges.push_back(spedge(1, 3, 25));
    spedges.push_back(spedge(2, 3, 14));
    spedges.push_back(spedge(2, 6, 21));
    spedges.push_back(spedge(3, 4, 20));
    spedges.push_back(spedge(3, 5, 8));
    spedges.push_back(spedge(3, 6, 12));
    spedges.push_back(spedge(3, 7, 12));
    spedges.push_back(spedge(3, 8, 16));
    spedges.push_back(spedge(4, 1, 9));
    spedges.push_back(spedge(4, 5, 19));
    spedges.push_back(spedge(5, 7, 11));
    spedges.push_back(spedge(6, 8, 11));
    spedges.push_back(spedge(7, 8, 9));
    adj.resize(spnodes.size());

    // Duyệt qua các cạnh và thêm vào danh sách kề
    for (const auto &spedge : spedges)
    {
        adj[spedge.start].push_back(spedge);
    }
}

void GenerateRandomConnectedGraph(std::vector<spnode> &spnodes, std::vector<spedge> &spedges, int maxNodes, int screenWidth, int screenHeight)
{
    srand(static_cast<unsigned>(time(nullptr)));

    // Số đỉnh ngẫu nhiên từ 2 đến maxNodes
    int numNodes = maxNodes;
    spedges.clear();
    spnodes.resize(numNodes);
    color_spedge.clear();
    color_spedge.resize(numNodes);
    adj.clear();
    adj.resize(numNodes);

    // Tính toán bán kính và tâm của vòng tròn
    float centerX = screenWidth / 2.0f;
    float centerY = screenHeight / 2.0f;
    float radius = std::min(screenWidth, screenHeight) / 3.0f; // Bán kính vòng tròn (1/3 kích thước nhỏ hơn của màn hình)

    // Gán vị trí cho các đỉnh theo vòng tròn
    for (int i = 0; i < numNodes; i++)
    {
        float angle = 2.0f * M_PI * i / numNodes; // Góc chia đều
        spnodes[i].position = {
            centerX + radius * cos(angle), // Tọa độ x
            centerY + radius * sin(angle)  // Tọa độ y
        };
        spnodes[i].velocity = {0, 0};
        spnodes[i].color = BLACK;
        spnodes[i].text = "INF";
    }

    // Tạo cây khung tối thiểu để đảm bảo đồ thị liên thông
    std::vector<bool> visited(numNodes, false);
    std::vector<int> nodesInTree;
    nodesInTree.push_back(0);
    visited[0] = true;

    while (nodesInTree.size() < numNodes)
    {
        int u = nodesInTree[rand() % nodesInTree.size()];
        int v = rand() % numNodes;
        if (!visited[v])
        {
            float weight = static_cast<float>(rand() % 20 + 1); // Trọng số ngẫu nhiên từ 1 đến 20
            spedges.push_back(spedge(u, v, weight));
            adj[u].push_back(spedge(u, v, weight));
            visited[v] = true;
            nodesInTree.push_back(v);
        }
    }

    for (int i = 0; i < numNodes; i++)
    {
        color_spedge[i].resize(numNodes, BLACK);
    }

    // Thêm một số cạnh ngẫu nhiên để tăng độ phức tạp
    int extraEdges = rand() % (numNodes * 2); // Tăng số lượng cạnh ngẫu nhiên
    for (int i = 0; i < extraEdges; i++)
    {
        int u = rand() % numNodes;
        int v = rand() % numNodes;
        if (u != v)
        {
            // Kiểm tra xem cạnh đã tồn tại hay chưa
            bool edgeExists = false;
            for (const auto &edge : adj[u])
            {
                if (edge.end == v)
                {
                    edgeExists = true;
                    break;
                }
            }
            if (!edgeExists)
            {
                float weight = static_cast<float>(rand() % 20 + 1);
                spedges.push_back(spedge(u, v, weight));
                adj[u].push_back(spedge(u, v, weight));
            }
        }
    }
}
void fix_updategraph(std::vector<spnode> &spnodes, std::vector<spedge> &spedges, int &selectedspnode, float C_rep, float c_spring, float L, float timeStep, float damping, int physicsIterations, float spnodeRadius, int screenWidth, int screenHeight)
{
    Vector2 mousePos = GetMousePosition();

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        for (int i = 0; i < int(spnodes.size()); i++)
        {
            if (EuclideanDistance(mousePos, spnodes[i].position) <= spnodeRadius * 1.5f)
            {
                selectedspnode = i;
                break;
            }
        }
    }

    if (selectedspnode != -1 && IsMouseButtonDown(MOUSE_LEFT_BUTTON))
    {
        spnodes[selectedspnode].position = mousePos;
        spnodes[selectedspnode].velocity = {0, 0};
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
    {
        selectedspnode = -1;
    }
}
void UpdateGraph(std::vector<spnode> &spnodes, std::vector<spedge> &spedges, int &selectedspnode, float C_rep, float c_spring, float L, float timeStep, float damping, int physicsIterations, float spnodeRadius, int screenWidth, int screenHeight)
{
    Vector2 mousePos = GetMousePosition();

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        for (int i = 0; i < int(spnodes.size()); i++)
        {
            if (EuclideanDistance(mousePos, spnodes[i].position) <= spnodeRadius * 1.5f)
            {
                selectedspnode = i;
                break;
            }
        }
    }

    if (selectedspnode != -1 && IsMouseButtonDown(MOUSE_LEFT_BUTTON))
    {
        spnodes[selectedspnode].position = mousePos;
        spnodes[selectedspnode].velocity = {0, 0};
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
    {
        selectedspnode = -1;
    }

    for (int iter = 0; iter < physicsIterations; iter++)
    {
        std::vector<Vector2> forces(spnodes.size(), {0, 0});

        // Tính lực đẩy giữa các spnode
        for (int i = 0; i < int(spnodes.size()); i++)
        {
            for (int j = i + 1; j < int(spnodes.size()); j++)
            {
                Vector2 delta = {spnodes[i].position.x - spnodes[j].position.x,
                                 spnodes[i].position.y - spnodes[j].position.y};
                float dist = EuclideanDistance(spnodes[i].position, spnodes[j].position);
                if (dist < 0.1f)
                    dist = 0.1f;
                float rep = C_rep / (dist * dist);
                Vector2 dir = {delta.x / dist, delta.y / dist};
                Vector2 forceRep = {dir.x * rep, dir.y * rep};

                forces[i].x += forceRep.x;
                forces[i].y += forceRep.y;
                forces[j].x -= forceRep.x;
                forces[j].y -= forceRep.y;
            }
        }

        // Tính lực hút trên các cạnh
        for (auto &spedge : spedges)
        {
            int u = spedge.start;
            int v = spedge.end;
            Vector2 delta = {spnodes[u].position.x - spnodes[v].position.x,
                             spnodes[u].position.y - spnodes[v].position.y};
            float dist = EuclideanDistance(spnodes[u].position, spnodes[v].position);
            if (dist < 0.1f)
                dist = 0.1f;
            float disp = dist - L;
            float att = c_spring * disp;
            Vector2 dir = {delta.x / dist, delta.y / dist};
            Vector2 forceAtt = {dir.x * att, dir.y * att};

            forces[u].x -= forceAtt.x;
            forces[u].y -= forceAtt.y;
            forces[v].x += forceAtt.x;
            forces[v].y += forceAtt.y;
        }

        for (int i = 0; i < int(spnodes.size()); i++)
        {
            if (i == selectedspnode)
                continue;
            spnodes[i].velocity.x = (spnodes[i].velocity.x + forces[i].x * timeStep) * damping;
            spnodes[i].velocity.y = (spnodes[i].velocity.y + forces[i].y * timeStep) * damping;
            spnodes[i].position.x += spnodes[i].velocity.x * timeStep;
            spnodes[i].position.y += spnodes[i].velocity.y * timeStep;

            if (spnodes[i].position.x < spnodeRadius)
                spnodes[i].position.x = spnodeRadius;
            if (spnodes[i].position.y < spnodeRadius)
                spnodes[i].position.y = spnodeRadius;
            if (spnodes[i].position.x > screenWidth - spnodeRadius)
                spnodes[i].position.x = screenWidth - spnodeRadius;
            if (spnodes[i].position.y > screenHeight - spnodeRadius)
                spnodes[i].position.y = screenHeight - spnodeRadius;
        }
    }
}
void DrawCenteredText(const std::string &text, Vector2 position, int fontSize, Color color)
{
    // Tính chiều rộng của chuỗi
    float textWidth = MeasureText(text.c_str(), GetFont().baseSize);

    // Điều chỉnh tọa độ x để căn giữa
    float centeredX = position.x - textWidth / 2.0f;

    // Vẽ chuỗi tại vị trí đã điều chỉnh
    // DrawText(text.c_str(), (centeredX), (position.y), fontSize, color);
    DrawTextEx(GetFont(), text.c_str(), {float(centeredX), float(position.y)}, GetFont().baseSize, 1, color);
}
void RenderGraph(const std::vector<spnode> &spnodes, const std::vector<spedge> &spedges, int selectedspnode, float spnodeRadius, bool isDirected)
{
    DrawCenteredText("SHORTEST PATH", {float(GetScreenWidth() / 2.0f), 50}, GetFont().baseSize * 2, BLACK);
    for (int i = 0; i < int(spnodes.size()); i++)
    {
        for (auto spedge : adj[i])
        {
            float spedgeLength = EuclideanDistance(spnodes[spedge.start].position, spnodes[spedge.end].position);

            Vector2 parentCenter = {spnodes[spedge.start].position.x, spnodes[spedge.start].position.y};
            Vector2 childCenter = {spnodes[spedge.end].position.x, spnodes[spedge.end].position.y};

            // Tính vector chỉ phương từ cha đến con
            Vector2 direction = {childCenter.x - parentCenter.x, childCenter.y - parentCenter.y};
            float len = sqrt(direction.x * direction.x + direction.y * direction.y);
            if (len != 0)
            {
                direction.x /= len;
                direction.y /= len;
            }

            // Tính điểm bắt đầu và kết thúc: cách tâm một khoảng bằng bán kính
            Vector2 startspedge = {parentCenter.x + direction.x * spnodeRadius, parentCenter.y + direction.y * spnodeRadius};
            Vector2 endspedge = {childCenter.x - direction.x * spnodeRadius, childCenter.y - direction.y * spnodeRadius};

            if (isDirected)
            {
                // Vẽ mũi tên nếu đồ thị có hướng
                drawArrowLine_graph(startspedge, endspedge, 4.0f, color_spedge[spedge.start][spedge.end]);
            }
            else
            {
                // Vẽ đường thẳng nếu đồ thị vô hướng
                DrawLineEx(startspedge, endspedge, 4.0f, color_spedge[spedge.start][spedge.end]);
            }

            // Tính trung điểm của cạnh
            Vector2 midPoint = {1.0 * (startspedge.x + endspedge.x) / 2.0f,
                                1.0 * (startspedge.y + endspedge.y) / 2.0f};

            // Tính vector nối từ spnode u đến spnode v
            Vector2 delta = {endspedge.x - startspedge.x,
                             endspedge.y - startspedge.y};
            float dist = sqrtf(1.0 * delta.x * delta.x + 1.0 * delta.y * delta.y);
            if (dist < 0.01f)
                dist = 0.01f; // tránh chia cho 0

            // Tính vector pháp tuyến (normalized)
            Vector2 normal = {-delta.y / dist, delta.x / dist};

            // Chọn offset (khoảng cách dịch chuyển từ trung điểm)
            float offset = GetFont().baseSize * 0.75 - 5; // bạn có thể điều chỉnh giá trị này
            Vector2 labelPos = {midPoint.x + normal.x * offset, midPoint.y + normal.y * offset};

            // Hiển thị trọng số tại vị trí labelPos
            std::string weightStr = std::to_string(static_cast<int>(spedge.weight));
            int tw = MeasureText(weightStr.c_str(), GetFont().baseSize);
            DrawTextEx(GetFont(), weightStr.c_str(), {labelPos.x - float(1.0 * tw / 2.0), labelPos.y - float(1.0 * GetFont().baseSize / 2.0)}, GetFont().baseSize, 1, color_spedge[spedge.start][spedge.end]);
        }
    }

    for (int i = 0; i < int(spnodes.size()); i++)
    {
        Color color = (i == selectedspnode) ? BLUE : spnodes[i].color;
        for (double thickness = 0.0; thickness < 3.0; thickness += 0.5)
            DrawCircleLinesV(spnodes[i].position, 20.0f + thickness, color);
        std::string text = std::to_string(i);
        int valueTextWidth = MeasureText(text.c_str(), GetFont().baseSize);
        DrawTextEx(GetFont(), text.c_str(), {spnodes[i].position.x - float(1.0 * valueTextWidth / 2.0), spnodes[i].position.y - float(1.0 * GetFont().baseSize / 2.0)}, GetFont().baseSize, 1, color);
        DrawCenteredText(spnodes[i].text, {spnodes[i].position.x, spnodes[i].position.y + 23}, GetFont().baseSize, RED);
    }
}
void rendershortestpath(int screenWidth, int screenHeight)
{
    BeginDrawing();
    ClearBackground(RAYWHITE);
    static float spnodeRadius = 20.0f;
    static int frameCount = 0;
    // ---- THÔNG SỐ ĐÃ ĐIỀU CHỈNH ----
    static float C_rep = 1000000.0f; // Lực đẩy (càng lớn, các spnode càng đẩy mạnh hơn)
    static float c_spring = 10.0f;   // Lực hút (càng lớn, các spnode càng hút mạnh hơn)
    static float L = 225.0f;         // Độ dài lò xo lý tưởng

    static float timeStep = 0.05f;    // Bước thời gian
    static float damping = 0.7f;      // Giảm damping để spnode di chuyển linh hoạt hơn
    static int physicsIterations = 5; // Số lần cập nhật vật lý mỗi khung hình
    static std::vector<spnode> spnodes;
    static std::vector<spedge> spedges;
    static int selectedspnode = -1;
    static Rectangle dijkstra_box = {300, 850, 100, 50};
    static bool dijkstraActive = false;
    static char dijkstraBuffer[MAX_INPUT_CHARS + 1] = {0}; // Buffer
    static int dijkstraIndex = 0;
    static std::vector<DrawAction> drawActions; // Index for input buffer
    static int stepdraw = 0;
    static bool pause = false;
    static int currentstep = 0;
    static bool fixed = false;
    static Rectangle pause_box = {380, 950, 40, 40};
    static Rectangle next_step_box = {440, 950, 40, 40};
    static Rectangle prev_step_box = {320, 950, 40, 40};
    static Rectangle go_to_begin = {260, 950, 40, 40};
    static Rectangle go_to_end = {500, 950, 40, 40};
    static Rectangle menu = {0, 600, 40, 300};
    static bool menuActive = false;

    // Seekbar variables
    static float sliderValue = 0.0f; // Normalized value (0.0 to 1.0)
    const int barWidth = 800;
    const int barHeight = 10;
    const int barX = 560;
    const int barY = 970;
    const int knobRadius = 10;
    // speed bar variables
    const int speedBarWidth = 200;
    const int speedBarHeight = 10;
    const int speedBarX = 1500;
    const int speedBarY = 970;
    const int speedKnobRadius = 10;
    static float speedSliderValue = 0.0f; // Normalized value (0.0 to 1.0)
    static float dijkstra_total_step = 120.0f;

    static Rectangle inputGraphBox = {300, 600, 300, 250};
    static bool inputGraphActive = false;
    static bool isDirected = true; // Biến trạng thái: true = có hướng, false = vô hướng
    static Rectangle fixed_box = {50, 600, 200, 50};
    static bool type_input = false;
    static GuiWindowFileDialogState fileDialogState; // Trạng thái của file dialog
    static bool fileDialogInitialized = false;       // Để kiểm tra xem dialog đã được khởi tạo chưa
    static bool dijkstra_query = false;
    static Rectangle randomGraphBox = {300, 800, 100, 50};
    static bool randomGraphActive = false;
    static char randomGraphBuffer[MAX_INPUT_CHARS + 1] = {0}; // Buffer for input
    static int randomGraphIndex = 0;
    static bool random_query = false;

    static Rectangle pseudoCodeBox = {1880, 600, 40, 300};
    static bool pseudoCodeActive = false;
    static std::vector<std::string> pseudoCode = {
        "1. Initialize distances and priority queue",
        "2. While priority queue is not empty:",
        "3.    Extract the node with the smallest distance",
        "4.    For each neighbor of the node:",
        "5.        If a shorter path is found:",
        "6.            Update the distance",
        "7.            Push the neighbor into the queue"};

    static int highlightedLine = -1;
    if (CheckButton_graph(pseudoCodeBox, "PSEUDO CODE"))
    {
        pseudoCodeActive = !pseudoCodeActive;
    }
    if (pseudoCodeActive)
    {
        DrawRectangleGradientV(pseudoCodeBox.x, pseudoCodeBox.y, pseudoCodeBox.width, pseudoCodeBox.height, SKYBLUE, DARKBLUE);
        static Texture2D codetexture = LoadTexture("res/right arrow.png");
        Texture2D *texture = &codetexture;
        Rectangle sourceRect = {0, 0, (float)texture->width, (float)texture->height};
        Rectangle destRect = {1880, 730, 40, 40};
        DrawTexturePro(*texture, sourceRect, destRect, {0, 0}, 0.0f, WHITE);
        DrawPseudoCode(pseudoCode, highlightedLine);
    }
    else
    {
        DrawRectangleGradientV(pseudoCodeBox.x, pseudoCodeBox.y, pseudoCodeBox.width, pseudoCodeBox.height, SKYBLUE, DARKBLUE);
        static Texture2D codetexture = LoadTexture("res/left arrow.png");
        Texture2D *texture = &codetexture;
        Rectangle sourceRect = {0, 0, (float)texture->width, (float)texture->height};
        Rectangle destRect = {1880, 730, 40, 40};
        DrawTexturePro(*texture, sourceRect, destRect, {0, 0}, 0.0f, WHITE);
    }
    if (!fileDialogInitialized)
    {
        fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
        fileDialogInitialized = true;
    }
    if (CheckButton_graph(menu, "menu"))
    {
        menuActive = !menuActive;
    }
    if (menuActive)
    {
        DrawRectangleGradientV(menu.x, menu.y, menu.width, menu.height, SKYBLUE, DARKBLUE);
        static Texture2D menutexture = LoadTexture("res/left arrow.png");
        Texture2D *texture = &menutexture;
        Rectangle sourceRect = {0, 0, (float)texture->width, (float)texture->height};
        Rectangle destRect = {0, 730, 40, 40};
        DrawTexturePro(*texture, sourceRect, destRect, {0, 0}, 0.0f, WHITE);
        if ((!fixed && CheckButton_graph(fixed_box, "FIX")) ||
            (fixed && CheckButton_graph(fixed_box, "UNFIX")))
        {
            // Toggle fixed state
            fixed = !fixed;
        }
        if (CheckButton_graph({50, 650, 200, 50}, isDirected ? "Undirected" : "Directed"))
        {
            isDirected = !isDirected; // Đổi trạng thái

            // Cập nhật danh sách kề
            adj.clear();
            adj.resize(spnodes.size());
            for (const auto &spedge : spedges)
            {
                adj[spedge.start].push_back(spedge);
                if (!isDirected)
                {
                    // Nếu là đồ thị vô hướng, thêm cạnh ngược
                    adj[spedge.end].push_back(spedge);
                }
            }

            // Chạy lại Dijkstra
            if (!spnodes.empty())
            {
                drawActions.clear();
                resetcolor(spnodes);
                stepdraw = 0;
            }
        }
        static bool inputGraphActive = false;
        if (CheckButton_graph({50, 700, 200, 50}, "Input Graph"))
        {
            inputGraphActive = !inputGraphActive;
        }
        if (inputGraphActive)
        {
            vector<string> inputGraphBuffer;
            inputGraphBuffer = HandleInputGraphUI(inputGraphBox, inputGraphActive, screenWidth, screenHeight);
            if ((!type_input && CheckButton_graph({300, 850, 150, 50}, "Edge List")) || (type_input && CheckButton_graph({300, 850, 150, 50}, "Adj Matrix")))
            {
                type_input = !type_input;
            }
            if (CheckButton_graph({450, 850, 150, 50}, "Submit"))
            {
                inputGraphActive = false;
                if (type_input)
                {
                    ParseAdjMatrix(inputGraphBuffer, spnodes, spedges);
                    drawActions.clear();
                    resetcolor(spnodes);
                    stepdraw = 0;
                }
                else
                {
                    ParseEdgeList(inputGraphBuffer, spnodes, spedges, isDirected);
                    drawActions.clear();
                    resetcolor(spnodes);
                    stepdraw = 0;
                }
            }
        }
        if (CheckButton_graph({50, 750, 200, 50}, "File"))
        {
            fileDialogState.windowActive = !fileDialogState.windowActive; // Kích hoạt file dialog
        }
        if (CheckButton_graph({50, 850, 200, 50}, "Dijkstra"))
        {
            dijkstra_query = !dijkstra_query;
        }
        if (dijkstra_query)
        {
            DrawTextEx(GetFont(), "S=", {dijkstra_box.x - 30, dijkstra_box.y + 10}, GetFont().baseSize, 1, BLACK);
            DrawBoxes_graph(dijkstra_box, dijkstraBuffer, frameCount, dijkstraActive);
            HandleInput_graph(dijkstra_box, dijkstraBuffer, dijkstraIndex, dijkstraActive);
            if (IsKeyPressed(KEY_ENTER))
            {
                cout << "Enter pressed" << endl;
                if (dijkstraBuffer[0] != '\0')
                {
                    int startspnode = std::stoi(dijkstraBuffer); // Convert string to integer
                    if (startspnode >= 0 && startspnode < int(spnodes.size()))
                    {
                        drawActions.clear();
                        resetcolor(spnodes);
                        Dijkstra(startspnode, spnodes, spedges, drawActions);
                        resetcolor(spnodes);
                        // dijkstra_total_step = total_step;
                        stepdraw = 0;
                    }
                    else
                    {
                        std::cout << "Invalid spnode index!" << std::endl;
                    }
                    dijkstraIndex = 0;
                    dijkstraBuffer[0] = '\0'; // Reset the buffer
                }
            }
        }
        if (CheckButton_graph({50, 800, 200, 50}, "Random"))
        {
            random_query = !random_query;
        }

        if (random_query)
        {
            DrawTextEx(GetFont(), "N=", {randomGraphBox.x - 30, randomGraphBox.y + 10}, GetFont().baseSize, 1, BLACK);
            DrawBoxes_graph(randomGraphBox, randomGraphBuffer, frameCount, randomGraphActive);
            HandleInput_graph(randomGraphBox, randomGraphBuffer, randomGraphIndex, randomGraphActive);

            if (IsKeyPressed(KEY_ENTER))
            {
                if (randomGraphBuffer[0] != '\0')
                {
                    int numNodes = std::stoi(randomGraphBuffer); // Convert input to integer
                    if (numNodes > 0)
                    {
                        GenerateRandomConnectedGraph(spnodes, spedges, numNodes, screenWidth, screenHeight);
                        drawActions.clear();
                        resetcolor(spnodes);
                        stepdraw = 0;
                    }
                    else
                    {
                        std::cout << "Invalid number of nodes!" << std::endl;
                    }
                    randomGraphIndex = 0;
                    randomGraphBuffer[0] = '\0'; // Reset the buffer
                }
            }
        }
    }
    else
    {
        DrawRectangleGradientV(menu.x, menu.y, menu.width, menu.height, SKYBLUE, DARKBLUE);
        static Texture2D menutexture = LoadTexture("res/right arrow.png");
        Texture2D *texture = &menutexture;
        Rectangle sourceRect = {0, 0, (float)texture->width, (float)texture->height};
        Rectangle destRect = {0, 730, 40, 40};
        DrawTexturePro(*texture, sourceRect, destRect, {0, 0}, 0.0f, WHITE);
    }
    if (!frameCount)
        InitializeGraph(spnodes, spedges, screenWidth, screenHeight);

    if (!(stepdraw < drawActions.size() - 1) && CheckButton_graph(pause_box,
                                                                  "replay"))
    {
        pause = false;
        stepdraw = 0;
        resetcolor(spnodes);
    }
    else
    {
        if ((!pause && stepdraw < drawActions.size() - 1 && CheckButton_graph(pause_box, "pause")) ||
            (pause && stepdraw < drawActions.size() - 1 && CheckButton_graph(pause_box, "play")))
        {
            // Toggle pause state
            pause = !pause;
        }
    }
    if (CheckButton_graph(go_to_begin, "go to begin") && stepdraw > 0)
    {
        stepdraw = 0;
        resetcolor(spnodes);
    }
    if (CheckButton_graph(go_to_end, "go to end") && stepdraw < int(drawActions.size()) - 1)
    {
        stepdraw = int(drawActions.size()) - 1;
        resetcolor(spnodes);
        for (int i = 0; i <= stepdraw; i++)
        {
            DrawAction drawAction = drawActions[i];
            switch (drawAction.type[0]) // Use the first character of the type string for switch
            {
            case 'n': // "nodes"
                spnodes[drawAction.u].color = drawAction.color;
                spnodes[drawAction.u].text = drawAction.value;
                break;
            case 'c': // "coloredges"
                color_spedge[drawAction.u][drawAction.v] = drawAction.color;
                color_spedge[drawAction.v][drawAction.u] = drawAction.color;
                break;
            case 'h':
                highlightedLine = drawAction.lineNumber;
                break;
            default:
                break;
            }
        }
    }
    // Seekbar logic
    Vector2 mousePoint = GetMousePosition();
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
    {
        if (mousePoint.x >= barX && mousePoint.x <= barX + barWidth &&
            mousePoint.y >= barY - 10 && mousePoint.y <= barY + barHeight + 10)
        {
            sliderValue = (float)(mousePoint.x - barX) / (float)barWidth;
            stepdraw = (int)(sliderValue * (drawActions.size() - 1));
            resetcolor(spnodes);
            for (int i = 0; i <= stepdraw; i++)
            {
                DrawAction drawAction = drawActions[i];
                switch (drawAction.type[0])
                {
                case 'n': // "nodes"
                    spnodes[drawAction.u].color = drawAction.color;
                    spnodes[drawAction.u].text = drawAction.value;
                    break;
                case 'c': // "coloredges"
                    color_spedge[drawAction.u][drawAction.v] = drawAction.color;
                    color_spedge[drawAction.v][drawAction.u] = drawAction.color;
                    break;
                case 'h':
                    highlightedLine = drawAction.lineNumber;
                    break;
                default:
                    break;
                }
            }
        }
        if (mousePoint.x >= speedBarX && mousePoint.x <= speedBarX + speedBarWidth &&
            mousePoint.y >= speedBarY - 10 && mousePoint.y <= speedBarY + speedBarHeight + 10)
        {
            speedSliderValue = (float)(mousePoint.x - speedBarX) / (float)speedBarWidth;
            total_step = 120 * (1.0f - speedSliderValue);
            // cout << total_step << endl;
        }
    }
    // Draw seekbar

    if (CheckButton_graph(next_step_box, "next step") && stepdraw < int(drawActions.size()) - 1 && stepdraw >= 0)
    {
        // cout << stepdraw << endl;
        stepdraw++;
        pause = true;
        DrawAction drawAction = drawActions[stepdraw];
        if (drawAction.type[0] == 'e')
        {
            int currentstep = stoi(drawAction.text);
            stepdraw += 120 - currentstep;
        }
        stepdraw = min(stepdraw, int(drawActions.size()) - 1);
        drawAction = drawActions[stepdraw];
        switch (drawAction.type[0]) // Use the first character of the type string for switch
        {
        case 'n': // "nodes"
            spnodes[drawAction.u].color = drawAction.color;
            spnodes[drawAction.u].text = drawAction.value;
            break;
        case 'c': // "coloredges"
            color_spedge[drawAction.u][drawAction.v] = drawAction.color;
            color_spedge[drawAction.v][drawAction.u] = drawAction.color;
            break;
        case 'h':
            highlightedLine = drawAction.lineNumber;
            break;
        default:
            break;
        }
    }
    if (CheckButton_graph(prev_step_box, "previous step") && stepdraw > 0)
    {
        stepdraw--;
        pause = true;
        resetcolor(spnodes);
        DrawAction drawAction = drawActions[stepdraw];
        if (drawAction.type[0] == 'e')
        {
            int currentstep = stoi(drawAction.text);
            stepdraw -= currentstep + 1;
        }
        stepdraw = max(0, stepdraw);
        for (int i = 0; i <= stepdraw; i++)
        {
            DrawAction drawAction = drawActions[i];
            switch (drawAction.type[0])
            {
            case 'n': // "nodes"
                spnodes[drawAction.u].color = drawAction.color;
                spnodes[drawAction.u].text = drawAction.value;
                break;
            case 'c': // "coloredges"
                color_spedge[drawAction.u][drawAction.v] = drawAction.color;
                color_spedge[drawAction.v][drawAction.u] = drawAction.color;
                break;
            case 'h':
                highlightedLine = drawAction.lineNumber;
                break;
            default:
                break;
            }
        }
    }
    if (!fixed)
        UpdateGraph(spnodes, spedges, selectedspnode, C_rep, c_spring, L, timeStep, damping, physicsIterations, spnodeRadius, screenWidth, screenHeight);
    else
        fix_updategraph(spnodes, spedges, selectedspnode, C_rep, c_spring, L, timeStep, damping, physicsIterations, spnodeRadius, screenWidth, screenHeight);
    RenderGraph(spnodes, spedges, selectedspnode, spnodeRadius, isDirected);
    if (!drawActions.empty() && stepdraw < int(drawActions.size()) && !pause && stepdraw >= 0)
    {
        DrawAction drawAction = drawActions[stepdraw];
        switch (drawAction.type[0]) // Use the first character of the type string for switch
        {
        case 'n': // "nodes"
            spnodes[drawAction.u].color = drawAction.color;
            spnodes[drawAction.u].text = drawAction.value;
            break;
        case 'e': // "edges"
            currentstep = stoi(drawAction.text);
            if (animationedge(drawAction.u, drawAction.v, spnodes, spedges, drawAction.color, currentstep))
            {
                stepdraw += dijkstra_total_step - currentstep;
            }
            break;
        case 'c': // "coloredges"
            color_spedge[drawAction.u][drawAction.v] = drawAction.color;
            color_spedge[drawAction.v][drawAction.u] = drawAction.color;
            break;
        case 'h':
            highlightedLine = drawAction.lineNumber;
            break;
        default:
            break;
        }
        stepdraw++;
        // frameCount++;
        // cout << stepdraw << endl;
    }
    sliderValue = (float)stepdraw / (float)(drawActions.size() - 1);
    DrawRectangle(barX, barY, barWidth, barHeight, LIGHTGRAY);
    DrawRectangleGradientH(barX, barY, (int)(barWidth * sliderValue), barHeight, SKYBLUE, DARKBLUE);
    DrawCircle(barX + (int)(sliderValue * barWidth), barY + barHeight / 2, knobRadius, DARKBLUE);

    DrawRectangle(speedBarX, speedBarY, speedBarWidth, speedBarHeight, LIGHTGRAY);
    DrawRectangleGradientH(speedBarX, speedBarY, (int)(speedBarWidth * speedSliderValue), speedBarHeight, SKYBLUE, DARKBLUE);
    DrawCircle(speedBarX + (int)(speedSliderValue * speedBarWidth), speedBarY + speedBarHeight / 2, speedKnobRadius, DARKBLUE);

    // Tính tốc độ hiện tại dựa trên giá trị của speedSliderValue
    int currentSpeed = static_cast<int>(1 + speedSliderValue * 9); // Tốc độ từ 1x đến 10x

    // Hiển thị tốc độ hiện tại bên cạnh thanh speed bar
    std::string speedText = std::to_string(currentSpeed) + "x";
    // DrawText(speedText.c_str(), speedBarX + speedBarWidth + 20, speedBarY - 5, 20, BLACK);
    DrawTextEx(GetFont(), speedText.c_str(), {float(speedBarX + speedBarWidth + 20), float(speedBarY) - 10}, GetFont().baseSize, 1, BLACK);
    if (pause)
    {
        if (!drawActions.empty() && stepdraw < int(drawActions.size()) && stepdraw >= 0)
        {
            DrawAction drawAction = drawActions[stepdraw];
            if (drawAction.type[0] == 'e')
            {
                currentstep = stoi(drawAction.text);
                animationedge(drawAction.u, drawAction.v, spnodes, spedges, drawAction.color, currentstep);
            }
        }
    }

    // Vẽ và xử lý File Dialog
    if (fileDialogState.windowActive)
    {
        GuiWindowFileDialog(&fileDialogState);

        if (fileDialogState.SelectFilePressed)
        {
            // Xử lý file được chọn
            std::string selectedFilePath = std::string(fileDialogState.dirPathText) + "\\" + std::string(fileDialogState.fileNameText);
            std::cout << "Selected file: " << selectedFilePath << std::endl;

            std::ifstream inputFile(selectedFilePath);
            if (inputFile.is_open())
            {
                std::vector<std::string> lines;
                std::string line;
                while (std::getline(inputFile, line))
                {
                    lines.push_back(line);
                }
                inputFile.close();

                // Giả sử file là danh sách cạnh
                ParseEdgeList(lines, spnodes, spedges, isDirected);
                drawActions.clear();
                resetcolor(spnodes);
                stepdraw = 0;
            }
            else
            {
                std::cerr << "Failed to open file: " << selectedFilePath << std::endl;
            }

            fileDialogState.SelectFilePressed = false; // Reset trạng thái
        }
    }

    EndDrawing();
    frameCount++;
    return;
}
void DrawBoxes_graph(Rectangle Box, const char *Buffer, int framesCounter, bool isActive)
{
    DrawRectangleRec(Box, LIGHTGRAY);
    // DrawText(Buffer, Box.x + 10, Box.y + 15, 20, BLACK);
    DrawTextEx(GetFont(), Buffer, {Box.x + GetFont().baseSize / 2, Box.y + 15}, GetFont().baseSize, 1, BLACK);
    int MAX_INPUT_CHARS = 10;
    if (((framesCounter / 20) % 2) == 0)
    {
        if (isActive && strlen(Buffer) < MAX_INPUT_CHARS)
            DrawText("_", Box.x + GetFont().baseSize / 2 + MeasureText(Buffer, GetFont().baseSize), Box.y + 15, GetFont().baseSize, BLACK);
    }
}

void HandleInput_graph(Rectangle box, char *buffer, int &index, bool &isActive)
{
    Vector2 mousePoint = GetMousePosition();
    if (CheckCollisionPointRec(mousePoint, box) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        isActive = true;
    }
    else if (!CheckCollisionPointRec(mousePoint, box) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        isActive = false;
    }

    if (isActive)
    {
        // Set the window's cursor to the I-Beam
        SetMouseCursor(MOUSE_CURSOR_IBEAM);

        // Get char pressed (unicode character) on the queue
        int key = GetCharPressed();

        // Check if more characters have been pressed on the same frame
        while (key > 0)
        {
            // NOTE: Only allow keys in range [32..125]
            if ((key >= 48) && (key <= 57) && (index < MAX_INPUT_CHARS))
            {
                buffer[index] = (char)key;
                buffer[index + 1] = '\0'; // Add null terminator at the end of the string.
                index++;
            }

            key = GetCharPressed(); // Check next character in the queue
        }

        if (IsKeyPressed(KEY_BACKSPACE))
        {
            index--;
            if (index < 0)
                index = 0;
            buffer[index] = '\0';
        }
    }
    else
    {
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    }
}

bool CheckButton_graph(Rectangle button, const char *text)
{
    static Texture2D pauseTexture = LoadTexture("res/pause.png");            // Load texture once
    static Texture2D playTexture = LoadTexture("res/play.png");              // Load texture once
    static Texture2D backwardTexture = LoadTexture("res/previous step.png"); // Load texture once
    static Texture2D beginTexture = LoadTexture("res/go to begin.png");      // Load texture once
    static Texture2D replayTexture = LoadTexture("res/replay.png");          // Load texture once
    static Texture2D forwardTexture = LoadTexture("res/next step.png");      // Load texture once
    static Texture2D endTexture = LoadTexture("res/go to end.png");          // Load texture once

    Vector2 mousePoint = GetMousePosition();
    bool mouseOnButton = CheckCollisionPointRec(mousePoint, button);

    if (mouseOnButton)
    {
        DrawRectangleRec(button, LIGHTGRAY);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            return true;
        }
    }
    else
    {
        DrawRectangleRec(button, WHITE);
    }

    // Draw the corresponding image based on the button text
    Texture2D *texture = nullptr;

    if (std::string(text) == "pause")
        texture = &pauseTexture;
    else if (std::string(text) == "play")
        texture = &playTexture;
    else if (std::string(text) == "previous step")
        texture = &backwardTexture;
    else if (std::string(text) == "go to begin")
        texture = &beginTexture;
    else if (std::string(text) == "replay")
        texture = &replayTexture;
    else if (std::string(text) == "next step")
        texture = &forwardTexture;
    else if (std::string(text) == "go to end")
        texture = &endTexture;
    else if (std::string(text) == "menu")
    {
        return false;
    }
    else if (std::string(text) == "PSEUDO CODE")
    {
        return false;
    }
    else
    {
        // Draw the text if no texture is found
        if (!mouseOnButton)
            DrawRectangleGradientH(button.x, button.y, button.width, button.height, SKYBLUE, DARKBLUE);
        DrawTextEx(GetFont(), text, {button.x + 10, button.y + 10}, GetFont().baseSize, 1, WHITE);
        return false; // No texture to draw, return false
    }
    if (texture != nullptr)
    {
        // Define the source rectangle (entire texture)
        Rectangle sourceRect = {0, 0, (float)texture->width, (float)texture->height};

        // Define the destination rectangle (scaled size and position)
        Rectangle destRect = {button.x, button.y, button.width, button.height};

        // Draw the resized texture
        DrawTexturePro(*texture, sourceRect, destRect, {0, 0}, 0.0f, WHITE);
    }

    return false;
}

bool IsValidEdgeList(const std::vector<std::string> &lines)
{
    if (lines.empty())
    {
        std::cerr << "Edge List is empty." << std::endl;
        return false;
    }

    std::istringstream firstLine(lines[0]);
    int V, E;
    if (!(firstLine >> V >> E) || V <= 0 || E < 0)
    {
        std::cerr << "Invalid Edge List format: Missing or invalid V and E in the first line." << std::endl;
        return false;
    }

    for (size_t i = 1; i < lines.size(); i++)
    {
        if (lines[i].empty())
            continue;

        std::istringstream iss(lines[i]);
        int u, v;
        float weight = 1.0f;

        if (!(iss >> u >> v) || u < 0 || v < 0 || u >= V || v >= V)
        {
            std::cerr << "Invalid Edge List format: Invalid edge at line " << i + 1 << "." << std::endl;
            return false;
        }

        if (iss >> weight && weight < 0)
        {
            std::cerr << "Invalid Edge List format: Negative weight at line " << i + 1 << "." << std::endl;
            return false;
        }
    }

    return true;
}

void ParseEdgeList(const std::vector<std::string> &lines, std::vector<spnode> &spnodes, std::vector<spedge> &spedges, bool &isDirected)
{
    cout << "parse ok" << endl;
    spnodes.clear();
    spedges.clear();
    adj.clear();
    color_spedge.clear();

    if (!IsValidEdgeList(lines))
        return;

    std::istringstream firstLine(lines[0]);
    int V, E;
    firstLine >> V >> E;

    spnodes.resize(V);
    adj.resize(V);
    color_spedge.resize(V);

    for (int i = 0; i < V; i++)
    {
        color_spedge[i].resize(V, BLACK);
    }

    for (int i = 0; i < V; i++)
    {
        spnodes[i].position = {static_cast<float>(rand() % GetScreenWidth()), static_cast<float>(rand() % GetScreenHeight())};
        spnodes[i].velocity = {0, 0};
    }

    for (size_t i = 1; i < lines.size(); i++)
    {
        if (lines[i].empty())
            continue;

        std::istringstream iss(lines[i]);
        int u, v;
        float weight = 1.0f; // Mặc định trọng số là 1 nếu không có

        if (iss >> u >> v) // Đọc hai đỉnh
        {
            if (!(iss >> weight)) // Nếu không có trọng số, dùng mặc định
            {
                weight = 1.0f;
            }

            // Thêm cạnh vào danh sách cạnh và danh sách kề
            spedges.push_back(spedge(u, v, weight));
            adj[u].push_back(spedge(u, v, weight));

            if (!isDirected) // Nếu đồ thị vô hướng
            {
                adj[v].push_back(spedge(v, u, weight));
            }
        }
    }
}

bool IsValidAdjMatrix(const std::vector<std::string> &lines)
{
    if (lines.empty())
    {
        std::cerr << "Adjacency Matrix is empty." << std::endl;
        return false;
    }

    std::istringstream firstLine(lines[0]);
    int V;
    if (!(firstLine >> V) || V <= 0)
    {
        std::cerr << "Invalid Adjacency Matrix format: Missing or invalid V in the first line." << std::endl;
        return false;
    }

    if ((int)lines.size() - 1 != V)
    {
        std::cerr << "Invalid Adjacency Matrix format: Number of rows does not match V." << std::endl;
        return false;
    }

    for (int i = 1; i <= V; i++)
    {
        std::istringstream iss(lines[i]);
        for (int j = 0; j < V; j++)
        {
            float weight;
            if (!(iss >> weight) || weight < 0)
            {
                std::cerr << "Invalid Adjacency Matrix format: Invalid weight at row " << i << ", column " << j + 1 << "." << std::endl;
                return false;
            }
        }
    }

    return true;
}

void ParseAdjMatrix(const std::vector<std::string> &lines, std::vector<spnode> &spnodes, std::vector<spedge> &spedges)
{
    spnodes.clear();
    spedges.clear();
    adj.clear();
    color_spedge.clear();
    if (lines.empty())
        return;
    if (!IsValidAdjMatrix(lines))
        return;
    std::istringstream firstLine(lines[0]);
    int V;
    if (!(firstLine >> V))
    {
        std::cerr << "Invalid Adjacency Matrix format: Missing V in the first line." << std::endl;
        return;
    }

    if ((int)lines.size() - 1 != V)
    {
        std::cerr << "Invalid Adjacency Matrix format: Number of rows does not match V." << std::endl;
        return;
    }

    spnodes.resize(V);
    adj.resize(V);
    color_spedge.resize(V);
    for (int i = 0; i < V; i++)
    {
        color_spedge[i].resize(V, BLACK);
    }
    for (int i = 0; i < V; i++)
    {
        spnodes[i].position = {static_cast<float>(rand() % GetScreenWidth()), static_cast<float>(rand() % GetScreenHeight())};
        spnodes[i].velocity = {0, 0};
    }
    for (int i = 0; i < V; i++)
    {
        std::istringstream iss(lines[i + 1]);
        for (int j = 0; j < V; j++)
        {
            float weight;
            if (!(iss >> weight))
            {
                std::cerr << "Invalid Adjacency Matrix format: Incorrect data at row " << i + 1 << "." << std::endl;
                return;
            }

            if (weight > 0)
            {
                spedges.push_back(spedge(i, j, weight));
                adj[i].push_back(spedge(i, j, weight));
            }
        }
    }
}
void DrawPseudoCode(const std::vector<std::string> &pseudoCode, int highlightedLine)
{
    int startX = 1320;                             // Vị trí bắt đầu vẽ
    int startY = 600;                              // Vị trí bắt đầu vẽ
    int lineHeight = GetFont().baseSize + 5;       // Chiều cao mỗi dòng
    DrawRectangle(1320, 600, 550, 300, LIGHTGRAY); // Vẽ nền cho hộp thoại
    for (size_t i = 0; i < pseudoCode.size(); i++)
    {
        Color color = (i == highlightedLine) ? RED : BLACK; // Highlight dòng hiện tại
        // DrawText(pseudoCode[i].c_str(), startX, startY + i * lineHeight, 20, color);
        if (i == highlightedLine)
            DrawRectangle(startX, startY + i * lineHeight, 550, lineHeight, YELLOW); // Vẽ nền cho dòng
        DrawTextEx(GetFont(), pseudoCode[i].c_str(), {float(startX), float(startY + i * lineHeight)}, GetFont().baseSize, 1, color);
    }
}