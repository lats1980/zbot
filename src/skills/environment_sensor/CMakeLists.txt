# Copyright (c) 2026 Lingao Meng
# SPDX-License-Identifier: Apache-2.0

if(CONFIG_ENVIRONMENT_SENSOR)
	target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/SKILL.c)

	set(SKILL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/SKILL.md)

	generate_inc_file_for_target(
		app
		${SKILL_FILE}
		${gen_dir}/skills/environment_sensor/SKILL.md.inc
	)
endif()
