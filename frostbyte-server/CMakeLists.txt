set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)

add_executable(frostbyte-server
  src/main.cpp
  src/interface.cpp)
target_link_libraries(frostbyte-server PRIVATE frostbytecore::headless ${CURSES_LIBRARIES})
target_include_directories(frostbyte-server PRIVATE ${CURSES_INCLUDE_DIRS})
target_link_options(frostbyte-server PRIVATE -static-libstdc++ -static-libgcc)
