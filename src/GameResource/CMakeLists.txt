file(GLOB GameResourceSources *.h *.cpp)
add_library(GameResource ${GameResourceSources})
target_include_directories(GameResource INTERFACE ./)
target_link_libraries(GameResource PUBLIC imgui_lib)