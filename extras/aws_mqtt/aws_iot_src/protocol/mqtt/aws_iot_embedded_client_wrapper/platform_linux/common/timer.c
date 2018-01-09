/*
 * Copyright 2010-2015 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

/**
 * @file timer.c
 * @brief Linux implementation of the timer interface.
 */

//#include <stddef.h>
//#include <sys/types.h>

#include <espressif/esp_common.h>
#include <espressif/esp_system.h>
#include "timer_linux.h"

char expired(Timer* timer) {
	return sdk_system_get_time() >= timer->end_time;
/*	struct timeval now, res;
	gettimeofday(&now, NULL);
	timersub(&timer->end_time, &now, &res);
	return res.tv_sec < 0 || (res.tv_sec == 0 && res.tv_usec <= 0);
*/
return 0;
}

void countdown_ms(Timer* timer, unsigned int timeout) {
	timer->end_time = sdk_system_get_time() + timeout * 1000;
/*	struct timeval now;
	gettimeofday(&now, NULL);
	struct timeval interval = { timeout / 1000, (timeout % 1000) * 1000 };
	timeradd(&now, &interval, &timer->end_time);
*/}

void countdown(Timer* timer, unsigned int timeout) {
	timer->end_time = sdk_system_get_time() + timeout * 1000 * 1000;
/*	struct timeval now;
	gettimeofday(&now, NULL);
	struct timeval interval = { timeout, 0 };
	timeradd(&now, &interval, &timer->end_time);
*/}

int left_ms(Timer* timer) {
	uint32_t now = sdk_system_get_time();
	return (now > timer->end_time) ? 0 : (timer->end_time - now) / 1000;
/*	struct timeval now, res;
	gettimeofday(&now, NULL);
	timersub(&timer->end_time, &now, &res);
	return (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000;
*/
return 0;
}

void InitTimer(Timer* timer) {
	timer->end_time = 0;
}

