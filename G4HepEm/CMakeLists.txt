

add_subdirectory (G4HepEmData)
add_subdirectory (G4HepEmInit)
add_subdirectory (G4HepEmRun)


include_directories (
 ${CMAKE_SOURCE_DIR}/G4HepEm/include
 ${G4HEPEMDATA_INC_DIR}
 ${G4HEPEMINIT_INC_DIR}
 ${G4HEPEMRUN_INC_DIR}
)

message (STATUS "??? ${G4HEPEMINIT_INC_DIR}")

file ( GLOB G4HEPEM_headers ${CMAKE_SOURCE_DIR}/G4HepEm/include/*.hh)
file ( GLOB G4HEPEM_sources ${CMAKE_SOURCE_DIR}/G4HepEm/src/*.cc)
if (BUILD_STATIC_LIBS)
  add_library (g4HepEm STATIC ${G4HEPEM_sources})
else ()
  add_library (g4HepEm SHARED ${G4HEPEM_sources})
endif ()

set_target_properties(g4HepEm PROPERTIES COMPILE_FLAGS ${CMAKE_CXX_FLAGS})
target_link_libraries(g4HepEm g4HepEmData g4HepEmInit g4HepEmRun) # ${Geant4_LIBRARIES})

## ----------------------------------------------------------------------------
## Install G4HepEm libraries and headers
install (FILES ${G4HEPEM_headers} DESTINATION includes)
install (TARGETS g4HepEm DESTINATION lib)

## ----------------------------------------------------------------------------
## Create and install G4HepEm CMake Config file
##
configure_file (${CMAKE_SOURCE_DIR}/cmake/G4HepEmConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/G4HepEmConfig.cmake @ONLY)

install (FILES
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/G4HepEmConfig.cmake
    DESTINATION lib/cmake/G4HepEm
)
