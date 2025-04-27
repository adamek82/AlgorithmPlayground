#pragma once

#include <vector>

/*
 * ClosestPairSolver.h
 *
 * Divide-and-conquer algorithm for the “closest pair of points” problem:
 *
 *   Given n points in the plane, find the two whose Euclidean distance is minimal.
 *
 * How it works:
 * 1. **Preprocessing**
 *    - Sort all points by x-coordinate → Px
 *    - Sort all points by y-coordinate → Py
 *
 * 2. **Divide**
 *    - Split Px into left half PxL and right half PxR at median index mid = n/2
 *    - Partition Py into Pyl and Pyr in O(n), preserving y-order, based on x < or ≥ median x
 *
 * 3. **Conquer** (recursively)
 *    - Compute closest pair in left half: δL = closestUtil(PxL, Pyl)
 *    - Compute closest pair in right half: δR = closestUtil(PxR, Pyr)
 *    - Let δ = min(δL, δR)
 *
 * 4. **Combine**
 *    - Build a “strip” of points from Py whose x-distance to median line < δ (O(n))
 *    - Scan the strip in y-order: for each point, compare only subsequent points whose y-difference < δ
 *      (geometric packing ⇒ at most a constant number of checks per point ⇒ O(n) total)
 *    - Take the best among left, right, and strip
 *
 * Recurrence:
 *   T(n) = 2 T(n/2) + O(n)
 * By the Master Theorem ⇒ T(n) = O(n log n).
 *
 * Space complexity: O(n) extra (for the Px, Py arrays and recursion overhead).
 */

/// A plain 2D point
struct Point {
    double x, y;
};

/// The result type: the closest pair plus their distance
struct PairDist {
    Point p1, p2;
    double dist;
};

/// Solver class providing a single public API for closest-pair
class ClosestPairSolver {
public:
    /// Find the closest pair among 'points' in O(n log n) time.
    /// Returns both points and their Euclidean distance.
    PairDist closestPair(std::vector<Point> points) const;

private:
    static PairDist bruteForce(const std::vector<Point>& P);
    static PairDist stripClosest(const std::vector<Point>& strip, double d);
    static PairDist closestUtil(std::vector<Point>& Px,
                                std::vector<Point>& Py);
    static double distance(const Point& a, const Point& b);
};