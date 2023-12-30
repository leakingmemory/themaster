cmake_minimum_required(VERSION 3.27)
project(drwhatsnot)

set(CMAKE_CXX_STANDARD 23)

find_package(wxWidgets REQUIRED COMPONENTS core base webview)
include(${wxWidgets_USE_FILE})

find_package(cpprestsdk REQUIRED)

string(REPLACE "/" "\\\\/" PREFIX_ESCAPED "${CMAKE_INSTALL_PREFIX}")

add_custom_target("InstallPrefix.h"
        ALL "cat" "${CMAKE_CURRENT_SOURCE_DIR}/InstallPrefix.h.in" | sed -e "s/PREFIX/${PREFIX_ESCAPED}/" > "${CMAKE_CURRENT_BINARY_DIR}/InstallPrefix.h"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/InstallPrefix.h.in"
)

add_library(crypto OBJECT sha2alg.cpp sha2alg.h PkceS256.cpp PkceS256.h OpensslRsa.cpp OpensslRsa.h
        Openssl.cpp Openssl.h  JwkPemRsaKey.cpp JwkPemRsaKeyJson.cpp JwkPemRsaKey.h Base64.cpp Base64.h)

add_executable(themaster TheMasterApp.cpp TheMasterApp.h TheMasterFrame.cpp TheMasterFrame.h ConnectDialog.cpp ConnectDialog.h
    FindPatientDialog.cpp FindPatientDialog.h PatientStoreInMemory.cpp PatientStoreInMemory.h
    CreatePatientDialog.cpp CreatePatientDialog.h WaitingForApiDialog.cpp WaitingForApiDialog.h
    HelseidLoginDialog.cpp HelseidLoginDialog.h HelseidAuthorization.cpp HelseidAuthorization.h)
target_link_libraries(themaster PRIVATE crypto)
target_link_libraries(themaster PRIVATE -lsfmbasisapi)
target_link_libraries(themaster PRIVATE cpprestsdk::cpprest -lcrypto)
target_link_libraries(themaster PRIVATE ${wxWidgets_LIBRARIES})
target_include_directories(themaster PRIVATE ${wxWidgets_INCLUDE})
target_include_directories(themaster PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(themaster "InstallPrefix.h")

add_custom_target(
        "TheMaster.desktop"
        ALL "cat" "${CMAKE_CURRENT_SOURCE_DIR}/TheMaster.desktop" | sed -e "s/PREFIX/${PREFIX_ESCAPED}/" > "${CMAKE_CURRENT_BINARY_DIR}/TheMaster.desktop"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/TheMaster.desktop"
)

install(TARGETS themaster DESTINATION bin)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/TheMasterLogo.png" DESTINATION share/themaster)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/TheMaster.desktop" DESTINATION share/applications)

enable_testing()

add_executable(genrsatest GenRsaTest.cpp)
target_link_libraries(genrsatest PRIVATE crypto)
target_link_libraries(genrsatest PRIVATE -lssl -lcrypto)

add_test(NAME GenRsaTest COMMAND genrsatest)

add_executable(jwktopemtest JwkToPemTest.cpp)
target_link_libraries(jwktopemtest PRIVATE crypto)
target_link_libraries(jwktopemtest PRIVATE -lssl -lcrypto)

add_test(NAME JwkToPemTest COMMAND jwktopemtest)
