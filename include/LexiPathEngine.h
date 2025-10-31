#pragma once

#include <algorithm>
#include <cstdint>
#include <limits>
#include <queue>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

/*
 * Lexicographic Shortest Paths with Dynamic Updates
 *
 * Structure:
 *   - DynamicDirectedGraph: pure graph container (edges, adjacency, add/remove, growth).
 *   - LexiSSSP: query engine that computes lexicographic shortest paths from fixed source S,
 *               caching results and recomputing lazily after graph mutations.
 *
 * Problem:
 *   - Directed graph with non-negative weights, fixed source S.
 *   - Operations: ADD u v w, REM u v w, ASK t.
 *   - For ASK t: among all S->t paths minimize total sum; among those minimize
 *     the maximum edge weight on the path. Output that minimal "max edge"; -1 if unreachable.
 *
 * Approach:
 *   - Dijkstra with labels (dist, bottleneck), ordered lexicographically.
 *   - No decrease-key: push new labels; drop stale entries on pop.
 *   - Dirty flag: any mutation sets dirty=true; first subsequent ASK triggers recompute.
 *
 * Complexity:
 *   - Recompute: O((N+M) log N) with a binary heap.
 *   - ASK when not dirty: O(1).
 *   - Memory: O(N+M).
 */

class DynamicDirectedGraph {
public:
    struct Edge {
        int u;
        int v;
        int w;
        bool alive;  // true if edge currently exists
    };

    struct Key {
        int u, v, w;
        bool operator==(const Key& o) const noexcept {
            return u == o.u && v == o.v && w == o.w;
        }
    };

    struct KeyHash {
        std::size_t operator()(const Key& k) const noexcept {
            // Lightweight mixer (good enough for hash bucketing)
            std::uint64_t x = static_cast<std::uint64_t>(k.u) * 1000003ULL;
            std::uint64_t y = static_cast<std::uint64_t>(k.v) * 911ULL;
            std::uint64_t z = static_cast<std::uint64_t>(k.w);
            return static_cast<std::size_t>(x ^ y ^ z);
        }
    };

    // One-based indexing convenience: we allocate adj of size (n_initial + 1).
    explicit DynamicDirectedGraph(int n_initial = 0);

    // Ensure all internal arrays can index node x (1-based friendly).
    void ensureNode(int x);

    // Add a directed edge (u -> v) with weight w; returns its edge-id.
    int addEdge(int u, int v, int w);

    // Remove ONE existing edge (u -> v, w). Returns true if removed.
    bool removeEdge(int u, int v, int w);

    // Read-only accessors (used by the engine):
    const std::vector<int>& outEdges(int u) const;
    const Edge& edgeById(int id) const;

    // Utilities
    int nodeCapacity() const;         // current highest index the graph can address (1-based)
    std::size_t edgeCount() const;    // number of edges ever added (alive + removed)

private:
    std::vector<Edge> edges_;                         // all edges (stable ids)
    std::vector<std::vector<int>> adj_;              // adjacency lists by edge-id
    std::unordered_map<Key, std::vector<int>, KeyHash> bucket_;  // (u,v,w)->stack of edge-ids
};


class LexiSSSP {
public:
    // Priority-queue item for Dijkstra in lexicographic order
    struct PQItem {
        long long dist;   // total path sum
        int bottleneck;   // maximum edge weight along the path
        int v;            // node index

        // Priority: smaller dist first; tie -> smaller bottleneck
        bool operator<(const PQItem& o) const noexcept {
            if (dist != o.dist) return dist > o.dist;       // min-heap by dist
            return bottleneck > o.bottleneck;               // tie-breaker by bottleneck
        }
    };

    static constexpr long long INF = (1LL << 62);

    // The engine takes a reference to the graph and a fixed source S.
    explicit LexiSSSP(DynamicDirectedGraph& g, int S);

    // Convenient wrappers that mutate the graph AND mark the engine dirty.
    void addEdgeCmd(int u, int v, int w);
    void removeEdgeCmd(int u, int v, int w);

    // If you load initial edges directly into the graph, call `touch()` once
    // to force a recompute on first ASK.
    void touch();

    // Answer the query for node t: -1 if unreachable; else minimal bottleneck
    // among shortest (by sum) S->t paths.
    int ask(int t);

private:
    DynamicDirectedGraph& g_;
    int S_;

    // Cached labels after the last recompute
    std::vector<long long> dist_;   // dist_[v] = minimal total sum from S_ to v
    std::vector<int>       bestMax_;// bestMax_[v] = minimal bottleneck among paths with dist_[v]
    bool dirty_;

    // Ensure arrays can index node x (graph may grow after engine construction).
    void growToInclude(int x);

    // Full recompute from S_ using lexicographic Dijkstra.
    void recompute();
};
