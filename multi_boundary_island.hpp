#pragma once
#include "coloring.hpp"
#include "embedding.hpp"
#include <cassert>
#include <concepts>
#include <fmt/ranges.h>
#include <fstream>
#include <iostream>
#include <numeric>
#include <queue>
#include <spdlog/spdlog.h>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
using std::cerr;
using std::endl;
using std::ifstream;
using std::pair;
using std::queue;
using std::string;
using std::vector;

vector<int> order_edges(int edge_size, const vector<int> &ring_sizes,
                        const vector<vector<pair<int, int>>> &EtoEE) {
    vector<int> n_adjacent(edge_size, 0);
    int sum_ring_size = std::accumulate(ring_sizes.begin(), ring_sizes.end(), 0);
    vector<bool> chosen(edge_size, false);
    for (int i = 0; i < sum_ring_size; i++) {
        chosen[i] = true;
        for (auto [f, g] : EtoEE[i]) {
            n_adjacent[f]++;
            n_adjacent[g]++;
        }
    }
    vector<int> order;
    for (int i = 0; i < edge_size - sum_ring_size; i++) {
        int max_adjacent = -1;
        int max_edge     = -1;
        for (int e = sum_ring_size; e < edge_size; e++) {
            if (chosen[e]) continue;
            if (n_adjacent[e] > max_adjacent) {
                max_adjacent = n_adjacent[e];
                max_edge     = e;
            }
        }
        assert(max_edge != -1);
        chosen[max_edge] = true;
        order.push_back(max_edge);
        for (auto [f, g] : EtoEE[max_edge]) {
            n_adjacent[f]++;
            n_adjacent[g]++;
        }
    }
    assert((int)order.size() == edge_size - sum_ring_size);
    return order;
}

struct MultiBoundaryIsland {
    const int vertex_deg2or3_size;
    const int edge_size;
    const vector<int> ring_sizes;
    const int pendant_edge_size;          // a pendant edge is incident to a vertex of degree 2
    vector<vector<pair<int, int>>> EtoEE; // edge -> (edge, edge)
    vector<vector<int>> inner_faces;
    vector<int> ordered_edges; // inner edges ordered to color effciently

    MultiBoundaryIsland(int vertex_deg2or3_size, int edge_size, const vector<int> &ring_sizes,
                        int pendant_edge_size, const vector<vector<int>> &rotations)
        : vertex_deg2or3_size(vertex_deg2or3_size), edge_size(edge_size), ring_sizes(ring_sizes),
          pendant_edge_size(pendant_edge_size) {
        assert(vertex_deg2or3_size == (int)rotations.size());
        EtoEE.resize(edge_size);
        for (size_t v = 0; v < rotations.size(); v++) {
            assert(rotations[v].size() == 3);
            const auto e0 = rotations[v][0];
            const auto e1 = rotations[v][1];
            const auto e2 = rotations[v][2];
            EtoEE[e0].emplace_back(e1, e2);
            EtoEE[e1].emplace_back(e2, e0);
            EtoEE[e2].emplace_back(e0, e1);
        }
        inner_faces = get_inner_faces(vertex_deg2or3_size, edge_size, ring_sizes, pendant_edge_size,
                                      rotations);
        for (size_t i = 0; i < inner_faces.size(); i++) {
            spdlog::debug("Inner face {}: {}", i, fmt::join(inner_faces[i], ", "));
        }
        ordered_edges = order_edges(edge_size, ring_sizes, EtoEE);
        spdlog::debug("Ordered edges: {}", fmt::join(ordered_edges, ", "));
    }

    static MultiBoundaryIsland fromFile(ifstream &ifs) {
        string line;
        // Line 1
        std::getline(ifs, line);
        std::istringstream iss1(line);
        int cubic_vertex_size;
        iss1 >> cubic_vertex_size;
        assert(cubic_vertex_size > 0);
        // Line 2
        std::getline(ifs, line);
        std::istringstream iss2(line);
        vector<int> ring_sizes;
        int ring_size;
        while (iss2 >> ring_size) {
            ring_sizes.push_back(ring_size);
            assert(ring_sizes.back() > 0);
        }
        // Line 3
        int pendant_edge_size;
        ifs >> pendant_edge_size;
        assert(pendant_edge_size >= 0);
        int sum_ring_size = std::accumulate(ring_sizes.begin(), ring_sizes.end(), 0);
        int edge_size     = (cubic_vertex_size * 3 - sum_ring_size - pendant_edge_size) / 2 +
                            sum_ring_size + pendant_edge_size;
        vector<vector<int>> rotations(cubic_vertex_size);
        for (int i = 0; i < cubic_vertex_size; i++) {
            for (int j = 0; j < 3; j++) {
                int e;
                ifs >> e;
                assert(0 <= e && e < edge_size);
                rotations[i].push_back(e);
            }
        }
        spdlog::info("Cubic vertex size: {}, Edge size: {}, Ring size: {}, Pendant edge size: {}",
                     cubic_vertex_size, edge_size, fmt::join(ring_sizes, ", "), pendant_edge_size);
        return MultiBoundaryIsland(cubic_vertex_size, edge_size, ring_sizes, pendant_edge_size,
                                   rotations);
    }

    bool CanColorWith(const MultiBoundaryColoring &colors, const vector<bool> &exists) const {
        assert(colors.compatible(ring_sizes));
        vector<int> color_vec = colors.VectorOf();
        color_vec.resize(edge_size, 0);
        bool res = color_dfs(0, color_vec, exists);
        return res;
    }

    bool color_dfs(size_t index, vector<int> &color_tmp, const vector<bool> &exists) const {
        if (index == ordered_edges.size()) {
            return true;
        }
        int e = ordered_edges[index];
        if (!exists[e]) {
            return color_dfs(index + 1, color_tmp, exists);
        }
        vector<int> isUsed(4, 0);
        for (auto [f, g] : EtoEE[e]) {
            assert(exists[f] || exists[g]); // vertex degree must not be 1
            if (exists[f] && exists[g]) {
                if (color_tmp[f] != 0) isUsed[color_tmp[f]]++;
                if (color_tmp[g] != 0) isUsed[color_tmp[g]]++;
            } else if (exists[f]) {
                if (color_tmp[f] != 0) {
                    for (int d = 1; d <= 3; d++) {
                        if (color_tmp[f] != d) {
                            isUsed[d]++;
                        }
                    }
                }
            } else if (exists[g]) {
                if (color_tmp[g] != 0) {
                    for (int d = 1; d <= 3; d++) {
                        if (color_tmp[g] != d) {
                            isUsed[d]++;
                        }
                    }
                }
            }
        }
        for (int c = 1; c <= 3; c++) {
            if (isUsed[c]) {
                continue;
            }
            color_tmp[e]   = c;
            bool colorable = color_dfs(index + 1, color_tmp, exists);
            color_tmp[e]   = 0;
            if (colorable) {
                return true;
            }
        }
        return false;
    }

    bool IsDeletable(const vector<bool> &boolExists, const vector<vector<int>> &EtoF) const {
        for (auto &face : inner_faces) {
            set<int> deletion;
            for (auto e : face) {
                if (!boolExists[e]) deletion.insert(e);
            }
            if (deletion.size() >= 3) return true;
        }
        for (int e = 0; e < edge_size; e++) {
            set<int> deletion;
            for (auto f : EtoF[e]) {
                for (auto g : inner_faces[f]) {
                    if (!boolExists[g]) deletion.insert(g);
                }
            }
            if (deletion.size() == 4) return true;
        }
        return false;
    }

    vector<vector<bool>> GetDeletableEdgeSetInternal(const vector<int8_t> &exists) const {
        vector<vector<int>> EtoF(edge_size);
        for (size_t f = 0; f < inner_faces.size(); f++) {
            for (auto e : inner_faces[f]) {
                EtoF[e].push_back(f);
            }
        }
        vector<vector<bool>> existsList;
        const int max_delete_count = 4;
        // exists: for each edge, -1 = unset, 0 = delete, 1 = keep
        // Assuming edge e has been updated, apply the condition that no degree-1 vertex exists
        auto place = [&](vector<int8_t> &exists, int e0) -> bool {
            assert(exists[e0] == 0 || exists[e0] == 1);
            queue<int> q;
            q.push(e0);
            while (!q.empty()) {
                int e = q.front();
                q.pop();
                for (auto [f, g] : EtoEE[e]) {
                    vector<int> unset;
                    int count1 = 0;
                    for (int h : {e, f, g}) {
                        if (exists[h] == 1) {
                            count1++;
                        } else if (exists[h] == -1) {
                            unset.push_back(h);
                        }
                    }
                    if (unset.size() == 0 && count1 == 1) {
                        return false;
                    } else if (unset.size() == 1 && count1 <= 1) {
                        exists[unset[0]] = count1;
                        q.push(unset[0]);
                    }
                }
            }
            return true;
        };
        auto recurse = [&](auto &&recurse, vector<int8_t> exists, int e) -> void {
            spdlog::trace("Checking edge {}", e);
            int delete_count = 0;
            for (auto val : exists)
                if (val == 0) delete_count += 1;
            if (delete_count > max_delete_count) {
                return; // prune
            }
            while (e < edge_size && exists[e] >= 0) {
                e++;
            }
            if (e == edge_size) {
                vector<bool> boolExists;
                std::transform(exists.begin(), exists.end(), std::back_inserter(boolExists),
                               [](int8_t elm) {
                                   assert(elm == 0 || elm == 1);
                                   return elm > 0;
                               });
                if ((0 < delete_count && delete_count < max_delete_count) ||
                    (delete_count == max_delete_count && IsDeletable(boolExists, EtoF))) {
                    existsList.push_back(boolExists);
                }
            } else {
                for (int8_t a : {0, 1}) {
                    vector<int8_t> exists2 = exists;
                    exists2[e]             = a;
                    if (place(exists2, e)) {
                        recurse(recurse, exists2, e + 1);
                    }
                }
            }
        };
        recurse(recurse, exists, 0);
        return existsList;
    }

    vector<vector<bool>> GetDeletableEdgeSet(void) const {
        vector<int8_t> exists(edge_size, -1);
        int sum_ring_size = std::accumulate(ring_sizes.begin(), ring_sizes.end(), 0);
        for (int i = 0; i < sum_ring_size + pendant_edge_size; i++)
            exists[i] = 1;
        return GetDeletableEdgeSetInternal(exists);
    }

    vector<bool> CheckColorability(const vector<MultiBoundaryColoring> &ringColorings) const {
        return CheckColorability(ringColorings, vector<bool>(edge_size, true));
    }

    vector<bool> CheckColorability(const vector<MultiBoundaryColoring> &ringColorings,
                                   const vector<bool> &exists) const {
        assert((int)exists.size() == edge_size);
        vector<bool> res;
        res.reserve(ringColorings.size());
        int feasibleCount = 0;
        for (auto &colors : ringColorings) {
            bool feasible = CanColorWith(colors, exists);
            res.push_back(feasible);
            if (feasible) {
                feasibleCount += 1;
                spdlog::trace("{}: OK", colors.StringOf());
            } else {
                spdlog::trace("{}: NG", colors.StringOf());
            }
        }
        spdlog::debug("Coloring result: {} / {}", feasibleCount, ringColorings.size());
        assert(feasibleCount > 0);
        return res;
    }
};
