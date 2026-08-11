#pragma once
#include "generate_colorings.hpp"
#include "generate_kempes.hpp"
#include "kempe.hpp"
#include "multi_boundary_island.hpp"
#include <algorithm>
#include <boost/functional/hash.hpp>
#include <fmt/ranges.h>
#include <map>
#include <optional>
#include <spdlog/spdlog.h>
#include <string>

using std::map;
using std::string;
using std::unordered_map;

int OneReduction(int colorNum, const vector<MultiBoundaryColoring> &normalColorings,
                 vector<bool> &feasible,
                 const unordered_map<vector<int>, vector<MultiBoundaryKempe>,
                                     boost::hash<vector<int>>> &allKempes,
                 vector<vector<int>> &kempeIndexes,
                 const unordered_map<MultiBoundaryColoring, int> &coloringRev) {
    auto &newFeasible     = feasible;
    const auto isFeasible = feasible;
    int updateCount       = 0;
    for (int i = 0; i < colorNum; i++) {
        auto &colors     = normalColorings[i];
        auto varFeasible = isFeasible[i];
        if (!varFeasible) {
            bool someColorWorks = false;
            spdlog::debug("Checking for Coloring: {}", colors.StringOf());
            for (int fix = 1; fix <= 3; fix++) {
                spdlog::debug("Checking for fix = {}", fix);
                vector<int> withoutSize = colors.sizeWithout(fix);
                auto &kempes            = allKempes.at(withoutSize);
                bool everyKempeWorks    = true;
                auto &kempeIndex        = kempeIndexes[i][fix - 1];
                spdlog::trace("Checking {} kempe chains from {}", kempes.size(), kempeIndex);
                for (; kempeIndex < (int)kempes.size(); kempeIndex++) {
                    auto &kempe    = kempes[kempeIndex];
                    bool changable = false;
                    spdlog::trace("[{}/{}] {}", kempeIndex, kempes.size(),
                                  fmt::join(kempe.kempes, ", "));
                    int changedIndex = 0;
                    for (const auto &changedColor : colors.GetKempeChanges(kempe, fix)) {
                        spdlog::trace("[[{}/]] {}", changedIndex, changedColor.StringOf());
                        if (!coloringRev.count(changedColor)) {
                            spdlog::trace("{} does not exist in rev", changedColor.StringOf());
                        }
                        if (feasible[coloringRev.at(changedColor)]) {
                            changable = true;
                            break;
                        }
                        changedIndex++;
                    }
                    if (!changable) {
                        spdlog::debug("Failed on [[{}/{}]] {}", kempeIndex, kempes.size(),
                                      fmt::join(kempe.kempes, ", "));
                        everyKempeWorks = false;
                        break;
                    }
                }
                if (everyKempeWorks) {
                    spdlog::debug("Every kempe chain works!");
                    someColorWorks = true;
                    break;
                }
            }
            newFeasible[i] = someColorWorks;
            updateCount += someColorWorks ? 1 : 0;
        }
    }
    for (int i = 0; i < colorNum; i++) {
        auto &colors = normalColorings[i];
        if (newFeasible[i]) {
            spdlog::trace("{}: OK", colors.StringOf());
        } else {
            spdlog::trace("{}: NG", colors.StringOf());
        }
    }
    return updateCount;
}

vector<bool> CheckDReducibility(const MultiBoundaryIsland &island, KempeType type,
                                ColorType cType) {
    unordered_map<vector<int>, vector<MultiBoundaryKempe>, boost::hash<vector<int>>> allKempes =
        LoadAllKempeFile(island.ring_sizes, type);
    vector<MultiBoundaryColoring> normalColorings = LoadColorFile(island.ring_sizes);
    vector<bool> isFeasible                       = island.CheckColorability(normalColorings);
    int feasibleCount = std::count(isFeasible.begin(), isFeasible.end(), true);
    int colorNum      = normalColorings.size();
    std::unordered_map<MultiBoundaryColoring, int> coloringRev;
    for (int i = 0; i < colorNum; i++) {
        coloringRev[normalColorings[i]] = i;
    }
    vector<vector<int>> kempeIndexes(colorNum, vector<int>(3));
    int iterationCount = 0;
    spdlog::info("Started D-reducibility check");
    while (feasibleCount != colorNum) {
        spdlog::info("#{}: Feasible / Total: {} / {}", iterationCount + 1, feasibleCount, colorNum);
        int updateCount = OneReduction(colorNum, normalColorings, isFeasible, allKempes,
                                       kempeIndexes, coloringRev);
        feasibleCount += updateCount;
        if (updateCount == 0) {
            break;
        }
        iterationCount++;
    }
    spdlog::info("#{}: Feasible / Total: {} / {}", iterationCount + 1, feasibleCount, colorNum);
    return isFeasible;
}

vector<int> get_deleted_edges(const vector<bool> &exists) {
    vector<int> deleted_edges;
    for (int i = 0; i < (int)exists.size(); i++) {
        if (!exists[i]) {
            deleted_edges.push_back(i);
        }
    }
    return deleted_edges;
}

void CheckCReducibility(const MultiBoundaryIsland &island, const vector<bool> &feasible,
                        ColorType cType) {
    vector<MultiBoundaryColoring> colorings = LoadColorFile(island.ring_sizes);
    int colorNum                            = colorings.size();
    spdlog::info("Started C-reducibility check");
    bool isCReducible = false;

    auto existsList = island.GetDeletableEdgeSet();
    vector<pair<int, vector<bool>>> existsCount(existsList.size());
    std::transform(existsList.begin(), existsList.end(), existsCount.begin(),
                   [](const vector<bool> &v) {
                       return std::make_pair(std::count(v.begin(), v.end(), false), v);
                   });
    std::sort(existsCount.begin(), existsCount.end(),
              [](auto &v1, auto &v2) { return v1.first < v2.first; });

    spdlog::info("Trying {} possible deletions", existsCount.size());
    int count             = 0;
    int max_deletion_size = 0;
    for (auto &[deletion_size, exists] : existsCount) {
        if (max_deletion_size < deletion_size) {
            max_deletion_size = deletion_size;
            spdlog::info("[{}/{}] Starting deletion of size {}", count, existsCount.size(),
                         deletion_size);
        }
        auto contFeasible      = island.CheckColorability(colorings, exists);
        bool badColoringExists = false;
        for (int i = 0; i < colorNum; i++) {
            if (contFeasible[i]) {
                spdlog::trace("[{}/{}] {} -> {}", i, colorNum, colorings[i].StringOf(),
                              static_cast<bool>(feasible[i]));
                if (!feasible[i]) {
                    badColoringExists = true;
                    break;
                }
            }
        }
        if (badColoringExists) {
            spdlog::debug("Bad color exists");
        } else {
            vector<int> deleted_edges = get_deleted_edges(exists);
            spdlog::info("All colors passed! Deleted: {}", fmt::join(deleted_edges, ", "));
            isCReducible = true;
            break;
        }
        count++;
    }
    if (isCReducible) {
        spdlog::info("Graph is C-reducible!");
    } else {
        spdlog::info("Graph is not C-reducible.");
    }
}

void EvaluateConf(string islandfile, KempeType type, ColorType cType) {
    switch (type) {
    case KempeType::HalfChain:
        spdlog::info("Kempe type: HalfChain");
        break;
    }
    switch (cType) {
    case ColorType::All:
        spdlog::info("Color Type: All");
        break;
    }
    ifstream ifs(islandfile);
    if (!ifs) {
        spdlog::error("Failed to read {}", islandfile);
        return;
    }
    MultiBoundaryIsland island = MultiBoundaryIsland::fromFile(ifs);
    auto feasible              = CheckDReducibility(island, type, cType);
    bool isDReducible = std::all_of(feasible.begin(), feasible.end(), [](bool b) { return b; });
    if (isDReducible) {
        spdlog::info("Graph is D-reducible!");
    } else {
        CheckCReducibility(island, feasible, cType);
    }
}
