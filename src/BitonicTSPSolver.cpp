#include "BitonicTSPSolver.h"
#include <algorithm>
#include <cmath>

BitonicTSP::BitonicTSP(int N)
    : dp(N+1, std::vector<double>(N+1, 0.0)) {}

inline double BitonicTSP::dist(const Point &p1, const Point &p2) const {
    double dx = p1.x - p2.x;
    double dy = p1.y - p2.y;
    return std::sqrt(dx*dx + dy*dy);
}

double BitonicTSP::solve(const std::vector<Point> &pts) {
    int N = static_cast<int>(pts.size());
    // Make local copy and sort by x-coordinate
    std::vector<Point> a = pts;
    std::sort(a.begin(), a.end(),
              [](auto &A, auto &B){ return A.x < B.x; });

    // Base case: one walker at a[N-2], other at a[j-1], finish at a[N-1]
    for (int j = 1; j <= N - 2; ++j) {
        dp[N-1][j] = dist(a[N-2], a[N-1])
                   + dist(a[j-1], a[N-1]);
    }

    // Fill DP bottom-up: i=N-2..1, j=i..1
    for (int i = N - 2; i >= 1; --i) {
        for (int j = i; j >= 1; --j) {
            double optionA = dp[i+1][j] + dist(a[i-1], a[i]);
            double optionB = dp[i+1][i] + dist(a[j-1], a[i]);
            dp[i][j] = std::min(optionA, optionB);
        }
    }

    // Final answer: both start at a[0] -> dp[1][1]
    return dp[1][1];
}
