#=============================================================================
#
#  Copyright (c) Kitware, Inc.
#  All rights reserved.
#  See LICENSE.txt for details.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
#=============================================================================

set(unit_tests
  UnitTestAllocator.cxx
  UnitTestCellTypes.cxx
  UnitTestCollection.cxx
  UnitTestBufferedCellAllocator.cxx
  UnitTestIncrementalAllocator.cxx
  UnitTestManager.cxx
  UnitTestModelToMesh3D.cxx
  UnitTestQueryTypes.cxx
  UnitTestReadWriteHandles.cxx
  UnitTestTypeSet.cxx
)

set(unit_tests_which_require_data
  UnitTestAddFileToCollection.cxx
  UnitTestCellSet.cxx
  UnitTestCreateMesh.cxx
  UnitTestCellField.cxx
  UnitTestExportMesh2DM.cxx
  UnitTestExportMesh3DM.cxx
  UnitTestExportMeshVTK.cxx
  UnitTestExtractMeshConstants.cxx
  UnitTestExtractTessellation.cxx
  UnitTestExtractTessellationOfModel.cxx

  UnitTestLoadMesh.cxx
  UnitTestMergeContactPoints.cxx
  UnitTestMeshMetrics.cxx
  UnitTestMeshSet.cxx
  UnitTestModelToMesh2D.cxx
  UnitTestPointConnectivity.cxx
  UnitTestPointField.cxx
  UnitTestPointLocator.cxx
  UnitTestPointSet.cxx
  UnitTestReadWriteMeshJSON.cxx
  UnitTestReclassifyEdges.cxx
  UnitTestRemoveMeshes.cxx
  UnitTestTypeSetFromData.cxx
  UnitTestWriteMesh.cxx
)

set(extra_libs)
if(SMTK_ENABLE_DISCRETE_SESSION)
  list(APPEND unit_tests_which_require_data
       TestElevateMesh.cxx
       UnitTestExtractOrderedTessellation.cxx
       )
  list(APPEND extra_libs
       smtkDiscreteSession
       smtkIOVTK
       vtkSMTKSourceExt
       )
endif()

smtk_unit_tests(
  LABEL "Mesh"
  SOURCES ${unit_tests}
  SOURCES_REQUIRE_DATA ${unit_tests_which_require_data}
  LIBRARIES smtkCore smtkCoreModelTesting ${extra_libs} ${Boost_LIBRARIES}
)

add_executable(TestGenerateHotStartData TestGenerateHotStartData.cxx)
target_compile_definitions(TestGenerateHotStartData PRIVATE "SMTK_SCRATCH_DIR=\"${CMAKE_BINARY_DIR}/Testing/Temporary\"")
target_link_libraries(TestGenerateHotStartData smtkCore ${Boost_LIBRARIES})

add_executable(TestInterpolateOntoMesh TestInterpolateOntoMesh.cxx)
target_compile_definitions(TestInterpolateOntoMesh PRIVATE "SMTK_SCRATCH_DIR=\"${CMAKE_BINARY_DIR}/Testing/Temporary\"")
target_link_libraries(TestInterpolateOntoMesh smtkCore ${Boost_LIBRARIES})

add_executable(TestWarpMesh TestWarpMesh.cxx)
target_compile_definitions(TestWarpMesh PRIVATE "SMTK_SCRATCH_DIR=\"${CMAKE_BINARY_DIR}/Testing/Temporary\"")
target_link_libraries(TestWarpMesh smtkCore ${Boost_LIBRARIES})

if (SMTK_DATA_DIR)
  add_test(NAME TestGenerateHotStartData
    COMMAND $<TARGET_FILE:TestGenerateHotStartData>
    "${SMTK_DATA_DIR}/model/2d/smtk/test2D.json")
  set_tests_properties(TestGenerateHotStartData PROPERTIES LABELS "Mesh")
  add_test(NAME TestGenerateHotStartDataCSV
    COMMAND $<TARGET_FILE:TestGenerateHotStartData>
    "${SMTK_DATA_DIR}/model/2d/smtk/test2D.json" 1)
  set_tests_properties(TestGenerateHotStartDataCSV PROPERTIES LABELS "Mesh")
  add_test(NAME TestInterpolateOntoMeshCells
    COMMAND $<TARGET_FILE:TestInterpolateOntoMesh>
    "${SMTK_DATA_DIR}/model/2d/smtk/test2D.json")
  set_tests_properties(TestInterpolateOntoMeshCells PROPERTIES LABELS "Mesh")
  add_test(NAME TestInterpolateOntoMeshPoints
    COMMAND $<TARGET_FILE:TestInterpolateOntoMesh>
    "${SMTK_DATA_DIR}/model/2d/smtk/test2D.json" 0)
  set_tests_properties(TestInterpolateOntoMeshPoints PROPERTIES LABELS "Mesh")
  add_test(NAME TestInterpolateOntoMeshCellsCSV
    COMMAND $<TARGET_FILE:TestInterpolateOntoMesh>
    "${SMTK_DATA_DIR}/model/2d/smtk/test2D.json" 0 1)
  set_tests_properties(TestInterpolateOntoMeshCellsCSV PROPERTIES LABELS "Mesh")
  add_test(NAME TestInterpolateOntoMeshPointsCSV
    COMMAND $<TARGET_FILE:TestInterpolateOntoMesh>
    "${SMTK_DATA_DIR}/model/2d/smtk/test2D.json" 1 1)
  set_tests_properties(TestInterpolateOntoMeshPointsCSV PROPERTIES LABELS "Mesh")
  add_test(NAME TestWarpMesh
    COMMAND $<TARGET_FILE:TestWarpMesh>
    "${SMTK_DATA_DIR}/model/2d/smtk/test2D.json" 0)
  add_test(NAME TestWarpMeshCSV
    COMMAND $<TARGET_FILE:TestWarpMesh>
    "${SMTK_DATA_DIR}/model/2d/smtk/test2D.json" 1)
endif()
