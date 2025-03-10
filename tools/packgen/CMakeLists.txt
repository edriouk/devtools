cmake_minimum_required(VERSION 3.14)

include(DumpCMakeProjectVersion)
include(ProjectVersionFromGitTag)
get_version_from_git_tag("tools/packgen/")

project(PackGen VERSION "${PROJECT_VERSION}")
dump_cmake_project_version()

configure_file(src/ProductInfo.h.in ProductInfo.h)

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT packgen)

# packgen library
add_library(packgenlib OBJECT src/PackGen.cpp include/PackGen.h)
target_link_libraries(packgenlib PUBLIC CrossPlatform RteFsUtils XmlTree XmlTreeSlim cxxopts yaml-cpp)
target_include_directories(packgenlib PRIVATE include ${PROJECT_BINARY_DIR})


# packgen target
add_executable(packgen src/PackGenMain.cpp include/PackGen.h)
if(MSVC)
  target_link_options(packgen PUBLIC "$<$<CONFIG:Debug>:/SAFESEH:NO>")
endif(MSVC)
set_property(TARGET packgen PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
target_link_libraries(packgen packgenlib)
target_include_directories(packgen PRIVATE include)

# packgen test
enable_testing()
add_subdirectory(test)