# Copyright (c) 2026 Lingao Meng
# SPDX-License-Identifier: Apache-2.0

if(CONFIG_DELAYED_WORK_SKILL)
	target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/SKILL.c)

	set(SKILL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/SKILL.md)

	generate_inc_file_for_target(
		app
		${SKILL_FILE}
		${gen_dir}/skills/delayed_work/SKILL.md.inc
	)
endif()
