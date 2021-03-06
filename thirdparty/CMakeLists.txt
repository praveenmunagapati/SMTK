add_subdirectory(cJSON)

# disable warning flags for cJSON. This is optional because we should be
# continually pushing updates to cJSON, rather than just using it. For the
# sanity of our own dashboards, though, it is nice to be able to distinguish
# between cJSON's warnings and our own.
option(SMTK_IGNORE_CJSON_WARNINGS "disable warning flags for cJSON" ON)
mark_as_advanced(SMTK_IGNORE_CJSON_WARNINGS)

if (SMTK_IGNORE_CJSON_WARNINGS)
  set(ignore_rules_for_cjson "")

   if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # disables: warning C4267: '=' : conversion from 'size_t' to 'int', possible loss of data
    set(ignore_rules_for_cjson "${ignore_rules_for_cjson} /wd4267")
    # disables: warning C4996: 'strncpy': This function or variable may be unsafe. Consider using strncpy_s instead.
    set(ignore_rules_for_cjson "${ignore_rules_for_cjson} /wd4996")
  endif()

  set_property(TARGET cJSON APPEND_STRING PROPERTY COMPILE_FLAGS ${ignore_rules_for_cjson})
endif()

if (NOT SMTK_USE_SYSTEM_MOAB)
  #guard against MOAB polluting the cmake C/CXX/Fortran flags
  set(save_CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
  set(save_CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  set(save_CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS}")

  add_subdirectory(moab)

  # On Mac OS X, we specify the install name dir for MOAB so that
  # it can be found when being linked to from external projects
  if (BUILD_SHARED_LIBS)
    set_target_properties(MOAB
      PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
  endif()

  # disable warning flags for MOAB. This is optional because we should be
  # continually pushing updates to MOAB, rather than just using it. For the
  # sanity of our own dashboards, though, it is nice to be able to distinguish
  # between MOAB's warnings and our own.
  option(SMTK_IGNORE_MOAB_WARNINGS "disable all warning flags for MOAB" ON)
  mark_as_advanced(SMTK_IGNORE_MOAB_WARNINGS)

  if (SMTK_IGNORE_MOAB_WARNINGS)
    set(ignore_rules_for_moab "")

    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
      # disables: warning C4101: unreferenced local variable
      set(ignore_rules_for_moab "${ignore_rules_for_moab} /wd4101")
      # disables: warning C4138: '*/' found outside of comment
      set(ignore_rules_for_moab "${ignore_rules_for_moab} /wd4138")
      # disables: warning C4146: unary minus operator applied to unsigned type,
      # result still unsigned
      set(ignore_rules_for_moab "${ignore_rules_for_moab} /wd4146")
      # disables: warning C4244: '=' : conversion from 'hsize_t' to 'long',
      # possible loss of data
      set(ignore_rules_for_moab "${ignore_rules_for_moab} /wd4244")
      # disables: warning C4251: 'moab::Range::mHead' : struct
      # 'moab::Range::PairNode' needs to have dll-interface to be used by
      # clients of class 'moab::Range
      set(ignore_rules_for_moab "${ignore_rules_for_moab} /wd4251")
      # disables: warning C4267: '=' : conversion from 'size_t' to 'int',
      # possible loss of data
      set(ignore_rules_for_moab "${ignore_rules_for_moab} /wd4267")
      # disables: warning C4275: non dll-interface class 'moab::ReaderIface'
      # used as base for dll-interface class 'moab::ReadNCDF'
      set(ignore_rules_for_moab "${ignore_rules_for_moab} /wd4275")
      # disables: warning C4305: 'return' : truncation from 'double' to 'float'
      set(ignore_rules_for_moab "${ignore_rules_for_moab} /wd4305")
      # disables: warning C4373: 'moab::Core::tag_get_data': virtual function
      # overrides 'moab::Interface::tag_get_data', previous versions of the
      # compiler did not override when parameters only differed by
      # const/volatile qualifiers
      set(ignore_rules_for_moab "${ignore_rules_for_moab} /wd4373")
      # disables: warning C4800: 'int' : forcing value to bool 'true' or 'false'
      # (performance warning)
      set(ignore_rules_for_moab "${ignore_rules_for_moab} /wd4800")
      # disables: warning C4996: 'strncpy': This function or variable may be
      # unsafe. Consider using strncpy_s instead.
      set(ignore_rules_for_moab "${ignore_rules_for_moab} /wd4996")
      # disables: WARNING: You need to implement DEPRECATED for this compiler
      set(ignore_rules_for_moab "${ignore_rules_for_moab} /DDEPRECATED= ")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
	         CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR
	         CMAKE_CXX_COMPILER_ID MATCHES "Intel")
      # disables: warning: unused variable '####'
      set(ignore_rules_for_moab
          "${ignore_rules_for_moab} -Wno-unused-const-variable")
      # disables: warning: ignoring attributes on template argument '####'
      set(ignore_rules_for_moab
          "${ignore_rules_for_moab} -Wno-ignored-attributes")
      if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # disables: warning: private field '####' is not used
        set(ignore_rules_for_moab
            "${ignore_rules_for_moab} -Wno-unused-private-field")
      endif()
    endif()

    foreach(moab_target MOAB moabio moab_loc_discr moabverdict mhdf)
      if (TARGET ${moab_target})
        set_property(TARGET ${moab_target} APPEND_STRING PROPERTY COMPILE_FLAGS ${ignore_rules_for_moab})
      endif()
    endforeach()
  endif()

  set(CMAKE_C_FLAGS "${save_CMAKE_C_FLAGS}" CACHE STRING "" FORCE)
  set(CMAKE_CXX_FLAGS "${save_CMAKE_CXX_FLAGS}" CACHE STRING "" FORCE)
  set(CMAKE_Fortran_FLAGS "${save_CMAKE_Fortran_FLAGS}" CACHE STRING "" FORCE)

  set(MOAB_DIR "${CMAKE_CURRENT_BINARY_DIR}/moab" CACHE STRING "" FORCE)

  #hide all of the CMake options that moab has. This is done to keep
  #the smtk ccmake/cmake-gui interface simple.
  mark_as_advanced(
  ENABLE_CGM
  ENABLE_CGNS
  ENABLE_HDF5
  ENABLE_IGEOM
  ENABLE_IMESH
  ENABLE_METIS
  ENABLE_MPI
  ENABLE_NETCDF
  ENABLE_PARMETIS
  ENABLE_PNETCDF
  ENABLE_SZIP
  ENABLE_TESTING
  ENABLE_VTK
  ENABLE_ZOLTAN
  GZIP
  MOAB_BUILD_DAGMC
  MOAB_BUILD_HEXMODOPS
  MOAB_BUILD_MBCHACO
  MOAB_BUILD_MBCONVERT
  MOAB_BUILD_MBCOUPLER
  MOAB_BUILD_MBDEPTH
  MOAB_BUILD_MBGSETS
  MOAB_BUILD_MBMEM
  MOAB_BUILD_MBPART
  MOAB_BUILD_MBSIZE
  MOAB_BUILD_MBSKIN
  MOAB_BUILD_MBSURFPLOT
  MOAB_BUILD_MBTAGPROP
  MOAB_BUILD_QVDUAL
  MOAB_BUILD_SPHEREDECOMP
  MOAB_FORCE_32_BIT_HANDLES
  MOAB_FORCE_64_BIT_HANDLES
  SED
  TAR
  verdict_DIR
  )
endif()

#if(SMTK_ENABLE_OPENCV)
#  set(Delaunay_USE_OPENCV ON)
#endif()
#if(SMTK_ENABLE_VTK_SUPPORT)
#  set(Delaunay_USE_VTK ON)
#endif()

add_subdirectory(delaunay)

# disable warning flags for delaunay.
option(SMTK_IGNORE_DELAUNAY_WARNINGS "disable warning flags for delaunay" ON)
mark_as_advanced(SMTK_IGNORE_DELAUNAY_WARNINGS)

if (SMTK_IGNORE_DELAUNAY_WARNINGS)
  set(ignore_rules_for_delaunay "")

   if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # disables: warning C4251: dll-interface warnings involving stl
    set(ignore_rules_for_delaunay "${ignore_rules_for_delaunay} /wd4251")
  endif()

  foreach(delaunay_target DelaunayDiscretization DelaunayMesh DelaunayMisc
      DelaunayShape DelaunayValidation DelaunayVisualization)
    set_property(TARGET ${delaunay_target} APPEND_STRING
      PROPERTY COMPILE_FLAGS ${ignore_rules_for_delaunay})
  endforeach()
endif()

if (SMTK_ENABLE_VXL_SUPPORT)
  # We need VXL for rgtl, vnl_algo, vnl, and vcl.
  find_package(VXL REQUIRED)
  if(NOT VXL_RPL_FOUND)
    message(FATAL_ERROR "vxl must be built with BUILD_RPL enabled")
  endif(NOT VXL_RPL_FOUND)
  if(NOT VXL_RPL_RGTL_FOUND)
    message(FATAL_ERROR "vxl must be built with BUILD_RPL_RGTL enabled")
  endif(NOT VXL_RPL_RGTL_FOUND)
  include(${VXL_CMAKE_DIR}/UseVXL.cmake)

  # Build the Tensor Voting library.
  add_subdirectory(rtvl)
endif()
