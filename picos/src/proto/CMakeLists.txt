add_library(nonna_proto INTERFACE)

target_sources(nonna_proto INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/nonna.pb.c
)

target_include_directories(nonna_proto INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(nonna_proto INTERFACE nanopb_hdr)



add_library(proto_framing STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/proto_framing.c
)

target_include_directories(proto_framing PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
