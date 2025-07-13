/*******************************************************************************
 * Feel free to use, modify, and/or distribute this code as you see fit.
 ******************************************************************************/
#ifndef FILE_IO_H_
#define FILE_IO_H_

#include <string>
#include <string_view>
#include <vector>

#include "graph.h"

namespace file_io {

// Reads the input file. Returns false if an error occurred.
bool ReadInputFile(std::string_view filename, graph::Boundary_i* boundary,
                   std::vector<graph::Node_i>* nodes);

// Writes the output file. Returns false if an error occurred.
bool WriteOutputFile(std::string_view filename,
                     const std::vector<graph::Edge_i>& edges);

}  // namespace file_io

#endif  // FILE_IO_H_
