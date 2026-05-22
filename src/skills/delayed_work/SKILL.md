# delayed_work

Schedule LLM chat messages with a time delay. Allows the agent to schedule future interactions with itself.

tools: tool_exec({"tool":"delayed_work","args":{...}})

## Actions

### schedule
Schedule a message to be sent to the LLM after a specified delay.

Parameters:
- `delay_seconds` (required): Delay in seconds before sending the message (1-86400)
- `message` (required): The message to send to the LLM after the delay
- `repeat` (optional): If true, the work will repeat indefinitely after each execution (default: false)

Example: `{"tool":"delayed_work","args":{"action":"schedule","delay_seconds":60,"message":"turn on LED0"}}`

Repeating example: `{"tool":"delayed_work","args":{"action":"schedule","delay_seconds":30,"message":"check temperature","repeat":true}}`

Returns: `{"status":"ok","work_id":<id>,"delay_seconds":<delay>,"repeat":<true|false>}`

Use case: When user says "turn on LEDs after 1 minute", the LLM should:
1. Parse the request to extract: delay=60 seconds, action="turn on LED0"
2. Call this skill with those parameters
3. After 60 seconds, the message "turn on LED0" is automatically sent to the LLM
4. The LLM will then execute the GPIO tool to turn on the LED

Repeating use case: When user says "check temperature every 30 seconds", the LLM should:
1. Parse the request to extract: delay=30 seconds, action="check temperature", repeat=true
2. Call this skill with repeat:true
3. The message will be sent every 30 seconds until cancelled

### cancel
Cancel a previously scheduled delayed work by its work ID.

Parameters:
- `work_id` (required): The work ID returned from the schedule action

Example: `{"tool":"delayed_work","args":{"action":"cancel","work_id":0}}`

Returns: `{"status":"ok","work_id":<id>}` or `{"error":"work not found"}`

### list
List all active (pending) delayed work items.

Example: `{"tool":"delayed_work","args":{"action":"list"}}`

Returns: `{"count":<n>,"items":[{"id":<id>,"remaining_ms":<ms>,"repeat":<true|false>,"message":"..."},...]}`

## Implementation Notes

- Maximum concurrent delayed works: CONFIG_DELAYED_WORK_MAX_ITEMS (default: 3)
- Maximum delay: 86400 seconds (24 hours)
- Minimum delay: 1 second
- When a delayed work executes, it sends the message to the agent as if it were a new user input
- The agent processes it through the normal ReAct loop with all tools available
- Repeating works will continue executing until cancelled or the system reboots
- Each repeating work counts as one slot in the maximum concurrent works limit
