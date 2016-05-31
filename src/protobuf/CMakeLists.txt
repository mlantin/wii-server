add_library (VRCom VRCom.pb.cc)

# Make sure the compiler can find include files for our library
# when other libraries or executables link to VRCom
target_include_directories (VRCom PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})