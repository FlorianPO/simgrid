cmake_minimum_required(VERSION 2.6)

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}")

add_executable(test_trace_integration test_trace_integration.c)

### Add definitions for compile
target_link_libraries(test_trace_integration simgrid)

set(tesh_files
  ${tesh_files}
  ${CMAKE_CURRENT_SOURCE_DIR}/trace.tesh
  PARENT_SCOPE
  )
set(xml_files
  ${xml_files}
  ${CMAKE_CURRENT_SOURCE_DIR}/test-hbp1.0-hbp1.0-hbp1.0.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/test-hbp1.0-hbp3.0-hbp4.0.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/test-hbp1.5-hbp1.5.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/test-hbp1-c0s0-c0s1.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/test-hbp1-c0s0-c1s0.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/test-hbp1-c0s1-c0s2.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/test-hbp1-c0s1-c2s2.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/test-hbp1-c0s2-c1s0.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/test-hbp1-c0s2-c1s1.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/test-hbp1-c1s1-c1s2.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/test-hbp1-c1s1-c3s2.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/test-hbp2.5-hbp1.5.xml
  PARENT_SCOPE
  )
set(teshsuite_src
  ${teshsuite_src}
  ${CMAKE_CURRENT_SOURCE_DIR}/test_trace_integration.c
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
