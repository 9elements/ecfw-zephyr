/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <zephyr.h>
#include <device.h>
#include <soc.h>
#include <drivers/espi.h>
#include <logging/log.h>
#include "board_config.h"
#include "espi_hub.h"
#include "pwrplane.h"
#include "task_handler.h"
#include "softstrap.h"
#include "vpd_section.h"

LOG_MODULE_REGISTER(ecfw, CONFIG_EC_LOG_LEVEL);

void main(void)
{
	int ret;

	/* Delayed start for debug */
	k_sleep(K_SECONDS(CONFIG_EC_DELAYED_BOOT));

	/* In platform N-1 rework doesn't route SMC_RST to MECC card, causing
	 * I2C glitches among other issues, add a delay to get postcodes
	 * until HW WA is possible.
	 */
	k_msleep(100);

	LOG_INF("EC FW Zephyr %p %s", k_current_get(), CONFIG_BOARD);

	/* The espi block needs to be initialized before the GPIOS. This is
	 * because we must query the boot mode before assigning functions
	 * to certain pins
	 */
	ret = espihub_init();
	if (ret) {
		LOG_ERR("Failed to init espi %d", ret);
		return;
	}

	ret = board_init();
	if (ret) {
		LOG_ERR("Failed to init board %d", ret);
		return;
	}

    /*
     * setup EMI in order to extract, store
     * and share VPDs with the host CPU.
     * */
    expose_vpd_section();

	strap_init();
	start_all_tasks();

	while (true) {
		k_msleep(2100);
	}
}
