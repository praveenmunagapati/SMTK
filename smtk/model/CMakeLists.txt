# set up sources to build
set(modelOps
  AddAuxiliaryGeometry
  AddImage
  AssignColors
  CloseModel
  CreateInstances
  EntityGroupOperator
  ExportModelJSON
  ExportSMTKModel
  GroupAuxiliaryGeometry
  LoadSMTKModel
  SaveSMTKModel
  SetProperty
  TerrainExtraction
)

set(modelSrcs
  Arrangement.cxx
  ArrangementHelper.cxx
  ArrangementKind.cxx
  AttributeAssignments.cxx
  AttributeListPhrase.cxx
  AuxiliaryGeometry.cxx
  AuxiliaryGeometryExtension.cxx
  Session.cxx
  SessionRef.cxx
  SessionIO.cxx
  SessionIOJSON.cxx
  SessionRegistrar.cxx
  CellEntity.cxx
  Chain.cxx
  EntityRef.cxx
  EntityRefArrangementOps.cxx
  DefaultSession.cxx
  DescriptivePhrase.cxx
  Edge.cxx
  EdgeUse.cxx
  Entity.cxx
  EntityIterator.cxx
  EntityListPhrase.cxx
  EntityPhrase.cxx
  EntityTypeSubphrases.cxx
  Face.cxx
  FaceUse.cxx
  Group.cxx
  GridInfo.cxx
  Instance.cxx
  Loop.cxx
  MeshListPhrase.cxx
  MeshPhrase.cxx
  Model.cxx
  Operator.cxx
  PointLocatorExtension.cxx
  PropertyListPhrase.cxx
  PropertyValuePhrase.cxx
  RemoteOperator.cxx
  Shell.cxx
  ShellEntity.cxx
  SimpleModelSubphrases.cxx
  Manager.cxx
  StoredResource.cxx
  SubphraseGenerator.cxx
  Tessellation.cxx
  UseEntity.cxx
  Vertex.cxx
  VertexUse.cxx
  Volume.cxx
  VolumeUse.cxx
)

set(modelHeaders
  Arrangement.h
  ArrangementHelper.h
  ArrangementKind.h
  AttributeAssignments.h
  AttributeListPhrase.h
  AuxiliaryGeometry.h
  AuxiliaryGeometryExtension.h
  Session.h
  SessionRef.h
  SessionIO.h
  SessionIOJSON.h
  SessionRegistrar.h
  CellEntity.h
  Chain.h
  EntityRef.h
  EntityRefArrangementOps.h
  DefaultSession.h
  DescriptivePhrase.h
  Edge.h
  EdgeUse.h
  Entity.h
  EntityIterator.h
  EntityListPhrase.h
  EntityPhrase.h
  EntityTypeBits.h
  EntityTypeSubphrases.h
  Events.h
  Face.h
  FaceUse.h
  FloatData.h
  GridInfo.h
  Group.h
  Instance.h
  IntegerData.h
  Loop.h
  MeshListPhrase.h
  MeshPhrase.h
  Model.h
  Operator.h
  PointLocatorExtension.h
  PropertyListPhrase.h
  PropertyValuePhrase.h
  RemoteOperator.h
  Shell.h
  ShellEntity.h
  SimpleModelSubphrases.h
  Manager.h
  StoredResource.h
  SubphraseGenerator.h
  StringData.h
  Tessellation.h
  UseEntity.h
  Vertex.h
  VertexUse.h
  Volume.h
  VolumeUse.h
)

smtk_session_json("${CMAKE_CURRENT_SOURCE_DIR}/DefaultSession.json" defSessionJSON)
foreach (operator ${modelOps})
  smtk_operator_xml("${CMAKE_CURRENT_SOURCE_DIR}/operators/${operator}.sbt" defOpXML)
  list(APPEND modelSrcs operators/${operator}.cxx)
  list(APPEND modelHeaders operators/${operator}.h)
endforeach()

#install the headers
smtk_public_headers(${modelHeaders})

if (SMTK_ENABLE_PYTHON_WRAPPING)
  add_subdirectory(pybind11)
endif()

if (SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
