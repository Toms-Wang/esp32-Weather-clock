# Embed the server root certificate into the final binary
#
# (If this was a component, we would set COMPONENT_EMBED_TXTFILES here.)

set(src
"https_request_example_main.c"
../myDriver/LCD/lcd.c
../myDriver/LED/LED.c
)

set(inc
"."
../myDriver/LCD/
../myDriver/LED/
)

idf_component_register(SRCS ${src}
                    INCLUDE_DIRS ${inc}
                    EMBED_TXTFILES server_root_cert.pem local_server_cert.pem
                    )
