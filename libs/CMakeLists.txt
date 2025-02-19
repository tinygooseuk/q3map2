add_subdirectory(filematch)
add_subdirectory(l_net)
add_subdirectory(mathlib)
add_subdirectory(picomodel)

add_library(libs
        bytebool.cpp bytebool.h
        )

find_package(GLIB REQUIRED)
target_include_directories(libs PRIVATE ${GLIB_INCLUDE_DIRS})
target_link_libraries(libs PRIVATE ${GLIB_LIBRARIES})
