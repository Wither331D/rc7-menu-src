cmake_minimum_required(VERSION 3.20)
project(rc7_menu_auth LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(rc7_menu_auth WIN32
  src/main.cpp
  src/auth.cpp
  src/config.cpp
  src/menu.cpp
  src/theme.cpp
  src/rc7_assets.rc
)

add_executable(rc7_resource_dump
  src/resource_dump.cpp
)

target_include_directories(rc7_menu_auth PRIVATE src)
target_include_directories(rc7_resource_dump PRIVATE src)

target_compile_definitions(rc7_menu_auth PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
target_compile_definitions(rc7_resource_dump PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)

target_link_libraries(rc7_menu_auth PRIVATE
  ws2_32
  advapi32
  user32
  gdi32
  shell32
  comdlg32
)

target_link_libraries(rc7_resource_dump PRIVATE
  advapi32
  user32
)
