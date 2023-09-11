cmake_minimum_required(VERSION 3.12)
project(DEMO_IMGUI)
set(CMAKE_CXX_STANDARD 14)
# add_executable(main MACOSX_BUNDLE main.cpp)
add_executable(main main.cpp)

function(initialize_imgui_lib)
    #set(CMAKE_FRAMEWORK_PATH "/Library/Frameworks")
    set(IMGUI_DIR "/Users/maojiahao/cppLibraries/package/imgui-master")

    find_library(SDL2Fwk SDL2 REQUIRED)
    message(STATUS "found SDL2Fwk=${SDL2Fwk}")

    set(SDL2_INCLUDE_DIR "${SDL2Fwk}/Headers")
    message(${SDL2_INCLUDE_DIR})
    #set(SDL2_INCLUDE_DIR "/opt/homebrew/include/SDL2" "/opt/homebrew/include")
    set(BREW_LIBRARIES_DIR "/opt/homebrew/lib")

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
    target_link_libraries(imgui_lib PUBLIC "${SDL2Fwk}" "-framework OpenGL")
    #target_link_libraries(imgui_lib PUBLIC "-framework SDL2" "-framework OpenGL")    
endfunction(initialize_imgui_lib)

initialize_imgui_lib()

target_link_libraries(main PUBLIC imgui_lib)

add_subdirectory(src/GameResource)
add_subdirectory(src/BFS_Solver)
add_subdirectory(src/PushBox)

target_link_libraries(main PUBLIC PushBox)

install(TARGETS main
        DESTINATION "${CMAKE_SOURCE_DIR}/bin")

set(CUR_TARGET main)
set_target_properties(${CUR_TARGET} PROPERTIES
    BUNDLE True
    MACOSX_BUNDLE_GUI_IDENTIFIER my.domain.style.identifier.${CUR_TARGET}
    MACOSX_BUNDLE_BUNDLE_NAME ${CUR_TARGET}
    MACOSX_BUNDLE_BUNDLE_VERSION "0.1"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/cmake/customtemplate.plist.in
)