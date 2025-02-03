# CMakeLists.txt project file for Raylib projects
# 
cmake_minimum_required(VERSION 3.30)
project(Snake C)

set(CMAKE_C_STANDARD 11)

# Include the command that downloads libraries
include(FetchContent)

# define a function for adding git dependencies
function(include_dependency libName gitURL gitTag)
    # setup the declare
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

add_executable(Snake main.c
        snake_interface.h
        snake_array.h
        snake_list.h
        snake_array.c
        snake_list.c)

# set the include directory
target_include_directories(Snake PRIVATE ${raylib_INCLUDE_DIRS})

# link all libraries to the project
target_link_libraries(Snake PRIVATE ${LIB1})