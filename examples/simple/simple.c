/* Very basic example that just demonstrates we can run at all!
 */
#include "espressif/esp_common.h"
#include "esp/uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define IOT_INFO
#define IOT_DEBUG
#define IOT_WARN
#define IOT_ERROR

#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_interface.h"
#include "aws_iot_config.h"

#include <string.h>
#include <unistd.h>

int MQTTcallbackHandler(MQTTCallbackParams params) {

	INFO("Subscribe callback");
	INFO("%.*s\t%.*s",
			(int)params.TopicNameLen, params.pTopicName,
			(int)params.MessageParams.PayloadLen, (char*)params.MessageParams.pPayload);

	return 0;
}

void disconnectCallbackHandler(void) {
	WARN("MQTT Disconnect");
	IoT_Error_t rc = NONE_ERROR;
	if(aws_iot_is_autoreconnect_enabled()){
		INFO("Auto Reconnect is enabled, Reconnecting attempt will start now");
	}else{
		WARN("Auto Reconnect not enabled. Starting manual reconnect...");
		rc = aws_iot_mqtt_attempt_reconnect();
		if(RECONNECT_SUCCESSFUL == rc){
			WARN("Manual Reconnect Successful");
		}else{
			WARN("Manual Reconnect Failed - %d", rc);
		}
	}
}

static IROM char HostAddress[255] = AWS_IOT_MQTT_HOST;
static MQTTConnectParams connectParams = {
                .enableAutoReconnect = 0,
                .pHostURL = AWS_IOT_MQTT_HOST,
                .port = AWS_IOT_MQTT_PORT,
                .RootCA = NULL,
                .RootCALen = 0,
                .DeviceCert = NULL,
                .DeviceCertLen = 0,
                .DevicePrivateKey = NULL,
                .DevicePrivateKeyLen = 0,
                .pClientID = NULL,
                .pUserName = NULL,
                .pPassword = NULL,
                .MQTTVersion = MQTT_3_1_1,
                .KeepAliveInterval_sec = 10,
                .isCleansession = true,
                .isWillMsgPresent = false,
                .will={.pTopicName = NULL, .pMessage = NULL, .isRetained = false, .qos = QOS_0},
                .mqttCommandTimeout_ms = 10000,
                .tlsHandshakeTimeout_ms = 20000,
                .isSSLHostnameVerify = true,
                .disconnectHandler = NULL
};

uint32_t port = AWS_IOT_MQTT_PORT;

uint32_t publishCount = 0;

#include "certs/cert.h"
#include "certs/private.h"
#include "certs/root.h"
void printmem() {
printf("MEM0 %u stack %lu\n", sdk_system_get_free_heap_size(), uxTaskGetStackHighWaterMark(NULL));
}
int mqtt_main() {
	IoT_Error_t rc = NONE_ERROR;

	INFO("\nAWS IoT SDK Version %d.%d.%d-%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);
printf("MEM %u\n", sdk_system_get_free_heap_size());
	//MQTTConnectParams connectParams = MQTTConnectParamsDefault;

	connectParams.KeepAliveInterval_sec = 10;
	connectParams.isCleansession = true;
	connectParams.MQTTVersion = MQTT_3_1_1;
	connectParams.pClientID = "csdk";
	connectParams.pHostURL = (char*)HostAddress;
	connectParams.port = port;
	connectParams.isWillMsgPresent = false;
	connectParams.RootCA = (unsigned char*)certs_root_der_crt;
	connectParams.RootCALen = certs_root_der_crt_len;
	connectParams.DeviceCert = (unsigned char*)certs_cert_der;
	connectParams.DeviceCertLen = certs_cert_der_len;
	connectParams.DevicePrivateKey = (unsigned char*)certs_private_der_key;
	connectParams.DevicePrivateKeyLen = certs_private_der_key_len;
	connectParams.mqttCommandTimeout_ms = 140000;
	connectParams.tlsHandshakeTimeout_ms = 140000;
	connectParams.isSSLHostnameVerify = true; // ensure this is set to true for production
	connectParams.disconnectHandler = disconnectCallbackHandler;

	INFO("Connecting...");
printf("MEM %u\n", sdk_system_get_free_heap_size());
	rc = aws_iot_mqtt_connect(&connectParams);
	if (NONE_ERROR != rc) {
		ERROR("Error(%d) connecting to %s:%d", rc, connectParams.pHostURL, connectParams.port);
	}
	//
	 // Enable Auto Reconnect functionality. Minimum and Maximum time of Exponential backoff are set in aws_iot_config.h
	 //  #AWS_IOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL
	 //  #AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL
	 //
	INFO("Connected!");
	rc = aws_iot_mqtt_autoreconnect_set_status(true);
	if (NONE_ERROR != rc) {
		ERROR("Unable to set Auto Reconnect to true - %d", rc);
		return rc;
	}
        char cTopic[20] = {0};
        sprintf(cTopic, "esp/%x", sdk_system_get_chip_id());
	printf("topic: %s\n", cTopic);
/*
	//MQTTSubscribeParams subParams = MQTTSubscribeParamsDefault;
	subParams.mHandler = MQTTcallbackHandler;
        char cTopic[20] = {0};
        sprintf(cTopic, "esp/%x", sdk_system_get_chip_id());
	subParams.pTopic = cTopic;
	subParams.qos = QOS_0;
	if (NONE_ERROR == rc) {
		INFO("Subscribing...");
		rc = aws_iot_mqtt_subscribe(&subParams);
		if (NONE_ERROR != rc) {
			ERROR("Error subscribing");
		}
	}
*/
	MQTTMessageParams Msg = MQTTMessageParamsDefault;
	Msg.qos = QOS_0;
        char cPayload[50] = {0};
	Msg.pPayload = (void *) cPayload;

	MQTTPublishParams Params = MQTTPublishParamsDefault;
	Params.pTopic = cTopic;

	while ((NETWORK_ATTEMPTING_RECONNECT == rc || RECONNECT_SUCCESSFUL == rc || NONE_ERROR == rc)) {
		//Max time the yield function will wait for read messages
		rc = aws_iot_mqtt_yield(60000);
		if(NETWORK_ATTEMPTING_RECONNECT == rc){
			INFO("-->sleep on reconnect");
vTaskDelay(1 / portTICK_RATE_MS);
			//sleep(1);
			// If the client is attempting to reconnect we will skip the rest of the loop.
			continue;
		}
		INFO("-->sleep");
		printmem();
		//sleep(1);
vTaskDelay(1 / portTICK_RATE_MS);
		sprintf(cPayload, "heartbeat %d", publishCount++);
		Msg.PayloadLen = strlen(cPayload) + 1;
		Params.MessageParams = Msg;
		rc = aws_iot_mqtt_publish(&Params);
	}

	if (NONE_ERROR != rc) {
		ERROR("An error occurred in the loop.\n");
	} else {
		INFO("Publish done\n");
	}

	return rc;
}

#include <timer_interface.h>
static xQueueHandle mainqueue;


void checkConn(void* args) {
/*vTaskDelay(1000);
int v = 0;
{
    //char buff[1024];
    //v += buff[1023];
    //printmem();
}
printmem();
//printf("MEM0 %u stack %lu\n", sdk_system_get_free_heap_size(), uxTaskGetStackHighWaterMark(NULL));
v += light();
printmem();
//printf("MEM1 %u stack %lu\n", sdk_system_get_free_heap_size(), uxTaskGetStackHighWaterMark(NULL));
*/
	struct ip_info tmp;
/*v += heavy();
printmem();
//printf("MEM2 %u stack %lu\n", sdk_system_get_free_heap_size(), uxTaskGetStackHighWaterMark(NULL));
printf("result %d\n", v);
printmem();
vTaskDelay(1000);
*/
	tmp.ip.addr = 0;
    Timer connect_timer;
    InitTimer(&connect_timer);
    countdown_ms(&connect_timer, 100000);
//printf("left %d\n", left_ms(&connect_timer));
	while (!sdk_wifi_get_ip_info(0x00, &tmp) || tmp.ip.addr == 0) {
		printf("no IP, left %d ms\n", left_ms(&connect_timer));
		vTaskDelay(100);
printf("MEM %u\n", sdk_system_get_free_heap_size());	
	}
	printf("connected\n");
	mqtt_main();
}


void user_init(void)
{
    uart_set_baud(0, 115200);
    printf("SDK version:%s\n", sdk_system_get_sdk_version());
//printf("MEM %u %u\n", sdk_system_get_free_heap_size(), xPortGetFreeHeapSize());
    mainqueue = xQueueCreate(3, sizeof(uint32_t));
    const static struct sdk_station_config config = {
        .ssid = "Connecting...",
        .password = "Ainopkv.YvopGr",
    };

    // required to call wifi_set_opmode before station_set_config 
    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config((struct sdk_station_config*)(&config));
printf("MEM %u\n", sdk_system_get_free_heap_size());
    xTaskCreate(checkConn, (const signed char*)"c", 2000, &mainqueue, 2, NULL);
    //xTaskCreate(task1, (signed char *)"tsk1", 256, &mainqueue, 2, NULL);
    //xTaskCreate(task2, (signed char *)"tsk2", 256, &mainqueue, 2, NULL);
    //Timer connect_timer;
    //InitTimer(&connect_timer);
    //countdown_ms(&connect_timer, 1000);
//printf("left %d\n", left_ms(&connect_timer));
}
