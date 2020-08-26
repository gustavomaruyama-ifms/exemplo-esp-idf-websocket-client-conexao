#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "esp_websocket_client.h"

void websocket_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
	printf("Entrou no manipulador de eventos do Websocket!\n");

	if(event_id == WEBSOCKET_EVENT_CONNECTED){
		printf("Conectado ao servidor Websocket\n");
	}

	if(event_id == WEBSOCKET_EVENT_DATA){
		esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;

		if(data->op_code == 1){
			printf("Tamanho da mensagem: %d\n", data->data_len);
			printf("Mensagem recebida: %.*s\n", data->data_len, (char*) data->data_ptr);
		}

		if(data->op_code == 10){
			printf("Recebido um PONG!\n");
		}
	}
}

void websocket_init(void){
	esp_websocket_client_config_t websoscket_config = {
			.uri = "ws://192.168.1.104:3000/exemplo/websocket"
	};
	esp_websocket_client_handle_t client = esp_websocket_client_init(&websoscket_config);
	esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, &websocket_event_handler, (void*)client);
	esp_websocket_client_start(client);
}

void wifi_event_handler(void *arg, esp_event_base_t event_base,	int32_t event_id, void *event_data)
{
	printf("Entrou no manipulador de eventos do WiFi!\n");
	if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START){
		printf("Modo Station de WiFi inicializado!\n");
		printf("Conectando com a rede WiFi.\n");
		esp_wifi_connect();
	}

	if(event_id == WIFI_EVENT_STA_CONNECTED){
		printf("Conectado a rede WiFi.\n");
	}

	if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP){
		ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
		printf("IP recebido: %s\n", ip4addr_ntoa(&event->ip_info.ip));
		printf("Iniciando Websocket.\n");
		websocket_init();
	}
}

void app_main(void) {
	nvs_flash_init();
	tcpip_adapter_init();

	esp_event_loop_create_default();
	esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL);
	esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL);

	wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
	wifi_config_t wifi_config = {
			.sta = {
					.ssid = "",
					.password = "",
			}
	};

	esp_wifi_init(&config);
	esp_wifi_set_mode(WIFI_MODE_STA);
	esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
	esp_wifi_start();
}
