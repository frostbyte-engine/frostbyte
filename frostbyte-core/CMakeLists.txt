include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

if(EMSCRIPTEN)
  set(PLATFORM Web CACHE STRING "" FORCE)
  add_compile_options(-fexceptions)
  add_link_options(-fexceptions)
endif()

# Base dependencies

set(OPENSSL_USE_STATIC_LIBS FALSE)
find_package(OpenSSL REQUIRED)

find_package(unofficial-git2 CONFIG QUIET)
if(EMSCRIPTEN)
  find_path(LIBGIT2_INCLUDE_DIR git2.h REQUIRED)
  find_library(LIBGIT2_LIBRARY NAMES git2 REQUIRED)

  add_library(libgit2::libgit2 STATIC IMPORTED)
  set_target_properties(libgit2::libgit2 PROPERTIES
    IMPORTED_LOCATION "${LIBGIT2_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${LIBGIT2_INCLUDE_DIR}")
  set(GIT2_TARGET libgit2::libgit2)
elseif(TARGET unofficial::git2::git2)
  set(GIT2_TARGET unofficial::git2::git2)
else()
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(LIBGIT2 REQUIRED IMPORTED_TARGET libgit2)
  set(GIT2_TARGET PkgConfig::LIBGIT2)
endif()

# Luau
set(LUAU_BUILD_CLI      OFF CACHE BOOL "" FORCE)
set(LUAU_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(LUAU_BUILD_WEB      OFF CACHE BOOL "" FORCE)
set(LUAU_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(LUAU_STATIC_CRT     OFF CACHE BOOL "" FORCE)
FetchContent_Declare(luau
  GIT_REPOSITORY https://github.com/luau-lang/luau.git
  GIT_TAG        0.683
  GIT_SHALLOW    TRUE
  EXCLUDE_FROM_ALL)

# nlohmann/json
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
set(JSON_Install    OFF CACHE BOOL "" FORCE)
FetchContent_Declare(nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.11.3
  GIT_SHALLOW    TRUE)

# BLAKE3 (CMake project lives in the c/ directory)
if(EMSCRIPTEN)
  set(BLAKE3_SIMD_TYPE "none" CACHE STRING "" FORCE)
  set(BLAKE3_NO_SSE41  ON CACHE BOOL "" FORCE)
  set(BLAKE3_NO_AVX2   ON CACHE BOOL "" FORCE)
  set(BLAKE3_NO_AVX512 ON CACHE BOOL "" FORCE)
endif()
FetchContent_Declare(blake3
  GIT_REPOSITORY https://github.com/BLAKE3-team/BLAKE3.git
  GIT_TAG        1.8.2
  GIT_SHALLOW    TRUE
  SOURCE_SUBDIR  c)

# zstd (CMake project lives in build/cmake)
set(ZSTD_BUILD_STATIC   ON  CACHE BOOL "" FORCE)
set(ZSTD_BUILD_SHARED   OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_CONTRIB  OFF CACHE BOOL "" FORCE)
FetchContent_Declare(zstd
  GIT_REPOSITORY https://github.com/facebook/zstd.git
  GIT_TAG        v1.5.7
  GIT_SHALLOW    TRUE
  SOURCE_SUBDIR  build/cmake)

# curl
if(NOT EMSCRIPTEN)
  set(BUILD_CURL_EXE      OFF CACHE BOOL "" FORCE)
  set(BUILD_SHARED_LIBS   OFF CACHE BOOL "" FORCE)
  set(BUILD_STATIC_LIBS   ON  CACHE BOOL "" FORCE)
  set(BUILD_TESTING       OFF CACHE BOOL "" FORCE)
  set(CURL_USE_OPENSSL    ON  CACHE BOOL "" FORCE)
  set(CURL_USE_LIBPSL     OFF CACHE BOOL "" FORCE)
  set(CURL_USE_LIBSSH2    OFF CACHE BOOL "" FORCE)
  set(CURL_USE_LIBSSH     OFF CACHE BOOL "" FORCE)
  set(USE_LIBIDN2         OFF CACHE BOOL "" FORCE)
  set(USE_NGHTTP2         OFF CACHE BOOL "" FORCE)
  set(CURL_BROTLI         OFF CACHE BOOL "" FORCE)
  set(CURL_ZSTD           OFF CACHE BOOL "" FORCE)
  set(CURL_DISABLE_LDAP   ON  CACHE BOOL "" FORCE)
  set(HTTP_ONLY           ON  CACHE BOOL "" FORCE)
  FetchContent_Declare(curl
    GIT_REPOSITORY https://github.com/curl/curl.git
    GIT_TAG        curl-8_19_0
    GIT_SHALLOW    TRUE)
endif()

set(BASE_DEPS luau nlohmann_json blake3 zstd)
if(NOT EMSCRIPTEN)
  list(APPEND BASE_DEPS curl)
endif()

FetchContent_MakeAvailable(${BASE_DEPS})

set(BASE_SOURCES
  src/base64.cpp
  src/common.cpp
  src/console.cpp
  src/environment.cpp
  src/fontloader.cpp
  src/frostbyte.cpp
  src/frostbyteoption.cpp
  src/git.cpp
  src/http.cpp
  src/imageloader.cpp
  src/imguitheme.cpp
  src/libraries
  src/scriptlanguage.cpp
  src/sysutils.cpp
  src/taskscheduler.cpp
  src/tests.cpp
  src/ui
  src/userdata.cpp

  src/engine/classes/baseplayergui.cpp
  src/engine/classes/bindableevent.cpp
  src/engine/classes/camera.cpp
  src/engine/classes/coregui.cpp
  src/engine/classes/datamodel.cpp
  src/engine/classes/encodingservice.cpp
  src/engine/classes/guibutton.cpp
  src/engine/classes/guiobject.cpp
  src/engine/classes/guiservice.cpp
  src/engine/classes/httpservice.cpp
  src/engine/classes/instance.cpp
  src/engine/classes/layercollector.cpp
  src/engine/classes/player.cpp
  src/engine/classes/players.cpp
  src/engine/classes/runservice.cpp
  src/engine/classes/serviceprovider.cpp
  src/engine/classes/startergui.cpp
  src/engine/classes/textbox.cpp
  src/engine/classes/textservice.cpp
  src/engine/classes/textshared.cpp
  src/engine/classes/tweenbase.cpp
  src/engine/classes/tweenservice.cpp
  src/engine/classes/userinputservice.cpp
  src/engine/classes/workspace.cpp

  src/engine/classes/frostbyte/frostbyteoptions.cpp
  src/engine/classes/frostbyte/frostbyteservice.cpp
  src/engine/classes/frostbyte/imguiservice.cpp

  src/engine/datatypes/brickcolor.cpp
  src/engine/datatypes/color3.cpp
  src/engine/datatypes/colorsequence.cpp
  src/engine/datatypes/colorsequencekeypoint.cpp
  src/engine/datatypes/content.cpp
  src/engine/datatypes/enum.cpp
  src/engine/datatypes/font.cpp
  src/engine/datatypes/numberrange.cpp
  src/engine/datatypes/numbersequence.cpp
  src/engine/datatypes/numbersequencekeypoint.cpp
  src/engine/datatypes/path2dcontrolpoint.cpp
  src/engine/datatypes/random.cpp
  src/engine/datatypes/rbxscriptconnection.cpp
  src/engine/datatypes/rbxscriptsignal.cpp
  src/engine/datatypes/rect.cpp
  src/engine/datatypes/tweeninfo.cpp
  src/engine/datatypes/udim2.cpp
  src/engine/datatypes/udim.cpp
  src/engine/datatypes/vector2.cpp
  src/engine/datatypes/vector3.cpp

  src/libraries/cachelib.cpp
  src/libraries/cryptlib.cpp
  src/libraries/debuglib.cpp
  src/libraries/drawentrylib.cpp
  src/libraries/drawingimmediate.cpp
  src/libraries/filesystemlib.cpp
  src/libraries/instructionlib.cpp

  src/ui/drawentrylist.cpp
  src/ui/fontexplorer.cpp
  src/ui/functionexplorer.cpp
  src/ui/imageexplorer.cpp
  src/ui/instanceexplorer.cpp
  src/ui/tableexplorer.cpp
  src/ui/ui.cpp
)

# frostbytecore_headless: just base dependencies
add_library(frostbytecore_headless STATIC ${BASE_SOURCES})
add_library(frostbytecore::headless ALIAS frostbytecore_headless)
target_include_directories(frostbytecore_headless PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_compile_definitions(frostbytecore_headless PUBLIC FROSTBYTE_HEADLESS=1)
if(WIN32)
    target_compile_definitions(frostbytecore_headless PRIVATE WIN32_LEAN_AND_MEAN NOGDI NOUSER NOMINMAX)
endif()

set(BASE_LIBS
  Luau.VM
  Luau.VM.Internals
  Luau.Compiler
  nlohmann_json::nlohmann_json
  OpenSSL::Crypto
  BLAKE3::blake3
  ${GIT2_TARGET}
  libzstd_static)

if(NOT EMSCRIPTEN)
  list(APPEND BASE_LIBS CURL::libcurl)
endif()

target_link_libraries(frostbytecore_headless PUBLIC ${BASE_LIBS})
target_compile_features(frostbytecore_headless PUBLIC cxx_std_17)

# frostbytecore_desktop: base dependencies + raylib / imgui / rlImGui
if(FROSTBYTE_BUILD_DESKTOP)
  # raylib
  set(BUILD_EXAMPLES      OFF CACHE BOOL "" FORCE)
  set(BUILD_GAMES         OFF CACHE BOOL "" FORCE)
  set(CUSTOMIZE_BUILD     ON  CACHE BOOL "" FORCE)
  set(SUPPORT_SCREEN_CAPTURE OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
    GIT_SHALLOW    TRUE)

  # ImGui
  FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.92.9
    GIT_SHALLOW    TRUE)

  # rlImGui
  FetchContent_Declare(rlimgui
    GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
    GIT_TAG        main
    GIT_SHALLOW    TRUE
    SOURCE_SUBDIR  _no_cmake_here)

  # ImGuiFileDialog
  FetchContent_Declare(imguifiledialog
    GIT_REPOSITORY https://github.com/aiekick/ImGuiFileDialog.git
    GIT_TAG        v0.6.8
    GIT_SHALLOW    TRUE
    SOURCE_SUBDIR  _no_cmake_here)

  FetchContent_MakeAvailable(raylib imgui rlimgui imguifiledialog)

  add_library(rlimgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${rlimgui_SOURCE_DIR}/rlImGui.cpp)
  target_include_directories(rlimgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${rlimgui_SOURCE_DIR})
  target_link_libraries(rlimgui PUBLIC raylib)
  target_compile_features(rlimgui PUBLIC cxx_std_17)

  add_library(imguifiledialog STATIC
    ${imguifiledialog_SOURCE_DIR}/ImGuiFileDialog.cpp)
  target_include_directories(imguifiledialog PUBLIC ${imguifiledialog_SOURCE_DIR})
  target_link_libraries(imguifiledialog PUBLIC rlimgui) # for imgui headers
  target_compile_features(imguifiledialog PUBLIC cxx_std_17)

  set(DESKTOP_SOURCES ${BASE_SOURCES})
  # TODO: most of the desktop-specific code now just has nop implementations of things, but we should just have them be desktop-only sources
  list(APPEND DESKTOP_SOURCES 
    src/engine/classes/frostbyte/frostbytemainmenubar.cpp)
  add_library(frostbytecore_desktop STATIC ${DESKTOP_SOURCES})
  add_library(frostbytecore::desktop ALIAS frostbytecore_desktop)
  target_include_directories(frostbytecore_desktop PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
  target_compile_definitions(frostbytecore_desktop PUBLIC FROSTBYTE_DESKTOP=1)
if(WIN32)
    target_compile_definitions(frostbytecore_desktop PRIVATE WIN32_LEAN_AND_MEAN NOGDI NOUSER NOMINMAX)
endif()
  set(DESKTOP_LIBS ${BASE_LIBS})
  list(APPEND DESKTOP_LIBS 
    rlimgui
    imguifiledialog)
  target_link_libraries(frostbytecore_desktop PUBLIC ${DESKTOP_LIBS})
  target_compile_features(frostbytecore_desktop PUBLIC cxx_std_17)
endif()
