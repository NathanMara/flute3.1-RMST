#include "steiner_tree_builder.h"

#include <vector>
#include <unordered_set>
#include <string>
#include <cassert>
#include <tuple>
#include <iostream>  // for std::cout

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
  Flute::Tree tree = Flute::flute(n, x.data(), y.data(), 8);

  // Collect all points from tree (for split checking)
  std::vector<graph::Node_i> all_points;
  int num_branches = 2 * tree.deg - 2;
  for (int i = 0; i < num_branches; ++i) {
    all_points.emplace_back(tree.branch[i].x, tree.branch[i].y);
  }

  // Use a set to avoid exact duplicate edges
  std::unordered_set<std::string> seen;

  auto make_key = [](const graph::Node_i& a, const graph::Node_i& b) {
    if (std::tie(a.x, a.y) < std::tie(b.x, b.y)) {
      return std::to_string(a.x) + "," + std::to_string(a.y) + "-" +
             std::to_string(b.x) + "," + std::to_string(b.y);
    } else {
      return std::to_string(b.x) + "," + std::to_string(b.y) + "-" +
             std::to_string(a.x) + "," + std::to_string(a.y);
    }
  };

  for (int i = 0; i < num_branches; ++i) {
    int j = tree.branch[i].n;

    graph::Node_i p1(tree.branch[i].x, tree.branch[i].y);
    graph::Node_i p2(tree.branch[j].x, tree.branch[j].y);

    if (p1.x == p2.x && p1.y == p2.y) continue;

    if (p1.x == p2.x || p1.y == p2.y) {
      std::string key = make_key(p1, p2);
      if (!seen.count(key)) {

        // ✅ Check for points in-between p1 and p2 (on same line)
        for (const auto& pt : all_points) {
          if ((pt.x == p1.x && p1.x == p2.x) &&  // vertical
              pt.y > std::min(p1.y, p2.y) && pt.y < std::max(p1.y, p2.y)) {
            std::cout << "Point between (" << p1.x << "," << p1.y << ") and (" << p2.x << "," << p2.y
                      << "): (" << pt.x << "," << pt.y << ")\n";
          }
          if ((pt.y == p1.y && p1.y == p2.y) &&  // horizontal
              pt.x > std::min(p1.x, p2.x) && pt.x < std::max(p1.x, p2.x)) {
            std::cout << "Point between (" << p1.x << "," << p1.y << ") and (" << p2.x << "," << p2.y
                      << "): (" << pt.x << "," << pt.y << ")\n";
          }
        }

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
