# Delayed Work Skill - Usage Examples

Quick reference for using the delayed_work skill in zbot.

## Basic Usage Pattern

When the user requests a delayed action, the LLM should:
1. Extract the delay time (convert to seconds if needed)
2. Formulate the message that should be sent after the delay
3. Call the delayed_work skill with schedule action

## Example 1: Simple Delayed Action

**User Input**: "Turn on LED0 after 1 minute"

**LLM Tool Call**:
```json
{
  "tool": "delayed_work",
  "args": {
    "action": "schedule",
    "delay_seconds": 60,
    "message": "turn on LED0"
  }
}
```

**Skill Response**:
```json
{
  "status": "ok",
  "work_id": 0,
  "delay_seconds": 60
}
```

**LLM Response to User**: "I've scheduled LED0 to turn on in 1 minute (60 seconds)."

**After 60 seconds** (automatic):
- Agent receives message: "turn on LED0"
- LLM processes it and calls the gpio tool

## Example 2: Multiple Time Units

**User Input**: "Blink LED after 2 minutes and 30 seconds"

**LLM Calculation**: 2 minutes = 120 seconds, 30 seconds = 30 seconds, total = 150 seconds

**LLM Tool Call**:
```json
{
  "tool": "delayed_work",
  "args": {
    "action": "schedule",
    "delay_seconds": 150,
    "message": "blink LED0 3 times"
  }
}
```

## Example 3: Check Sensor with Delay

**User Input**: "Check the temperature in 5 minutes and tell me if it's too hot"

**LLM Tool Call**:
```json
{
  "tool": "delayed_work",
  "args": {
    "action": "schedule",
    "delay_seconds": 300,
    "message": "read temperature sensor and report if above 30C"
  }
}
```

**After 300 seconds**:
- Agent receives: "read temperature sensor and report if above 30C"
- LLM calls environment_sensor skill
- LLM evaluates the temperature and responds

## Example 4: Sequential Delayed Actions

**User Input**: "Turn on LED0 in 30 seconds, then turn on LED1 in 60 seconds"

**LLM Tool Call 1**:
```json
{
  "tool": "delayed_work",
  "args": {
    "action": "schedule",
    "delay_seconds": 30,
    "message": "turn on LED0"
  }
}
```

**LLM Tool Call 2**:
```json
{
  "tool": "delayed_work",
  "args": {
    "action": "schedule",
    "delay_seconds": 60,
    "message": "turn on LED1"
  }
}
```

**LLM Response**: "I've scheduled LED0 to turn on in 30 seconds and LED1 to turn on in 60 seconds."

## Example 5: Reminder After Action

**User Input**: "Blink the LED now and remind me in 2 minutes"

**LLM Tool Call 1** (immediate):
```json
{
  "tool": "gpio",
  "args": {
    "action": "blink",
    "count": 5
  }
}
```

**LLM Tool Call 2** (delayed):
```json
{
  "tool": "delayed_work",
  "args": {
    "action": "schedule",
    "delay_seconds": 120,
    "message": "This is your reminder from 2 minutes ago - the LED blinked successfully"
  }
}
```

## Example 6: Cancel a Scheduled Task

**User Input**: "Actually, cancel that"

**LLM** (assumes last work_id was stored in context):
```json
{
  "tool": "delayed_work",
  "args": {
    "action": "cancel",
    "work_id": 0
  }
}
```

**Skill Response**:
```json
{
  "status": "ok",
  "work_id": 0
}
```

**LLM Response**: "I've cancelled the scheduled task."

## Example 7: List Pending Tasks

**User Input**: "What do I have scheduled?"

**LLM Tool Call**:
```json
{
  "tool": "delayed_work",
  "args": {
    "action": "list"
  }
}
```

**Skill Response**:
```json
{
  "count": 2,
  "items": [
    {
      "id": 0,
      "remaining_ms": 45000,
      "message": "turn on LED0"
    },
    {
      "id": 1,
      "remaining_ms": 135000,
      "message": "check temperature"
    }
  ]
}
```

**LLM Response**: "You have 2 scheduled tasks:
1. 'turn on LED0' in 45 seconds
2. 'check temperature' in 2 minutes 15 seconds"

## Example 8: Periodic Checks with Repeat

**User Input**: "Check temperature every 30 seconds"

**LLM Tool Call**:
```json
{
  "tool": "delayed_work",
  "args": {
    "action": "schedule",
    "delay_seconds": 30,
    "message": "check temperature and report",
    "repeat": true
  }
}
```

**Skill Response**:
```json
{
  "status": "ok",
  "work_id": 0,
  "delay_seconds": 30,
  "repeat": true
}
```

**LLM Response**: "I've scheduled temperature checks every 30 seconds. This will continue until you ask me to cancel it (work_id: 0)."

**Behavior**: The temperature check will execute every 30 seconds indefinitely until cancelled.

**To Cancel Later**:
```
You: stop checking the temperature
```

**LLM**:
```json
{
  "tool": "delayed_work",
  "args": {
    "action": "cancel",
    "work_id": 0
  }
}
```

## Example 9: Conditional Delayed Action

**User Input**: "If the LED is off, turn it on after 30 seconds"

**LLM Tool Call 1** (check current state):
```json
{
  "tool": "gpio",
  "args": {
    "action": "read",
    "pin": "led0"
  }
}
```

**If LED is off**, then:
```json
{
  "tool": "delayed_work",
  "args": {
    "action": "schedule",
    "delay_seconds": 30,
    "message": "turn on LED0"
  }
}
```

**LLM Response**: "LED0 is currently off. I've scheduled it to turn on in 30 seconds."

## Example 9: Blink LED Repeatedly

**User Input**: "Blink LED0 every 2 seconds"

**LLM Tool Call**:
```json
{
  "tool": "delayed_work",
  "args": {
    "action": "schedule",
    "delay_seconds": 2,
    "message": "blink LED0 once",
    "repeat": true
  }
}
```

**LLM Response**: "I've set LED0 to blink every 2 seconds. It will continue until you ask me to stop."

## Example 10: Monitor Sensor with Threshold

**User Input**: "Check if temperature is above 30C every minute"

**LLM Tool Call**:
```json
{
  "tool": "delayed_work",
  "args": {
    "action": "schedule",
    "delay_seconds": 60,
    "message": "read temperature sensor and alert if above 30C",
    "repeat": true
  }
}
```

**LLM Response**: "I'm now monitoring temperature every minute. I'll alert you if it goes above 30°C."

**Automatic Messages** (every 60 seconds):
- Agent receives: "read temperature sensor and alert if above 30C"
- LLM calls environment_sensor skill
- If temp > 30C, LLM responds with alert
- Work automatically reschedules for next check

## Example 11: Complex Scenario

**User Input**: "Blink SOS, wait 1 minute, then check if temperature is above 25C and turn on LED1 if it is"

**LLM Tool Call 1** (immediate):
```json
{
  "tool": "gpio",
  "args": {
    "action": "sos"
  }
}
```

**LLM Tool Call 2** (delayed):
```json
{
  "tool": "delayed_work",
  "args": {
    "action": "schedule",
    "delay_seconds": 60,
    "message": "read temperature, if above 25C turn on LED1, otherwise just report temperature"
  }
}
```

**LLM Response**: "I've sent the SOS signal and scheduled a temperature check in 1 minute. If it's above 25°C, LED1 will turn on automatically."

## Example 12: Heartbeat Monitor

**User Input**: "Send me a heartbeat message every 10 seconds so I know the system is alive"

**LLM Tool Call**:
```json
{
  "tool": "delayed_work",
  "args": {
    "action": "schedule",
    "delay_seconds": 10,
    "message": "System heartbeat - all systems operational",
    "repeat": true
  }
}
```

**LLM Response**: "I've set up a heartbeat every 10 seconds. You'll receive regular status updates."

**Behavior**: Every 10 seconds, the agent receives the heartbeat message and can respond with system status.

## Example 13: Toggle LED Repeatedly

**User Input**: "Toggle LED0 on and off every 5 seconds"

**LLM Tool Call 1** (turn on first):
```json
{
  "tool": "gpio",
  "args": {
    "action": "write",
    "pin": "led0",
    "value": 1
  }
}
```

**LLM Tool Call 2** (set up toggle):
```json
{
  "tool": "delayed_work",
  "args": {
    "action": "schedule",
    "delay_seconds": 5,
    "message": "toggle LED0 state",
    "repeat": true
  }
}
```

**LLM Response**: "LED0 is now on and will toggle every 5 seconds."

## Example 14: List with Repeating Works

**User Input**: "What tasks are running?"

**LLM Tool Call**:
```json
{
  "tool": "delayed_work",
  "args": {
    "action": "list"
  }
}
```

**Skill Response**:
```json
{
  "count": 3,
  "items": [
    {
      "id": 0,
      "remaining_ms": 8000,
      "repeat": true,
      "message": "check temperature and report"
    },
    {
      "id": 1,
      "remaining_ms": 3000,
      "repeat": false,
      "message": "turn on LED1"
    },
    {
      "id": 2,
      "remaining_ms": 15000,
      "repeat": true,
      "message": "System heartbeat"
    }
  ]
}
```

**LLM Response**: "You have 3 scheduled tasks:
1. Temperature check (repeating every check) - next in 8 seconds
2. Turn on LED1 (one-time) - in 3 seconds  
3. System heartbeat (repeating) - next in 15 seconds"

## Time Conversion Reference

Common time conversions the LLM should handle:

| User Says | Seconds |
|-----------|---------|
| 1 second | 1 |
| 5 seconds | 5 |
| 10 seconds | 10 |
| 30 seconds | 30 |
| 1 minute | 60 |
| 2 minutes | 120 |
| 5 minutes | 300 |
| 10 minutes | 600 |
| 30 minutes | 1800 |
| 1 hour | 3600 |
| 2 hours | 7200 |
| 12 hours | 43200 |
| 1 day | 86400 (max) |

## Error Handling

### Too Many Scheduled Works

**Skill Response**:
```json
{
  "error": "maximum delayed works reached (10)"
}
```

**LLM Response**: "I'm sorry, but the maximum number of scheduled tasks (10) has been reached. Please cancel some existing tasks or wait for them to complete."

### Invalid Delay

**Skill Response**:
```json
{
  "error": "delay_seconds must be between 1 and 86400"
}
```

**LLM Response**: "The delay must be between 1 second and 24 hours (86400 seconds)."

### Work Not Found (Cancel)

**Skill Response**:
```json
{
  "error": "work not found",
  "work_id": 5
}
```

**LLM Response**: "That task (ID: 5) could not be found. It may have already completed or been cancelled."

## Best Practices for LLMs

1. **Clear Messages**: Make scheduled messages clear and actionable
   - Good: "turn on LED0"
   - Bad: "do that thing we discussed"

2. **Context in Message**: Include context in the delayed message
   - Good: "check temperature and report if above 30C"
   - Bad: "check it"

3. **User Confirmation**: Always confirm scheduled tasks to the user
   - Include the delay time and what will happen
   - Mention the work_id if the user might want to cancel

4. **Track Work IDs**: Keep work_ids in context for potential cancellation

5. **Handle Limits**: Check the count from list action and warn if approaching limit

6. **Natural Language**: Convert user's natural time expressions to seconds correctly
   - "in a minute" = 60 seconds
   - "after 5 mins" = 300 seconds
   - "in half an hour" = 1800 seconds
