#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"

static const char *TAG = "sniffer";

// Maximum number of MAC addresses to track
#define MAX_MAC_ADDRESSES 100

// Array to store previously seen MAC addresses
static uint8_t mac_addresses[MAX_MAC_ADDRESSES][6];
static int mac_count = 0;

// Function to check if MAC address is already in the list
bool is_mac_seen(const uint8_t *mac) {
    for (int i = 0; i < mac_count; i++) {
        if (memcmp(mac_addresses[i], mac, 6) == 0) {
            return true;  // MAC address found
        }
    }
    return false;
}

// Function to add a new MAC address to the list
void add_mac_address(const uint8_t *mac) {
    if (mac_count < MAX_MAC_ADDRESSES) {
        memcpy(mac_addresses[mac_count], mac, 6);
        mac_count++;
    } else {
        // Overwrite the oldest MAC address
        memmove(mac_addresses, mac_addresses + 1, (MAX_MAC_ADDRESSES - 1) * 6);
        memcpy(mac_addresses[MAX_MAC_ADDRESSES - 1], mac, 6);
    }
}

// Callback for received Wi-Fi packets
static void wifi_sniffer_packet_handler(void *buff, wifi_promiscuous_pkt_type_t type) {
    const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buff;
    const uint8_t *payload = ppkt->payload;

    if (type == WIFI_PKT_MGMT) {
        // Beacon frame or probe response
        if (payload[0] == 0x80 || payload[0] == 0x50) {
            uint8_t ssid_len = payload[37];
            if (ssid_len > 0 && ssid_len < 33) {
                char ssid[33] = {0};
                memcpy(ssid, &payload[38], ssid_len);

                uint8_t mac[6] = { payload[10], payload[11], payload[12], payload[13], payload[14], payload[15] };

                // Check if this MAC address has been seen before
                if (!is_mac_seen(mac)) {
                    // Print SSID, RSSI, and MAC in aligned columns
                    printf("📡 SSID: %-30s | RSSI: %-4d | MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
                        ssid, ppkt->rx_ctrl.rssi,
                        payload[10], payload[11], payload[12],
                        payload[13], payload[14], payload[15]);

                    // Add the MAC address to the list to avoid duplicates
                    add_mac_address(mac);
                }
            }
        }
    } else if (type == WIFI_PKT_DATA) {
        // Optional: Print MAC + RSSI for data frames
        uint8_t mac[6] = { payload[10], payload[11], payload[12], payload[13], payload[14], payload[15] };
        if (!is_mac_seen(mac)) {
            // Print DATA Frame with aligned columns
            printf("📦 DATA Frame        | RSSI: %-4d   | MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
                ppkt->rx_ctrl.rssi,
                payload[10], payload[11], payload[12],
                payload[13], payload[14], payload[15]);

            // Add the MAC address to the list to avoid duplicates
            add_mac_address(mac);
        }
    }
}


// Task to cycle through Wi-Fi channels
void channel_hop_task(void *pvParameters) {
    uint8_t channel = 1;
    while (true) {
        esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
        // Suppressed the log output
        // ESP_LOGI(TAG, "Switched to channel %d", channel);
        channel = (channel % 13) + 1;  // Loop channels 1-13
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}


// Main entry point
void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
    ESP_ERROR_CHECK(esp_wifi_start());

    // Start sniffer
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_packet_handler));

    // Start channel hopping task
    xTaskCreate(channel_hop_task, "channel_hop_task", 2048, NULL, 1, NULL);

    ESP_LOGI(TAG, "🚀 Sniffer started. Scanning Wi-Fi activity...");
}
