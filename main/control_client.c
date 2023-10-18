#include "include/control_client.h"

static const char *TAG = "control client";

static int socket_fd = -1;
static uint8_t tx_buffer[DGRAM_SIZE];
static struct sockaddr_in dest_addr;

static void create_socket() {
    socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (socket_fd < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return;
    }
    ESP_LOGI(TAG, "Socket created");
}

uint8_t* cc_get_buffer() {
    // on index 0 is the message type we dont want to overwrite
    return tx_buffer + 1;
}

void cc_send_data(uint8_t message_type, uint8_t* data, size_t data_len) {
    tx_buffer[0] = message_type;
    if (data != cc_get_buffer()) {
        ESP_LOGI(TAG, "Copying data to tx_buffer.");
        memcpy(tx_buffer + 1, data, data_len);
    } 
    int err = sendto(socket_fd, tx_buffer, data_len+1, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0) {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
    }
}

void cc_set_address(in_addr_t dest_ip, uint16_t dest_port) {
    // create socket if not already created
    if (socket_fd < 0) {
        create_socket();
    }
    // setting global address struct to be used until next send_frame call
    dest_addr.sin_addr.s_addr = dest_ip;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(dest_port);
}