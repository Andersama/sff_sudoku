# CMakeList.txt : CMake project for lc_sudoku_solver, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (sff_sudoku_solver "sff_sudoku_solver.cpp" "sff_sudoku_solver.h")
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

# TODO: Add tests and install targets if needed.
