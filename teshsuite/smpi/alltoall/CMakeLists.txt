cmake_minimum_required(VERSION 2.6)

if(enable_smpi)
  if(WIN32)
    set(CMAKE_C_FLAGS "-include ${CMAKE_HOME_DIRECTORY}/include/smpi/smpi_main.h")
  else()
    set(CMAKE_C_COMPILER "${CMAKE_BINARY_DIR}/smpi_script/bin/smpicc")
  endif()

  set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}")

  include_directories(BEFORE "${CMAKE_HOME_DIRECTORY}/include/smpi")
  add_executable(alltoall2 alltoall2.c)
  add_executable(alltoall_basic alltoall_basic.c)
  add_executable(alltoall_coll alltoall_coll.c)
  target_link_libraries(alltoall2 simgrid)
  target_link_libraries(alltoall_basic simgrid)
  target_link_libraries(alltoall_coll simgrid)
endif()


set(tesh_files
  ${tesh_files}
  ${CMAKE_CURRENT_SOURCE_DIR}/alltoall_coll.tesh
  PARENT_SCOPE
  )
set(xml_files
  ${xml_files}
  PARENT_SCOPE
  )
set(examples_src
  ${examples_src}
  ${CMAKE_CURRENT_SOURCE_DIR}/alltoall_coll.c
  ${CMAKE_CURRENT_SOURCE_DIR}/alltoall_basic.c
  ${CMAKE_CURRENT_SOURCE_DIR}/alltoall2.c
  PARENT_SCOPE
  )
set(bin_files
  ${bin_files}
  PARENT_SCOPE
  )
set(txt_files
  ${txt_files}

  PARENT_SCOPE
  )
