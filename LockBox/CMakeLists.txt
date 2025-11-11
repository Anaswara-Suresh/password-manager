cmake_minimum_required(VERSION 3.16)
project(LockBox VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql)

set(SOURCE_FILES
    "${CMAKE_SOURCE_DIR}/main.cpp"
    "${CMAKE_SOURCE_DIR}/mainwindow.cpp"       # 🟢 Add this
    "${CMAKE_SOURCE_DIR}/addpasswordpage.cpp"
    "${CMAKE_SOURCE_DIR}/database.cpp"
    "${CMAKE_SOURCE_DIR}/utils.cpp"
    "${CMAKE_SOURCE_DIR}/crypto.cpp"
    "${CMAKE_SOURCE_DIR}/passwordlist.cpp"
)

set(HEADER_FILES
    "${CMAKE_SOURCE_DIR}/mainwindow.h"          # 🟢 Add this
    "${CMAKE_SOURCE_DIR}/addpasswordpage.h"
    "${CMAKE_SOURCE_DIR}/database.h"
    "${CMAKE_SOURCE_DIR}/utils.h"
    "${CMAKE_SOURCE_DIR}/crypto.h"
    "${CMAKE_SOURCE_DIR}/passwordlist.h"
)

set(UI_FILES
    "${CMAKE_SOURCE_DIR}/mainwindow.ui"         # 🟢 Add this
    "${CMAKE_SOURCE_DIR}/addpasswordpage.ui"
    "${CMAKE_SOURCE_DIR}/passwordlist.ui"
)

qt_add_executable(LockBox
    MANUAL_FINALIZATION
    ${SOURCE_FILES}
    ${HEADER_FILES}
    ${UI_FILES}
    style.qss
)

set(SODIUM_BASE_DIR "C:/libsodium-win64")

target_include_directories(LockBox PRIVATE
    "${CMAKE_SOURCE_DIR}/Header Files"
    "${SODIUM_BASE_DIR}/include"
)

target_link_directories(LockBox PRIVATE
    "${SODIUM_BASE_DIR}/lib"
)

target_link_libraries(LockBox PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
    libsodium.a
)

set_target_properties(LockBox PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(LockBox)
endif()
