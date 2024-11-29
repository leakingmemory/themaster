cmake_minimum_required(VERSION 3.27)
project(themaster)

set(CMAKE_CXX_STANDARD 23)

find_package(wxWidgets REQUIRED COMPONENTS core base webview)
include(${wxWidgets_USE_FILE})

find_package(cpprestsdk REQUIRED)

string(REPLACE "/" "\\\\/" PREFIX_ESCAPED "${CMAKE_INSTALL_PREFIX}")

add_custom_target("InstallPrefix.h"
        ALL "cat" "${CMAKE_CURRENT_SOURCE_DIR}/InstallPrefix.h.in" | sed -e "s/PREFIX/${PREFIX_ESCAPED}/" > "${CMAKE_CURRENT_BINARY_DIR}/InstallPrefix.h"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/InstallPrefix.h.in"
)

add_executable(themaster TheMasterApp.cpp TheMasterApp.h TheMasterFrame.cpp TheMasterFrame.h ConnectDialog.cpp ConnectDialog.h
    FindPatientDialog.cpp FindPatientDialog.h PatientStoreInMemory.cpp PatientStoreInMemory.h
    CreatePatientDialog.cpp CreatePatientDialog.h WaitingForApiDialog.cpp WaitingForApiDialog.h
    HelseidLoginDialog.cpp HelseidLoginDialog.h HelseidAuthorization.cpp HelseidAuthorization.h
    DataDirectory.cpp DataDirectory.h PatientStoreInMemoryWithPersistence.cpp PatientStoreInMemoryWithPersistence.h
    MagistralBuilderDialog.cpp MagistralBuilderDialog.h  UnitsOfMeasure.cpp UnitsOfMeasure.h
    MedicalCodedValue.cpp MedicalCodedValue.h PrescriptionDialog.cpp PrescriptionDialog.h
    MagistralMedicament.cpp MagistralMedicament.h PrescriptionData.cpp PrescriptionData.h FestDbUi.cpp FestDbUi.h
    DownloadFestDialog.cpp DownloadFestDialog.h FestSerializerUi.cpp FestSerializerUi.h
    FestObjectStreamUi.cpp FestObjectStreamUi.h FestDb.cpp FestDb.h FindMedicamentDialog.cpp FindMedicamentDialog.h
    WxDebouncer.cpp WxDebouncer.h SfmMedicamentMapper.cpp SfmMedicamentMapper.h
    FestVersionsDialog.cpp FestVersionsDialog.h PrescriptionDetailsDialog.cpp PrescriptionDetailsDialog.h
    RecallPrescriptionDialog.cpp RecallPrescriptionDialog.h FestExploreVersionDialog.cpp FestExploreVersionDialog.h
    FestDiffVersionsDialog.cpp FestDiffVersionsDialog.h FestDbQuotasDialog.cpp FestDbQuotasDialog.h
    CeasePrescriptionDialog.cpp CeasePrescriptionDialog.h DateTime.cpp DateTime.h SignPllDialog.cpp SignPllDialog.h
    MedBundleData.cpp MedBundleData.h PrescriptionChangesService.cpp PrescriptionChangesService.h
    GetLegemiddelKortdoser.cpp GetLegemiddelKortdoser.h MedicamentVisitor.cpp MedicamentVisitor.h
    GetMedicamentDosingUnit.cpp GetMedicamentDosingUnit.h AdvancedDosingPeriod.cpp AdvancedDosingPeriod.h
    AdvancedDosingPeriodDialog.cpp AdvancedDosingPeriodDialog.h EreseptdosingDialog.cpp EreseptdosingDialog.h
    DateOnly.cpp DateOnly.h TimeConst.cpp TimeConst.h Duration.cpp Duration.h Lazy.cpp Lazy.h
    WxDateConversions.h WxDateConversions.cpp EditTreatmentDialog.cpp EditTreatmentDialog.h
    CaveDetailsDialog.cpp CaveDetailsDialog.h RegisterCaveDialog.cpp RegisterCaveDialog.h)
target_link_libraries(themaster PRIVATE -lsfmbasisapi -ljjwtid -lmedfest)
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
