cmake_minimum_required(VERSION 3.16)
project(LockBox VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql)

# ---- Source files ----
set(SOURCE_FILES
    "${CMAKE_SOURCE_DIR}/main.cpp"
    "${CMAKE_SOURCE_DIR}/addpasswordpage.cpp"
    "${CMAKE_SOURCE_DIR}/database.cpp"
    "${CMAKE_SOURCE_DIR}/utils.cpp"
    "${CMAKE_SOURCE_DIR}/crypto.cpp"
    "${CMAKE_SOURCE_DIR}/loginwindow.cpp"
)

# ---- Header files ----
set(HEADER_FILES
    "${CMAKE_SOURCE_DIR}/addpasswordpage.h"
    "${CMAKE_SOURCE_DIR}/database.h"
    "${CMAKE_SOURCE_DIR}/utils.h"
    "${CMAKE_SOURCE_DIR}/crypto.h"
    "${CMAKE_SOURCE_DIR}/loginwindow.h"
)

# ---- UI files ----
set(UI_FILES
    "${CMAKE_SOURCE_DIR}/addpasswordpage.ui"
    "${CMAKE_SOURCE_DIR}/loginwindow.ui"
)

# ---- Define executable ----
qt_add_executable(LockBox
    MANUAL_FINALIZATION
    ${SOURCE_FILES}
    ${HEADER_FILES}
    ${UI_FILES}
)

# ---- Platform-specific libsodium ----
if(WIN32)
    # Windows setup
    set(SODIUM_ROOT "C:/libsodium/libsodium-win64")
    set(SODIUM_INCLUDE_DIR "${SODIUM_ROOT}/include")
    set(SODIUM_LIB_DIR "${SODIUM_ROOT}/lib")

    message(STATUS "Linking libsodium from ${SODIUM_ROOT}")

    target_include_directories(LockBox PRIVATE ${SODIUM_INCLUDE_DIR})
    target_link_directories(LockBox PRIVATE ${SODIUM_LIB_DIR})
    target_link_libraries(LockBox PRIVATE libsodium)

elseif(APPLE)
    # macOS setup (Homebrew)
    find_path(SODIUM_INCLUDE_DIR sodium.h PATHS /opt/homebrew/include /usr/local/include)
    find_library(SODIUM_LIBRARY sodium PATHS /opt/homebrew/lib /usr/local/lib)

    if(SODIUM_INCLUDE_DIR AND SODIUM_LIBRARY)
        message(STATUS "Found libsodium on macOS:")
        message(STATUS "  Include: ${SODIUM_INCLUDE_DIR}")
        message(STATUS "  Library: ${SODIUM_LIBRARY}")

        target_include_directories(LockBox PRIVATE ${SODIUM_INCLUDE_DIR})
        target_link_libraries(LockBox PRIVATE ${SODIUM_LIBRARY})
    else()
        message(FATAL_ERROR "libsodium not found on macOS. Please install via Homebrew: brew install libsodium")
    endif()
endif()

# ---- Include project headers ----
target_include_directories(LockBox PRIVATE "${CMAKE_SOURCE_DIR}")

# ---- Link Qt libraries ----
target_link_libraries(LockBox PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
)

# ---- Platform properties ----
set_target_properties(LockBox PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# ---- Finalize for Qt6 ----
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(LockBox)
endif()