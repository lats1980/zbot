/*
 * Copyright (c) 2026 Lingao Meng
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Skill: delayed_work
 * Schedule LLM chat messages with a time delay.
 * arg: JSON string with "action" field plus action-specific fields.
 */

#include <zephyr/kernel.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "skill.h"
#include "json_util.h"
#include "agent.h"
#include "telegram.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(delayed_work_skill, LOG_LEVEL_INF);

#if IS_ENABLED(CONFIG_DELAYED_WORK_SKILL)

#define MAX_DELAYED_WORKS CONFIG_DELAYED_WORK_MAX_ITEMS
#define MESSAGE_MAX_LEN   AGENT_INPUT_MAX_LEN

struct delayed_work_item {
	struct k_work_delayable work;
	char message[MESSAGE_MAX_LEN];
	int id;
	int64_t chat_id;
	bool active;
	bool repeat;
	int64_t scheduled_at_ms;
	uint32_t delay_ms;
};

static struct delayed_work_item g_delayed_works[MAX_DELAYED_WORKS];
static int g_next_work_id;
static K_MUTEX_DEFINE(g_work_mutex);

static void delayed_work_response_cb(int err, const char *content,
				     bool is_intermediate, void *user_data)
{
	struct delayed_work_item *item = (struct delayed_work_item *)user_data;

	if (is_intermediate) {
		LOG_INF("delayed_work[%d] intermediate: %s", item->id, content);
		tg_send_message(item->chat_id, content);
		return;
	}

	if (err) {
		LOG_ERR("delayed_work[%d] error: %d", item->id, err);
		tg_send_message(item->chat_id, "An error occurred while processing the delayed work.");
	} else {
		LOG_INF("delayed_work[%d] completed: %s", item->id, content);
		tg_send_message(item->chat_id, content);
	}
}

static void delayed_work_handler(struct k_work *work)
{
	struct k_work_delayable *dwork = k_work_delayable_from_work(work);
	struct delayed_work_item *item =
		CONTAINER_OF(dwork, struct delayed_work_item, work);

	LOG_INF("Executing delayed work[%d]%s: %s", 
		item->id, item->repeat ? " (repeating)" : "", item->message);

	int rc = agent_submit_input(item->message, delayed_work_response_cb,
				     (void *)item);
	if (rc) {
		LOG_ERR("Failed to submit delayed work[%d]: %d", item->id, rc);
	}

	k_mutex_lock(&g_work_mutex, K_FOREVER);
	
	if (item->repeat) {
		/* Reschedule the work for the next iteration */
		item->scheduled_at_ms = k_uptime_get();
		k_work_schedule(&item->work, K_MSEC(item->delay_ms));
		LOG_INF("Rescheduled repeating work[%d] with %dms delay",
			item->id, item->delay_ms);
	} else {
		item->active = false;
	}
	
	k_mutex_unlock(&g_work_mutex);
}

static int allocate_work_item(void)
{
	k_mutex_lock(&g_work_mutex, K_FOREVER);

	for (int i = 0; i < MAX_DELAYED_WORKS; i++) {
		if (!g_delayed_works[i].active) {
			g_delayed_works[i].active = true;
			g_delayed_works[i].id = g_next_work_id++;
			k_mutex_unlock(&g_work_mutex);
			return i;
		}
	}

	k_mutex_unlock(&g_work_mutex);
	return -ENOMEM;
}

static int find_work_by_id(int work_id)
{
	for (int i = 0; i < MAX_DELAYED_WORKS; i++) {
		if (g_delayed_works[i].active && g_delayed_works[i].id == work_id) {
			return i;
		}
	}
	return -ENOENT;
}

static int get_int_param(const char *json, const char *key, int default_val)
{
	char search[64];
	const char *pos;

	snprintf(search, sizeof(search), "\"%s\"", key);
	pos = strstr(json, search);
	if (!pos) {
		return default_val;
	}
	pos += strlen(search);
	while (*pos == ' ' || *pos == ':' || *pos == '\t') {
		pos++;
	}
	return (int)strtol(pos, NULL, 10);
}

static bool get_bool_param(const char *json, const char *key, bool default_val)
{
	char search[64];
	const char *pos;

	snprintf(search, sizeof(search), "\"%s\"", key);
	pos = strstr(json, search);
	if (!pos) {
		return default_val;
	}
	pos += strlen(search);
	while (*pos == ' ' || *pos == ':' || *pos == '\t') {
		pos++;
	}
	
	/* Check for "true" or "false" */
	if (strncmp(pos, "true", 4) == 0) {
		return true;
	}
	if (strncmp(pos, "false", 5) == 0) {
		return false;
	}
	
	/* Also accept 1 or 0 */
	return (int)strtol(pos, NULL, 10) != 0;
}

static int action_schedule(const char *arg, char *result, size_t res_len)
{
	int delay_seconds = get_int_param(arg, "delay_seconds", -1);
	bool repeat = get_bool_param(arg, "repeat", false);
	char message[MESSAGE_MAX_LEN] = {0};
	int idx;
	struct delayed_work_item *item;

	if (delay_seconds < 1 || delay_seconds > 86400) {
		snprintf(result, res_len,
			 "{\"error\":\"delay_seconds must be between 1 and 86400\"}");
		return -EINVAL;
	}

	if (json_get_str(arg, "message", message, sizeof(message)) <= 0) {
		snprintf(result, res_len,
			 "{\"error\":\"message field is required\"}");
		return -EINVAL;
	}

	idx = allocate_work_item();
	if (idx < 0) {
		snprintf(result, res_len,
			 "{\"error\":\"maximum delayed works reached (%d)\"}",
			 MAX_DELAYED_WORKS);
		return -ENOMEM;
	}

	item = &g_delayed_works[idx];
	strncpy(item->message, message, sizeof(item->message) - 1);
	item->message[sizeof(item->message) - 1] = '\0';
	item->repeat = repeat;
	item->scheduled_at_ms = k_uptime_get();
	item->delay_ms = delay_seconds * 1000;
	item->chat_id = telegram_get_chat_id();

	k_work_init_delayable(&item->work, delayed_work_handler);
	k_work_schedule(&item->work, K_MSEC(item->delay_ms));

	LOG_INF("Scheduled work[%d] with %ds delay%s: %s",
		item->id, delay_seconds, repeat ? " (repeating)" : "", message);

	snprintf(result, res_len,
		 "{\"status\":\"ok\",\"work_id\":%d,\"delay_seconds\":%d,\"repeat\":%s}",
		 item->id, delay_seconds, repeat ? "true" : "false");
	return 0;
}

static int action_cancel(const char *arg, char *result, size_t res_len)
{
	int work_id = get_int_param(arg, "work_id", -1);
	int idx;
	struct delayed_work_item *item;

	if (work_id < 0) {
		snprintf(result, res_len,
			 "{\"error\":\"work_id field is required\"}");
		return -EINVAL;
	}

	k_mutex_lock(&g_work_mutex, K_FOREVER);
	idx = find_work_by_id(work_id);
	if (idx < 0) {
		k_mutex_unlock(&g_work_mutex);
		snprintf(result, res_len,
			 "{\"error\":\"work not found\",\"work_id\":%d}",
			 work_id);
		return -ENOENT;
	}

	item = &g_delayed_works[idx];
	k_work_cancel_delayable(&item->work);
	item->active = false;
	k_mutex_unlock(&g_work_mutex);

	LOG_INF("Cancelled work[%d]", work_id);

	snprintf(result, res_len,
		 "{\"status\":\"ok\",\"work_id\":%d}",
		 work_id);
	return 0;
}

static int action_list(const char *arg, char *result, size_t res_len)
{
	ARG_UNUSED(arg);
	int64_t now_ms = k_uptime_get();
	size_t pos = 0;
	int count = 0;
	int n;

	n = snprintf(result + pos, res_len - pos, "{\"count\":");
	if (n < 0 || (size_t)n >= res_len - pos) {
		return -ENOMEM;
	}
	pos += n;

	k_mutex_lock(&g_work_mutex, K_FOREVER);

	for (int i = 0; i < MAX_DELAYED_WORKS; i++) {
		if (g_delayed_works[i].active) {
			count++;
		}
	}

	n = snprintf(result + pos, res_len - pos, "%d,\"items\":[", count);
	if (n < 0 || (size_t)n >= res_len - pos) {
		k_mutex_unlock(&g_work_mutex);
		return -ENOMEM;
	}
	pos += n;

	bool first = true;

	for (int i = 0; i < MAX_DELAYED_WORKS; i++) {
		if (!g_delayed_works[i].active) {
			continue;
		}

		struct delayed_work_item *item = &g_delayed_works[i];
		int64_t elapsed_ms = now_ms - item->scheduled_at_ms;
		int64_t remaining_ms = (int64_t)item->delay_ms - elapsed_ms;

		if (remaining_ms < 0) {
			remaining_ms = 0;
		}

		char escaped_msg[MESSAGE_MAX_LEN * 2];
		json_escape(item->message, escaped_msg, sizeof(escaped_msg));

		n = snprintf(result + pos, res_len - pos,
			     "%s{\"id\":%d,\"remaining_ms\":%lld,\"repeat\":%s,\"message\":\"%s\"}",
			     first ? "" : ",",
			     item->id,
			     remaining_ms,
			     item->repeat ? "true" : "false",
			     escaped_msg);
		if (n < 0 || (size_t)n >= res_len - pos) {
			k_mutex_unlock(&g_work_mutex);
			return -ENOMEM;
		}
		pos += n;
		first = false;
	}

	k_mutex_unlock(&g_work_mutex);

	n = snprintf(result + pos, res_len - pos, "]}");
	if (n < 0 || (size_t)n >= res_len - pos) {
		return -ENOMEM;
	}
	pos += n;

	return 0;
}

static const char delayed_work_md[] = {
#include "skills/delayed_work/SKILL.md.inc"
	0x00
};

static int delayed_work_handler_main(const char *arg, char *result, size_t res_len)
{
	char action[32] = {0};

	if (!arg || arg[0] == '\0') {
		snprintf(result, res_len,
			 "{\"error\":\"delayed_work: missing 'action' field\","
			 "\"actions\":[\"schedule\",\"cancel\",\"list\"]}");
		return -EINVAL;
	}

	json_get_str(arg, "action", action, sizeof(action));

	if (strcmp(action, "schedule") == 0) {
		return action_schedule(arg, result, res_len);
	}
	if (strcmp(action, "cancel") == 0) {
		return action_cancel(arg, result, res_len);
	}
	if (strcmp(action, "list") == 0) {
		return action_list(arg, result, res_len);
	}

	snprintf(result, res_len,
		 "{\"error\":\"unknown action '%s'\","
		 "\"actions\":[\"schedule\",\"cancel\",\"list\"]}",
		 action);
	return -ENOENT;
}

SKILL_DEFINE(delayed_work_skill, "delayed_work",
	     "Schedule LLM chat messages with a time delay for future agent interactions.",
	     delayed_work_md, sizeof(delayed_work_md) - 1,
	     delayed_work_handler_main);

#endif /* IS_ENABLED(CONFIG_DELAYED_WORK_SKILL) */
