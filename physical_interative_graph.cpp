// static float C_rep = 1000000.0f; // Lực đẩy (càng lớn, các spnode càng đẩy mạnh hơn)
// static float c_spring = 10.0f;   // Lực hút (càng lớn, các spnode càng hút mạnh hơn)
// static float L = 225.0f;         // Độ dài lò xo lý tưởng

// static float timeStep = 0.05f;    // Bước thời gian
// static float damping = 0.7f;      // Giảm damping để spnode di chuyển linh hoạt hơn
// static int physicsIterations = 5; // Số lần cập nhật vật lý mỗi khung hình





// void UpdateGraph(std::vector<spnode> &spnodes, std::vector<spedge> &spedges, int &selectedspnode, float C_rep, float c_spring, float L, float timeStep, float damping, int physicsIterations, float spnodeRadius, int screenWidth, int screenHeight)
// {
//     Vector2 mousePos = GetMousePosition();

//     if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
//     {
//         for (int i = 0; i < int(spnodes.size()); i++)
//         {
//             if (EuclideanDistance(mousePos, spnodes[i].position) <= spnodeRadius * 1.5f)
//             {
//                 selectedspnode = i;
//                 break;
//             }
//         }
//     }

//     if (selectedspnode != -1 && IsMouseButtonDown(MOUSE_LEFT_BUTTON))
//     {
//         spnodes[selectedspnode].position = mousePos;
//         spnodes[selectedspnode].velocity = {0, 0};
//     }

//     if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
//     {
//         selectedspnode = -1;
//     }

//     for (int iter = 0; iter < physicsIterations; iter++)
//     {
//         std::vector<Vector2> forces(spnodes.size(), {0, 0});

//         // Tính lực đẩy giữa các spnode
//         for (int i = 0; i < int(spnodes.size()); i++)
//         {
//             for (int j = i + 1; j < int(spnodes.size()); j++)
//             {
//                 Vector2 delta = {spnodes[i].position.x - spnodes[j].position.x,
//                                  spnodes[i].position.y - spnodes[j].position.y};
//                 float dist = EuclideanDistance(spnodes[i].position, spnodes[j].position);
//                 if (dist < 0.1f)
//                     dist = 0.1f;
//                 float rep = C_rep / (dist * dist);
//                 Vector2 dir = {delta.x / dist, delta.y / dist};
//                 Vector2 forceRep = {dir.x * rep, dir.y * rep};

//                 forces[i].x += forceRep.x;
//                 forces[i].y += forceRep.y;
//                 forces[j].x -= forceRep.x;
//                 forces[j].y -= forceRep.y;
//             }
//         }

//         // Tính lực hút trên các cạnh
//         for (auto &spedge : spedges)
//         {
//             int u = spedge.start;
//             int v = spedge.end;
//             Vector2 delta = {spnodes[u].position.x - spnodes[v].position.x,
//                              spnodes[u].position.y - spnodes[v].position.y};
//             float dist = EuclideanDistance(spnodes[u].position, spnodes[v].position);
//             if (dist < 0.1f)
//                 dist = 0.1f;
//             float disp = dist - L;
//             float att = c_spring * disp;
//             Vector2 dir = {delta.x / dist, delta.y / dist};
//             Vector2 forceAtt = {dir.x * att, dir.y * att};

//             forces[u].x -= forceAtt.x;
//             forces[u].y -= forceAtt.y;
//             forces[v].x += forceAtt.x;
//             forces[v].y += forceAtt.y;
//         }

//         for (int i = 0; i < int(spnodes.size()); i++)
//         {
//             if (i == selectedspnode)
//                 continue;
//             spnodes[i].velocity.x = (spnodes[i].velocity.x + forces[i].x * timeStep) * damping;

            // spnodes[i].velocity.y = (spnodes[i].velocity.y + forces[i].y * timeStep) * damping;
            //             spnodes[i].position.x += spnodes[i].velocity.x * timeStep;
            //             spnodes[i].position.y += spnodes[i].velocity.y * timeStep;

            //             if (spnodes[i].position.x < spnodeRadius)
            //                 spnodes[i].position.x = spnodeRadius;
            //             if (spnodes[i].position.y < spnodeRadius)
            //                 spnodes[i].position.y = spnodeRadius;
            //             if (spnodes[i].position.x > screenWidth - spnodeRadius)
            //                 spnodes[i].position.x = screenWidth - spnodeRadius;
            //             if (spnodes[i].position.y > screenHeight - spnodeRadius)
            //                 spnodes[i].position.y = screenHeight - spnodeRadius;
            //         }
            //     }
            // }
