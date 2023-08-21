# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'RelWithDebInfo' as none was specified.")
  set(CMAKE_BUILD_TYPE
      RelWithDebInfo
      CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui, ccmake
  set_property(
    CACHE CMAKE_BUILD_TYPE
    PROPERTY STRINGS
             "Debug"
             "Release"
             "MinSizeRel"
             "RelWithDebInfo")
endif()

#generates compile_commands.json for use with clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_LTO
    "Enable Link Time Optimization (LTO), if supported."
    OFF)

if(ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(
        RESULT
        result
        OUTPUT
        output)
    if(result)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
    else()
        message(SEND_ERROR "LTO is not supported: ${output}")
    endif()
endif()