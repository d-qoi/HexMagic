
if(WIN32)
    cmake_minimum_required(VERSION 3.6) #for VS options
else()
    cmake_minimum_required(VERSION 2.6)
endif()

# Enable debug symbols by default, must be done before project() statement
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()
# (you can also set it on the command line: -D CMAKE_BUILD_TYPE=Release)

set(EXECUTABLE_NAME "hexmagic")
set (CMAKE_CXX_STANDARD 11)
project(${EXECUTABLE_NAME})

add_definitions ( -DEXEC_NAME="${EXECUTABLE_NAME}" )
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

add_subdirectory(libsoundio)
set(CMAKE_INSTALL_PREFIX ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

file(GLOB_RECURSE HEADERS "src/*.h")
file(GLOB_RECURSE SOURCES_CPP "src/*.cpp")
file(GLOB_RECURSE SOURCES_C "src/*.c")
file(GLOB_RECURSE RESOURCES "resources/*.frag" "resources/*.vert" "resources/*.glsl")

set( SOURCES ${SOURCES_CPP} ${SOURCES_C} )
add_executable(${EXECUTABLE_NAME} ${SOURCES} ${HEADERS} ${RESOURCES})

find_package( OpenGL REQUIRED )
set(LIBS ${OPENGL_gl_LIBRARY})

if(WIN32)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${EXECUTABLE_NAME})
    set(SFML_ROOT "C:/dev/SFML-2.4.2")
endif()

# Detect and add SFML
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})
find_package(SFML 2 REQUIRED network audio graphics window system)
if(SFML_FOUND)
  include_directories(${SFML_INCLUDE_DIR})
  set(LIBS ${LIBS} ${SFML_LIBRARIES} ${SFML_DEPENDENCIES})
endif()

if(WIN32)
add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${SFML_INCLUDE_DIR}/../bin"
        "${PROJECT_BINARY_DIR}" )
endif()

add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "resources"
    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/resources/")
    #"${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}")
    #"${PROJECT_BINARY_DIR}" )
    #"${RUNTIME_OUTPUT_DIRECTORY}" )
    #"Debug" )

if(UNIX AND NOT APPLE)
    set(LIBS ${LIBS} ${CMAKE_DL_LIBS})
endif()

if(APPLE)
    find_library(COREFOUNDATION CoreFoundation)
    set(LIBS ${LIBS} ${COREFOUNDATION})
endif()
include_directories(AFTER "${CMAKE_BINARY_DIR}/include/include" "${CMAKE_BINARY_DIR}/include")
target_link_libraries(${EXECUTABLE_NAME} ${LIBS})



