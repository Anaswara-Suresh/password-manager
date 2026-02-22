cmake_minimum_required(VERSION 3.16)
project(LockBox VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------- Qt ----------------
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql Network WebSockets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql Network WebSockets)

# ---------------- OpenSSL ----------------
find_package(OpenSSL REQUIRED)

# ---- Source files ----
set(SOURCE_FILES
    "${CMAKE_SOURCE_DIR}/main.cpp"
    "${CMAKE_SOURCE_DIR}/mainwindow.cpp"
    "${CMAKE_SOURCE_DIR}/addpasswordpage.cpp"
    "${CMAKE_SOURCE_DIR}/database.cpp"
    "${CMAKE_SOURCE_DIR}/utils.cpp"
    "${CMAKE_SOURCE_DIR}/crypto.cpp"
    "${CMAKE_SOURCE_DIR}/loginwindow.cpp"
    "${CMAKE_SOURCE_DIR}/passwordlist.cpp"
    "${CMAKE_SOURCE_DIR}/autolockmanager.cpp"
    "${CMAKE_SOURCE_DIR}/hibpchecker.cpp"
    "${CMAKE_SOURCE_DIR}/autofillmanager.cpp"
    "${CMAKE_SOURCE_DIR}/ipcserver.cpp"
    "${CMAKE_SOURCE_DIR}/vaultsession.cpp"
    "${CMAKE_SOURCE_DIR}/vaultstate.cpp"
    "${CMAKE_SOURCE_DIR}/vault_exporter.cpp"
    "${CMAKE_SOURCE_DIR}/vault_importer.cpp"
    "${CMAKE_SOURCE_DIR}/totp.cpp"
    "${CMAKE_SOURCE_DIR}/totpdialog.cpp"
    "${CMAKE_SOURCE_DIR}/browserbridge/websocketserver.cpp"
    "${CMAKE_SOURCE_DIR}/browserbridge/pairingmanager.cpp"
    "${CMAKE_SOURCE_DIR}/browserbridge/clientsession.cpp"
    "${CMAKE_SOURCE_DIR}/browserbridge/aesgcm.cpp"
)

# ---- Header files ----
set(HEADER_FILES
    "${CMAKE_SOURCE_DIR}/mainwindow.h"
    "${CMAKE_SOURCE_DIR}/addpasswordpage.h"
    "${CMAKE_SOURCE_DIR}/database.h"
    "${CMAKE_SOURCE_DIR}/utils.h"
    "${CMAKE_SOURCE_DIR}/crypto.h"
    "${CMAKE_SOURCE_DIR}/loginwindow.h"
    "${CMAKE_SOURCE_DIR}/passwordlist.h"
    "${CMAKE_SOURCE_DIR}/autolockmanager.h"
    "${CMAKE_SOURCE_DIR}/hibpchecker.h"
    "${CMAKE_SOURCE_DIR}/vault_exporter.h"
    "${CMAKE_SOURCE_DIR}/vault_importer.h"
    "${CMAKE_SOURCE_DIR}/totp.h"
    "${CMAKE_SOURCE_DIR}/totpdialog.h"
    "${CMAKE_SOURCE_DIR}/browserbridge/websocketserver.h"
    "${CMAKE_SOURCE_DIR}/browserbridge/pairingmanager.h"
    "${CMAKE_SOURCE_DIR}/browserbridge/clientsession.h"
    "${CMAKE_SOURCE_DIR}/browserbridge/aesgcm.h"
)

# ---- UI files ----
set(UI_FILES
    "${CMAKE_SOURCE_DIR}/mainwindow.ui"
    "${CMAKE_SOURCE_DIR}/addpasswordpage.ui"
    "${CMAKE_SOURCE_DIR}/loginwindow.ui"
    "${CMAKE_SOURCE_DIR}/passwordlist.ui"
)

# ---- Define executable ----
qt_add_executable(LockBox
    MANUAL_FINALIZATION
    ${SOURCE_FILES}
    ${HEADER_FILES}
    ${UI_FILES}
)

# ---------------- libsodium ----------------
if(WIN32)
    set(SODIUM_ROOT "C:/libsodium/libsodium-win64")
    set(SODIUM_INCLUDE_DIR "${SODIUM_ROOT}/include")
    set(SODIUM_LIB_DIR "${SODIUM_ROOT}/lib")

    target_include_directories(LockBox PRIVATE ${SODIUM_INCLUDE_DIR})
    target_link_directories(LockBox PRIVATE ${SODIUM_LIB_DIR})
    target_link_libraries(LockBox PRIVATE libsodium)

elseif(APPLE)
    find_path(SODIUM_INCLUDE_DIR sodium.h PATHS /opt/homebrew/include /usr/local/include)
    find_library(SODIUM_LIBRARY sodium PATHS /opt/homebrew/lib /usr/local/lib)

    if(SODIUM_INCLUDE_DIR AND SODIUM_LIBRARY)
        target_include_directories(LockBox PRIVATE ${SODIUM_INCLUDE_DIR})
        target_link_libraries(LockBox PRIVATE ${SODIUM_LIBRARY})
    else()
        message(FATAL_ERROR "libsodium not found. Install with: brew install libsodium")
    endif()
endif()

# ---------------- Include headers ----------------
target_include_directories(LockBox PRIVATE "${CMAKE_SOURCE_DIR}")

# ---------------- Link libraries ----------------
target_link_libraries(LockBox PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::WebSockets
    OpenSSL::Crypto
)

# ---------------- Platform properties ----------------
set_target_properties(LockBox PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

configure_file(${CMAKE_SOURCE_DIR}/LockBox_Logo.png LockBox_Logo.png COPYONLY)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(LockBox)
endif()

# ---------------- Native Messaging Host ----------------
add_executable(lockbox_native_host
    "${CMAKE_SOURCE_DIR}/nativehost/main.cpp"
    "${CMAKE_SOURCE_DIR}/vaultstate.cpp"
)

target_include_directories(lockbox_native_host PRIVATE
    "${CMAKE_SOURCE_DIR}"
)

target_link_libraries(lockbox_native_host PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
)

set_target_properties(lockbox_native_host PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)