# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES)
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS "hal/servo_hal.c" "hal/battery_status_hal.c" "hal/motor_hal.c" "protocol/tccp.c" "protocol/rtp.c" "protocol/rtcp.c" "hal/camera_hal.c" "hal/led_hal.c" "wifi.c" "main.c" "congestion_control.c" "storage.c")
set(COMPONENT_ADD_INCLUDEDIRS "")

register_component()
