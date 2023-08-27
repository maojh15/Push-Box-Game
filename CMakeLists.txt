cmake_minimum_required(VERSION 3.12)
project(DEMO_IMGUI)
set(CMAKE_CXX_STANDARD 14)
add_executable(main main.cpp)

set(IMGUI_DIR "/Users/maojiahao/cppLibraries/package/imgui-master")
set(SDL2_INCLUDE_DIR "/opt/homebrew/include/SDL2" "/opt/homebrew/include")
set(BREW_LIBRARIES_DIR "/opt/homebrew/lib")

set(MY_CPP_INCLUDE_DIR "/Users/maojiahao/cppLibraries/include")

add_library(imgui_lib 
    "${IMGUI_DIR}/imgui.cpp" 
    "${IMGUI_DIR}/imgui_demo.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp")
target_include_directories(imgui_lib PUBLIC 
    ${SDL2_INCLUDE_DIR}
    "${IMGUI_DIR}"
     "${IMGUI_DIR}/backends")
target_link_directories(imgui_lib PUBLIC ${BREW_LIBRARIES_DIR})
target_link_libraries(imgui_lib PUBLIC SDL2 "-framework OpenGL")

target_link_libraries(main PUBLIC imgui_lib)

add_library(PushBox PushBox.cpp PushBox.h GameResource.cpp GameResource.h)
target_include_directories(PushBox PUBLIC ${MY_CPP_INCLUDE_DIR} ${SDL2_INCLUDE_DIR})
target_link_libraries(main PUBLIC PushBox)

install(TARGETS main
        DESTINATION "${CMAKE_SOURCE_DIR}/bin")