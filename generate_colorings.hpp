#pragma once
#include "coloring.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;

void GenerateColorings(int max_size) {
    std::filesystem::create_directories("color/");
    for (int s = 1; s <= max_size; s++) {
        auto res      = Coloring::GetColorings(s);
        auto filename = "color/colors_" + std::to_string(s) + ".txt";
        ofstream ofs(filename);
        spdlog::info("Writing to {}", filename);
        ofs << res.size() << endl;
        for (auto &col : res) {
            ofs << col.StringOf() << endl;
        }
    }
}

vector<MultiBoundaryColoring> LoadColorFile(const vector<int> &ring_sizes) {
    vector<MultiBoundaryColoring> res;
    int sum_ring_size = std::accumulate(ring_sizes.begin(), ring_sizes.end(), 0);
    if (sum_ring_size == 0) {
        string str = "";
        res.push_back(MultiBoundaryColoring::split(str, ring_sizes));
        return res;
    }
    auto filename = "color/colors_" + std::to_string(sum_ring_size) + ".txt";
    ifstream ifs(filename);
    if (!ifs) {
        spdlog::critical("Error: Failed to open {}", filename);
        throw std::runtime_error("Error opening " + filename);
    }
    spdlog::debug("Reading from {}", filename);
    int len;
    ifs >> len;
    assert(len >= 0);
    for (int i = 0; i < len; i++) {
        string str;
        ifs >> str;
        res.push_back(MultiBoundaryColoring::split(str, ring_sizes));
    }
    return res;
}
