cmake_minimum_required(VERSION 3.0.0)
project(iodevice VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(iodevice main.c)

find_package(PkgConfig)

PKG_SEARCH_MODULE(libevdev REQUIRED libevdev)
target_include_directories(iodevice PUBLIC ${libevdev_INCLUDE_DIRS})
target_link_libraries(iodevice PUBLIC ${libevdev_LIBRARIES})

PKG_SEARCH_MODULE(x11 REQUIRED x11)
target_include_directories(iodevice PUBLIC ${x11_INCLUDE_DIRS})
target_link_libraries(iodevice PUBLIC ${x11_LIBRARIES})

PKG_SEARCH_MODULE(xcb REQUIRED xcb)
target_include_directories(iodevice PUBLIC ${xcb_INCLUDE_DIRS})
target_link_libraries(iodevice PUBLIC ${xcb_LIBRARIES})

PKG_SEARCH_MODULE(xcb-icccm REQUIRED xcb-icccm)
target_include_directories(iodevice PUBLIC ${xcb-icccm_INCLUDE_DIRS})
target_link_libraries(iodevice PUBLIC ${xcb-icccm_LIBRARIES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
