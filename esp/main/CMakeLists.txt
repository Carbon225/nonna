idf_component_register(
    SRCS
        http_params.c
        logo.c
        main.c
        ota.c
        wifi_man.c

        nanopb/pb_common.c
        nanopb/pb_encode.c
        nanopb/pb_decode.c

        proto/nonna.pb.c
        proto/proto_framing.c

    INCLUDE_DIRS
        .
        nanopb
        proto
)
