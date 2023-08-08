# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS "hw_controller.c" "control_server.c" "wifi.c" "main.c")
set(COMPONENT_ADD_INCLUDEDIRS "")

register_component()
