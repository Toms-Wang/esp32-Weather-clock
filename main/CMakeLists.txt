# Embed the server root certificate into the final binary
#
# (If this was a component, we would set COMPONENT_EMBED_TXTFILES here.)

set(srcs 
"https_request_example_main.c"
../myDriver/LED/LED.c
../myDriver/LCD/lcd.c
../myDriver/weather/weather.c
../myDriver/tim_data/tim_data.c
../myDriver/gui/gui.c
 )

set(inc 
"."
../myDriver/LED/
../myDriver/LCD/
../myDriver/weather/
../myDriver/tim_data/
../myDriver/gui/
 )

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS ${inc}
                    EMBED_TXTFILES server_root_cert.pem local_server_cert.pem)
