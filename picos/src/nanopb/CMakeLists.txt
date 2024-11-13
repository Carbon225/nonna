add_library(nanopb_hdr INTERFACE)

target_include_directories(nanopb_hdr INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)



add_library(nanopb_common INTERFACE)

target_sources(nanopb_common INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/pb_common.c
)

target_link_libraries(nanopb_common INTERFACE nanopb_hdr)



add_library(nanopb_decode INTERFACE)

target_sources(nanopb_decode INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/pb_decode.c
)

target_link_libraries(nanopb_decode INTERFACE nanopb_hdr nanopb_common)



add_library(nanopb_encode INTERFACE)

target_sources(nanopb_encode INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/pb_encode.c
)

target_link_libraries(nanopb_encode INTERFACE nanopb_hdr nanopb_common)
