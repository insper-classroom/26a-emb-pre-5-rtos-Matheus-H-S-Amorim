#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

const int BTN_PIN_R = 28;
const int BTN_PIN_Y = 21;

const int LED_PIN_R = 5;
const int LED_PIN_Y = 10;

// Recursos RTOS — únicos globais permitidos
SemaphoreHandle_t xSemaphore_r;
SemaphoreHandle_t xSemaphore_y;

void btn_callback(uint gpio, uint32_t events) {
    if (events == GPIO_IRQ_EDGE_FALL) {
        if (gpio == BTN_PIN_R) {
            xSemaphoreGiveFromISR(xSemaphore_r, 0);
        } else if (gpio == BTN_PIN_Y) {
            xSemaphoreGiveFromISR(xSemaphore_y, 0);
        }
    }
}

void btn_r_task(void *p) {
    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);
    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true,
                                       &btn_callback);
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(10)); // a task só inicializa o HW e mantém a IRQ viva ali
    }
}

void btn_y_task(void *p) {
    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, GPIO_IN);
    gpio_pull_up(BTN_PIN_Y);
    gpio_set_irq_enabled(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true); // mesmo callback global lá
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void led_r_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);
    gpio_put(LED_PIN_R, 0);

    int blinking = 0;
    while (true) {
        // Checa semáforo sem bloquear p poder piscar ao mesmo tempo
        if (xSemaphoreTake(xSemaphore_r, 0) == pdTRUE) {
            blinking = !blinking;
            if (!blinking)
                gpio_put(LED_PIN_R, 0); // apaga ao parar
        }

        if (blinking) {
            gpio_put(LED_PIN_R, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

void led_y_task(void *p) {
    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);
    gpio_put(LED_PIN_Y, 0);

    int blinking = 0;
    while (true) {
        if (xSemaphoreTake(xSemaphore_y, 0) == pdTRUE) {
            blinking = !blinking;
            if (!blinking)
                gpio_put(LED_PIN_Y, 0);
        }

        if (blinking) {
            gpio_put(LED_PIN_Y, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(LED_PIN_Y, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

int main() {
    stdio_init_all();
    // printf("Start RTOS \n"); // comentado p n atrapalhar aa simulação

    xSemaphore_r = xSemaphoreCreateBinary();
    xSemaphore_y = xSemaphoreCreateBinary();

    xTaskCreate(btn_r_task, "BTN_R",  256, NULL, 2, NULL);
    xTaskCreate(btn_y_task, "BTN_Y",  256, NULL, 2, NULL);
    xTaskCreate(led_r_task, "LED_R",  256, NULL, 1, NULL);
    xTaskCreate(led_y_task, "LED_Y",  256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}