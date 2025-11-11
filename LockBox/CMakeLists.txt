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

# ---- macOS libsodium ----
if(APPLE)
    execute_process(
        COMMAND brew --prefix libsodium
        OUTPUT_VARIABLE SODIUM_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if (EXISTS "${SODIUM_PREFIX}/lib/libsodium.dylib")
        message(STATUS "✅ Linking libsodium from ${SODIUM_PREFIX}")
        target_include_directories(LockBox PRIVATE ${SODIUM_PREFIX}/include)
        target_link_directories(LockBox PRIVATE ${SODIUM_PREFIX}/lib)
        target_link_libraries(LockBox PRIVATE sodium)
    else()
        message(FATAL_ERROR "❌ libsodium not found. Please install it via: brew install libsodium")
    endif()
endif()

# ---- Include paths ----
target_include_directories(LockBox PRIVATE
    "${CMAKE_SOURCE_DIR}"
)

# ---- Link Qt + Sodium ----
target_link_libraries(LockBox PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
    sodium
)

set_target_properties(LockBox PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# ---- Finalize for Qt6 ----
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(LockBox)
endif()
