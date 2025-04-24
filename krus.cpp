// #include<iostream>
// #include<vector>
// #include<algorithm>
// using namespace std;
// int find_pa(int u, vector<int> pa){
//     if(u == pa[u]) return u;
//     pa[u] = find_pa(pa[u], pa);
//     return pa[u];
// }
// typedef pair<int, int> ii;
// typedef pair<int, ii> wuv; 
// int main(){
//     int n, m;
//     cin >> n >> m;
//     vector<int> pa(n + 1);
//     for(int i = 0; i <= n; i++) pa[i] = i;
//     vector<wuv> edges;
//     for(int i = 1; i <= m; i++){
//         int u, v, w;
//         cin >> u >> v >> w;
//         edges.push_back({w, {u, v}});
//     }

//     sort(edges.begin(), edges.end());
//     int number_mst_edges = 0;
//     int current_weight_mst = 0;
//     for(wuv e: edges){
//         int w = e.first;
//         int u = e.second.first;
//         int v = e.second.second;
//         int pau = find_pa(u, pa);
//         int pav = find_pa(v, pa);
//         if(pau == pav) continue;
//         pa[pau] = pav;
//         number_mst_edges++;
//         current_weight_mst += w;
//         cout << u << " --- " << v << '\n';
//         if(number_mst_edges == n - 1) break;
//     }
//     cout << "Total weight of minimum spanning tree : " << current_weight_mst << '\n';
//     return 0;
// }
// /*
// input test:
// 7 8 
// 1 3 4
// 1 2 7 
// 1 5 1 
// 2 5 1
// 2 6 1
// 6 4 9
// 5 6 6
// 3 7 2
// */