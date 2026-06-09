#include "lvglDrivers.h"
#include "lv_conf.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"

static SemaphoreHandle_t lvglMutex;

bool lvglLock(TickType_t xBlockTime)
{
    if (xSemaphoreTake(lvglMutex, xBlockTime) == pdTRUE)
    {
        return true;
    }
    return false;
}

bool lvglUnlock()
{
    if (xSemaphoreGive(lvglMutex) == pdTRUE)
    {
        return true;
    }
    return false;
}

static void lvglTask(void *pvParameters)
{
    while (1)
    {
        xSemaphoreTake(lvglMutex, portMAX_DELAY);
        uint32_t time_till_next = lv_timer_handler();
        xSemaphoreGive(lvglMutex);
        vTaskDelay(pdMS_TO_TICKS(time_till_next));
    }
}

static void my_flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
{
    uint32_t *buf = (uint32_t *)px_map;
    int32_t x, y;
    for (y = area->y1; y <= area->y2; y++)
    {
        for (x = area->x1; x <= area->x2; x++)
        {
            BSP_LCD_DrawPixel(x, y, *buf);
            buf++;
        }
    }

    // IMPORTANT!!!
    // Inform LVGL that you are ready with the flushing and buf is not used anymore
    lv_display_flush_ready(display);
}

static void my_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    TS_StateTypeDef TS_State;
    BSP_TS_GetState(&TS_State);

    if (TS_State.touchDetected != 0)
    {
        data->point.x = TS_State.touchX[0];
        data->point.y = TS_State.touchY[0];
        data->state = LV_INDEV_STATE_PRESSED;
    }
    else
    {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}
static uint32_t my_tick_get_cb(void) {
    return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

