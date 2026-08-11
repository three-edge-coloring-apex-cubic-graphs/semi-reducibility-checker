#pragma once
#include "kempe.hpp"
#include <boost/functional/hash.hpp>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <spdlog/spdlog.h>
#include <unordered_map>
using std::ifstream;
using std::map;
using std::ofstream;
using std::set;
using std::unordered_map;

enum class KempeType { HalfChain };

void GenerateKempes(int max_size) {
    std::filesystem::create_directories("kempes/half");
    for (int n = 1; n <= max_size; n++) {
        auto res      = GetPlanarHalfKempes(n);
        auto filename = "kempes/half/kempes_" + std::to_string(n) + ".txt";
        ofstream ofs(filename);
        spdlog::info("Writing to {}", filename);
        vector<string> resv(res.begin(), res.end());
        sort(resv.begin(), resv.end(), [](const string &s1, const string &s2) {
            for (size_t i = 0; i < s1.size(); i++) {
                if (s2.size() <= i) {
                    return false;
                }
                if (s1[i] != s2[i]) {
                    if (s1[i] == '.') return false;
                    if (s2[i] == '.') return true;
                    return s1[i] < s2[i];
                }
            }
            return false;
        });
        ofs << resv.size() << endl;
        for (auto &s : resv) {
            ofs << s << endl;
        }
    }
}

vector<MultiBoundaryKempe> LoadSingleKempeFile(int size, KempeType type) {
    assert(size >= 0);
    if (size == 0) {
        return {MultiBoundaryKempe({""})};
    }
    auto filename = fmt::format("kempes/half/kempes_{}.txt", size);
    ifstream ifs(filename);
    if (!ifs) {
        spdlog::critical("Error: Failed to open {}", filename);
        throw std::runtime_error("Failed to open " + filename);
    }
    spdlog::debug("Reading from {}", filename);
    int len;
    ifs >> len;
    assert(len >= 0);
    vector<MultiBoundaryKempe> res;
    for (int i = 0; i < len; i++) {
        string str;
        ifs >> str;
        res.push_back(MultiBoundaryKempe({str}));
    }
    return res;
}

unordered_map<vector<int>, vector<MultiBoundaryKempe>, boost::hash<vector<int>>>
LoadAllKempeFile(size_t index, const vector<int> &ring_sizes, KempeType type) {
    assert(index <= ring_sizes.size());
    if (index == ring_sizes.size()) {
        vector<int> empty_key;
        return {{empty_key, {MultiBoundaryKempe({})}}};
    }
    unordered_map<vector<int>, vector<MultiBoundaryKempe>, boost::hash<vector<int>>> res;
    auto suffixes = LoadAllKempeFile(index + 1, ring_sizes, type);
    assert(ring_sizes[index] >= 0);
    for (int s = 0; s <= ring_sizes[index]; s++) {
        auto current_kempes = LoadSingleKempeFile(s, type);
        for (const auto &[suffix_key, suffix_kempes] : suffixes) {
            vector<int> key;
            key.reserve(suffix_key.size() + 1);
            key.push_back(s);
            key.insert(key.end(), suffix_key.begin(), suffix_key.end());
            set<MultiBoundaryKempe> merged_kempes;
            for (const auto &current_kempe : current_kempes) {
                for (const auto &suffix_kempe : suffix_kempes) {
                    merged_kempes.insert(MultiBoundaryKempe::merge(current_kempe, suffix_kempe));
                }
            }
            res.insert(
                {key, vector<MultiBoundaryKempe>(merged_kempes.begin(), merged_kempes.end())});
        }
    }
    return res;
}

unordered_map<vector<int>, vector<MultiBoundaryKempe>, boost::hash<vector<int>>>
LoadAllKempeFile(const vector<int> &ring_sizes, KempeType type) {
    return LoadAllKempeFile(0, ring_sizes, type);
}
