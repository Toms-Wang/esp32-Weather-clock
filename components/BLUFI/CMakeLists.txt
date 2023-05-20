set(srcs
"blufi_example_main.c"
"blufi_security.c"
"blufi_init.c"
)

set(inc 
"."
)

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS ${inc}
                    REQUIRES bt nvs_flash  WIFI mbedtls
)
