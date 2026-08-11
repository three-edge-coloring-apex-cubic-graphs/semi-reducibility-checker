#pragma once
#include "kempe.hpp"
#include <cassert>
#include <cppcoro/generator.hpp>
#include <numeric>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

using std::pair;
using std::string;
using std::unordered_map;
using std::unordered_set;
using std::vector;

enum class ColorType { All };

struct Coloring {
    string str;
    Coloring(string str) : str(str) {}
    bool operator==(const Coloring &o) const { return str == o.str; }
    bool operator<(const Coloring &o) const { return str < o.str; }
    const string &StringOf() const { return str; }
    vector<int> VectorOf() const {
        vector<int> res;
        for (auto &c : str) {
            res.push_back(c - '0');
        }
        return res;
    }

    int sizeWithout(int c) const {
        int res = 0;
        for (auto d : str)
            res += (d != char(c + '0') ? 1 : 0);
        return res;
    }

    static set<Coloring> GetColorings(int n) {
        assert(n >= 1);
        set<Coloring> res;
        auto dfs = [&](auto &&dfs, int i, string &color) -> void {
            if (i == n) {
                Coloring coloring = Coloring(color);
                res.insert(coloring.GetLexicalMin(color));
                return;
            }
            for (char c = '1'; c <= '3'; c++) {
                color.push_back(c);
                dfs(dfs, i + 1, color);
                color.pop_back();
            }
            return;
        };
        string tmp = "1";
        dfs(dfs, 1, tmp);
        return res;
    }

    static Coloring GetLexicalMin(const string &str) {
        unordered_map<char, char> m;
        char a = '1';
        string res;
        for (auto &c : str) {
            if (!m.count(c)) {
                m[c] = a++;
            }
            res.push_back(m.at(c));
        }
        return Coloring(res);
    }
};

struct MultiBoundaryColoring {
    vector<Coloring> colorings;
    MultiBoundaryColoring(const vector<Coloring> &colorings) : colorings(colorings) {}
    bool operator==(const MultiBoundaryColoring &other) const {
        return colorings == other.colorings;
    }
    bool operator<(const MultiBoundaryColoring &other) const { return colorings < other.colorings; }

    bool compatible(const vector<int> &ring_sizes) const {
        if (colorings.size() != ring_sizes.size()) {
            return false;
        }
        for (size_t i = 0; i < colorings.size(); i++) {
            if (colorings[i].str.size() != (size_t)ring_sizes[i]) {
                return false;
            }
        }
        return true;
    }

    string StringOf() const {
        string res;
        for (const auto &coloring : colorings) {
            res += coloring.StringOf();
        }
        return res;
    }

    vector<int> VectorOf() const {
        vector<int> res;
        for (const auto &coloring : colorings) {
            auto v = coloring.VectorOf();
            res.insert(res.end(), v.begin(), v.end());
        }
        return res;
    }

    static MultiBoundaryColoring split(const string &str, const vector<int> &ring_sizes) {
        assert(std::accumulate(ring_sizes.begin(), ring_sizes.end(), 0) == (int)str.size());
        vector<Coloring> res;
        size_t offset = 0;
        for (size_t i = 0; i < ring_sizes.size(); i++) {
            res.push_back(Coloring(str.substr(offset, ring_sizes[i])));
            offset += ring_sizes[i];
        }
        return MultiBoundaryColoring(res);
    }

    vector<int> sizeWithout(int c) const {
        vector<int> sizes;
        for (const auto &coloring : colorings) {
            sizes.push_back(coloring.sizeWithout(c));
        }
        return sizes;
    }

    static MultiBoundaryColoring GetLexicalMin(vector<Coloring> &colorings) {
        unordered_map<char, char> m;
        char a = '1';
        for (auto &coloring : colorings) {
            for (auto &c : coloring.str) {
                if (!m.count(c)) {
                    m[c] = a++;
                }
                c = m.at(c);
            }
        }
        return MultiBoundaryColoring(colorings);
    }

    cppcoro::generator<MultiBoundaryColoring> GetKempeChanges(const MultiBoundaryKempe &kempe,
                                                              int fix) const {
        int kempe_size     = kempe.size();
        int num_singletons = kempe.num_singletons();
        assert((kempe_size - num_singletons) % 2 == 0);
        int num_matches = (kempe_size - num_singletons) / 2;
        assert(0 <= num_matches && num_matches < 64);
        assert(0 <= num_singletons && num_singletons < 64);
        for (unsigned long long dotBits = 0ull; dotBits < (1ull << num_singletons); dotBits++) {
            for (unsigned long long bits = 0ull; bits < (1ull << num_matches); bits++) {
                vector<Coloring> new_colorings = colorings;
                int dotIndex                   = 0;
                for (size_t i = 0; i < new_colorings.size(); i++) {
                    const string &kempe_str = kempe.kempes[i];
                    int matchIndex          = 0;
                    for (auto &c : new_colorings[i].str) {
                        if (c == fix + '0') continue;
                        if (kempe_str[matchIndex] != '.') {
                            int k = int(kempe_str[matchIndex] - 'a');
                            matchIndex++;
                            if (k < 0) continue;
                            if (bits & (1ULL << k)) {
                                c = '0' + ((c - '0') ^ fix);
                            }
                        } else {
                            if (dotBits & (1ULL << dotIndex)) {
                                c = '0' + ((c - '0') ^ fix);
                            }
                            matchIndex++;
                            dotIndex++;
                        }
                    }
                }
                assert(dotIndex == num_singletons);
                co_yield MultiBoundaryColoring::GetLexicalMin(new_colorings);
            }
        }
        co_return;
    }
};

namespace std {
template <> struct hash<MultiBoundaryColoring> {
    std::size_t operator()(const MultiBoundaryColoring &c) const {
        return std::hash<string>()(c.StringOf());
    }
};
} // namespace std