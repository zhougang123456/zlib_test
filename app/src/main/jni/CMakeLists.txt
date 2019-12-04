# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

add_definitions(-D HAVE_CONFIG_H -D CMAKE_DEBUG)

find_library( # Defines the name of the path variable that stores the
              # location of the NDK library.
              log-lib

              # Specifies the name of the NDK library that
              # CMake needs to locate.
              log )

link_directories(
        ./lib
)

################################################################################
add_library(
        glz-debug
        SHARED
        ./src/glzencode.c
)

target_include_directories(glz-debug PUBLIC
        ./include
        )

target_link_libraries(
        glz-debug
        intl
        jpeg
        z
        #log
        jnigraphics
        ${log-lib}
)