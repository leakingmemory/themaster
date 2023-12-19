cmake_minimum_required(VERSION 3.27)
project(drwhatsnot)

set(CMAKE_CXX_STANDARD 23)

find_package(wxWidgets REQUIRED)
include(${wxWidgets_USE_FILE})

find_package(cpprestsdk REQUIRED)

add_executable(themaster TheMasterApp.cpp TheMasterApp.h TheMasterFrame.cpp TheMasterFrame.h ConnectDialog.cpp ConnectDialog.h
    FindPatientDialog.cpp FindPatientDialog.h PatientStoreInMemory.cpp PatientStoreInMemory.h
    CreatePatientDialog.cpp CreatePatientDialog.h WaitingForApiDialog.cpp WaitingForApiDialog.h)
target_link_libraries(themaster PRIVATE -lsfmbasisapi)
target_link_libraries(themaster PRIVATE cpprestsdk::cpprest -lcrypto)
target_link_libraries(themaster PRIVATE ${wxWidgets_LIBRARIES})
target_include_directories(themaster PRIVATE ${wxWidgets_INCLUDE})

string(REPLACE "/" "\\\\/" PREFIX_ESCAPED "${CMAKE_INSTALL_PREFIX}")

add_custom_target(
        "TheMaster.desktop"
        ALL "cat" "${CMAKE_CURRENT_SOURCE_DIR}/TheMaster.desktop" | sed -e "s/PREFIX/${PREFIX_ESCAPED}/" > "${CMAKE_CURRENT_BINARY_DIR}/TheMaster.desktop"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/TheMaster.desktop"
)

install(TARGETS themaster DESTINATION bin)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/TheMasterLogo.png" DESTINATION share/themaster)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/TheMaster.desktop" DESTINATION share/applications)

