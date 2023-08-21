cmake_minimum_required(VERSION 3.16)

project(
    houdini 
    LANGUAGES CXX)

include(cmake/StandardSettings.cmake)
include(cmake/HoudiniCompileOptions.cmake)

#link this dummy library to set the C++ standard/compile-time options
add_library(houdini_options INTERFACE)
target_compile_features(houdini_options INTERFACE cxx_std_17)

add_library(houdini_warnings INTERFACE)
include(cmake/CompilerWarnings.cmake)
set_project_warnings(houdini_warnings)

include(cmake/Sanitizers.cmake)
enable_sanitizers(houdini_options)

include(cmake/Doxygen.cmake)
enable_doxygen(${PROJECT_SOURCE_DIR})

include(cmake/StaticAnalyzers.cmake)

if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
option(ENABLE_BUILD_WITH_TIME_TRACE "Enable -ftime-trace to generate time tracing .json files on clang" OFF)
if(ENABLE_BUILD_WITH_TIME_TRACE)
target_compile_options(project_options INTERFACE -ftime-trace)
endif()
endif()

#disable exceptions and rtti, if needed
if(DISABLE_RTTI)
target_compile_options(houdini_options INTERFACE -fno-rtti)
target_compile_definitions(houdini_options INTERFACE RTTI_ENABLED)
endif()

if(DISABLE_EXCEPTIONS)
target_compile_options(houdini_options INTERFACE -fno-exceptions)
endif()
# add subdirectories

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
include(CTest)
endif()


#find_package(yaml_cpp_vendor REQUIRED)
find_package(Boost 1.71 REQUIRED)

add_library(
    houdini
    INTERFACE
)

target_include_directories(
    houdini
    INTERFACE
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_link_libraries(houdini 
    INTERFACE 
    Boost::boost
    houdini_options
    houdini_warnings
    )

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
add_subdirectory(test)
endif()

if(ENABLE_EXAMPLES)
message(
  "Building examples."
    )
    add_subdirectory(examples)
endif()

if(ENABLE_BENCHMARKS)
    message(
        "Building benchmarks."
    )
    add_subdirectory(benchmark)
endif()



#ament_target_dependencies(houdini PUBLIC yaml_cpp_vendor Boost)


# install(
#     DIRECTORY include/ 
#     DESTINATION include
# )


install(
    TARGETS ${PROJECT_NAME} houdini_options houdini_warnings
    EXPORT ${PROJECT_NAME}Targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION lib
)

