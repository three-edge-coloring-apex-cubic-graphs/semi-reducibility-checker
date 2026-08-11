#pragma once
#include <cassert>
#include <numeric>
#include <spdlog/spdlog.h>
#include <string>
#include <utility>
#include <vector>
using std::pair;
using std::string;
using std::vector;

struct Dart {
    int head;
    int rev;
    int succ;
    int pred;
    Dart(int head, int rev, int succ, int pred) : head(head), rev(rev), succ(succ), pred(pred) {}
};

vector<vector<int>> get_all_rotations(int vertex_deg2or3_size, int sum_ring_size,
                                      int pendant_edge_size, const vector<vector<int>> &rotations) {
    vector<vector<int>> rotations_including_degree1or2(vertex_deg2or3_size + sum_ring_size);
    for (size_t v = 0; v < rotations.size(); v++) {
        for (int i = 0; i < 3; i++) {
            if (sum_ring_size <= rotations[v][i] &&
                rotations[v][i] < sum_ring_size + pendant_edge_size) {
                continue; // remove pendant edges
            }
            rotations_including_degree1or2[v].push_back(rotations[v][i]);
        }
    }
    for (int i = 0; i < sum_ring_size; i++) {
        rotations_including_degree1or2[vertex_deg2or3_size + i].push_back(i);
    }
    return rotations_including_degree1or2;
}

pair<vector<Dart>, vector<int>> build_dart_representation(int vertex_size, int edge_size,
                                                          const vector<vector<int>> &rotations) {
    vector<vector<int>> EtoV(edge_size);
    for (int v = 0; v < vertex_size; v++) {
        for (int e : rotations[v]) {
            assert(0 <= e && e < edge_size);
            EtoV[e].push_back(v);
        }
    }
    vector<vector<int>> EV2dart(edge_size, vector<int>(vertex_size, -1));
    vector<int> dart2edge;
    dart2edge.reserve(2 * edge_size);
    int fresh_dart_id = 0;
    for (int e = 0; e < edge_size; e++) {
        assert(EtoV[e].size() == 2 || EtoV[e].size() == 0);
        for (size_t i = 0; i < EtoV[e].size(); i++) {
            int v         = EtoV[e][i];
            EV2dart[e][v] = fresh_dart_id++;
            dart2edge.push_back(e);
        }
    }
    vector<Dart> darts(dart2edge.size(), Dart(-1, -1, -1, -1));
    for (int v = 0; v < vertex_size; v++) {
        size_t d = rotations[v].size();
        for (size_t i = 0; i < d; i++) {
            int e                     = rotations[v][i];
            int u                     = EtoV[e][0] == v ? EtoV[e][1] : EtoV[e][0];
            int e_succ                = rotations[v][(i + 1) % d];
            int e_pred                = rotations[v][(i + d - 1) % d];
            darts[EV2dart[e][v]].head = v;
            darts[EV2dart[e][v]].rev  = EV2dart[e][u];
            darts[EV2dart[e][v]].succ = EV2dart[e_succ][v];
            darts[EV2dart[e][v]].pred = EV2dart[e_pred][v];
        }
    }
    return {darts, dart2edge};
}

vector<vector<int>> get_faces(int vertex_size, int edge_size,
                              const vector<vector<int>> &rotations) {
    auto [darts, dart2edge] = build_dart_representation(vertex_size, edge_size, rotations);
    vector<bool> visited(darts.size(), false);
    vector<vector<int>> faces;
    for (int e = 0; e < (int)darts.size(); e++) {
        if (visited[e]) continue;
        vector<int> face;
        int e_cur = e;
        do {
            visited[e_cur] = true;
            face.push_back(dart2edge[e_cur]);
            e_cur = darts[darts[e_cur].succ].rev;
        } while (e_cur != e);
        faces.push_back(face);
    }
    return faces;
}

vector<vector<int>> extract_inner_faces(int sum_ring_size, const vector<vector<int>> &faces) {
    vector<vector<int>> inner_faces;
    for (auto &face : faces) {
        bool is_inner_face = true;
        for (auto e : face) {
            if (e < sum_ring_size) {
                is_inner_face = false;
                break;
            }
        }
        if (is_inner_face) {
            inner_faces.push_back(face);
        }
    }
    return inner_faces;
}

vector<vector<int>> get_inner_faces(int vertex_deg2or3_size, int edge_size,
                                    const vector<int> &ring_sizes, int pendant_edge_size,
                                    const vector<vector<int>> &rotations) {
    int sum_ring_size = std::accumulate(ring_sizes.begin(), ring_sizes.end(), 0);
    vector<vector<int>> rotations_including_degree1or2 =
        get_all_rotations(vertex_deg2or3_size, sum_ring_size, pendant_edge_size, rotations);
    vector<vector<int>> faces =
        get_faces(vertex_deg2or3_size + sum_ring_size, edge_size, rotations_including_degree1or2);
    int V = vertex_deg2or3_size + sum_ring_size;
    int E = edge_size - pendant_edge_size;
    int F = faces.size();
    assert(V - E + F == 2); // Euler's formula
    return extract_inner_faces(sum_ring_size, faces);
}