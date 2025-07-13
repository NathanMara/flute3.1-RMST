/*******************************************************************************
 * Feel free to use, modify, and/or distribute this code as you see fit.
 ******************************************************************************/
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "file_io.h"
#include "graph.h"
#include "steiner_tree_builder.h"

int main(int argc, char** argv) {
  // Parse the command-line arguments.
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <input_file> <output_file>\n";
    return EXIT_FAILURE;
  }
  std::string_view input_file = argv[1];
  std::string_view output_file = argv[2];

  // Read the input file.
  graph::Boundary_i boundary;
  std::vector<graph::Node_i> nodes;
  if (!file_io::ReadInputFile(input_file, &boundary, &nodes)) {
    std::cerr << "Failed to read the input file: " << input_file << "\n";
    return EXIT_FAILURE;
  }

  // Run the Steiner tree algorithm.
  steiner::SteinerTreeBuilder builder;
  const std::vector<graph::Edge_i> edges = builder.Solve(boundary, nodes);

  // Write the output file.
  if (!file_io::WriteOutputFile(output_file, edges)) {
    std::cerr << "Failed to write the output file: " << output_file << "\n";
    return EXIT_FAILURE;
  }

  // Exit successfully.
  return EXIT_SUCCESS;
}
