cmake_minimum_required(VERSION 3.27)
project(themaster)

set(CMAKE_CXX_STANDARD 23)

find_package(wxWidgets REQUIRED COMPONENTS core base webview)
include(${wxWidgets_USE_FILE})

find_package(cpprestsdk REQUIRED)

if (${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
    add_compile_options(-DFREEBSD)
    add_compile_options(-I/usr/local/include)
    add_link_options(-L/usr/local/lib)
else()
    add_compile_options(-fconstexpr-ops-limit=536870912)
endif()

string(REPLACE "/" "\\\\/" PREFIX_ESCAPED "${CMAKE_INSTALL_PREFIX}")

add_subdirectory(CsvGenerator)

if (WIN32)
    add_custom_target("InstallPrefix.h"
            ALL ${CMAKE_COMMAND} -E "copy" "${CMAKE_CURRENT_SOURCE_DIR}/InstallPrefix.h.fallback" "${CMAKE_CURRENT_BINARY_DIR}/InstallPrefix.h"
            DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/InstallPrefix.h.fallback"
    )
    add_subdirectory(win32)
else()
    add_custom_target("InstallPrefix.h"
            ALL "cat" "${CMAKE_CURRENT_SOURCE_DIR}/InstallPrefix.h.in" | sed -e "s/PREFIX/${PREFIX_ESCAPED}/" > "${CMAKE_CURRENT_BINARY_DIR}/InstallPrefix.h"
            DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/InstallPrefix.h.in"
    )
endif()

add_custom_target("ICD-10-codeset.gen.h"
            ALL "${CMAKE_CURRENT_BINARY_DIR}/CsvGenerator/CsvGenerator" "${CMAKE_CURRENT_SOURCE_DIR}/ICD-10-codeset.csv" > "${CMAKE_CURRENT_BINARY_DIR}/ICD-10-codeset.gen.h"
            DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/ICD-10-codeset.csv" "${CMAKE_CURRENT_BINARY_DIR}/CsvGenerator/CsvGenerator"
)

add_library(codelists OBJECT MedicalCodedValue.cpp MedicalCodedValue.h ICD10.cpp ICD10.h)
target_include_directories(codelists PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
add_dependencies(codelists "ICD-10-codeset.gen.h")

add_executable(themaster TheMasterApp.cpp TheMasterApp.h TheMasterFrame.cpp TheMasterFrame.h ConnectDialog.cpp ConnectDialog.h
    FindPatientDialog.cpp FindPatientDialog.h PatientStoreInMemory.cpp PatientStoreInMemory.h
    CreatePatientDialog.cpp CreatePatientDialog.h WaitingForApiDialog.cpp WaitingForApiDialog.h
    HelseidLoginDialog.cpp HelseidLoginDialog.h HelseidAuthorization.cpp HelseidAuthorization.h
    DataDirectory.cpp DataDirectory.h PatientStoreInMemoryWithPersistence.cpp PatientStoreInMemoryWithPersistence.h
    MagistralBuilderDialog.cpp MagistralBuilderDialog.h  UnitsOfMeasure.cpp UnitsOfMeasure.h
    PrescriptionDialog.cpp PrescriptionDialog.h ComboSearchControl.cpp ComboSearchControl.h
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
    CaveDetailsDialog.cpp CaveDetailsDialog.h RegisterCaveDialog.cpp RegisterCaveDialog.h
    ConnectToPllDialog.cpp ConnectToPllDialog.h MerchTree.cpp MerchTree.h MerchData.cpp MerchData.h
    PrescribeMerchandiseDialog.cpp PrescribeMerchandiseDialog.h GetLegemiddelRefunds.cpp GetLegemiddelRefunds.h
    EditReferenceNumbersDialog.cpp EditReferenceNumbersDialog.h Uuid.cpp Uuid.h MedicamentSearch.cpp MedicamentSearch.h
    ClientAssertion.cpp ClientAssertion.h)
target_link_libraries(themaster PRIVATE codelists)
if (WIN32)
    set_target_properties(themaster PROPERTIES WIN32_EXECUTABLE TRUE)
    target_link_libraries(themaster PRIVATE ${SFMBASISAPI_LIBRARY} ${LIBJJWTID_LIBRARY} ${MEDFEST_LIBRARY})
    target_link_libraries(themaster PRIVATE cpprestsdk::cpprest)
else()
    target_link_libraries(themaster PRIVATE -lsfmbasisapi -ljjwtid -lmedfest)
    target_link_libraries(themaster PRIVATE cpprestsdk::cpprest -lcrypto)
endif()
target_link_libraries(themaster PRIVATE ${wxWidgets_LIBRARIES})
target_include_directories(themaster PRIVATE ${wxWidgets_INCLUDE})
target_include_directories(themaster PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
if (WIN32)
    target_link_libraries(themaster PRIVATE win32)
    target_include_directories(themaster PRIVATE ${SFMBASISAPI_INCLUDE_DIR})
    target_include_directories(themaster PRIVATE ${Boost_INCLUDE_DIRS})
    target_include_directories(themaster PRIVATE ${NLOHMANN_INCLUDE_DIR})
endif()
add_dependencies(themaster "InstallPrefix.h")

if (WIN32)
    install(TARGETS themaster DESTINATION bin)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/TheMasterLogo.png" DESTINATION bin)
else()
    add_custom_target(
            "TheMaster.desktop"
            ALL "cat" "${CMAKE_CURRENT_SOURCE_DIR}/TheMaster.desktop" | sed -e "s/PREFIX/${PREFIX_ESCAPED}/" > "${CMAKE_CURRENT_BINARY_DIR}/TheMaster.desktop"
            DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/TheMaster.desktop"
    )

    install(TARGETS themaster DESTINATION bin)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/TheMasterLogo.png" DESTINATION share/themaster)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/TheMaster.desktop" DESTINATION share/applications)
endif()