add_library (easywsclient easywsclient.cpp)

# Make sure the compiler can find include files for our easywsclient library
# when other libraries or executables link to easywsclient
target_include_directories (easywsclient PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})