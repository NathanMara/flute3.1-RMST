#include "steiner_tree_builder.h"

#include <vector>
#include <unordered_set>
#include <string>
#include <cassert>
#include <tuple>
#include <optional>
#include <functional>
#include <algorithm>

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

// Get list of all nodes strictly between p1 and p2 that exist in all_nodes
std::vector<graph::Node_i> get_nodes_between(const graph::Node_i& p1, const graph::Node_i& p2,
                                             const std::unordered_set<graph::Node_i>& all_nodes) {
  std::vector<graph::Node_i> nodes_between;

  if (p1.y == p2.y) {
    int y = p1.y;
    for (int x = p1.x + 1; x < p2.x; ++x) {
      if (all_nodes.count(graph::Node_i(x, y))) {
        nodes_between.emplace_back(x, y);
      }
    }
  } else if (p1.x == p2.x) {
    int x = p1.x;
    for (int y = p1.y + 1; y < p2.y; ++y) {
      if (all_nodes.count(graph::Node_i(x, y))) {
        nodes_between.emplace_back(x, y);
      }
    }
  }

  return nodes_between;
}

// Check if a new edge would overlap and return adjusted or split edges
std::vector<std::pair<graph::Node_i, graph::Node_i>>
resolve_overlap(const graph::Node_i& a, const graph::Node_i& b,
                const std::unordered_set<std::pair<graph::Node_i, graph::Node_i>, pair_hash>& seen,
                const std::unordered_set<graph::Node_i>& all_nodes) {
  
  graph::Node_i p1 = (std::tie(a.x, a.y) < std::tie(b.x, b.y)) ? a : b;
  graph::Node_i p2 = (std::tie(a.x, a.y) < std::tie(b.x, b.y)) ? b : a;

  // Check overlap against existing edges and split if necessary
  for (const auto& edge : seen) {
    graph::Node_i q1 = edge.first;
    graph::Node_i q2 = edge.second;
    if (std::tie(q1.x, q1.y) > std::tie(q2.x, q2.y)) std::swap(q1, q2);

    if (p1.x == p2.x && q1.x == q2.x && p1.x == q1.x) {
      // Vertical overlap
      int p_start = std::min(p1.y, p2.y);
      int p_end = std::max(p1.y, p2.y);
      int q_start = std::min(q1.y, q2.y);
      int q_end = std::max(q1.y, q2.y);

      if (!(p_end <= q_start || p_start >= q_end)) {
        // Overlapping intervals, split edge into disjoint parts excluding the overlapping region
        std::vector<std::pair<graph::Node_i, graph::Node_i>> parts;

        if (p_start < q_start) {
          auto first_part = resolve_overlap(p1, graph::Node_i(p1.x, q_start), seen, all_nodes);
          parts.insert(parts.end(), first_part.begin(), first_part.end());
        }
        if (p_end > q_end) {
          auto second_part = resolve_overlap(graph::Node_i(p1.x, q_end), p2, seen, all_nodes);
          parts.insert(parts.end(), second_part.begin(), second_part.end());
        }
        return parts;
      }
    }

    if (p1.y == p2.y && q1.y == q2.y && p1.y == q1.y) {
      // Horizontal overlap
      int p_start = std::min(p1.x, p2.x);
      int p_end = std::max(p1.x, p2.x);
      int q_start = std::min(q1.x, q2.x);
      int q_end = std::max(q1.x, q2.x);

      if (!(p_end <= q_start || p_start >= q_end)) {
        std::vector<std::pair<graph::Node_i, graph::Node_i>> parts;

        if (p_start < q_start) {
          auto first_part = resolve_overlap(p1, graph::Node_i(q_start, p1.y), seen, all_nodes);
          parts.insert(parts.end(), first_part.begin(), first_part.end());
        }
        if (p_end > q_end) {
          auto second_part = resolve_overlap(graph::Node_i(q_end, p1.y), p2, seen, all_nodes);
          parts.insert(parts.end(), second_part.begin(), second_part.end());
        }
        return parts;
      }
    }
  }

  // No overlap, split at intermediate nodes (all nodes between p1 and p2)
  std::vector<graph::Node_i> intermediate_nodes;
  if (p1.x == p2.x) {
    int x = p1.x;
    for (const auto& node : all_nodes) {
      if (node.x == x && node.y > p1.y && node.y < p2.y) {
        intermediate_nodes.push_back(node);
      }
    }
    std::sort(intermediate_nodes.begin(), intermediate_nodes.end(), [](const auto& n1, const auto& n2) {
      return n1.y < n2.y;
    });
  } else if (p1.y == p2.y) {
    int y = p1.y;
    for (const auto& node : all_nodes) {
      if (node.y == y && node.x > p1.x && node.x < p2.x) {
        intermediate_nodes.push_back(node);
      }
    }
    std::sort(intermediate_nodes.begin(), intermediate_nodes.end(), [](const auto& n1, const auto& n2) {
      return n1.x < n2.x;
    });
  } else {
    // Not axis aligned, return as is
    return { std::make_pair(p1, p2) };
  }

  // Build edges p1 -> intermediate nodes -> p2
  std::vector<std::pair<graph::Node_i, graph::Node_i>> result;
  graph::Node_i last = p1;
  for (const auto& node : intermediate_nodes) {
    if (!(last == node)) {
      result.emplace_back(last, node);
    }
    last = node;
  }
  if (!(last == p2)) {
    result.emplace_back(last, p2);
  }

  return result;
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
  std::unordered_set<graph::Node_i> all_nodes;

  for (int i = 0; i < 2 * tree.deg - 2; ++i) {
    all_nodes.emplace(tree.branch[i].x, tree.branch[i].y);
  }

  std::vector<std::pair<graph::Node_i, graph::Node_i>> diagonal_edges;

  int num_branches = 2 * tree.deg - 2;
  for (int i = 0; i < num_branches; ++i) {
    int j = tree.branch[i].n;

    graph::Node_i p1(tree.branch[i].x, tree.branch[i].y);
    graph::Node_i p2(tree.branch[j].x, tree.branch[j].y);

    if (p1 == p2) continue;

    if (p1.x == p2.x || p1.y == p2.y) {
      auto new_edges = resolve_overlap(p1, p2, seen, all_nodes);
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
    auto new_edges = resolve_overlap(a, b, seen, all_nodes);
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

    bool valid = (p1 != mid1 && mid1 != p2) &&
                  !resolve_overlap(p1, mid1, seen, all_nodes).empty() &&
                  !resolve_overlap(mid1, p2, seen, all_nodes).empty() &&
                  get_nodes_between(p1, mid1, all_nodes).empty() &&
                  get_nodes_between(mid1, p2, all_nodes).empty();

    if (valid) {
      try_add(p1, mid1);
      try_add(mid1, p2);
      all_nodes.insert(mid1);
    } else {
      try_add(p1, mid2);
      try_add(mid2, p2);
      all_nodes.insert(mid2);
    }
  }

  return edges;
}

}  // namespace steiner
