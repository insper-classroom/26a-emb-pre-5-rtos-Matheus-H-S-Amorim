#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>

const int BTN_PIN_R = 28;
const int BTN_PIN_G = 26;

const int LED_PIN_R = 4;
const int LED_PIN_G = 6;
QueueHandle_t xQueueR;
QueueHandle_t xQueueG;

/* LED G */
void led_2_task(void *p) {
    gpio_init(LED_PIN_G);
    gpio_set_dir(LED_PIN_G, GPIO_OUT);

    int delay = 0;

    while (true) {
        if (xQueueReceive(xQueueG, &delay, 0)) {
            printf("G %d\n", delay);
        }

        if (delay > 0) {
            gpio_put(LED_PIN_G, 1);
            vTaskDelay(pdMS_TO_TICKS(delay));
            gpio_put(LED_PIN_G, 0);
            vTaskDelay(pdMS_TO_TICKS(delay));
        }
    }
}

/* BTN G */
void btn_2_task(void *p) {
    gpio_init(BTN_PIN_G);
    gpio_set_dir(BTN_PIN_G, GPIO_IN);
    gpio_pull_up(BTN_PIN_G);

    int delay = 0;

    while (true) {
        if (!gpio_get(BTN_PIN_G)) {

            while (!gpio_get(BTN_PIN_G)) {
                vTaskDelay(pdMS_TO_TICKS(1));
            }

            if (delay < 1000) delay += 100;
            else delay = 100;

            xQueueSend(xQueueG, &delay, 0);
        }
    }
}

int main() {
    stdio_init_all();

    xQueueR = xQueueCreate(32, sizeof(int));
    xQueueG = xQueueCreate(32, sizeof(int));

    xTaskCreate(led_1_task, "LED_R", 256, NULL, 1, NULL);
    xTaskCreate(btn_1_task, "BTN_R", 256, NULL, 1, NULL);

    xTaskCreate(led_2_task, "LED_G", 256, NULL, 1, NULL);
    xTaskCreate(btn_2_task, "BTN_G", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true);
}