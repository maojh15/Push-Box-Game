file(GLOB BFS_Solver_Sources *.cpp *.h)
add_library(BFS_Solver ${BFS_Solver_Sources})
target_include_directories(BFS_Solver INTERFACE ./)
target_link_libraries(BFS_Solver PRIVATE imgui_lib GameResource)