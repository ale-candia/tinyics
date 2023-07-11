#### Build ICS-SIM library ####

set(source_files
    icsim/industrial-network-builder.cc
    icsim/industrial-plant.cc
    icsim/industrial-process.cc
    icsim/modbus.cc
    icsim/plc-application.cc
    icsim/scada-application.cc
    icsim/utils.cc
    icsim/modbus-command.cc
    icsim/modbus-request.cc
    icsim/modbus-response.cc
)

set(lib_name "icssim")

add_library(${lib_name} SHARED ${source_files})

add_dependencies(${lib_name} libcore libcsma libinternet libnetwork)

target_include_directories(${lib_name} PRIVATE
    ${CMAKE_SOURCE_DIR}/external/ns-3/build/include/
    ${CMAKE_CURRENT_LIST_DIR}/icsim/icsim/
)

target_link_directories(${lib_name} PRIVATE
    ${CMAKE_SOURCE_DIR}/external/ns-3/build/lib/
)

target_link_libraries(${lib_name}
    libcore
    libcsma
    libinternet
    libnetwork
)

