cmake_minimum_required(VERSION 3.30)
project(Lab_0x11h C)

set(CMAKE_C_STANDARD 11)

# Include the command that downloads libraries
include(FetchContent)

# define a function for adding git dependencies
function(include_dependency libName gitURL gitTag)
    FetchContent_Declare(${libName}
            GIT_REPOSITORY ${gitURL}
            GIT_TAG        ${gitTag}
            GIT_SHALLOW    TRUE
            GIT_PROGRESS   TRUE
    )
    FetchContent_MakeAvailable(${libName})
endfunction()

# add raylib support
set(LIB1 raylib)
find_package(${LIB1} QUIET)
if (NOT ${LIB1}_FOUND)
    message(STATUS "Getting ${LIB1} from Github")
    include_dependency(${LIB1} https://github.com/raysan5/raylib.git 5.5)
else()
    message(STATUS "Using local ${LIB1}")
endif()

add_executable(Lab_0x11h main.c
        item.h
        inventory.c
        inventory.h
        item.c
        display.c
)

# set the include directory
target_include_directories(Lab_0x11h PRIVATE ${raylib_INCLUDE_DIRS})

# link all libraries to the project
target_link_libraries(Lab_0x11h PRIVATE ${LIB1})

# Copy icons directory to build directory
file(COPY ${CMAKE_SOURCE_DIR}/icons DESTINATION ${CMAKE_BINARY_DIR})