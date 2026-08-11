#pragma once
#include <algorithm>
#include <cassert>
#include <iostream>
#include <set>
#include <spdlog/spdlog.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
using std::cout;
using std::endl;
using std::reverse;
using std::set;
using std::string;
using std::unordered_map;
using std::unordered_set;
using std::vector;

inline unordered_set<string> GetValidParens(int n) {
    assert(n >= 1);
    if (n == 1) {
        return {"aa"};
    }
    unordered_set<string> res;
    for (int i = 1; i < n; i++) {
        auto ls = GetValidParens(i);
        auto rs = GetValidParens(n - i);
        for (auto &l : ls) {
            for (auto &r : rs) {
                string r_shifted = r;
                for (auto &c : r_shifted) {
                    c += i;
                }
                res.insert(l + r_shifted);
            }
        }
    }
    auto ms = GetValidParens(n - 1);
    for (auto &s : ms) {
        string s_shifted = s;
        for (auto &c : s_shifted) {
            c += 1;
        }
        res.insert("a" + s_shifted + "a");
    }
    return res;
}

inline void reassignDot(string &s) {
    unordered_map<char, int> nums;
    char a = 'a';
    for (auto &c : s) {
        if (c == '.') continue;
        if (nums.count(c)) {
        } else {
            nums[c] = a++;
        }
        c = nums[c];
    }
}

inline unordered_set<string> GetPlanarHalfKempes(int n) {
    if (n == 1) return {"."};
    unordered_set<string> res;
    unordered_set<string> alls;
    for (int p = n / 2; p >= 1; p--) {
        auto kempes = GetValidParens(p);
        for (auto &kempe : kempes) {
            string mask = string(2 * p, '+') + string(n - 2 * p, '.');
            do {
                bool dotdot = false;
                for (int i = 0; i < n; i++) {
                    if (mask[i] == '.' && mask[(i + 1) % n] == '.') {
                        dotdot = true;
                        break;
                    }
                }
                if (dotdot) continue;
                string fusedKempe{""};
                int j = 0;
                for (int i = 0; i < n; i++) {
                    if (mask[i] == '+') {
                        char c = kempe[j++];
                        fusedKempe.push_back(c);
                    } else {
                        fusedKempe.push_back('.');
                    }
                }
                reassignDot(fusedKempe);
                alls.insert(fusedKempe);
                spdlog::trace("kempe: {}, mask: {}, fused: {}, n: {}", kempe, mask, fusedKempe, n);
                bool redundantKempe = false;
                for (int i = 0; i < n; i++) {
                    for (int j = i + 1; j < n; j++) {
                        if (fusedKempe[i] == '.' && fusedKempe[j] == '.') {
                            string addedKempe = fusedKempe;
                            addedKempe[i]     = 'a' + n + 1;
                            addedKempe[j]     = 'a' + n + 1;
                            reassignDot(addedKempe);
                            if (alls.count(addedKempe)) {
                                redundantKempe = true;
                            }
                        }
                    }
                }
                if (!redundantKempe) {
                    res.insert(fusedKempe);
                }
            } while (std::next_permutation(mask.begin(), mask.end()));
        }
    }
    spdlog::debug("Size: {}, Generated: {}, Outputted: {}", n, alls.size(), res.size());
    return res;
}

struct MultiBoundaryKempe {
    vector<string> kempes;
    MultiBoundaryKempe(const vector<string> &kempes) : kempes(kempes) {}
    bool operator<(const MultiBoundaryKempe &other) const { return kempes < other.kempes; }

    static MultiBoundaryKempe merge(const MultiBoundaryKempe &a, const MultiBoundaryKempe &b) {
        int offset                  = a.num_matches();
        MultiBoundaryKempe b_offset = b.add_offset(offset);
        vector<string> res;
        res.insert(res.end(), a.kempes.begin(), a.kempes.end());
        res.insert(res.end(), b_offset.kempes.begin(), b_offset.kempes.end());
        return MultiBoundaryKempe(res);
    }

    int num_matches(void) const {
        set<char> matches;
        for (const string &s : kempes) {
            for (char c : s) {
                if (c != '.') {
                    matches.insert(c);
                }
            }
        }
        if (matches.empty()) return 0;
        const char last_match = static_cast<char>('a' + static_cast<int>(matches.size()) - 1);
        assert(*matches.begin() == 'a' && *matches.rbegin() == last_match);
        return matches.size();
    }

    int size(void) const {
        int n = 0;
        for (const string &s : kempes) {
            n += s.size();
        }
        return n;
    }

    int num_singletons(void) const {
        int n_singletons = 0;
        for (const string &s : kempes) {
            for (char c : s) {
                if (c == '.') {
                    n_singletons++;
                }
            }
        }
        return n_singletons;
    }

    MultiBoundaryKempe add_offset(char offset) const {
        vector<string> res;
        for (const string &s : kempes) {
            string new_s = s;
            for (char &c : new_s) {
                if (c != '.') {
                    c += offset;
                }
            }
            res.push_back(new_s);
        }
        return MultiBoundaryKempe(res);
    }
};
