#include "tccp.h"

#define PORT CONFIG_UDP_PORT

static const char *TAG = "tccp";

static int socket_fd = -1;
static struct sockaddr_in dest_addr;

static esp_event_loop_handle_t tccp_server_handle;

ESP_EVENT_DEFINE_BASE(CONTROL_EVENT);

static void create_socket() {
    socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (socket_fd < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return;
    }
    ESP_LOGI(TAG, "Socket created");
}

static void set_address(in_addr_t dest_ip) {
    if (dest_ip == dest_addr.sin_addr.s_addr) {
        return;
    }
    // create socket 
    create_socket();
    // setting global address struct to be used until next send_frame call
    dest_addr.sin_addr.s_addr = dest_ip;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(TCCP_PORT);
}

static void send_data(uint8_t* data, size_t data_len) {
    if (socket_fd < 0) {
        return;
    }
    int err = sendto(socket_fd, data, data_len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0) {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
    }
};

static void udp_server_task(void *pvParameters) {
    char rx_buffer[128];
    char addr_str[128];
    int addr_family = (int)pvParameters;
    int ip_protocol = 0;
    struct sockaddr_in6 dest_addr;

    while (1) {
        struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
        dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(PORT);
        ip_protocol = IPPROTO_IP;

        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created");
        // Set timeout
        // struct timeval timeout;
        // timeout.tv_sec = 10;
        // timeout.tv_usec = 0;
        // setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);

        int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err < 0) {
            ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        }
        ESP_LOGI(TAG, "Socket bound, port %d", PORT);

        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t socklen = sizeof(source_addr);

        while (1) {
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);
            // Error occurred during receiving
            if (len < 0) {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                break;
            }
            // Data received
            else {
                // Get the sender's ip address as string
                inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
                // save address for sending back
                set_address(((struct sockaddr_in *)&source_addr)->sin_addr.s_addr);

                tccp_header_t *header = (tccp_header_t*)rx_buffer;
                if (header->type == TCCP_TYPE_CONTROL) {
                    ESP_ERROR_CHECK(esp_event_post_to(tccp_server_handle, CONTROL_EVENT, TCCP_CONTROL_EVENT, rx_buffer, sizeof(tccp_control_t), portMAX_DELAY));
                } else if (header->type == TCCP_TYPE_TELEMETRY) {
                    ESP_LOGW(TAG, "Not supposed to receive telemetry packets.");
                } else if (header->type == TCCP_TYPE_STREAM_CONTROL) {
                    ESP_ERROR_CHECK(esp_event_post_to(tccp_server_handle, CONTROL_EVENT, TCCP_STREAM_CONTROL_EVENT, rx_buffer, sizeof(tccp_stream_control_t), portMAX_DELAY));
                } else if (header->type == TCCP_TYPE_RTT) {
                    // RTT packets are not handled by main application but by us directly
                    tccp_rtt_t rtt_ack;
                    rtt_ack.header.type = TCCP_TYPE_RTT;
                    rtt_ack.timestamp = esp_timer_get_time() / 1000;
                    send_data((uint8_t*)&rtt_ack, sizeof(tccp_rtt_t));
                } else {
                    ESP_LOGW(TAG, "Unknown packet type.");
                }
            }
        }

        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}

in_addr_t* get_address_ref() {
    return &dest_addr.sin_addr.s_addr;
}

void init_tccp(void) {
    esp_event_loop_args_t loop_with_task_args = {
        .queue_size = 5,
        .task_name = "tccp_loop_task", // task will be created
        .task_priority = uxTaskPriorityGet(NULL),
        .task_stack_size = 3072,
        .task_core_id = tskNO_AFFINITY
    };

    ESP_ERROR_CHECK(esp_event_loop_create(&loop_with_task_args, &tccp_server_handle));
}

void register_tccp_event_handler(void (*handler)(void*, esp_event_base_t, int32_t, void*)) {
    ESP_ERROR_CHECK(esp_event_handler_register_with(tccp_server_handle, CONTROL_EVENT, ESP_EVENT_ANY_ID, handler, NULL));
}

void start_tccp(void) {
    xTaskCreate(udp_server_task, "udp_server", 3072, (void*)AF_INET, 4, NULL);
}

void send_telemetry(uint16_t battery_voltage, uint8_t current_fps, uint16_t min_frame_latency, int8_t wifi_rssi) {
    tccp_telemetry_t telemetry;
    telemetry.header.type = TCCP_TYPE_TELEMETRY;
    telemetry.battery_voltage = battery_voltage;
    telemetry.current_fps = current_fps;
    telemetry.min_frame_latency = min_frame_latency;
    telemetry.wifi_rssi = wifi_rssi;
    send_data((uint8_t*)&telemetry, sizeof(tccp_telemetry_t));
}