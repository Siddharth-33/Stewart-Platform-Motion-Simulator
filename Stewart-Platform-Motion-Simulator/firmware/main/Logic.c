#include <stdio.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/ledc.h"
#include "esp_log.h"

#define UART_NUM UART_NUM_0
#define BUF_SIZE 1024
#define TAG "STEWART"

// ================= SERVO GPIOs =================
#define SERVO_1_GPIO 2
#define SERVO_2_GPIO 22
#define SERVO_3_GPIO 23

// ================= LEDC CONFIG =================
#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_FREQUENCY 50
#define LEDC_RESOLUTION LEDC_TIMER_13_BIT

// ================= SERVO PULSE =================
#define SERVO_MIN_PULSEWIDTH_US 500
#define SERVO_MAX_PULSEWIDTH_US 2500

#define SERVO_1_CHANNEL LEDC_CHANNEL_0
#define SERVO_2_CHANNEL LEDC_CHANNEL_1
#define SERVO_3_CHANNEL LEDC_CHANNEL_2

// ================= PLATFORM LIMITS =================
#define MAX_ROLL   40.0f
#define MAX_PITCH  40.0f
#define MAX_HEIGHT 20.0f

// ================= INVERTED SERVO RANGE =================
#define SERVO_MIN_ANGLE     0.0f
#define SERVO_MAX_ANGLE     90.0f
#define SERVO_CENTER_ANGLE  35.0f

// ================= SMOOTHING =================
#define LPF_ALPHA        0.12f   // input smoothing
#define MAX_SERVO_STEP   1.0f    // degrees per update
#define CONTROL_PERIOD_MS 20     // 50 Hz

// ================= GLOBAL STATE =================
float current_roll = 0, current_pitch = 0, current_height = 0;
float filt_roll = 0, filt_pitch = 0, filt_height = 0;

float last_s1 = SERVO_CENTER_ANGLE;
float last_s2 = SERVO_CENTER_ANGLE;
float last_s3 = SERVO_CENTER_ANGLE;

// ================= HELPERS =================
float low_pass(float prev, float input) {
    return prev + LPF_ALPHA * (input - prev);
}

float slew_limit(float prev, float target) {
    float diff = target - prev;
    if (diff > MAX_SERVO_STEP) diff = MAX_SERVO_STEP;
    if (diff < -MAX_SERVO_STEP) diff = -MAX_SERVO_STEP;
    return prev + diff;
}

// ================= ANGLE → DUTY =================
uint32_t angle_to_duty(float angle) {
    if (angle < SERVO_MIN_ANGLE) angle = SERVO_MIN_ANGLE;
    if (angle > SERVO_MAX_ANGLE) angle = SERVO_MAX_ANGLE;

    float physical_angle = 180.0f - angle;

    float pulse_width =
        SERVO_MIN_PULSEWIDTH_US +
        (physical_angle / 180.0f) *
        (SERVO_MAX_PULSEWIDTH_US - SERVO_MIN_PULSEWIDTH_US);

    return (uint32_t)((pulse_width / 20000.0f) * 8191);
}

// ================= SERVO CONTROL =================
void servo_set_angle(ledc_channel_t channel, float angle) {
    ledc_set_duty(LEDC_MODE, channel, angle_to_duty(angle));
    ledc_update_duty(LEDC_MODE, channel);
}

void servo_init(ledc_channel_t channel, int gpio) {
    ledc_channel_config_t ch = {
        .channel    = channel,
        .duty       = angle_to_duty(SERVO_CENTER_ANGLE),
        .gpio_num   = gpio,
        .speed_mode = LEDC_MODE,
        .timer_sel  = LEDC_TIMER,
        .hpoint     = 0
    };
    ledc_channel_config(&ch);
}

// ================= PLATFORM KINEMATICS =================
void calculate_platform_angles(float roll, float pitch, float height,
                               float *s1, float *s2, float *s3) {

    roll   *= MAX_ROLL;
    pitch  *= MAX_PITCH;
    height *= MAX_HEIGHT;

    float a1 = 0.0f, a2 = 120.0f, a3 = 240.0f;

    float l1 = height + roll * cosf(a1 * M_PI / 180.0f) +
                         pitch * sinf(a1 * M_PI / 180.0f);

    float l2 = height + roll * cosf(a2 * M_PI / 180.0f) +
                         pitch * sinf(a2 * M_PI / 180.0f);

    float l3 = height + roll * cosf(a3 * M_PI / 180.0f) +
                         pitch * sinf(a3 * M_PI / 180.0f);

    l1 *= 0.5f; l2 *= 0.5f; l3 *= 0.5f;

    *s1 = SERVO_CENTER_ANGLE + l1;
    *s2 = SERVO_CENTER_ANGLE + l2;
    *s3 = SERVO_CENTER_ANGLE + l3;
}

// ================= CONTROL LOOP =================
void update_platform(void) {
    float t1, t2, t3;

    calculate_platform_angles(
        current_roll, current_pitch, current_height,
        &t1, &t2, &t3
    );

    last_s1 = slew_limit(last_s1, t1);
    last_s2 = slew_limit(last_s2, t2);
    last_s3 = slew_limit(last_s3, t3);

    servo_set_angle(SERVO_1_CHANNEL, last_s1);
    servo_set_angle(SERVO_2_CHANNEL, last_s2);
    servo_set_angle(SERVO_3_CHANNEL, last_s3);
}

void control_task(void *arg) {
    while (1) {
        update_platform();
        vTaskDelay(pdMS_TO_TICKS(CONTROL_PERIOD_MS));
    }
}

// ================= UART =================
void parse_motion_data(const char *data) {
    float r, p, h;
    if (sscanf(data, "%f,%f,%f", &r, &p, &h) == 3) {
        filt_roll   = low_pass(filt_roll, r);
        filt_pitch  = low_pass(filt_pitch, p);
        filt_height = low_pass(filt_height, h);

        current_roll   = filt_roll;
        current_pitch  = filt_pitch;
        current_height = filt_height;
    }
}

void uart_rx_task(void *arg) {
    uint8_t buf[BUF_SIZE];
    char line[128];
    int pos = 0;

    while (1) {
        int len = uart_read_bytes(UART_NUM, buf, BUF_SIZE, 20 / portTICK_PERIOD_MS);
        for (int i = 0; i < len; i++) {
            if (buf[i] == '\n' || buf[i] == '\r') {
                if (pos > 0) {
                    line[pos] = '\0';
                    parse_motion_data(line);
                    pos = 0;
                }
            } else if (pos < sizeof(line) - 1) {
                line[pos++] = buf[i];
            }
        }
    }
}

// ================= MAIN =================
void app_main(void) {
    ESP_LOGI(TAG, "Smooth Stewart Platform Controller");

    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT
    };

    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM, &uart_config);

    ledc_timer_config_t timer = {
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER,
        .duty_resolution = LEDC_RESOLUTION,
        .freq_hz = LEDC_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer);

    servo_init(SERVO_1_CHANNEL, SERVO_1_GPIO);
    servo_init(SERVO_2_CHANNEL, SERVO_2_GPIO);
    servo_init(SERVO_3_CHANNEL, SERVO_3_GPIO);

    xTaskCreate(uart_rx_task, "uart_rx", 4096, NULL, 10, NULL);
    xTaskCreate(control_task, "control", 4096, NULL, 9, NULL);
}

