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
    .frame_size = FRAMESIZE_VGA,//QQVGA-UXGA, For ESP32, do not use sizes above QVGA when not JPEG. The performance of the ESP32-S series has improved a lot, but JPEG mode always gives better frame rates.

    .jpeg_quality = 15, //0-63, for OV series camera sensors, lower number means higher quality
    .fb_count = 1, //When jpeg mode is used, if fb_count more than one, the driver will work in continuous mode.
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY//CAMERA_GRAB_LATEST. Sets when buffers should be filled
};

static void send_frame_metadata(size_t tag, size_t height, size_t width, uint8_t pixelformat, size_t buf_len) {
    frame_metadata_event_t* frame_metadata = (frame_metadata_event_t*) cc_get_buffer();
    frame_metadata->tag = tag;
    frame_metadata->height = height;
    frame_metadata->width = width;
    frame_metadata->pixel_format = pixelformat;
    frame_metadata->frame_size = buf_len;
    cc_send_data(PROTOCOL_OUT_FRAME_METADATA, (uint8_t*) frame_metadata, sizeof(frame_metadata_event_t));
}

// send a fragment of a frame to a server
static void send_fragment(uint8_t * fragment, size_t fragment_size, uint32_t tag, uint32_t fragment_id) {
    if (fragment_size > DGRAM_SIZE - 11) {
        ESP_LOGE(TAG, "Fragment size too large");
        return;
    }
    frame_fragment_event_t* frame_fragment = (frame_fragment_event_t*) cc_get_buffer();
    frame_fragment->tag = tag;
    frame_fragment->fragment_id = fragment_id;
    frame_fragment->fragment_size = fragment_size;
    memcpy(cc_get_buffer()+sizeof(frame_fragment_event_t), fragment, fragment_size);
    cc_send_data(PROTOCOL_OUT_FRAME_FRAGMENT, (uint8_t*) frame_fragment, sizeof(frame_fragment_event_t) + fragment_size);
}

// Send a frame to a server
static void send_frame(uint8_t *frame_buf, size_t frame_buf_len, uint32_t tag) {

    // fragmentation of frame
    size_t fragment_max_size = DGRAM_SIZE - sizeof(frame_fragment_event_t) - 1; // 1 for message type
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

