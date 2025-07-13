/*******************************************************************************
 * Feel free to use, modify, and/or distribute this code as you see fit.
 ******************************************************************************/
#include "file_io.h"

#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "graph.h"

namespace file_io {

bool ReadInputFile(std::string_view filename, graph::Boundary_i* boundary,
                   std::vector<graph::Node_i>* nodes) {
  // Open the input file.
  std::ifstream fin(filename.data());
  if (!fin.is_open()) {
    std::cerr << "Failed to open the input file: " << filename << "\n";
    return false;
  }

  // NOTE: This is a simple file parser. No error handling is implemented.
  // Make sure the input file is well-formed.
  // The input file format is as follows:
  // ---------------------------
  // [boundary_xl] [boundary_yh] [boundary_xh] [boundary_yh]
  // [node_count]
  // [x1] [y1]
  // [x2] [y2]
  // ...
  // [xn] [yn]
  // ---------------------------

  // Read the boundary.
  int min_x = 0, min_y = 0, max_x = 0, max_y = 0;
  fin >> min_x >> min_y >> max_x >> max_y;
  boundary->xl = min_x;
  boundary->yl = min_y;
  boundary->xh = max_x;
  boundary->yh = max_y;

  // Read the number of nodes.
  int num_nodes = 0;
  fin >> num_nodes;

  // Read the nodes.
  nodes->resize(num_nodes);
  for (int i = 0; i < num_nodes; ++i) {
    fin >> (*nodes)[i].x >> (*nodes)[i].y;
  }

  // Close the input file.
  fin.close();

  return true;
}

bool WriteOutputFile(std::string_view filename,
                     const std::vector<graph::Edge_i>& edges) {
  // Open the output file.
  std::ofstream fout(filename.data());
  if (!fout.is_open()) {
    return false;
  }

  // The output file format is as follows:
  // ---------------------------
  // [edge_count]
  // [e1_x1] [e1_y1] [e1_x2] [e1_y2]
  // [e2_x1] [e2_y1] [e2_x2] [e2_y2]
  // ...
  // [em_x1] [em_y1] [em_x2] [em_y2]
  // ---------------------------

  // Write the number of edges.
  fout << edges.size();

  // Write the edges.
  for (const graph::Edge_i& edge : edges) {
    fout << "\n"
         << edge.start.x << " " << edge.start.y << " " << edge.end.x << " "
         << edge.end.y;
  }

  // Close the output file.
  fout.close();

  return true;
}

}  // namespace file_io
