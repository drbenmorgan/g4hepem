cmake_minimum_required(VERSION 3.18...3.28)
# Must enable languages here so that calls after FetchContent works
project(trackingmanager-offload)

#----------------------------------------------------------------------------
# Find Geant4 package
#
find_package(Geant4 REQUIRED)

#----------------------------------------------------------------------------
# Find G4HepEm
#
find_package(G4HepEm REQUIRED)

add_executable(trackingmanager-offload trackingmanager-offload.cc)
target_link_libraries(trackingmanager-offload ${Geant4_LIBRARIES} ${G4HepEm_LIBRARIES})
