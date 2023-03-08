cmake_minimum_required(VERSION 3.0.0)
project(iodevice VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(iodevice main.c)

find_package(PkgConfig)

pkg_check_modules(libevdev REQUIRED libevdev)
target_include_directories(iodevice PUBLIC ${libevdev_INCLUDE_DIRS})
target_link_libraries(iodevice PUBLIC ${libevdev_LIBRARIES})

pkg_check_modules(x11 REQUIRED x11)
target_include_directories(iodevice PUBLIC ${x11_INCLUDE_DIRS})
target_link_libraries(iodevice PUBLIC ${x11_LIBRARIES})

pkg_check_modules(glib REQUIRED glib-2.0)
target_include_directories(iodevice PUBLIC ${glib_INCLUDE_DIRS})
target_link_libraries(iodevice PUBLIC ${glib_LIBRARIES})

pkg_check_modules(lua REQUIRED lua)
target_include_directories(iodevice PUBLIC ${lua_INCLUDE_DIRS})
target_link_libraries(iodevice PUBLIC ${lua_LIBRARIES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
