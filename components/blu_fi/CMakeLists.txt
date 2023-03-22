set(srcs 
"blu_fi.c"
"blufi_init.c"
"blufi_security.c"
 )

set(inc 
"include"
 )

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS ${inc}
                    REQUIRES nvs_flash bt
                    )

