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