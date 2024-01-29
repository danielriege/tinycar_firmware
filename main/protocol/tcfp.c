#include "tcfp.h"

static const char *TAG = "tcfp";

static int tcfp_socket_fd = -1;
static struct sockaddr_in dest_addr;
static uint8_t tx_buffer[DGRAM_SIZE];
static uint16_t frame_num = 5623; // arbitrary value
static uint32_t last_frame_packet_count = 0;

void tcfp_set_destination(in_addr_t* dest_ip) {
    if (*dest_ip == dest_addr.sin_addr.s_addr) {
        return;
    }
    // create socket
    tcfp_socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (tcfp_socket_fd < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return;
    }
    ESP_LOGI(TAG, "Socket created");
    // setting global address struct to be used until next send_frame call
    dest_addr.sin_addr.s_addr = *dest_ip;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(RTP_PORT);
}

static int send_tcfp_packet(uint8_t* data, size_t data_len, uint8_t seq_num, uint32_t fragment_offset, uint8_t fragment_count, uint32_t timestamp, uint16_t width, uint16_t height, uint8_t marker, uint8_t rtt_start) {
    if (data_len > DGRAM_SIZE - sizeof(tcfp_header_t)) {
        ESP_LOGE(TAG, "Data size too large");
        return -1;
    }
    // create TCFP header
    tcfp_header_t* header = (tcfp_header_t*) tx_buffer;
    header->timestamp = timestamp;
    header->frame_num = frame_num;
    header->seq_num = seq_num;
    header->rtt_start = rtt_start;
    header->marker = marker;
    header->fragment_offset = fragment_offset;
    header->fragment_count = fragment_count;
    header->width = width / 8;
    header->height = height / 8;
    // copy data to tx_buffer
    memcpy(tx_buffer + sizeof(tcfp_header_t), data, data_len);
    
    // send TCFP packet
    int err = sendto(tcfp_socket_fd, tx_buffer, sizeof(tcfp_header_t) + data_len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0) {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
        return err;
    }
    return 0;
};

int tcfp_send_frame(uint8_t *frame, size_t size, uint16_t width, uint16_t height, uint8_t rtt_start) {
    if (tcfp_socket_fd < 0) {
        return -1;
    }
    uint32_t timestamp = esp_timer_get_time() / 1000;
    // fragmentation of frame
    size_t fragment_max_size = DGRAM_SIZE - sizeof(tcfp_header_t); 
    size_t fragment_count = size / fragment_max_size + 1;
    last_frame_packet_count = fragment_count;

    frame_num = (frame_num + 1) % 65536;

    for (size_t i = 0; i < fragment_count; i++) {
        size_t fragment_size = fragment_max_size;
        uint8_t marker = 0; // set on last fragment
        if (i == fragment_count - 1) {
            fragment_size = size % fragment_max_size;
            marker = 1;
        }
        if (send_tcfp_packet(frame + i*fragment_max_size, fragment_size, i, i*fragment_max_size, fragment_count, timestamp, width, height, marker, rtt_start) < 0) {
            ESP_LOGE(TAG, "Error occurred during sending TCFP packet");
            return -1;
        }
    }
    return 0;
}


uint32_t tcfp_get_last_packet_count() {
    return last_frame_packet_count;
}