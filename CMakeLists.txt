cmake_minimum_required(VERSION 3.22)

set(PROJECT_NAME VolumetricRenderer)
project(${PROJECT_NAME} C CXX)

set(CMAKE_CXX_STANDARD 20)
set(WARNINGS_AS_ERRORS OFF)
set(ENABLE_PVS_STUDIO OFF)

set(ENABLE_UBSan OFF)
set(ENABLE_ASAN OFF)
set(ENABLE_TSan OFF)
set(ENABLE_MSAN OFF)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    add_compile_definitions(_DEBUG)
else()
    add_compile_definitions(NDEBUG)
endif()

include(cmake/CompilerWarnings.cmake)

set(INC ${CMAKE_SOURCE_DIR}/include)
set(SRC ${CMAKE_SOURCE_DIR}/source)
set(VND ${CMAKE_SOURCE_DIR}/vendor)

add_executable(${PROJECT_NAME}
        # Header only
        ${SRC}/Etna/Core/Environment.h

        # Source only
        ${SRC}/Etna/Core/EntryPoint.cpp
        ${SRC}/EtnaApplication.cpp

        # Header - Source
        ${SRC}/Etna/Core/Etna.h         ${SRC}/Etna/Core/Etna.cpp
        ${SRC}/Etna/Core/Clock.h        ${SRC}/Etna/Core/Clock.cpp
        ${SRC}/Etna/Core/Exception.h    ${SRC}/Etna/Core/Exception.cpp
        ${SRC}/Etna/Core/ImGuiTheme.h   ${SRC}/Etna/Core/ImGuiTheme.cpp

        # Layers
        ${SRC}/Etna/Layers/Canvas/Canvas.h     ${SRC}/Etna/Layers/Canvas/Canvas.cpp

        # Third-party
        ${VND}/loguru/loguru.hpp
        ${VND}/loguru/loguru.cpp

        ${VND}/imgui/imconfig.h
        ${VND}/imgui/imgui.h
        ${VND}/imgui/imgui_impl_glfw.h
        ${VND}/imgui/imgui_impl_vulkan.h
        ${VND}/imgui/imgui_internal.h
        ${VND}/imgui/imstb_rectpack.h
        ${VND}/imgui/imstb_textedit.h
        ${VND}/imgui/imstb_truetype.h
        ${VND}/imgui/imgui_impl_glfw.cpp
        ${VND}/imgui/imgui_impl_vulkan.cpp
        ${VND}/imgui/imgui.cpp
        ${VND}/imgui/imgui_demo.cpp
        ${VND}/imgui/imgui_draw.cpp
        ${VND}/imgui/imgui_tables.cpp
        ${VND}/imgui/imgui_widgets.cpp
)


target_include_directories(${PROJECT_NAME} PRIVATE ${SRC} ${INC} ${VND})

##########################################################
# Vendor
##########################################################

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)

target_include_directories(${PROJECT_NAME}
        PUBLIC ${Vulkan_INCLUDE_DIR}
        PUBLIC ${GLFW_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
        ${Vulkan_LIBRARIES}
        glfw
)

##########################################################
# Fixed CMakeLists.txt part
##########################################################

INSTALL(PROGRAMS
        $<TARGET_FILE:${PROJECT_NAME}>
        DESTINATION bin)

set(ALL_TARGETS ${PROJECT_NAME})

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

###
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
###

# Include CMake setup
include(cmake/main-config.cmake)