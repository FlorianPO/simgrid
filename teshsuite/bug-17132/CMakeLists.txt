cmake_minimum_required(VERSION 2.6)

if(enable_smpi)
  if(WIN32)
      set(CMAKE_C_FLAGS "-include ${CMAKE_HOME_DIRECTORY}/include/smpi/smpi_main.h")
  else()
      set(CMAKE_C_COMPILER "${CMAKE_BINARY_DIR}/smpi_script/bin/smpicc")
  endif()


  set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}")

  include_directories(BEFORE "${CMAKE_HOME_DIRECTORY}/include/smpi")
  add_executable(bug-17132 ${CMAKE_HOME_DIRECTORY}/teshsuite/bug-17132/bug-17132.c)
  target_link_libraries(bug-17132 simgrid)

  set(tesh_files
    ${tesh_files}
    ${CMAKE_CURRENT_SOURCE_DIR}/bug-17132.tesh
    ${CMAKE_CURRENT_SOURCE_DIR}/bug-17132-surf-debug.tesh
    PARENT_SCOPE)
  set(xml_files
    ${xml_files}
    PARENT_SCOPE)
  set(teshsuite_src
    ${teshsuite_src}
    ${CMAKE_CURRENT_SOURCE_DIR}/bug-17132.c
    PARENT_SCOPE)
  set(bin_files
    ${bin_files}
    PARENT_SCOPE)
  set(txt_files
    ${txt_files}
    ${CMAKE_CURRENT_SOURCE_DIR}/hostfile.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/README
    PARENT_SCOPE)

endif(enable_smpi)
