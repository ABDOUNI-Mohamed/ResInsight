
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()

set (SOURCE_GROUP_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RicCloseCaseFeature.h
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterFeatureImpl.h
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterInsertFeature.h
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterInsertExec.h
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterNewFeature.h
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterNewExec.h
${CEE_CURRENT_LIST_DIR}RicNewViewFeature.h
${CEE_CURRENT_LIST_DIR}RicPropertyFilterNewExec.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterExecImpl.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterInsertExec.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterInsertFeature.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewFeature.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterFeatureImpl.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewExec.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewSliceIFeature.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewSliceJFeature.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewSliceKFeature.h
${CEE_CURRENT_LIST_DIR}RicImportFormationNamesFeature.h
${CEE_CURRENT_LIST_DIR}RicReloadFormationNamesFeature.h

${CEE_CURRENT_LIST_DIR}RicWellLogsImportFileFeature.h

${CEE_CURRENT_LIST_DIR}RicTogglePerspectiveViewFeature.h
${CEE_CURRENT_LIST_DIR}RicExportToLasFileFeature.h
${CEE_CURRENT_LIST_DIR}RicExportToLasFileResampleUi.h
${CEE_CURRENT_LIST_DIR}RicSnapshotViewToClipboardFeature.h
${CEE_CURRENT_LIST_DIR}RicImportGeoMechCaseFeature.h
${CEE_CURRENT_LIST_DIR}RicImportSummaryCaseFeature.h

${CEE_CURRENT_LIST_DIR}RicExportFaultsFeature.h
${CEE_CURRENT_LIST_DIR}RicExportMultipleSnapshotsFeature.h

${CEE_CURRENT_LIST_DIR}RicNewSimWellFractureFeature.h
${CEE_CURRENT_LIST_DIR}RicNewSimWellFractureAtPosFeature.h
${CEE_CURRENT_LIST_DIR}RicNewFractureDefinitionFeature.h
${CEE_CURRENT_LIST_DIR}RicNewWellPathFractureFeature.h
${CEE_CURRENT_LIST_DIR}RicNewWellPathFractureAtPosFeature.h
${CEE_CURRENT_LIST_DIR}RicFractureDefinitionsDeleteAllFeature.h
${CEE_CURRENT_LIST_DIR}RicFracturesDeleteAllFeature.h
${CEE_CURRENT_LIST_DIR}RicWellPathFracturesDeleteAllFeature.h
${CEE_CURRENT_LIST_DIR}RicSimWellFracturesDeleteAllFeature.h
${CEE_CURRENT_LIST_DIR}RicExportFractureWellCompletionFeature.h


# General delete of any object in a child array field
${CEE_CURRENT_LIST_DIR}RicDeleteItemExec.h
${CEE_CURRENT_LIST_DIR}RicDeleteItemExecData.h
${CEE_CURRENT_LIST_DIR}RicDeleteItemFeature.h

${CEE_CURRENT_LIST_DIR}RicCommandFeature.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RicCloseCaseFeature.cpp
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterFeatureImpl.cpp
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterInsertFeature.cpp
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterInsertExec.cpp
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterNewFeature.cpp
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterNewExec.cpp
${CEE_CURRENT_LIST_DIR}RicNewViewFeature.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterExecImpl.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterInsertExec.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterInsertFeature.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewFeature.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterFeatureImpl.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewExec.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewSliceIFeature.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewSliceJFeature.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewSliceKFeature.cpp
${CEE_CURRENT_LIST_DIR}RicImportFormationNamesFeature.cpp
${CEE_CURRENT_LIST_DIR}RicReloadFormationNamesFeature.cpp

${CEE_CURRENT_LIST_DIR}RicTogglePerspectiveViewFeature.cpp
${CEE_CURRENT_LIST_DIR}RicExportToLasFileFeature.cpp
${CEE_CURRENT_LIST_DIR}RicExportToLasFileResampleUi.cpp
${CEE_CURRENT_LIST_DIR}RicSnapshotViewToClipboardFeature.cpp
${CEE_CURRENT_LIST_DIR}RicImportGeoMechCaseFeature.cpp
${CEE_CURRENT_LIST_DIR}RicImportSummaryCaseFeature.cpp

${CEE_CURRENT_LIST_DIR}RicExportFaultsFeature.cpp
${CEE_CURRENT_LIST_DIR}RicExportMultipleSnapshotsFeature.cpp

${CEE_CURRENT_LIST_DIR}RicNewSimWellFractureFeature.cpp
${CEE_CURRENT_LIST_DIR}RicNewSimWellFractureAtPosFeature.cpp
${CEE_CURRENT_LIST_DIR}RicNewFractureDefinitionFeature.cpp

${CEE_CURRENT_LIST_DIR}RicNewWellPathFractureFeature.cpp
${CEE_CURRENT_LIST_DIR}RicNewWellPathFractureAtPosFeature.cpp
${CEE_CURRENT_LIST_DIR}RicFractureDefinitionsDeleteAllFeature.cpp
${CEE_CURRENT_LIST_DIR}RicWellPathFracturesDeleteAllFeature.cpp
${CEE_CURRENT_LIST_DIR}RicSimWellFracturesDeleteAllFeature.cpp
${CEE_CURRENT_LIST_DIR}RicExportFractureWellCompletionFeature.cpp





# General delete of any object in a child array field
${CEE_CURRENT_LIST_DIR}RicDeleteItemExec.cpp
${CEE_CURRENT_LIST_DIR}RicDeleteItemExecData.cpp
${CEE_CURRENT_LIST_DIR}RicDeleteItemFeature.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFeature" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CEE_CURRENT_LIST_DIR}CMakeLists_files.cmake )
