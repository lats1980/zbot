/*
 * Copyright (c) 2026 Lingao Meng
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Skill: environment_sensor
 * Read temperature, pressure, and humidity from environment sensor.
 * arg: JSON string with "action" field plus action-specific fields.
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "skill.h"
#include "json_util.h"
#include "env_sensor.h"

#if IS_ENABLED(CONFIG_ENVIRONMENT_SENSOR)

static bool g_init;

static void ensure_init(void)
{
	if (g_init) {
		return;
	}
	
	if (env_sensor_init() == 0) {
		g_init = true;
	}
}

static int action_read_temperature(const char *arg, char *result, size_t res_len)
{
	ARG_UNUSED(arg);
	struct sensor_value temp_val;
	int ret;

	ret = env_sensor_read_temperature(&temp_val);
	if (ret) {
		snprintf(result, res_len,
			 "{\"error\":\"failed to read temperature: %d\"}", ret);
		return ret;
	}

	snprintf(result, res_len,
		 "{\"temperature\":%d.%06d,\"unit\":\"C\",\"status\":\"ok\"}",
		 temp_val.val1, temp_val.val2);

	return 0;
}

static int action_read_pressure(const char *arg, char *result, size_t res_len)
{
	ARG_UNUSED(arg);
	struct sensor_value press_val;
	int ret;

	ret = env_sensor_read_pressure(&press_val);
	if (ret) {
		snprintf(result, res_len,
			 "{\"error\":\"failed to read pressure: %d\"}", ret);
		return ret;
	}

	snprintf(result, res_len,
		 "{\"pressure\":%d.%06d,\"unit\":\"kPa\",\"status\":\"ok\"}",
		 press_val.val1, press_val.val2);
	return 0;
}

static int action_read_humidity(const char *arg, char *result, size_t res_len)
{
	ARG_UNUSED(arg);
	struct sensor_value humid_val;
	int ret;

	ret = env_sensor_read_humidity(&humid_val);
	if (ret) {
		snprintf(result, res_len,
			 "{\"error\":\"failed to read humidity: %d\"}", ret);
		return ret;
	}

	snprintf(result, res_len,
		 "{\"humidity\":%d.%06d,\"unit\":\"%%\",\"status\":\"ok\"}",
		 humid_val.val1, humid_val.val2);
	return 0;
}

static int action_read_all(const char *arg, char *result, size_t res_len)
{
	ARG_UNUSED(arg);
	struct sensor_value temp_val, press_val, humid_val;
	int ret;

	ret = env_sensor_read_temperature(&temp_val);
	if (ret) {
		snprintf(result, res_len,
			 "{\"error\":\"failed to read temperature: %d\"}", ret);
		return ret;
	}

	ret = env_sensor_read_pressure(&press_val);
	if (ret) {
		snprintf(result, res_len,
			 "{\"error\":\"failed to read pressure: %d\"}", ret);
		return ret;
	}

	ret = env_sensor_read_humidity(&humid_val);
	if (ret) {
		snprintf(result, res_len,
			 "{\"error\":\"failed to read humidity: %d\"}", ret);
		return ret;
	}

	snprintf(result, res_len,
		 "{\"temperature\":%d.%06d,\"pressure\":%d.%06d,\"humidity\":%d.%06d,\"status\":\"ok\"}",
		 temp_val.val1, temp_val.val2, press_val.val1, press_val.val2, humid_val.val1, humid_val.val2);
	return 0;
}

static const char environment_sensor_md[] = {
#include "skills/environment_sensor/SKILL.md.inc"
	0x00
};

static int environment_sensor_handler(const char *arg, char *result, size_t res_len)
{
	char action[32] = {0};

	ensure_init();

	if (!g_init) {
		snprintf(result, res_len,
			 "{\"error\":\"environment sensor not initialized\"}");
		return -ENODEV;
	}

	if (!arg || arg[0] == '\0') {
		snprintf(result, res_len,
			 "{\"error\":\"environment_sensor: missing 'action' field\","
			 "\"actions\":[\"read_temperature\",\"read_pressure\",\"read_humidity\",\"read_all\"]}");
		return -EINVAL;
	}

	json_get_str(arg, "action", action, sizeof(action));

	if (strcmp(action, "read_temperature") == 0) {
		return action_read_temperature(arg, result, res_len);
	}
	if (strcmp(action, "read_pressure") == 0) {
		return action_read_pressure(arg, result, res_len);
	}
	if (strcmp(action, "read_humidity") == 0) {
		return action_read_humidity(arg, result, res_len);
	}
	if (strcmp(action, "read_all") == 0) {
		return action_read_all(arg, result, res_len);
	}

	snprintf(result, res_len,
		 "{\"error\":\"unknown action '%s'\","
		 "\"actions\":[\"read_temperature\",\"read_pressure\",\"read_humidity\",\"read_all\"]}",
		 action);
	return -ENOENT;
}

SKILL_DEFINE(environment_sensor_skill, "environment_sensor",
	     "Read temperature, pressure, and humidity from environment sensor.",
	     environment_sensor_md, sizeof(environment_sensor_md) - 1,
	     environment_sensor_handler);

#endif /* IS_ENABLED(CONFIG_ENVIRONMENT_SENSOR) */
