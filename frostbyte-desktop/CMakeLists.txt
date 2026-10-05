add_executable(frostbyte-desktop
  src/main.cpp
  src/imguitheme.cpp)
target_link_libraries(frostbyte-desktop PRIVATE frostbytecore::desktop)

target_link_options(frostbyte-desktop PRIVATE -static-libstdc++ -static-libgcc)
if(EMSCRIPTEN)
  if(DEFINED ENV{HOME})
    set(USER_HOME_DIR "$ENV{HOME}")
  else()
    set(USER_HOME_DIR "$ENV{USERPROFILE}")
  endif()

  set(ASSETS_DIR "${USER_HOME_DIR}/frostbyte/assets")
  if(NOT EXISTS "${ASSETS_DIR}")
    message(FATAL_ERROR "Ensure ${ASSETS_DIR} exists before building with emscripten.")
  endif()

  target_link_options(frostbyte-desktop PRIVATE
    "--preload-file=${ASSETS_DIR}@/home/web_user/frostbyte/assets"
    "--preload-file=${CMAKE_SOURCE_DIR}/luascripts@/home/web_user/frostbyte/luascripts"
    -sUSE_GLFW=3
    -sASYNCIFY
    -sASSERTIONS
    -fexceptions
    -sNO_DISABLE_EXCEPTION_CATCHING
    -sEXCEPTION_STACK_TRACES
    "-sEXPORTED_RUNTIME_METHODS=['getExceptionMessage']"
    -sALLOW_MEMORY_GROWTH=1)
endif()
