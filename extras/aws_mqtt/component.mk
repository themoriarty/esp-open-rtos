# Component makefile for extras/aws_mqtt

IOT_CLIENT_DIR=$(aws_mqtt_ROOT)/aws_iot_src
IOT_INCLUDE_DIRS += -I $(IOT_CLIENT_DIR)/protocol/mqtt
IOT_INCLUDE_DIRS += -I $(IOT_CLIENT_DIR)/protocol/mqtt/aws_iot_embedded_client_wrapper
IOT_INCLUDE_DIRS += -I $(IOT_CLIENT_DIR)/protocol/mqtt/aws_iot_embedded_client_wrapper/platform_linux
IOT_INCLUDE_DIRS += -I $(IOT_CLIENT_DIR)/protocol/mqtt/aws_iot_embedded_client_wrapper/platform_linux/common
IOT_INCLUDE_DIRS += -I $(IOT_CLIENT_DIR)/protocol/mqtt/aws_iot_embedded_client_wrapper/platform_linux/mbedtls
IOT_INCLUDE_DIRS += -I $(IOT_CLIENT_DIR)/utils

PLATFORM_COMMON_DIR = $(IOT_CLIENT_DIR)/protocol/mqtt/aws_iot_embedded_client_wrapper/platform_linux/common
MQTT_MBED_DIR = $(IOT_CLIENT_DIR)/protocol/mqtt/aws_iot_embedded_client_wrapper/platform_linux/mbedtls

MQTT_DIR = $(aws_mqtt_ROOT)/aws_mqtt_embedded_client_lib
MQTT_C_DIR = $(MQTT_DIR)/MQTTClient-C/src
MQTT_EMB_DIR = $(MQTT_DIR)/MQTTPacket/src

MQTT_INCLUDE_DIR += -I $(MQTT_EMB_DIR)
MQTT_INCLUDE_DIR += -I $(MQTT_C_DIR)


INC_DIRS += $(IOT_INCLUDE_DIRS) $(MQTT_INCLUDE_DIR) 


# args for passing into compile rule generation
aws_mqtt_SRC_DIR =  $(aws_mqtt_ROOT) $(MQTT_C_DIR) $(MQTT_EMB_DIR) $(PLATFORM_COMMON_DIR) $(MQTT_MBED_DIR) $(IOT_CLIENT_DIR)/protocol/mqtt/aws_iot_embedded_client_wrapper

$(eval $(call component_compile_rules,aws_mqtt))
