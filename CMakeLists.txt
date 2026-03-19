cmake_minimum_required(VERSION 3.16)
project(dist_sys_monitor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_NO_SYSTEM_FROM_IMPORTED ON)

option(BUILD_SHARED_LIBS "Build shared libs" OFF)
option(BUILD_MANAGER "Build manager binary" ON)
option(ENABLE_EBPF "Enable eBPF collectors (libbpf required)" OFF)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(DEFAULT_BUILD_AGENT ON)
else()
  set(DEFAULT_BUILD_AGENT OFF)
endif()
option(BUILD_AGENT "Build agent binary (Linux only)" ${DEFAULT_BUILD_AGENT})

if (BUILD_AGENT AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
  message(FATAL_ERROR "BUILD_AGENT=ON is only supported on Linux.")
endif()

if (ENABLE_EBPF AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
  message(FATAL_ERROR "ENABLE_EBPF=ON is only supported on Linux.")
endif()

# On Apple Silicon, avoid picking stale /usr/local headers from Intel Homebrew.
if (APPLE AND EXISTS "/opt/homebrew/include")
  add_compile_options(-I/opt/homebrew/include)
  include_directories(BEFORE /opt/homebrew/include)
endif()

set(protobuf_MODULE_COMPATIBLE OFF)
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

include("${Protobuf_DIR}/protobuf-generate.cmake")

include_directories(${Protobuf_INCLUDE_DIRS})

set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/proto)
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

set(PROTO_FILES
    ${PROTO_DIR}/metrics.proto
)

add_library(proto_generated)

protobuf_generate(
    TARGET proto_generated
    LANGUAGE cpp
    PROTOS ${PROTO_FILES}
)

protobuf_generate(
    TARGET proto_generated
    LANGUAGE grpc
    PROTOS ${PROTO_FILES}
    PLUGIN "protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>"
    GENERATE_EXTENSIONS .grpc.pb.cc .grpc.pb.h
)

set_target_properties(proto_generated PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${GENERATED_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${GENERATED_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${GENERATED_DIR}
)

target_include_directories(proto_generated PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/proto
)

target_link_libraries(proto_generated
    PUBLIC
        gRPC::grpc++
        protobuf::libprotobuf
)

add_library(common
    src/common/metrics_types.cc
    src/common/metrics_convert.cc
)

target_include_directories(common PUBLIC include src/common)

target_link_libraries(common PUBLIC proto_generated)

set(AGENT_SRCS
    src/agent/agent_main.cpp
    src/agent/agent_config.cc
    src/agent/collectors/cpu_load.cc
    src/agent/collectors/cpu_stats.cc
    src/agent/collectors/softirq_stats.cc
    src/agent/collectors/memory_disk.cc
    src/agent/collectors/net_stats.cc
)

if (ENABLE_EBPF)
  list(APPEND AGENT_SRCS src/agent/collectors/ebpf_net_collector.cc)
endif()

if (BUILD_AGENT)
  add_executable(agent ${AGENT_SRCS})

  target_include_directories(agent PUBLIC include)

  target_link_libraries(agent PRIVATE common gRPC::grpc++ protobuf::libprotobuf)

  if (ENABLE_EBPF)
    target_compile_definitions(agent PRIVATE DSM_ENABLE_EBPF)
    target_link_libraries(agent PRIVATE bpf elf)
  endif()
endif()

if (BUILD_MANAGER)
  find_package(SQLite3 REQUIRED)

  add_executable(manager
      src/manager/manager_main.cpp
      src/manager/manager_service.cc
      src/manager/scoring.cc
      src/manager/db/sqlite_store.cc
  )

  target_include_directories(manager PUBLIC include)

  target_link_libraries(manager PRIVATE common gRPC::grpc++ protobuf::libprotobuf SQLite::SQLite3)
endif()
