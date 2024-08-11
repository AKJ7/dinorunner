#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "dinorunner::dinorunner_shared" for configuration "Release"
set_property(TARGET dinorunner::dinorunner_shared APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(dinorunner::dinorunner_shared PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libdinorunner.so.1.0.0"
  IMPORTED_SONAME_RELEASE "libdinorunner.so.1"
  )

list(APPEND _cmake_import_check_targets dinorunner::dinorunner_shared )
list(APPEND _cmake_import_check_files_for_dinorunner::dinorunner_shared "${_IMPORT_PREFIX}/lib/libdinorunner.so.1.0.0" )

# Import target "dinorunner::dinorunner_static" for configuration "Release"
set_property(TARGET dinorunner::dinorunner_static APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(dinorunner::dinorunner_static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libdinorunner.a"
  )

list(APPEND _cmake_import_check_targets dinorunner::dinorunner_static )
list(APPEND _cmake_import_check_files_for_dinorunner::dinorunner_static "${_IMPORT_PREFIX}/lib/libdinorunner.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
