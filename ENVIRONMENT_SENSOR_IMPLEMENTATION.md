# Environment Sensor Implementation for zbot

## Overview
This document describes the implementation of the environment sensor feature for zbot, enabling temperature, pressure, and humidity readings through a new skill interface.

## Files Created

### 1. Kconfig Configuration
- **File**: `Kconfig`
- **Description**: Main Kconfig file for zbot with `CONFIG_ENVIRONMENT_SENSOR` option
- **Options**:
  - `CONFIG_ENVIRONMENT_SENSOR`: Enable/disable environment sensor support
  - `CONFIG_ENV_SENSOR_GAS_RES_SIM_BASE`: Base value for simulated gas resistance (default: 10000 Ohm)
  - `CONFIG_ENV_SENSOR_GAS_RES_SIM_MAX_DIFF`: Max difference for simulated gas resistance (default: 1000 Ohm)

### 2. Environment Sensor Driver
- **Files**: `src/env_sensor.h`, `src/env_sensor.c`
- **Description**: Core environment sensor driver adapted from LWM2M client implementation
- **Functions**:
  - `env_sensor_init()`: Initialize the environment sensor device
  - `env_sensor_read_temperature()`: Read temperature in degrees Celsius
  - `env_sensor_read_pressure()`: Read pressure in kilopascals (kPa)
  - `env_sensor_read_humidity()`: Read humidity as percentage (%)
  - `env_sensor_read_gas_resistance()`: Read gas resistance in Ohms
- **Supported Sensors**:
  - Real hardware: BME680/BME688 (via I2C on Thingy53)
  - Simulated: `sensor_sim` (on nRF7002-DK, nRF54L20-DK, native_sim)

### 3. Environment Sensor Skill
- **Directory**: `src/skills/environment_sensor/`
- **Files**: `SKILL.c`, `SKILL.md`, `CMakeLists.txt`
- **Description**: Zbot skill interface for environment sensor operations
- **Actions**:
  - `read_temperature`: Read current temperature
  - `read_pressure`: Read current air pressure
  - `read_humidity`: Read current humidity
  - `read_all`: Read all sensor values at once

#### Skill Usage Examples

```json
// Read temperature
{"tool":"environment_sensor","args":{"action":"read_temperature"}}
// Returns: {"temperature":22.50,"unit":"C","status":"ok"}

// Read pressure
{"tool":"environment_sensor","args":{"action":"read_pressure"}}
// Returns: {"pressure":101.32,"unit":"kPa","status":"ok"}

// Read humidity
{"tool":"environment_sensor","args":{"action":"read_humidity"}}
// Returns: {"humidity":45.20,"unit":"%","status":"ok"}

// Read all values at once
{"tool":"environment_sensor","args":{"action":"read_all"}}
// Returns: {"temperature":22.50,"pressure":101.32,"humidity":45.20,"status":"ok"}
```

## Files Modified

### 1. Build Configuration
- **File**: `CMakeLists.txt`
- **Changes**: Added conditional compilation of `env_sensor.c` when `CONFIG_ENVIRONMENT_SENSOR` is enabled

- **File**: `src/skills/CMakeLists.txt`
- **Changes**: Added `add_subdirectory(environment_sensor)` to build the new skill

### 2. Project Configuration
- **File**: `prj.conf`
- **Changes**: 
  - Enabled `CONFIG_ENVIRONMENT_SENSOR=y`
  - Enabled `CONFIG_SENSOR=y` for Zephyr sensor subsystem

### 3. Board-Specific Configuration

#### Thingy53 (nrf5340_cpuapp)
- **File**: `boards/thingy53_nrf5340_cpuapp.conf`
- **Changes**: Added BME680 sensor driver support
  ```
  CONFIG_SENSOR=y
  CONFIG_BME680=y
  ```
- **File**: `boards/thingy53_nrf5340_cpuapp.overlay`
- **Changes**: Added `bme680` label to existing BME688 sensor node for device tree lookup

#### nRF7002-DK (nrf5340_cpuapp)
- **File**: `boards/nrf7002dk_nrf5340_cpuapp.conf`
- **Changes**: Added sensor simulator support
  ```
  CONFIG_SENSOR=y
  CONFIG_SENSOR_SIM=y
  ```
- **File**: `boards/nrf7002dk_nrf5340_cpuapp.overlay` (new)
- **Changes**: Added `sensor_sim` device tree node for simulated sensor

#### nRF54L20-DK (nrf54lm20a_cpuapp)
- **File**: `boards/nrf54lm20dk_nrf54lm20a_cpuapp.conf`
- **Changes**: Added sensor simulator support
  ```
  CONFIG_SENSOR=y
  CONFIG_SENSOR_SIM=y
  ```
- **File**: `boards/nrf54lm20dk_nrf54lm20a_cpuapp.overlay` (new)
- **Changes**: Added `sensor_sim` device tree node for simulated sensor

#### Native Simulation (native_sim)
- **File**: `boards/native_sim.conf`
- **Changes**: Added sensor simulator support
  ```
  CONFIG_SENSOR=y
  CONFIG_SENSOR_SIM=y
  ```
- **File**: `boards/native_sim.overlay`
- **Changes**: Added `sensor_sim` device tree node for simulated sensor

## Architecture

### Skill Registration Flow
1. `SKILL_DEFINE()` macro in `SKILL.c` creates a static skill node
2. `SYS_INIT()` automatically registers the skill at boot time (PRE_KERNEL_1 priority)
3. Skill becomes available to LLM through the tool_exec interface

### Sensor Read Flow
1. LLM calls `tool_exec` with environment_sensor tool and action parameter
2. Skill handler parses the JSON action and routes to appropriate function
3. Sensor driver fetches samples from hardware/simulator
4. Results are formatted as JSON and returned to LLM

### Conditional Compilation
- All environment sensor code is wrapped with `#if IS_ENABLED(CONFIG_ENVIRONMENT_SENSOR)`
- When disabled, the skill is not compiled or registered
- This allows the feature to be easily enabled/disabled per board

## Hardware Support

### Thingy53
- **Sensor**: BME688 (Bosch Sensortec)
- **Interface**: I2C (address 0x76)
- **Capabilities**: Temperature, Pressure, Humidity, Gas Resistance
- **Device Tree Label**: `bme680`

### Other Boards (nRF7002-DK, nRF54L20-DK, native_sim)
- **Sensor**: Nordic sensor simulator (`sensor_sim`)
- **Interface**: Virtual/simulated
- **Capabilities**: Temperature, Pressure, Humidity
- **Device Tree Label**: `sensor_sim`

## Building

To build zbot with environment sensor support:

```bash
# For Thingy53 (uses real BME688 sensor)
west build -b thingy53_nrf5340_cpuapp

# For nRF7002-DK (uses sensor simulator)
west build -b nrf7002dk_nrf5340_cpuapp

# For nRF54L20-DK (uses sensor simulator)
west build -b nrf54lm20dk_nrf54lm20a_cpuapp

# For native simulation (uses sensor simulator)
west build -b native_sim
```

## Testing

After flashing the firmware, the environment sensor skill should be automatically registered. You can verify by:

1. Check the skill list in the shell:
   ```
   uart:~$ zbot skill list
   ```

2. Test the skill through the LLM interface or directly via tool_exec:
   ```json
   {"tool":"environment_sensor","args":{"action":"read_all"}}
   ```

3. Expected response format:
   ```json
   {"temperature":22.50,"pressure":101.32,"humidity":45.20,"status":"ok"}
   ```

## References

- Original implementation: `nrf/samples/cellular/lwm2m_client/src/sensors/`
- GPIO skill reference: `src/skills/gpio/SKILL.c`
- Skill framework: `src/skills/skill.h`
- Zephyr sensor API: https://docs.zephyrproject.org/latest/hardware/peripherals/sensor.html
- BME680 driver: https://docs.zephyrproject.org/latest/build/dts/api/bindings/sensor/bosch,bme680.html
