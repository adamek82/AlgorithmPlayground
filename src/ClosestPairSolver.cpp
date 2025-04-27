#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include "ClosestPairSolver.h"

//------------------------------------------------------------------------------
// Public API: prepare sorted arrays and invoke the recursive routine.
//------------------------------------------------------------------------------
PairDist ClosestPairSolver::closestPair(std::vector<Point> points) const {
    size_t n = points.size();
    if (n < 2) {
        throw std::invalid_argument("Need at least two points");
    }

    // Build Px (sorted by x) and Py (sorted by y)
    std::vector<Point> Px = points;
    std::vector<Point> Py = points;
    std::sort(Px.begin(), Px.end(),
              [](const Point& a, const Point& b){ return a.x < b.x; });
    std::sort(Py.begin(), Py.end(),
              [](const Point& a, const Point& b){ return a.y < b.y; });

    return closestUtil(Px, Py);
}

//------------------------------------------------------------------------------
// Base-case brute-force for ≤ 3 points: O(1) work overall
//------------------------------------------------------------------------------
PairDist ClosestPairSolver::bruteForce(const std::vector<Point>& P) {
    size_t n = P.size();
    PairDist best{{0,0},{0,0}, std::numeric_limits<double>::infinity()};
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            double d = distance(P[i], P[j]);
            if (d < best.dist) {
                best = { P[i], P[j], d };
            }
        }
    }
    return best;
}

//------------------------------------------------------------------------------
// Scan the strip (sorted by y) in O(m), since each inner loop
// runs only while (y_j - y_i) < best.dist (constant # of iterations).
//------------------------------------------------------------------------------
PairDist ClosestPairSolver::stripClosest(const std::vector<Point>& strip,
                                         double d) {
    PairDist best{{0,0},{0,0}, d};
    size_t m = strip.size();
    for (size_t i = 0; i < m; ++i) {
        for (size_t j = i + 1;
             j < m && (strip[j].y - strip[i].y) < best.dist;
             ++j)
        {
            double dd = distance(strip[i], strip[j]);
            if (dd < best.dist) {
                best = { strip[i], strip[j], dd };
            }
        }
    }
    return best;
}

//------------------------------------------------------------------------------
// Recursive divide-and-conquer core: Px sorted by x, Py sorted by y
//------------------------------------------------------------------------------
PairDist ClosestPairSolver::closestUtil(std::vector<Point>& Px,
                                        std::vector<Point>& Py) {
    size_t n = Px.size();
    if (n <= 3) {
        return bruteForce(Px);
    }

    size_t mid = n / 2;
    Point midP = Px[mid];

    // Partition Py into points left/right of median, preserving y-order
    std::vector<Point> Pyl; Pyl.reserve(mid);
    std::vector<Point> Pyr; Pyr.reserve(n - mid);
    for (const auto& p : Py) {
        if (p.x < midP.x ||
            (p.x == midP.x && Pyl.size() < mid))
        {
            Pyl.push_back(p);
        } else {
            Pyr.push_back(p);
        }
    }

    // Split Px
    std::vector<Point> PxL(Px.begin(), Px.begin() + mid);
    std::vector<Point> PxR(Px.begin() + mid, Px.end());

    // Recurse on both halves
    PairDist leftRes  = closestUtil(PxL, Pyl);
    PairDist rightRes = closestUtil(PxR, Pyr);
    PairDist best     = (leftRes.dist < rightRes.dist ? leftRes : rightRes);

    // Build the strip of candidates within best.dist of the midline
    double d = best.dist;
    std::vector<Point> strip; strip.reserve(n);
    for (const auto& p : Py) {
        if (std::fabs(p.x - midP.x) < d) {
            strip.push_back(p);
        }
    }

    // Check the strip
    PairDist stripRes = stripClosest(strip, d);
    return (stripRes.dist < best.dist ? stripRes : best);
}

//------------------------------------------------------------------------------
// Euclidean distance helper
//------------------------------------------------------------------------------
double ClosestPairSolver::distance(const Point& a, const Point& b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return std::sqrt(dx*dx + dy*dy);
}