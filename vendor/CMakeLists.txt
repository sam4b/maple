
add_library(imgui-filebrowser STATIC "imfilebrowser.cpp" "imfilebrowser.hpp")
target_include_directories(imgui-filebrowser PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

find_package(imgui CONFIG REQUIRED)
target_link_libraries(imgui-filebrowser PRIVATE imgui::imgui)
