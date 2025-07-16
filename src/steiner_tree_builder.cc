#include "steiner_tree_builder.h"

#include <vector>
#include <unordered_set>
#include <string>
#include <cassert>
#include <tuple>
#include <optional>
#include <functional>
#include <algorithm>  // for std::sort

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

// Check if a new edge would overlap and return adjusted edges or split edges if nodes in between
std::vector<std::pair<graph::Node_i, graph::Node_i>>
resolve_overlap(const graph::Node_i& a, const graph::Node_i& b,
                const std::unordered_set<std::pair<graph::Node_i, graph::Node_i>, pair_hash>& seen,
                const std::vector<graph::Node_i>& all_nodes) {
  graph::Node_i p1 = (std::tie(a.x, a.y) < std::tie(b.x, b.y)) ? a : b;
  graph::Node_i p2 = (std::tie(a.x, a.y) < std::tie(b.x, b.y)) ? b : a;

  // Check for overlapping existing edges (your existing logic)
  for (const auto& edge : seen) {
    graph::Node_i q1 = edge.first;
    graph::Node_i q2 = edge.second;
    if (std::tie(q1.x, q1.y) > std::tie(q2.x, q2.y)) std::swap(q1, q2);
    if (q1 == q2) continue;

    if (p1.y == p2.y && q1.y == q2.y && p1.y == q1.y) {
      int p_start = std::min(p1.x, p2.x);
      int p_end = std::max(p1.x, p2.x);
      int q_start = std::min(q1.x, q2.x);
      int q_end = std::max(q1.x, q2.x);
      if (!(p_end <= q_start || p_start >= q_end)) {
        if (p1.x >= q_start && p1.x < q_end && p2.x >= q_start && p2.x < q_end) return {};
        if (p1.x >= q_start && p1.x < q_end) {
          graph::Node_i new_p1(q_end, p1.y);
          if (new_p1 == p2) return {};
          return {std::make_pair(new_p1, p2)};
        }
        if (p2.x >= q_start && p2.x < q_end) {
          graph::Node_i new_p2(q_start - 1, p2.y);
          if (p1 == new_p2) return {};
          return {std::make_pair(p1, new_p2)};
        }
      }
    }
    if (p1.x == p2.x && q1.x == q2.x && p1.x == q1.x) {
      int p_start = std::min(p1.y, p2.y);
      int p_end = std::max(p1.y, p2.y);
      int q_start = std::min(q1.y, q2.y);
      int q_end = std::max(q1.y, q2.y);
      if (!(p_end <= q_start || p_start >= q_end)) {
        if (p1.y >= q_start && p1.y < q_end && p2.y >= q_start && p2.y < q_end) return {};
        if (p1.y >= q_start && p1.y < q_end) {
          graph::Node_i new_p1(p1.x, q_end);
          if (new_p1 == p2) return {};
          return {std::make_pair(new_p1, p2)};
        }
        if (p2.y >= q_start && p2.y < q_end) {
          graph::Node_i new_p2(p2.x, q_start - 1);
          if (p1 == new_p2) return {};
          return {std::make_pair(p1, new_p2)};
        }
      }
    }
  }

  // Now check if edge passes through any node in between endpoints, split if so
  std::vector<graph::Node_i> nodes_between;

  if (p1.y == p2.y) { // horizontal edge
    int y = p1.y;
    int x_start = p1.x;
    int x_end = p2.x;
    for (const auto& node : all_nodes) {
      if (node.y == y && node.x > x_start && node.x < x_end) {
        nodes_between.push_back(node);
      }
    }
  } else if (p1.x == p2.x) { // vertical edge
    int x = p1.x;
    int y_start = p1.y;
    int y_end = p2.y;
    for (const auto& node : all_nodes) {
      if (node.x == x && node.y > y_start && node.y < y_end) {
        nodes_between.push_back(node);
      }
    }
  }

  if (!nodes_between.empty()) {
    // Sort nodes_between to split edges in order
    std::sort(nodes_between.begin(), nodes_between.end(), [&](const graph::Node_i& n1, const graph::Node_i& n2) {
      if (p1.x == p2.x) return n1.y < n2.y;  // vertical line: sort by y
      else return n1.x < n2.x;               // horizontal line: sort by x
    });

    std::vector<std::pair<graph::Node_i, graph::Node_i>> split_edges;

    graph::Node_i last = p1;
    for (const auto& node : nodes_between) {
      if (last != node) {
        split_edges.emplace_back(last, node);
      }
      last = node;
    }
    if (last != p2) {
      split_edges.emplace_back(last, p2);
    }
    return split_edges;
  }

  // No splitting needed, return single edge
  return { std::make_pair(p1, p2) };
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

  Flute::Tree tree = Flute::flute(n, x.data(), y.data(), 9);
  std::unordered_set<std::pair<graph::Node_i, graph::Node_i>, pair_hash> seen;
  std::vector<std::pair<graph::Node_i, graph::Node_i>> diagonal_edges;

  int num_branches = 2 * tree.deg - 2;
  for (int i = 0; i < num_branches; ++i) {
    int j = tree.branch[i].n;

    graph::Node_i p1(tree.branch[i].x, tree.branch[i].y);
    graph::Node_i p2(tree.branch[j].x, tree.branch[j].y);

    if (p1 == p2) continue;

    if (p1.x == p2.x || p1.y == p2.y) {
      // resolve_overlap now returns vector of edges
      auto new_edges = resolve_overlap(p1, p2, seen, nodes);
      for (const auto& e : new_edges) {
        auto canon = canonical(e.first, e.second);
        if (seen.find(canon) == seen.end()) {
          edges.emplace_back(canon.first, canon.second);
          seen.insert(canon);
        }
      }
    } else {
      diagonal_edges.emplace_back(p1, p2);
    }
  }

  auto try_add = [&](const graph::Node_i& a, const graph::Node_i& b) {
    auto new_edges = resolve_overlap(a, b, seen, nodes);
    for (const auto& e : new_edges) {
      auto canon = canonical(e.first, e.second);
      if (seen.find(canon) == seen.end()) {
        edges.emplace_back(canon.first, canon.second);
        seen.insert(canon);
      }
    }
  };

  for (const auto& [p1, p2] : diagonal_edges) {
    graph::Node_i mid1(p1.x, p2.y);
    graph::Node_i mid2(p2.x, p1.y);

    bool valid1 = (p1 != mid1 && mid1 != p2) &&
                  !resolve_overlap(p1, mid1, seen, nodes).empty() &&
                  !resolve_overlap(mid1, p2, seen, nodes).empty();

    if (valid1) {
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
