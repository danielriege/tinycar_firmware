#include "include/camera.h"

static const char *TAG = "camera";

//TINYCAR ESP PIN MAP
#define CAM_PIN_PWDN    -1 //power down is not used
#define CAM_PIN_RESET   -1 //software reset will be performed
#define CAM_PIN_XCLK    15
#define CAM_PIN_SIOD    4
#define CAM_PIN_SIOC    5

#define CAM_PIN_D9      16
#define CAM_PIN_D8      17
#define CAM_PIN_D7      18
#define CAM_PIN_D6      12
#define CAM_PIN_D5      10
#define CAM_PIN_D4      8
#define CAM_PIN_D3      9
#define CAM_PIN_D2      11
#define CAM_PIN_D1      -1
#define CAM_PIN_D0      -1
#define CAM_PIN_VSYNC   6
#define CAM_PIN_HREF    7
#define CAM_PIN_PCLK    13

#define UDP_DGRAM_SIZE  1024

static camera_config_t camera_config = {
    .pin_pwdn  = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sccb_sda = CAM_PIN_SIOD,
    .pin_sccb_scl = CAM_PIN_SIOC,

    .pin_d7 = CAM_PIN_D9,
    .pin_d6 = CAM_PIN_D8,
    .pin_d5 = CAM_PIN_D7,
    .pin_d4 = CAM_PIN_D6,
    .pin_d3 = CAM_PIN_D5,
    .pin_d2 = CAM_PIN_D4,
    .pin_d1 = CAM_PIN_D3,
    .pin_d0 = CAM_PIN_D2,

    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,

    .xclk_freq_hz = 20000000,//EXPERIMENTAL: Set to 16MHz on ESP32-S2 or ESP32-S3 to enable EDMA mode
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG,//YUV422,GRAYSCALE,RGB565,JPEG
    .frame_size = FRAMESIZE_HD,//QQVGA-UXGA, For ESP32, do not use sizes above QVGA when not JPEG. The performance of the ESP32-S series has improved a lot, but JPEG mode always gives better frame rates.

    .jpeg_quality = 10, //0-63, for OV series camera sensors, lower number means higher quality
    .fb_count = 1, //When jpeg mode is used, if fb_count more than one, the driver will work in continuous mode.
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY//CAMERA_GRAB_LATEST. Sets when buffers should be filled
};

static uint8_t tx_buffer[UDP_DGRAM_SIZE];

static void send_frame_metadata(size_t tag, size_t height, size_t width, uint8_t pixelformat, size_t buf_len) {
    uint8_t frame_metadata[14];
    // first byte is frame metadata descriptor
    frame_metadata[0] = 0x00;
    // next 4 bytes are tag
    frame_metadata[1] = tag >> 24;
    frame_metadata[2] = tag >> 16;
    frame_metadata[3] = tag >> 8;
    frame_metadata[4] = tag & 0xFF;
    // next 2 bytes are height
    frame_metadata[5] = height >> 8;
    frame_metadata[6] = height & 0xFF;
    // next 2 bytes are width
    frame_metadata[7] = width >> 8;
    frame_metadata[8] = width & 0xFF;
    // next byte is pixelformat
    frame_metadata[9] = pixelformat;
    // next 4 bytes are buf_len
    frame_metadata[10] = buf_len >> 24;
    frame_metadata[11] = buf_len >> 16;
    frame_metadata[12] = buf_len >> 8;
    frame_metadata[13] = buf_len & 0xFF;
    send_data(frame_metadata, 14);
}

// send a fragment of a frame to a server
static void send_fragment(uint8_t * fragment, size_t fragment_size, uint32_t tag, uint32_t fragment_id) {
    if (fragment_size > UDP_DGRAM_SIZE - 11) {
        ESP_LOGE(TAG, "Fragment size too large");
        return;
    }
    // first byte is fragment descriptor
    tx_buffer[0] = 0x01;
    // next 4 bytes are tag
    tx_buffer[1] = tag >> 24;
    tx_buffer[2] = tag >> 16;
    tx_buffer[3] = tag >> 8;
    tx_buffer[4] = tag & 0xFF;
    // next 4 bytes are fragment id
    tx_buffer[5] = fragment_id >> 24;
    tx_buffer[6] = fragment_id >> 16;
    tx_buffer[7] = fragment_id >> 8;
    tx_buffer[8] = fragment_id & 0xFF;
    // next 2 bytes are fragment size
    tx_buffer[9] = fragment_size >> 8;
    tx_buffer[10] = fragment_size & 0xFF;
    // next bytes are fragment
    memcpy(tx_buffer+11, fragment, fragment_size);
    send_data(tx_buffer, fragment_size+11);
}

// Send a frame to a server
static void send_frame(uint8_t *frame_buf, size_t frame_buf_len, uint32_t tag) {

    // fragmentation of frame
    size_t fragment_max_size = UDP_DGRAM_SIZE - 12;
    size_t fragment_count = frame_buf_len / fragment_max_size + 1;
    for (size_t i = 0; i < fragment_count; i++) {
        size_t fragment_size = fragment_max_size;
        if (i == fragment_count - 1) {
            fragment_size = frame_buf_len % fragment_max_size;
        }
        send_fragment(frame_buf + i*fragment_max_size, fragment_size, tag, i);
    }
}

esp_err_t camera_init(){
    //initialize the camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera Init Failed");
        return err;
    }

    return ESP_OK;
}

void camera_set_config(uint8_t resolution, uint8_t jpeg_quality){
    esp_camera_deinit();
    camera_config.frame_size = resolution;
    camera_config.jpeg_quality = jpeg_quality;
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera Init Failed");
    }
}

esp_err_t camera_capture_and_send(uint32_t tag){
    //acquire a frame
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Camera Capture Failed");
        return ESP_FAIL;
    }
    
    send_frame_metadata(tag, fb->height, fb->width, fb->format, fb->len);
    send_frame(fb->buf, fb->len, tag);
  
    //return the frame buffer back to the driver for reuse
    esp_camera_fb_return(fb);
    return ESP_OK;
}

