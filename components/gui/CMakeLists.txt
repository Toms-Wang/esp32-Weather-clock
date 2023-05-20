set(srcs 
"gui.c"
)

set(inc 
"."
)

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS ${inc}
                    REQUIRES  LCD tim_data weather
                    )