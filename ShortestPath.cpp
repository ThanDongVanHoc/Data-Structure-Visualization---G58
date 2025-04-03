#include "ShortestPath.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <bits/stdc++.h>
#include "var.h"
using namespace std;
const int MAX_INPUT_CHARS = 10;
std::vector<std::vector<spedge>> adj;
vector<vector<Color>> color_spedge;
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

void animationedge(int u, int v, const std::vector<spnode> &spnodes, std::vector<spedge> &spedges, Color color)
{
    float spedgeLength = EuclideanDistance(spnodes[u].position, spnodes[v].position);
    // cout << spedgeLength << endl;

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
    int spnodeRadius = 20;
    // Tính điểm bắt đầu và kết thúc: cách tâm một khoảng bằng bán kính
    Vector2 startspedge = {parentCenter.x + direction.x * spnodeRadius, parentCenter.y + direction.y * spnodeRadius};
    Vector2 endspedge = {childCenter.x - direction.x * spnodeRadius, childCenter.y - direction.y * spnodeRadius};
    int thickness = 4.0f;
    for (float step = 0.1; step <= 1.0; step += 0.015f)
    {
        Vector2 direct = {startspedge.x + double((endspedge.x - startspedge.x) * step), startspedge.y + double((endspedge.y - startspedge.y) * step)};
        float spnodeRadius = 20.0f;
        int selectedspnode = -1;
        RenderGraph(spnodes, spedges, selectedspnode, spnodeRadius);
        DrawLineEx(startspedge, direct, thickness, color);
        EndDrawing();
        BeginDrawing();
        ClearBackground(RAYWHITE);
    }
    return;
}
void animate(std::vector<spnode> &spnodes, std::vector<spedge> &spedges)
{
    float spnodeRadius = 20.0f;
    int selectedspnode = -1;
    RenderGraph(spnodes, spedges, selectedspnode, spnodeRadius);
    EndDrawing();
    BeginDrawing();
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Delay for animation
    ClearBackground(RAYWHITE);
}
void Dijkstra(int start, vector<spnode> &spnodes, std::vector<spedge> &spedges)
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
    while (!pq.empty())
    {
        auto k = pq.top();
        int currentDist = k.first;
        int u = k.second;
        pq.pop();
        if (currentDist > distances[u])
            continue;
        spnodes[u].color = RED;
        animate(spnodes, spedges);
        for (const auto &spedge : adj[u])
        {
            int v = spedge.end;
            if (v == u)
                v = spedge.start;
            int weight = spedge.weight;
            if (distances[u] + weight < distances[v])
            {
                animationedge(u, v, spnodes, spedges, ORANGE);
                color_spedge[u][v] = ORANGE;
                color_spedge[v][u] = ORANGE;
                spnodes[v].text = std::to_string(int(distances[u] + weight));
                if (previous[v] != -1)
                {
                    animationedge(previous[v], v, spnodes, spedges, GRAY);
                    color_spedge[previous[v]][v] = GRAY;
                    color_spedge[v][previous[v]] = GRAY;
                }
                animate(spnodes, spedges);
                distances[v] = distances[u] + weight;
                previous[v] = u;
                pq.push({distances[v], v});
            }
            else if (distances[u] + weight > distances[v])
            {
                animationedge(u, v, spnodes, spedges, GRAY);
                color_spedge[u][v] = GRAY;
                color_spedge[v][u] = GRAY;
                animate(spnodes, spedges);
            }
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
void RenderGraph(const std::vector<spnode> &spnodes, const std::vector<spedge> &spedges, int selectedspnode, float spnodeRadius)
{
    DrawCenteredText("Dijkstra Visualize", {float(GetScreenWidth() / 2.0f), 100}, GetFont().baseSize * 2, BLACK);
    for (int i = 0; i < int(spnodes.size()); i++)
    {
        for (auto spedge : adj[i])
        {
            float spedgeLength = EuclideanDistance(spnodes[spedge.start].position, spnodes[spedge.end].position);
            // cout << spedgeLength << endl;

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

            // DrawLineEx(startspedge, endspedge, 2.0f, color_spedge[spedge.start][spedge.end]);
            drawArrowLine_graph(startspedge, endspedge, 4.0f, color_spedge[spedge.start][spedge.end]);
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
            // DrawText(weightStr.c_str(), (labelPos.x) - tw / 2, (labelPos.y) - 10, 20, BLACK);
            DrawTextEx(GetFont(), weightStr.c_str(), {labelPos.x - float(1.0 * tw / 2.0), labelPos.y - float(1.0 * GetFont().baseSize / 2.0)}, GetFont().baseSize, 1, BLACK);
        }
    }

    for (int i = 0; i < int(spnodes.size()); i++)
    {
        Color color = (i == selectedspnode) ? BLUE : spnodes[i].color;
        for (double thickness = 0.0; thickness < 3.0; thickness += 0.5)
            DrawCircleLinesV(spnodes[i].position, 20.0f + thickness, color);
        std::string text = std::to_string(i);
        int valueTextWidth = MeasureText(text.c_str(), GetFont().baseSize);
        // DrawText(text.c_str(), spnodes[i].position.x - valueTextWidth / 2, spnodes[i].position.y - 10, 20, BLUE);
        DrawTextEx(GetFont(), text.c_str(), {spnodes[i].position.x - float(1.0 * valueTextWidth / 2.0), spnodes[i].position.y - float(1.0 * GetFont().baseSize / 2.0)}, GetFont().baseSize, 1, color);
        DrawCenteredText(spnodes[i].text, {spnodes[i].position.x, spnodes[i].position.y + 23}, GetFont().baseSize, RED);
    }
}
void rendershortestpath(int screenWidth, int screenHeight)
{
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
    static Rectangle dijkstra_box = {1200, 200, 200, 50};
    static bool dijkstraActive = false;
    static char dijkstraBuffer[MAX_INPUT_CHARS + 1] = {0}; // Buffer
    static int dijkstraIndex = 0;                          // Index for input buffer
    if (!frameCount)
        InitializeGraph(spnodes, spedges, screenWidth, screenHeight);
    DrawBoxes_graph(dijkstra_box, dijkstraBuffer, frameCount, dijkstraActive);
    HandleInput_graph(dijkstra_box, dijkstraBuffer, dijkstraIndex, dijkstraActive);
    if (IsKeyPressed(KEY_ENTER))
    {
        resetcolor(spnodes);
        int startspnode = std::stoi(dijkstraBuffer); // Convert string to integer
        if (startspnode >= 0 && startspnode < int(spnodes.size()))
        {
            resetcolor(spnodes);
            Dijkstra(startspnode, spnodes, spedges);
        }
        else
        {
            std::cout << "Invalid spnode index!" << std::endl;
        }
        dijkstraIndex = 0;
        dijkstraBuffer[0] = '\0'; // Reset the buffer
    }
    UpdateGraph(spnodes, spedges, selectedspnode, C_rep, c_spring, L, timeStep, damping, physicsIterations, spnodeRadius, screenWidth, screenHeight);
    BeginDrawing();
    ClearBackground(RAYWHITE);
    RenderGraph(spnodes, spedges, selectedspnode, spnodeRadius);
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

    // DrawText(text, button.x + 10, button.y + 10, 20, BLACK);
    DrawTextEx(GetFont(), text, {button.x + GetFont().baseSize / 2, button.y + GetFont().baseSize / 2}, GetFont().baseSize, 1, BLACK);
    return false;
}