# environment_sensor

Unified environment sensor skill for reading temperature, pressure, and humidity.
tools: tool_exec({"tool":"environment_sensor","args":{...}})

## Actions

### read_temperature
Read the current temperature in degrees Celsius.

Example: `{"tool":"environment_sensor","args":{"action":"read_temperature"}}`

Returns: `{"temperature":<value>,"unit":"C","status":"ok"}`

### read_pressure
Read the current air pressure in kilopascals (kPa).

Example: `{"tool":"environment_sensor","args":{"action":"read_pressure"}}`

Returns: `{"pressure":<value>,"unit":"kPa","status":"ok"}`

### read_humidity
Read the current air humidity as a percentage.

Example: `{"tool":"environment_sensor","args":{"action":"read_humidity"}}`

Returns: `{"humidity":<value>,"unit":"%","status":"ok"}`

### read_all
Read all environment sensor values at once (temperature, pressure, humidity).

Example: `{"tool":"environment_sensor","args":{"action":"read_all"}}`

Returns: `{"temperature":<value>,"pressure":<value>,"humidity":<value>,"status":"ok"}`
