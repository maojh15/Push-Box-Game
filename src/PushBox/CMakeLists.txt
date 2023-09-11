file(GLOB PushBoxSources *.h *.cpp)
add_library(PushBox ${PushBoxSources})
target_include_directories(PushBox INTERFACE ./)
target_link_libraries(PushBox PUBLIC GameResource BFS_Solver)