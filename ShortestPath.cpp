#include "ShortestPath.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <bits/stdc++.h>
using namespace std;
const int MAX_INPUT_CHARS = 10;
std::vector<std::vector<Edge>> adj;
vector<vector<Color>> color_edge;
typedef pair<int, int> ii;
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
void animate(std::vector<Node> &nodes, std::vector<Edge> &edges)
{
    float nodeRadius = 20.0f;
    int selectedNode = -1;
    RenderGraph(nodes, edges, selectedNode, nodeRadius);
    EndDrawing();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    BeginDrawing();
    ClearBackground(RAYWHITE);
}
void Dijkstra(int start, vector<Node> &nodes, std::vector<Edge> &edges)
{
    std::vector<int> previous;
    std::vector<float> distances;
    previous.resize(nodes.size() + 1);
    distances.resize(nodes.size() + 1);
    int n = nodes.size();
    long long INF = 1e9 + 7;
    distances.assign(n, INF);
    previous.assign(n, -1);
    distances[start] = 0;
    priority_queue<ii, vector<ii>, greater<ii>> pq;
    pq.push({0, start});
    nodes[start].text = "source,0";
    while (!pq.empty())
    {
        auto k = pq.top();
        int currentDist = k.first;
        int u = k.second;
        pq.pop();
        if (currentDist > distances[u])
            continue;
        nodes[u].color = RED;
        animate(nodes, edges);
        for (const auto &edge : adj[u])
        {
            int v = edge.end;
            if (v == u)
                v = edge.start;
            int weight = edge.weight;
            if (distances[u] + weight < distances[v])
            {

                color_edge[u][v] = ORANGE;
                color_edge[v][u] = ORANGE;
                nodes[v].text = std::to_string(int(distances[u] + weight));
                if (previous[v] != -1)
                {
                    color_edge[previous[v]][v] = GRAY;
                    color_edge[v][previous[v]] = GRAY;
                }
                animate(nodes, edges);
                distances[v] = distances[u] + weight;
                previous[v] = u;
                pq.push({distances[v], v});
            }
            else if (distances[u] + weight > distances[v])
            {
                color_edge[u][v] = GRAY;
                color_edge[v][u] = GRAY;
                animate(nodes, edges);
            }
        }
    }
}
void resetcolor(std::vector<Node> &nodes)
{
    for (int i = 0; i < nodes.size(); i++)
    {
        nodes[i].color = BLACK;
        nodes[i].text = "INF";
    }
    for (int i = 0; i < nodes.size(); i++)
    {
        for (auto &edge : adj[i])
        {
            color_edge[edge.start][edge.end] = BLACK;
            color_edge[edge.end][edge.start] = BLACK;
        }
    }
}
float EuclideanDistance(Vector2 a, Vector2 b)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return sqrtf(dx * dx + dy * dy);
}
void InitializeGraph(std::vector<Node> &nodes, std::vector<Edge> &edges, int screenWidth, int screenHeight)
{
    const int numNodes = 9;
    srand(static_cast<unsigned>(time(nullptr)));
    nodes.resize(numNodes);
    color_edge.resize(numNodes);
    for (int i = 0; i < numNodes; i++)
    {
        color_edge[i].resize(numNodes, BLACK);
    }
    for (int i = 0; i < numNodes; i++)
    {
        nodes[i].position = {static_cast<float>(rand() % screenWidth), static_cast<float>(rand() % screenHeight)};
        nodes[i].velocity = {0, 0};
    }
    edges.push_back(Edge(0, 1, 8));
    edges.push_back(Edge(0, 2, 12));
    edges.push_back(Edge(1, 2, 13));
    edges.push_back(Edge(1, 3, 25));
    edges.push_back(Edge(2, 3, 14));
    edges.push_back(Edge(2, 6, 21));
    edges.push_back(Edge(3, 4, 20));
    edges.push_back(Edge(3, 5, 8));
    edges.push_back(Edge(3, 6, 12));
    edges.push_back(Edge(3, 7, 12));
    edges.push_back(Edge(3, 8, 16));
    edges.push_back(Edge(4, 1, 9));
    edges.push_back(Edge(4, 5, 19));
    edges.push_back(Edge(5, 7, 11));
    edges.push_back(Edge(6, 8, 11));
    edges.push_back(Edge(7, 8, 9));
    adj.resize(nodes.size());

    // Duyệt qua các cạnh và thêm vào danh sách kề
    for (const auto &edge : edges)
    {
        adj[edge.start].push_back(edge);
    }
}

void UpdateGraph(std::vector<Node> &nodes, std::vector<Edge> &edges, int &selectedNode, float C_rep, float c_spring, float L, float timeStep, float damping, int physicsIterations, float nodeRadius, int screenWidth, int screenHeight)
{
    Vector2 mousePos = GetMousePosition();

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        for (int i = 0; i < nodes.size(); i++)
        {
            if (EuclideanDistance(mousePos, nodes[i].position) <= nodeRadius * 1.5f)
            {
                selectedNode = i;
                break;
            }
        }
    }

    if (selectedNode != -1 && IsMouseButtonDown(MOUSE_LEFT_BUTTON))
    {
        nodes[selectedNode].position = mousePos;
        nodes[selectedNode].velocity = {0, 0};
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
    {
        selectedNode = -1;
    }

    for (int iter = 0; iter < physicsIterations; iter++)
    {
        std::vector<Vector2> forces(nodes.size(), {0, 0});

        // Tính lực đẩy giữa các node
        for (int i = 0; i < nodes.size(); i++)
        {
            for (int j = i + 1; j < nodes.size(); j++)
            {
                Vector2 delta = {nodes[i].position.x - nodes[j].position.x,
                                 nodes[i].position.y - nodes[j].position.y};
                float dist = EuclideanDistance(nodes[i].position, nodes[j].position);
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
        for (auto &edge : edges)
        {
            int u = edge.start;
            int v = edge.end;
            Vector2 delta = {nodes[u].position.x - nodes[v].position.x,
                             nodes[u].position.y - nodes[v].position.y};
            float dist = EuclideanDistance(nodes[u].position, nodes[v].position);
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

        for (int i = 0; i < nodes.size(); i++)
        {
            if (i == selectedNode)
                continue;
            nodes[i].velocity.x = (nodes[i].velocity.x + forces[i].x * timeStep) * damping;
            nodes[i].velocity.y = (nodes[i].velocity.y + forces[i].y * timeStep) * damping;
            nodes[i].position.x += nodes[i].velocity.x * timeStep;
            nodes[i].position.y += nodes[i].velocity.y * timeStep;

            if (nodes[i].position.x < nodeRadius)
                nodes[i].position.x = nodeRadius;
            if (nodes[i].position.y < nodeRadius)
                nodes[i].position.y = nodeRadius;
            if (nodes[i].position.x > screenWidth - nodeRadius)
                nodes[i].position.x = screenWidth - nodeRadius;
            if (nodes[i].position.y > screenHeight - nodeRadius)
                nodes[i].position.y = screenHeight - nodeRadius;
        }
    }
}
void DrawCenteredText(const std::string &text, Vector2 position, int fontSize, Color color)
{
    // Tính chiều rộng của chuỗi
    float textWidth = MeasureText(text.c_str(), fontSize);

    // Điều chỉnh tọa độ x để căn giữa
    float centeredX = position.x - textWidth / 2.0f;

    // Vẽ chuỗi tại vị trí đã điều chỉnh
    DrawText(text.c_str(), (centeredX), (position.y), fontSize, color);
}
void RenderGraph(const std::vector<Node> &nodes, const std::vector<Edge> &edges, int selectedNode, float nodeRadius)
{
    for (int i = 0; i < nodes.size(); i++)
    {
        for (auto edge : adj[i])
        {
            float edgeLength = EuclideanDistance(nodes[edge.start].position, nodes[edge.end].position);
            // cout << edgeLength << endl;

            Vector2 parentCenter = {nodes[edge.start].position.x, nodes[edge.start].position.y};
            Vector2 childCenter = {nodes[edge.end].position.x, nodes[edge.end].position.y};

            // Tính vector chỉ phương từ cha đến con
            Vector2 direction = {childCenter.x - parentCenter.x, childCenter.y - parentCenter.y};
            float len = sqrt(direction.x * direction.x + direction.y * direction.y);
            if (len != 0)
            {
                direction.x /= len;
                direction.y /= len;
            }

            // Tính điểm bắt đầu và kết thúc: cách tâm một khoảng bằng bán kính
            Vector2 startEdge = {parentCenter.x + direction.x * nodeRadius, parentCenter.y + direction.y * nodeRadius};
            Vector2 endEdge = {childCenter.x - direction.x * nodeRadius, childCenter.y - direction.y * nodeRadius};

            // DrawLineEx(startEdge, endEdge, 2.0f, color_edge[edge.start][edge.end]);
            drawArrowLine_graph(startEdge, endEdge, 2.0f, color_edge[edge.start][edge.end]);
            // Tính trung điểm của cạnh
            Vector2 midPoint = {1.0 * (startEdge.x + endEdge.x) / 2.0f,
                                1.0 * (startEdge.y + endEdge.y) / 2.0f};

            // Tính vector nối từ node u đến node v
            Vector2 delta = {endEdge.x - startEdge.x,
                             endEdge.y - startEdge.y};
            float dist = sqrtf(1.0 * delta.x * delta.x + 1.0 * delta.y * delta.y);
            if (dist < 0.1f)
                dist = 0.1f; // tránh chia cho 0

            // Tính vector pháp tuyến (normalized)
            Vector2 normal = {-delta.y / dist, delta.x / dist};

            // Chọn offset (khoảng cách dịch chuyển từ trung điểm)
            float offset = 13.0f; // bạn có thể điều chỉnh giá trị này
            Vector2 labelPos = {midPoint.x + normal.x * offset, midPoint.y + normal.y * offset};

            // Hiển thị trọng số tại vị trí labelPos
            std::string weightStr = std::to_string(static_cast<int>(edge.weight));
            DrawText(weightStr.c_str(), (labelPos.x), (labelPos.y), 17, BLACK);
        }
    }

    for (int i = 0; i < nodes.size(); i++)
    {
        Color color = (i == selectedNode) ? BLUE : nodes[i].color;
        for (double thickness = 0.0; thickness < 3.0; thickness += 0.5)
            DrawCircleLinesV(nodes[i].position, 20.0f + thickness, color);
        std::string text = std::to_string(i);
        int valueTextWidth = MeasureText(text.c_str(), 20.0f);
        DrawText(text.c_str(), nodes[i].position.x - valueTextWidth / 2, nodes[i].position.y - 10, 20, BLUE);
        DrawCenteredText(nodes[i].text, {nodes[i].position.x, nodes[i].position.y + 23}, 20, RED);
    }
}
void rendershortestpath(int screenWidth, int screenHeight)
{
    static float nodeRadius = 20.0f;
    static int frameCount = 0;
    // ---- THÔNG SỐ ĐÃ ĐIỀU CHỈNH ----
    static float C_rep = 700000.0f; // Lực đẩy (càng lớn, các node càng đẩy mạnh hơn)
    static float c_spring = 15.0f;  // Lực hút (càng lớn, các node càng hút mạnh hơn)
    static float L = 150.0f;        // Độ dài lò xo lý tưởng

    static float timeStep = 0.05f;    // Bước thời gian
    static float damping = 0.75f;     // Giảm damping để node di chuyển linh hoạt hơn
    static int physicsIterations = 5; // Số lần cập nhật vật lý mỗi khung hình

    static std::vector<Node> nodes;
    static std::vector<Edge> edges;
    static int selectedNode = -1;
    static Rectangle dijkstra_box = {1200, 200, 200, 50};
    static bool dijkstraActive = false;
    static char dijkstraBuffer[MAX_INPUT_CHARS + 1] = {0}; // Buffer
    static int dijkstraIndex = 0;                          // Index for input buffer
    if (!frameCount)
        InitializeGraph(nodes, edges, screenWidth, screenHeight);
    DrawBoxes_graph(dijkstra_box, dijkstraBuffer, frameCount, dijkstraActive);
    HandleInput_graph(dijkstra_box, dijkstraBuffer, dijkstraIndex, dijkstraActive);
    if (IsKeyPressed(KEY_ENTER))
    {
        resetcolor(nodes);
        int startNode = std::stoi(dijkstraBuffer); // Convert string to integer
        if (startNode >= 0 && startNode < nodes.size())
        {
            resetcolor(nodes);
            Dijkstra(startNode, nodes, edges);
        }
        else
        {
            std::cout << "Invalid node index!" << std::endl;
        }
        dijkstraIndex = 0;
        dijkstraBuffer[0] = '\0'; // Reset the buffer
    }
    UpdateGraph(nodes, edges, selectedNode, C_rep, c_spring, L, timeStep, damping, physicsIterations, nodeRadius, screenWidth, screenHeight);
    BeginDrawing();
    ClearBackground(RAYWHITE);
    RenderGraph(nodes, edges, selectedNode, nodeRadius);
    EndDrawing();
    frameCount++;
    return;
}
void DrawBoxes_graph(Rectangle Box, const char *Buffer, int framesCounter, bool isActive)
{
    DrawRectangleRec(Box, LIGHTGRAY);
    DrawText(Buffer, Box.x + 10, Box.y + 15, 20, BLACK);
    int MAX_INPUT_CHARS = 10;
    if (((framesCounter / 20) % 2) == 0)
    {
        if (isActive && strlen(Buffer) < MAX_INPUT_CHARS)
            DrawText("_", Box.x + 10 + MeasureText(Buffer, 20), Box.y + 15, 20, BLACK);
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
        DrawRectangleRec(button, GRAY);
    }

    DrawText(text, button.x + 10, button.y + 10, 20, BLACK);
    return false;
}