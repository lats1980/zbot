# Copyright (c) 2026 Lingao Meng
# SPDX-License-Identifier: Apache-2.0

target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/SKILL.c)

set(SKILL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/SKILL.md)

if(CONFIG_BOARD_THINGY53_NRF5340_CPUAPP)
    set(SKILL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/SKILL.md.thingy53)
endif()

generate_inc_file_for_target(
    app
    ${SKILL_FILE}
    ${gen_dir}/skills/gpio/SKILL.md.inc
)
