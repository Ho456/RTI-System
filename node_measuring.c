#include <stdio.h>
#include <string.h>
#include "esp_now.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_mac.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "math.h"

#define WCHAN 8
#define nodenum 16
#define MAXID 1024

static const int num = 4;
static const int tosend = num - 1;
static const uint8_t macbase[] = {0x1a, 0xff, 0x00, 0x00, 0x00, 0x00};
static const char* checkmsg = "check";
static const char* scanmsg = "scan";
static const char* datamsg = "data";
static const char* notemsg = "note";
static const char* rsmsg = "restart";
static const uint8_t mac01[] = {0x1a, 0x01, 0x00, 0x00, 0x00, 0x00};
static const uint8_t mac02[] = {0x1a, 0x02, 0x00, 0x00, 0x00, 0x00};
static const uint8_t mac03[] = {0x1a, 0x03, 0x00, 0x00, 0x00, 0x00};
static const uint8_t mac04[] = {0x1a, 0x04, 0x00, 0x00, 0x00, 0x00};
static const uint8_t mac05[] = {0x1a, 0x05, 0x00, 0x00, 0x00, 0x00};
static const uint8_t mac06[] = {0x1a, 0x06, 0x00, 0x00, 0x00, 0x00};
static const uint8_t mac07[] = {0x1a, 0x07, 0x00, 0x00, 0x00, 0x00};
static const uint8_t mac08[] = {0x1a, 0x08, 0x00, 0x00, 0x00, 0x00};
static const uint8_t mac09[] = {0x1a, 0x09, 0x00, 0x00, 0x00, 0x00};
static const uint8_t mac10[] = {0x1a, 0x10, 0x00, 0x00, 0x00, 0x00};
static const uint8_t mac11[] = {0x1a, 0x11, 0x00, 0x00, 0x00, 0x00};
static const uint8_t mac12[] = {0x1a, 0x12, 0x00, 0x00, 0x00, 0x00};
static const uint8_t mac13[] = {0x1a, 0x13, 0x00, 0x00, 0x00, 0x00};
static const uint8_t mac14[] = {0x1a, 0x14, 0x00, 0x00, 0x00, 0x00};
static const uint8_t mac15[] = {0x1a, 0x15, 0x00, 0x00, 0x00, 0x00};
static const uint8_t mac16[] = {0x1a, 0x16, 0x00, 0x00, 0x00, 0x00};
static const uint8_t* maclist[] = {mac01, mac02, mac03, mac04, mac05, mac06, mac07, mac08, mac09, mac10, mac11, mac12, mac13, mac14, mac15, mac16};
bool scanned[nodenum];
int scan_expect = 0;
int check_expect = 0;
bool isSent = false;
bool ack = false;

typedef struct packet {
    int value[nodenum];
    int nodeid;
    int pktid;
    char info[16];
} packet;

packet receive, rdata, note;

static void oncsirecv(void *ctx, wifi_csi_info_t *info) {
    if (memcmp(info->dmac, maclist[num - 1], 6) != 0 || memcmp(info->mac, macbase, 6) == 0) {
        return;
    }
    int nodeid = 0;
    for (int i = 0; i < nodenum; i++) {
        if (memcmp(info->mac, maclist[i], 6) == 0) {
            nodeid = i + 1;
            break;
        }
    }
    if (scanned[nodeid - 1] == false) {
        rdata.value[nodeid - 1] = info->rx_ctrl.rssi;    //This line of code is to collect RSSI values from every other measuring nodes
        
        // This block of code is to enable CSI data collection(amplitude). However, the value of the CSI amplitude does not decrease when the link is blocked by an object like RSSI.
        
        /*int sum = 0;
        for (int i = 0; i < (info->len / 2); i++) {
            sum+=(pow(info->buf[2*i], 2) + pow(info->buf[2*i + 1], 2));
        }
        rdata.value[nodeid - 1] = sum; */
        
        if (nodeid == tosend) {
            for (int i = 0; i < nodenum; i++) {
                if (i == num - 1 || i == num) {
                    continue;
                }
                esp_now_send(maclist[i], (uint8_t*) &note, sizeof(note));
            }
            esp_now_send((num == nodenum) ? macbase : maclist[num], (uint8_t*) &note, sizeof(note));
            note.pktid = (note.pktid + 1) % MAXID;
        }
        scanned[nodeid - 1] = true;
    }
}

static void ondatarecv(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
    if (memcmp(recv_info->des_addr, maclist[num - 1], 6) != 0) {
        return;
    }
    memcpy(&receive, data, sizeof(packet));
    if(receive.nodeid == 0) {
        if (strcmp(receive.info, scanmsg) == 0 && receive.pktid == scan_expect) {
            scan_expect = (scan_expect + 1) % MAXID;
            for (int i = 0; i < nodenum; i++) {
                if (i == num - 1 || i == num) {
                    continue;
                }
                esp_now_send(maclist[i], (uint8_t*) &note, sizeof(note));
            }
            esp_now_send(maclist[num], (uint8_t*) &note, sizeof(note));
            note.pktid = (note.pktid + 1) % MAXID;
        }
        else if (strcmp(receive.info, checkmsg) == 0 && receive.pktid == check_expect) {
            check_expect = (check_expect + 1) % MAXID;
            esp_now_send(macbase, (uint8_t*) &rdata, sizeof(rdata));
            rdata.pktid = (rdata.pktid + 1) % MAXID;
            for (int i = 0; i < nodenum; i++) {
                if (i == num - 1) {
                    continue;
                }
                scanned[i] = false;
            }
        }
        else if (strcmp(receive.info, rsmsg) == 0) {
            esp_restart();
        }
    }
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    strcpy(rdata.info, datamsg);
    rdata.nodeid = num;
    rdata.pktid = 0;
    strcpy(note.info, notemsg);
    note.nodeid = num;
    note.pktid = 0;
    for (int i = 0; i < nodenum; i++) {
        scanned[i] = (i == num - 1) ? true : false;
    }
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_config_espnow_rate(WIFI_IF_STA, WIFI_PHY_RATE_MCS0_SGI));
    ESP_ERROR_CHECK(esp_wifi_set_channel(WCHAN, WIFI_SECOND_CHAN_NONE));
    ESP_ERROR_CHECK(esp_wifi_set_mac(WIFI_IF_STA, maclist[num - 1]));
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));
    wifi_csi_config_t csi_config = {
        .lltf_en           = true,
        .htltf_en          = true,
        .stbc_htltf2_en    = true,
        .ltf_merge_en      = true,
        .channel_filter_en = true,
        .manu_scale        = false,
        .shift             = false,
    };
    ESP_ERROR_CHECK(esp_wifi_set_csi_config(&csi_config));
    ESP_ERROR_CHECK(esp_wifi_set_csi_rx_cb(oncsirecv, NULL));
    ESP_ERROR_CHECK(esp_wifi_set_csi(true));
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_recv_cb(ondatarecv));
    esp_now_peer_info_t peer;
    peer.channel = WCHAN;
    peer.ifidx = WIFI_IF_STA;
    peer.encrypt = false;
    memcpy(peer.peer_addr, macbase, 6);
    ESP_ERROR_CHECK(esp_now_add_peer(&peer));
    for (int i = 0; i < nodenum; i++) {
        if (i == num - 1) {
            continue;
        }
        memcpy(peer.peer_addr, maclist[i], 6);
        ESP_ERROR_CHECK(esp_now_add_peer(&peer));
    }
}
