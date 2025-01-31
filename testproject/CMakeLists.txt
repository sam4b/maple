
add_executable(testproject "test.cpp")

target_link_libraries(testproject PRIVATE Maple)
target_link_libraries(testproject PRIVATE editor)