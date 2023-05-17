set(srcs 
"WIFI.c"
)

set(inc 
"."
)

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS ${inc}
                    REQUIRES  bt
                    )