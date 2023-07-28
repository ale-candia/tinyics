#### Build ICS-SIM library ####

set(source_files
    tinyics/industrial-network-builder.cc
    tinyics/industrial-plant.cc
    tinyics/industrial-process.cc
    tinyics/modbus.cc
    tinyics/plc-application.cc
    tinyics/plc-state.cc
    tinyics/scada-application.cc
    tinyics/utils.cc
    tinyics/modbus-command.cc
    tinyics/modbus-request.cc
    tinyics/modbus-response.cc
)

set(lib_name "tinyics")

add_library(${lib_name} SHARED ${source_files})

add_dependencies(${lib_name} libcsma libinternet)

target_include_directories(${lib_name} PRIVATE
    ${CMAKE_SOURCE_DIR}/external/ns-3/build/include/
    ${CMAKE_CURRENT_LIST_DIR}/tinyics/
)

target_link_directories(${lib_name} PRIVATE
    ${CMAKE_SOURCE_DIR}/external/ns-3/build/lib/
)

target_link_libraries(${lib_name}
    libcsma
    libinternet
)

