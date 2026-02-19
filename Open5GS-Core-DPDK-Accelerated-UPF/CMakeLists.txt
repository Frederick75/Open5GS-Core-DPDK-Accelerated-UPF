cmake_minimum_required(VERSION 3.16)
project(open5gs_core_dpdk_upf C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(USE_DPDK "Enable DPDK fast path if DPDK is available" OFF)
option(ENABLE_SANITIZERS "Enable ASAN/UBSAN (Debug only)" OFF)

if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND ENABLE_SANITIZERS)
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

include_directories(${CMAKE_SOURCE_DIR}/src)

# --- Common libs ---
add_library(common
  src/common/log.c
  src/common/config.c
  src/common/net.c
  src/common/timeutil.c
  src/common/ringbuf.c
)
target_link_libraries(common pthread)

add_library(pfcp
  src/pfcp/pfcp.c
  src/pfcp/pfcp_codec.c
  src/pfcp/pfcp_rules.c
)
target_link_libraries(pfcp common)

# --- DPDK detection (optional) ---
if (USE_DPDK)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(DPDK REQUIRED libdpdk)
  add_compile_definitions(USE_DPDK=1)
  include_directories(${DPDK_INCLUDE_DIRS})
  link_directories(${DPDK_LIBRARY_DIRS})
endif()

# --- UPF ---
add_executable(upf
  src/upf/upf_main.c
  src/upf/upf_pfcp_srv.c
  src/upf/upf_gtpu.c
  src/upf/upf_n6.c
  src/upf/upf_dataplane.c
  src/upf/upf_metrics.c
  src/upf/session_table.c
)
target_link_libraries(upf pfcp common)

if (USE_DPDK)
  target_link_libraries(upf ${DPDK_LIBRARIES})
endif()

# --- SMF ---
add_executable(smf
  src/smf/smf_main.c
  src/smf/smf_pfcp_cli.c
  src/smf/smf_sessions.c
  src/smf/smf_ngap_stub.c
  src/smf/smf_metrics.c
  src/smf/smf.c
)
target_link_libraries(smf pfcp common)

# --- Install ---
install(TARGETS upf smf RUNTIME DESTINATION bin)
