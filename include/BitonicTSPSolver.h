#pragma once

#include <vector>
#include "ClosestPairSolver.h"

// Class that encapsulates the bitonic TSP DP logic
class BitonicTSP {
private:
    // dp[i][j]: remaining bitonic-tour length when
    //  - one walker is at point index (i-1),
    //  - the other is at point index (j-1),
    //  - and all points 1..max(i,j) have already been visited.
    // Table is (N+1)x(N+1), 1-based.
    std::vector<std::vector<double>> dp;

    // Euclidean distance between two points
    inline double dist(const Point &p1, const Point &p2) const;

public:
    // Construct for N points
    explicit BitonicTSP(int N);

    // Compute the length of the shortest bitonic tour over pts[0..N-1]
    // pts may be unsorted; the function sorts by x internally.
    double solve(const std::vector<Point> &pts);
};