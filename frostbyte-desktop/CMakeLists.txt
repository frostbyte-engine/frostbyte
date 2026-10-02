add_executable(frostbyte-desktop
  src/main.cpp
  src/imguitheme.cpp)
target_link_libraries(frostbyte-desktop PRIVATE frostbytecore::desktop)

target_link_options(frostbyte-desktop PRIVATE -static-libstdc++ -static-libgcc)
