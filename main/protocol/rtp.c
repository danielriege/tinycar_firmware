#include "rtp.h"

static const char *TAG = "rtp";

static int rtp_socket_fd = -1;
static struct sockaddr_in dest_addr;
static uint16_t sequence_number = 2638; // arbitrary start value
static uint8_t tx_buffer[DGRAM_SIZE];
static uint8_t ms_delay = 0;
static uint32_t last_frame_packet_count = 0;

void rtp_set_destination(in_addr_t* dest_ip) {
    // create socket
    rtp_socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (rtp_socket_fd < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return;
    }
    ESP_LOGI(TAG, "Socket created");
    // setting global address struct to be used until next send_frame call
    dest_addr.sin_addr.s_addr = *dest_ip;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(RTP_PORT);
}

static int send_rtp_packet(uint8_t* data, size_t data_len, uint32_t fragment_offset, uint32_t timestamp, uint16_t width, uint16_t height, uint8_t marker) {
    if (data_len > DGRAM_SIZE - sizeof(rtp_header_t) - sizeof(jpeg_header_t)) {
        ESP_LOGE(TAG, "Data size too large");
        return -1;
    }
    // create RTP header
    rtp_header_t* header = (rtp_header_t*) tx_buffer;
    header->version = 2;
    header->padding = 0;
    header->extension = 0;
    header->csrc_count = 0;
    header->marker = marker;
    header->payload_type = 26; // MJPEG
    header->sequence_number = htons(sequence_number);
    header->timestamp = htonl(timestamp);
    header->ssrc = 7189333; // arbitrary value
    sequence_number = (sequence_number + 1) % 65536;
    // create JPEG header
    jpeg_header_t* jpeg_header = (jpeg_header_t*) (tx_buffer + sizeof(rtp_header_t));
    jpeg_header->type_specific = 0;
    jpeg_header->fragment_offset = htonl(fragment_offset);
    jpeg_header->type = 0; // ignored
    jpeg_header->q = 0; // ignored
    jpeg_header->width = width / 8;
    jpeg_header->height = height / 8;
    // copy data to tx_buffer
    memcpy(tx_buffer + sizeof(rtp_header_t) + sizeof(jpeg_header_t), data, data_len);
    // send RTP packet
    int err = sendto(rtp_socket_fd, tx_buffer, sizeof(rtp_header_t) + sizeof(jpeg_header_t) + data_len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0) {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
        return err;
    }
    return 0;
};

int rtp_send_frame(uint8_t *frame, size_t size, uint16_t width, uint16_t height) {
    if (rtp_socket_fd < 0) {
        return -1;
    }
    uint32_t timestamp = esp_timer_get_time() / 1000;
    // fragmentation of frame
    size_t fragment_max_size = DGRAM_SIZE - sizeof(rtp_header_t) - sizeof(jpeg_header_t); 
    size_t fragment_count = size / fragment_max_size + 1;
    last_frame_packet_count = fragment_count;
    for (size_t i = 0; i < fragment_count; i++) {
        size_t fragment_size = fragment_max_size;
        uint8_t marker = 0; // set on last fragment
        if (i == fragment_count - 1) {
            fragment_size = size % fragment_max_size;
            marker = 1;
        }
        if (send_rtp_packet(frame + i*fragment_max_size, fragment_size, i*fragment_max_size, timestamp, width, height, marker) < 0) {
            ESP_LOGE(TAG, "Error occurred during sending RTP packet");
            return -1;
        }
        // set delay
        vTaskDelay(ms_delay / portTICK_PERIOD_MS);
    }
    return 0;
}

void rtp_set_packet_delay(uint8_t delay) {
    ms_delay = delay;
}

uint32_t rtp_get_last_packet_count() {
    return last_frame_packet_count;
}