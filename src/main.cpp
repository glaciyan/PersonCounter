#include "esp_log.h"
#include "driver/rmt_tx.h"
#include "soc/clk_tree_defs.h"

#include <thread>

#define GPIO_LED (GPIO_NUM_38)

const char *TAG = "STAMP";

extern "C" void app_main()
{
    rmt_tx_channel_config_t config{};
    config.gpio_num = GPIO_LED;
    config.clk_src = RMT_CLK_SRC_APB; // Check Power Management

    config.resolution_hz = 10'000'000; // 10MHz -> 100ns period

    config.mem_block_symbols = 64; // 24 * 10
    config.trans_queue_depth = 4;   // 4 backlog items, source: my ass
    config.flags.invert_out = false;
    config.flags.with_dma = false;
    config.flags.io_loop_back = false;
    config.flags.io_od_mode = false;

    ESP_LOGI(TAG, "res: %lu\n", config.resolution_hz);

    rmt_channel_handle_t channel = nullptr;

    ESP_ERROR_CHECK(rmt_new_tx_channel(&config, &channel));
    ESP_ERROR_CHECK(rmt_enable(channel));

    while (true)
    {
        ESP_LOGI(TAG, "Hello World");
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}