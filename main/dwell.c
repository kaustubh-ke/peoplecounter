#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "esp_event_loop.h"
#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "esp_spiffs.h"
#include "esp_timer.h"
//#include <time.h>
#include "sqlite3.h"
#include <string.h>
#include "esp_log.h"
#include "stdlib.h"
#include "dwell.h"

#define WIFI_CHANNEL_SWITCH_INTERVAL (10000)
#define WIFI_CHANNEL_MAX (13)
//struct timeval now;
int mac_count = 0;
uint8_t mac_lib[100][6];
static time_t start_time;
int q;
int val[100];
int l ;
int dwell_t =0;
char sql[1024];
sqlite3 *db1;
typedef struct
{
  unsigned frame_ctrl : 16;
  unsigned duration_id : 16;
  uint8_t addr1[6]; /* receiver address */
  uint8_t addr2[6]; /* sender address */
  uint8_t addr3[6]; /* filtering address */
  unsigned sequence_ctrl : 16;
  uint8_t addr4[6]; /* optional */
} wifi_ieee80211_mac_hdr_t;

typedef struct
{
  wifi_ieee80211_mac_hdr_t hdr;
  uint8_t payload[0]; /* network data ended with 4 bytes csum (CRC32) */
} wifi_ieee80211_packet_t;

const char* data = "Callback function called";
static int callback(void *data, int argc, char **argv, char **azColName) {
   int i;
  // printf("%s: ", (const char*)data);
   for (i = 0; i<argc; i++){
     printf("%s = %s ", azColName[i], argv[i] ? argv[i] : "NULL");
   }
  printf("\n");
   return 0;
}

int db_open(const char *filename, sqlite3 **db) {
   int rc = sqlite3_open(filename, db);
   if (rc) {
       printf("Can't open database: %s\n", sqlite3_errmsg(*db));
       return rc;
   } else {
      // printf("Opened database successfully\n");
   }
   return rc;
}

char *zErrMsg = 0;
int db_exec(sqlite3 *db, const char *sql) {
  // printf("%s\n", sql);
   //int64_t start = esp_timer_get_time();
   int rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
   if (rc != SQLITE_OK) {
  //     printf("SQL error: %s\n", zErrMsg);
       sqlite3_free(zErrMsg);
   } else {
     //  printf("Operation done successfully\n");
   }
  // printf("Time taken: %lld\n", esp_timer_get_time()-start);
   return rc;
}

int check_mac_only(const uint8_t addr2[6])
{
  gettimeofday(&now, NULL);
  start_time = now.tv_sec;
  for ( q = 0; q < mac_count; q++)
  {
    bool flag = true;
    for (int j = 0; j < 3; j++)
    {

      if (mac_lib[q][j] != addr2[j])
      {
        flag = false;
        break;
      }

    }
    if (flag == true)
      return 0;
  }
  for (int j = 0; j < 3; j++)
  {
    mac_lib[mac_count][j] = addr2[j];
  }
  if(mac_count>=7000)
  {
	  unlink("/spiffs/test1.db");
	  esp_restart();
  }
  mac_count++;

  return 1;
}
static wifi_country_t wifi_country = {.cc = "IN", .schan = 1, .nchan = 13};
static esp_err_t event_handler(void *ctx, system_event_t *event);
//static void wifi_sniffer_init(void);
static void wifi_sniffer_set_channel(uint8_t channel);
static void wifi_sniffer_packet_handler(void *buff, wifi_promiscuous_pkt_type_t type);

void channelSet()
{
	uint8_t channel = 1;
	while(channel<=13)
	{
	vTaskDelay(WIFI_CHANNEL_SWITCH_INTERVAL / portTICK_PERIOD_MS);
	wifi_sniffer_set_channel(channel);
	//printf("switch time : %ld \n",now.tv_sec);
	channel = (channel % WIFI_CHANNEL_MAX) + 1;
	
	  }

}

static int callback2(void *data, int argc, char **argv, char **azColName) {
   int i;
   for (i = 0; i<argc; i++)
   {
      // printf(" %s ", argv[i] ? argv[i] : "NULL");
       val[l]= atoi( argv[i]);
      // printf("%s = %s ", azColName[i], argv[i] ? argv[i] : "NULL");
       l++;
   }

  // printf("\n");
   return 0;
}
int db_exec1(sqlite3 *db, const char *sql) {
  // printf("%s\n", sql);
   //int64_t start = esp_timer_get_time();
   int rc = sqlite3_exec(db, sql, callback2, (void*)data, &zErrMsg);
   if (rc != SQLITE_OK) {
     //  printf("SQL error: %s\n", zErrMsg);
       sqlite3_free(zErrMsg);
   } else {
      // printf("Operation done successfully \n");
   }
  // printf("Time taken: %lld\n", esp_timer_get_time()-start);
   return rc;
}

void *dwellTime(int *brr, int c)
{
	int asize = c-1;
	static int arr[10];
	vTaskDelay(brr[asize]*100);
	l=0 ;
	db_open("/spiffs/test1.db", &db1);
	db_exec1(db1, "SELECT dwell_time FROM test1");
	sqlite3_close(db1);

	printf("Total no. of data is %d \n", l);
	//printf("values of array in integer \n");

	for(int m =0;m<asize;m++)
	{
		arr[m]=0;
		//printf("%d arr[m]\n ", arr[m]);
	}

	for(int n=0;n<l;n++)
	{
		for(int m=0; m<asize;m++)
		{

	      if(val[n]>=brr[m] && val[n]<brr[m+1])
			{
				arr[m]=arr[m]+1;
				//break;
			//	printf("%d anamana \n ",arr[m]);
			}
	      if(val[n]==brr[asize])
			{
				arr[m]=arr[m]+1;
				//break;
			//	printf("%d anamana \n ",arr[m]);
			}

		}
		//printf("%d \n",val[n]);
	}
	return arr;
}
void wifi_sniffer_init(void)
{
	esp_wifi_set_promiscuous(false);
	esp_vfs_spiffs_conf_t conf =
	{
	      .base_path = "/spiffs",
	      .partition_label = NULL,
	      .max_files = 5,
	      .format_if_mount_failed = true
	};
	esp_vfs_spiffs_register(&conf);
	size_t total = 0, used = 0;
	esp_spiffs_info(NULL, &total, &used);
	nvs_flash_init();
	// remove existing file
	for(int i=0;i<=mac_count;i++)
	{
		for(int j=0;j<6;j++)
		{
			mac_lib[i][j]=0;
		}
	}

	unlink("/spiffs/test1.db");
	sqlite3_initialize();
	db_open("/spiffs/test1.db", &db1);
	db_exec(db1, "CREATE TABLE test1 (mac_address TEXT ,start_time INTEGER, end_time INTEGER, dwell_time INTEGER );");
	tcpip_adapter_init();
	//ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_country(&wifi_country)); /* set country for channel range [1, 13] */
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
	ESP_ERROR_CHECK(esp_wifi_start());
	esp_wifi_set_promiscuous(true);
	esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_packet_handler);
	xTaskCreate(&channelSet,"channelSet",4000,NULL,6,NULL);
}

void wifi_sniffer_set_channel(uint8_t channel)
{
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
}

void wifi_sniffer_packet_handler(void *buff, wifi_promiscuous_pkt_type_t type)
{
  if (type != WIFI_PKT_MGMT)
    return;

  const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buff;
  const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
  const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;

  if(check_mac_only(hdr->addr3))
  {
	  //printf("insert time : %ld \n",now.tv_sec);
	  db_open("/spiffs/test1.db", &db1);
	  sprintf(sql, "INSERT INTO test1 ( mac_address, start_time, end_time, dwell_time) VALUES( '%02x', %ld, %ld, %d);", hdr->addr2[0], now.tv_sec, now.tv_sec, dwell_t);
	  db_exec(db1, sql);
	  sqlite3_close(db1);
  }
  else
  {
  	  vTaskDelay(100);
  	  db_open("/spiffs/test1.db", &db1);
  	  sprintf(sql, "UPDATE test1 SET end_time = %ld WHERE mac_address = '%02x' ;",now.tv_sec, mac_lib[q][0]);
  	  db_exec(db1, sql);
  	  sprintf(sql, "UPDATE test1 SET  dwell_time =(end_time-start_time);");
  	  // sprintf(sql, "SELECT mac_address, start_time,end_time,(end_time-start_time) AS dwell_time FROM test1;");
  	  db_exec(db1, sql);
  	  sqlite3_close(db1);
  }

}
