#include "steiner_tree_builder.h"

#include <vector>
#include <unordered_set>
#include <string>
#include <cassert>
#include <tuple>
#include <functional>

#include "graph.h"
#include "flute.h"

namespace steiner {

namespace Flute = ::Flute;

// Hash for pair of nodes
struct pair_hash {
  std::size_t operator()(const std::pair<graph::Node_i, graph::Node_i>& p) const {
    auto h1 = std::hash<int>()(p.first.x) ^ (std::hash<int>()(p.first.y) << 1);
    auto h2 = std::hash<int>()(p.second.x) ^ (std::hash<int>()(p.second.y) << 1);
    return h1 ^ (h2 << 1);
  }
};

// Return canonical edge order (lexicographic)
std::pair<graph::Node_i, graph::Node_i> canonical(const graph::Node_i& a, const graph::Node_i& b) {
  return (std::tie(a.x, a.y) < std::tie(b.x, b.y)) ? std::make_pair(a, b) : std::make_pair(b, a);
}

// Check if a new edge would overlap with any existing edge in seen
bool is_overlapping(const graph::Node_i& a, const graph::Node_i& b,
                    const std::unordered_set<std::pair<graph::Node_i, graph::Node_i>, pair_hash>& seen) {
  graph::Node_i p1 = (std::tie(a.x, a.y) < std::tie(b.x, b.y)) ? a : b;
  graph::Node_i p2 = (std::tie(a.x, a.y) < std::tie(b.x, b.y)) ? b : a;

  for (const auto& edge : seen) {
    graph::Node_i q1 = edge.first;
    graph::Node_i q2 = edge.second;

    if (std::tie(q1.x, q1.y) > std::tie(q2.x, q2.y)) std::swap(q1, q2);

    if (p1 == p2 || q1 == q2) continue;  // Skip self-loops

    // Check horizontal overlap
    if (p1.y == p2.y && q1.y == q2.y && p1.y == q1.y) {
      int p_start = std::min(p1.x, p2.x);
      int p_end   = std::max(p1.x, p2.x);
      int q_start = std::min(q1.x, q2.x);
      int q_end   = std::max(q1.x, q2.x);
      if (!(p_end <= q_start || p_start >= q_end)) return true;
    }

    // Check vertical overlap
    if (p1.x == p2.x && q1.x == q2.x && p1.x == q1.x) {
      int p_start = std::min(p1.y, p2.y);
      int p_end   = std::max(p1.y, p2.y);
      int q_start = std::min(q1.y, q2.y);
      int q_end   = std::max(q1.y, q2.y);
      if (!(p_end <= q_start || p_start >= q_end)) return true;
    }
  }

  return false;
}

std::vector<graph::Edge_i> SteinerTreeBuilder::Solve(
    const graph::Boundary_i& /*boundary*/,
    const std::vector<graph::Node_i>& nodes) {

  std::vector<graph::Edge_i> edges;
  int n = static_cast<int>(nodes.size());
  if (n <= 1) return edges;

  static bool lut_loaded = false;
  if (!lut_loaded) {
    Flute::readLUT();
    lut_loaded = true;
  }

  std::vector<int> x(n), y(n);
  for (int i = 0; i < n; ++i) {
    x[i] = nodes[i].x;
    y[i] = nodes[i].y;
  }

  Flute::Tree tree = Flute::flute(n, x.data(), y.data(), FLUTE_D);
  std::unordered_set<std::pair<graph::Node_i, graph::Node_i>, pair_hash> seen;
  std::vector<std::pair<graph::Node_i, graph::Node_i>> diagonal_edges;

  int num_branches = 2 * tree.deg - 2;
  for (int i = 0; i < num_branches; ++i) {
    int j = tree.branch[i].n;

    graph::Node_i p1(tree.branch[i].x, tree.branch[i].y);
    graph::Node_i p2(tree.branch[j].x, tree.branch[j].y);

    if (p1 == p2) continue;  // skip degenerate edges

    if (p1.x == p2.x || p1.y == p2.y) {
      auto canon = canonical(p1, p2);
      if (!is_overlapping(canon.first, canon.second, seen)) {
        edges.emplace_back(canon.first, canon.second);
        seen.insert(canon);
      }
    } else {
      diagonal_edges.emplace_back(p1, p2);  // defer diagonals
    }
  }

  auto try_add = [&](const graph::Node_i& a, const graph::Node_i& b) {
    if (a == b) return;
    auto canon = canonical(a, b);
    if (!is_overlapping(canon.first, canon.second, seen)) {
      edges.emplace_back(canon.first, canon.second);
      seen.insert(canon);
    }
  };

  for (const auto& [p1, p2] : diagonal_edges) {
    graph::Node_i mid1(p1.x, p2.y);
    graph::Node_i mid2(p2.x, p1.y);

    bool valid1 = (p1 != mid1 && mid1 != p2) &&
                  !is_overlapping(p1, mid1, seen) &&
                  !is_overlapping(mid1, p2, seen);

    bool valid2 = (p1 != mid2 && mid2 != p2) &&
                  !is_overlapping(p1, mid2, seen) &&
                  !is_overlapping(mid2, p2, seen);

    if (valid1 || !valid2) {
      try_add(p1, mid1);
      try_add(mid1, p2);
    } else {
      try_add(p1, mid2);
      try_add(mid2, p2);
    }
  }

  return edges;
}

}  // namespace steiner
