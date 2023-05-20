set(srcs 
"weather.c"
)

set(inc 
"."
)

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS ${inc}
                    REQUIRES  esp_wifi esp_event nvs_flash json esp_http_client esp_netif esp-tls mbedtls
                    )