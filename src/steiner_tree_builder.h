/*******************************************************************************
 * Feel free to use, modify, and/or distribute this code as you see fit.
 ******************************************************************************/
#ifndef STEINER_TREE_BUILDER_H_
#define STEINER_TREE_BUILDER_H_

#include <vector>

#include "graph.h"

namespace steiner {

class SteinerTreeBuilder {
 public:
  // Constructors and destructor.
  SteinerTreeBuilder() = default;
  SteinerTreeBuilder(const SteinerTreeBuilder&) = delete;
  SteinerTreeBuilder& operator=(const SteinerTreeBuilder&) = delete;
  SteinerTreeBuilder(SteinerTreeBuilder&&) = delete;
  SteinerTreeBuilder& operator=(SteinerTreeBuilder&&) = delete;
  ~SteinerTreeBuilder() = default;

  // Solves the Steiner tree problem and returns the edges of the Steiner tree.
  std::vector<graph::Edge_i> Solve(const graph::Boundary_i& boundary,
                                   const std::vector<graph::Node_i>& nodes);
};

}  // namespace steiner

#endif  // STEINER_TREE_BUILDER_H_
