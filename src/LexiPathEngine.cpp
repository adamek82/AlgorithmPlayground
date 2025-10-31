#include "LexiPathEngine.h"
#include <iostream>

/* ========================= DynamicDirectedGraph ========================= */

DynamicDirectedGraph::DynamicDirectedGraph(int n_initial)
    : adj_(static_cast<std::size_t>(n_initial + 1))  // 1-based convenience
{
    edges_.reserve(1024);
    bucket_.reserve(1024);
}

void DynamicDirectedGraph::ensureNode(int x) {
    if (x < 0) return;
    std::size_t need = static_cast<std::size_t>(x) + 1;
    if (adj_.size() < need) {
        adj_.resize(need);
    }
}

int DynamicDirectedGraph::addEdge(int u, int v, int w) {
    ensureNode(u);
    ensureNode(v);
    int id = static_cast<int>(edges_.size());
    edges_.push_back(Edge{u, v, w, true});
    adj_[static_cast<std::size_t>(u)].push_back(id);
    bucket_[Key{u, v, w}].push_back(id);
    return id;
}

bool DynamicDirectedGraph::removeEdge(int u, int v, int w) {
    auto it = bucket_.find(Key{u, v, w});
    if (it == bucket_.end() || it->second.empty()) return false;
    int id = it->second.back();
    it->second.pop_back();
    edges_[static_cast<std::size_t>(id)].alive = false;
    return true;
}

const std::vector<int>& DynamicDirectedGraph::outEdges(int u) const {
    static const std::vector<int> kEmpty;
    if (u < 0 || static_cast<std::size_t>(u) >= adj_.size()) return kEmpty;
    return adj_[static_cast<std::size_t>(u)];
}

const DynamicDirectedGraph::Edge& DynamicDirectedGraph::edgeById(int id) const {
    return edges_[static_cast<std::size_t>(id)];
}

int DynamicDirectedGraph::nodeCapacity() const {
    return static_cast<int>(adj_.size()) - 1; // 1-based capacity
}

std::size_t DynamicDirectedGraph::edgeCount() const {
    return edges_.size();
}


/* =============================== LexiSSSP =============================== */

LexiSSSP::LexiSSSP(DynamicDirectedGraph& g, int S)
    : g_(g),
      S_(S),
      dist_(static_cast<std::size_t>(g.nodeCapacity() + 1), INF),
      bestMax_(static_cast<std::size_t>(g.nodeCapacity() + 1), std::numeric_limits<int>::max()),
      dirty_(true) // force first ASK to recompute
{}

void LexiSSSP::addEdgeCmd(int u, int v, int w) {
    g_.addEdge(u, v, w);
    growToInclude(std::max(u, v));
    dirty_ = true;
}

void LexiSSSP::removeEdgeCmd(int u, int v, int w) {
    if (g_.removeEdge(u, v, w)) {
        dirty_ = true;
    }
}

void LexiSSSP::touch() {
    dirty_ = true;
}

int LexiSSSP::ask(int t) {
    growToInclude(t);
    if (dirty_) recompute();
    return (dist_[static_cast<std::size_t>(t)] == INF)
           ? -1
           : bestMax_[static_cast<std::size_t>(t)];
}

void LexiSSSP::growToInclude(int x) {
    if (x < 0) return;
    g_.ensureNode(x); // keep graph consistent first
    std::size_t need = static_cast<std::size_t>(x) + 1;
    if (dist_.size() < need) {
        dist_.resize(need, INF);
        bestMax_.resize(need, std::numeric_limits<int>::max());
    }
}

void LexiSSSP::recompute() {
    // Ensure arrays cover current graph capacity (in case nodes were added).
    growToInclude(g_.nodeCapacity());

    std::fill(dist_.begin(), dist_.end(), INF);
    std::fill(bestMax_.begin(), bestMax_.end(), std::numeric_limits<int>::max());

    growToInclude(S_);
    std::priority_queue<PQItem> pq;
    dist_[static_cast<std::size_t>(S_)] = 0;
    bestMax_[static_cast<std::size_t>(S_)] = 0;
    pq.push(PQItem{0, 0, S_});

    while (!pq.empty()) {
        PQItem cur = pq.top(); pq.pop();

        // Drop stale entries: the label has been improved since this item was pushed.
        if (cur.dist != dist_[static_cast<std::size_t>(cur.v)] ||
            cur.bottleneck != bestMax_[static_cast<std::size_t>(cur.v)]) {
            continue;
        }

        // Relax outgoing edges
        for (int eid : g_.outEdges(cur.v)) {
            const auto& e = g_.edgeById(eid);
            if (!e.alive) continue;

            long long nd = cur.dist + static_cast<long long>(e.w);
            int nb = std::max(cur.bottleneck, e.w);

            std::size_t v_idx = static_cast<std::size_t>(e.v);
            if (nd < dist_[v_idx] || (nd == dist_[v_idx] && nb < bestMax_[v_idx])) {
                dist_[v_idx]    = nd;
                bestMax_[v_idx] = nb;
                pq.push(PQItem{nd, nb, e.v});
            }
        }
    }

    dirty_ = false;
}
