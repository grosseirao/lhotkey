cmake_minimum_required(VERSION 3.0.0)
project(lhotkey VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(lhotkey main.c)

find_package(PkgConfig)

pkg_check_modules(libevdev REQUIRED libevdev)
target_include_directories(lhotkey PUBLIC ${libevdev_INCLUDE_DIRS})
target_link_libraries(lhotkey PUBLIC ${libevdev_LIBRARIES})

pkg_check_modules(x11 REQUIRED x11)
target_include_directories(lhotkey PUBLIC ${x11_INCLUDE_DIRS})
target_link_libraries(lhotkey PUBLIC ${x11_LIBRARIES})

pkg_check_modules(glib REQUIRED glib-2.0)
target_include_directories(lhotkey PUBLIC ${glib_INCLUDE_DIRS})
target_link_libraries(lhotkey PUBLIC ${glib_LIBRARIES})

pkg_check_modules(lua REQUIRED lua)
target_include_directories(lhotkey PUBLIC ${lua_INCLUDE_DIRS})
target_link_libraries(lhotkey PUBLIC ${lua_LIBRARIES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
