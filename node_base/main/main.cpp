#include "Arduino.h"

extern "C" {
    #include <stdio.h>
    #include <string.h>
    #include "esp_now.h"
    #include "esp_wifi.h"
    #include "nvs_flash.h"
    #include "esp_mac.h"
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    void app_main(void);
    //static void ondatarecv(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len);
}

#define WCHAN 8
#define MAXID 1024

static const int nodenum = 16;
int values[nodenum][nodenum - 1];
static const char* scanmsg = "scan";
static const char* checkmsg = "check";
static const char* datamsg = "data";
static const char* rsmsg = "restart";
int bootcnt = 0;
int note_expect = 0;
int data_expect[nodenum];
static const uint8_t macbase[] = {0x1a, 0xff, 0x00, 0x00, 0x00, 0x00};
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
int datacnt = 0;
bool nodeboot[nodenum];
bool boot = false;
bool ack = false;

typedef struct packet {
  int value[nodenum];
  int nodeid;
  int pktid;
  char info[16];
} packet;

packet receive, check, scan, restart;

static void ondatasent(const uint8_t *mac, esp_now_send_status_t status) {
  ack = true;
  if (!boot) {
    int nodeid = 0;
    for (int i = 0; i < nodenum; i++) {
      if (memcmp(mac, maclist[i], 6) == 0) {
        nodeid = i + 1;
      }
    }
    if (status == ESP_NOW_SEND_SUCCESS) {
      bootcnt++;
      nodeboot[nodeid - 1] = true;
      //printf("node#%d booted\n", nodeid);
      if (bootcnt == nodenum) {
        //printf("All set\n");
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        ESP_ERROR_CHECK(esp_now_send(maclist[0], (uint8_t*) &scan, sizeof(scan)));
        scan.pktid = (scan.pktid + 1) % MAXID;
        boot = true;
      }
    }
  }
}

static void ondatarecv(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
  memcpy(&receive, data, sizeof(receive));
  if (memcmp(recv_info->src_addr, maclist[nodenum - 1], 6) == 0 && receive.pktid == note_expect) {
    note_expect = (note_expect + 1) % MAXID;
    ESP_ERROR_CHECK(esp_now_send(maclist[0], (uint8_t*) &check, sizeof(check)));
  }
  else if (strcmp(receive.info, datamsg) == 0 && receive.pktid == data_expect[receive.nodeid - 1]) {
    //printf("Data received from #%d\n", receive.nodeid);
    data_expect[receive.nodeid - 1] = (data_expect[receive.nodeid - 1] + 1) % MAXID;
    int j = 0;
    for (int i = 0; i < nodenum; i++) {
      if (i == receive.nodeid - 1) {
        continue;
      }
      values[receive.nodeid - 1][j] = receive.value[i];
      j++;
    }
    datacnt++;
    if (receive.nodeid < nodenum) {
      ESP_ERROR_CHECK(esp_now_send(maclist[receive.nodeid], (uint8_t*) &check, sizeof(check)));
    }
    else {
      check.pktid = (check.pktid + 1) % MAXID;
    }
    if (datacnt == nodenum) {
      Serial.write('|');
      for (int i = 0; i < nodenum; i++) {
        //printf("node#%d: ", i + 1);
        for (int j = 0; j < nodenum - 1; j++) {
          //printf("%d ", values[i][j]);
          int rss = values[i][j] * -1;
          Serial.write(rss);
        }
        //printf("\n");
      }
      //printf("\n");
      //vTaskDelay(2500 / portTICK_PERIOD_MS);
      datacnt = 0;
      ESP_ERROR_CHECK(esp_now_send(maclist[0], (uint8_t*) &scan, sizeof(scan)));
      scan.pktid = (scan.pktid + 1) % MAXID;
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
    for (int i = 0; i < nodenum; i++) {
      nodeboot[i] = false;
      data_expect[i] = 0;
    }
    Serial.begin(115200);
    strcpy(check.info, checkmsg);
    check.pktid = 0;
    check.nodeid = 0;
    strcpy(scan.info, scanmsg);
    scan.pktid = 0;
    scan.nodeid = 0;
    strcpy(restart.info, rsmsg);
    restart.nodeid = 0;
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_config_espnow_rate(WIFI_IF_STA, WIFI_PHY_RATE_MCS0_SGI));
    ESP_ERROR_CHECK(esp_wifi_set_channel(WCHAN, WIFI_SECOND_CHAN_NONE));
    ESP_ERROR_CHECK(esp_wifi_set_mac(WIFI_IF_STA, macbase));
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_recv_cb(ondatarecv));
    ESP_ERROR_CHECK(esp_now_register_send_cb(ondatasent));
    esp_now_peer_info_t peer;
    peer.channel = WCHAN;
    peer.encrypt = false;
    peer.ifidx = WIFI_IF_STA;
    for (int i = 0; i < nodenum; i++) {
      memcpy(peer.peer_addr, maclist[i], 6);
      ESP_ERROR_CHECK(esp_now_add_peer(&peer));
    }
    for (int i = 0; i < nodenum; i++) {
      while (true) {
        ESP_ERROR_CHECK(esp_now_send(maclist[i], (uint8_t*) &restart, sizeof(restart)));
        while (!ack);
        ack = false;
        if (nodeboot[i]) {
          break;
        }
        vTaskDelay(1 / portTICK_PERIOD_MS);
      }
    }
    //ESP_ERROR_CHECK(esp_now_send(maclist[14], (uint8_t*) &restart, sizeof(restart)));
}