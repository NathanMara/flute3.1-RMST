#include "steiner_tree_builder.h"

#include <vector>
#include <unordered_set>
#include <string>
#include <cassert>
#include <tuple>

#include "graph.h"
#include "flute.h"  // FLUTE header (from src/flute3)

namespace steiner {

// Alias FLUTE namespace for convenience
namespace Flute = ::Flute;

std::vector<graph::Edge_i> SteinerTreeBuilder::Solve(
    const graph::Boundary_i& /*boundary*/,
    const std::vector<graph::Node_i>& nodes) {

  std::vector<graph::Edge_i> edges;

  int n = static_cast<int>(nodes.size());
  if (n <= 1) return edges;

  // Initialize lookup tables once per program execution
  static bool lut_loaded = false;
  if (!lut_loaded) {
    Flute::readLUT();  // Loads POWV9.dat and POST9.dat from current working directory
    lut_loaded = true;
  }

  // Prepare coordinate arrays
  std::vector<int> x(n), y(n);
  for (int i = 0; i < n; ++i) {
    x[i] = nodes[i].x;
    y[i] = nodes[i].y;
  }

  // Compute the Steiner tree using FLUTE
  Flute::Tree tree = Flute::flute(n, x.data(), y.data(), FLUTE_D);

  // Use a set to avoid exact duplicate edges
  std::unordered_set<std::string> seen;

  int num_branches = 2 * tree.deg - 2;
  for (int i = 0; i < num_branches; ++i) {
    int j = tree.branch[i].n;

    graph::Node_i p1(tree.branch[i].x, tree.branch[i].y);
    graph::Node_i p2(tree.branch[j].x, tree.branch[j].y);

    if (p1.x == p2.x && p1.y == p2.y) continue;  // Skip self-loop edges

    auto make_key = [](const graph::Node_i& a, const graph::Node_i& b) {
      // Canonical lexicographic edge key
      if (std::tie(a.x, a.y) < std::tie(b.x, b.y)) {
        return std::to_string(a.x) + "," + std::to_string(a.y) + "-" +
               std::to_string(b.x) + "," + std::to_string(b.y);
      } else {
        return std::to_string(b.x) + "," + std::to_string(b.y) + "-" +
               std::to_string(a.x) + "," + std::to_string(a.y);
      }
    };

    // Add edges (split L-shapes)
    if (p1.x == p2.x || p1.y == p2.y) {
      std::string key = make_key(p1, p2);
      if (!seen.count(key)) {
        edges.emplace_back(p1, p2);
        seen.insert(key);
      }
    } else {
      graph::Node_i mid(p1.x, p2.y);

      std::string key1 = make_key(p1, mid);
      std::string key2 = make_key(mid, p2);

      if (!seen.count(key1)) {
        edges.emplace_back(p1, mid);
        seen.insert(key1);
      }
      if (!seen.count(key2)) {
        edges.emplace_back(mid, p2);
        seen.insert(key2);
      }
    }
  }

  return edges;
}

}  // namespace steiner
