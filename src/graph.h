/*******************************************************************************
 * Feel free to use, modify, and/or distribute this code as you see fit.
 ******************************************************************************/
#ifndef GRAPH_H_
#define GRAPH_H_

#include <tuple>       // for std::tie
#include <functional>  // for std::hash

namespace graph {

// Boundary struct.
// A boundary is defined by the minimum and maximum x and y coordinates.
template <typename T>
struct Boundary {
  // Constructors.
  Boundary(T xl = 0, T yl = 0, T xh = 0, T yh = 0)
      : xl(xl), yl(yl), xh(xh), yh(yh) {}

  T xl;  // Lower bound of the x-coordinate.
  T yl;  // Lower bound of the y-coordinate.
  T xh;  // Upper bound of the x-coordinate.
  T yh;  // Upper bound of the y-coordinate.
};

// Node struct.
// Only 2D nodes are used in this assignment.
template <typename T>
struct Node {
  // Constructors.
  Node(T x = 0, T y = 0) : x(x), y(y) {}

  T x;  // x-coordinate.
  T y;  // y-coordinate.

  // Equality operators (needed for unordered_set and comparisons)
  bool operator==(const Node<T>& other) const {
    return x == other.x && y == other.y;
  }
  bool operator!=(const Node<T>& other) const {
    return !(*this == other);
  }

  // Lexicographic order (optional, useful for canonical edge comparisons)
  bool operator<(const Node<T>& other) const {
    return std::tie(x, y) < std::tie(other.x, other.y);
  }
};

// Edge struct.
// An edge connects two nodes.
template <typename T>
struct Edge {
  // Constructors.
  Edge(Node<T> start, Node<T> end) : start(start), end(end) {}

  Node<T> start;  // Start node.
  Node<T> end;    // End node.
};

// Define aliases for convenience.
// Only 'int' is used in this assignment.
using Boundary_i = Boundary<int>;
using Node_i = Node<int>;
using Edge_i = Edge<int>;

}  // namespace graph

// Hash specialization for graph::Node<int> (must be outside the namespace)
namespace std {
template <>
struct hash<graph::Node<int>> {
  std::size_t operator()(const graph::Node<int>& p) const {
    // Combine hashes for x and y
    return std::hash<int>()(p.x) ^ (std::hash<int>()(p.y) << 1);
  }
};
}

#endif  // GRAPH_H_
