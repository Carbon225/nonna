add_library(nonna_proto INTERFACE)

target_sources(nonna_proto INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/nonna.pb.c
)

target_include_directories(nonna_proto INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(nonna_proto INTERFACE nanopb_hdr)
