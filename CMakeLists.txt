# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("sff_sudoku_solver")

# Include sub-projects.
add_subdirectory ("sff_sudoku_solver")

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/hard_sudokus.txt
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})