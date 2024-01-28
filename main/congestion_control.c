#include "congestion_control.h"

void update_congestion_control(congestion_control_t* congestion_control_parameters) {
    uint32_t interval_max_fps = 1000 / MAX_FPS;

    congestion_control_parameters->packet_delay = 10;
    congestion_control_parameters->tranmission_interval = 1000;
}

congestion_control_t congestion_control_init() {
    congestion_control_t congestion_control_parameters;
    update_congestion_control(&congestion_control_parameters);
    return congestion_control_parameters;
}