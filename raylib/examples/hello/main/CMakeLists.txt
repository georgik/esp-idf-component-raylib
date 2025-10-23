idf_component_register(
    SRCS "hello.c"
    INCLUDE_DIRS "."
    REQUIRES
        espressif__esp-box-3_noglib
    PRIV_REQUIRES
        georgik__raylib
)
