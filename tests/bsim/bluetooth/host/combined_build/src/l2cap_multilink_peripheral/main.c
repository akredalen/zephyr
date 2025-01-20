/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>

#include "bs_tracing.h"
#include "bstests.h"
#include "babblekit/testcase.h"

extern void entrypoint_multilink_peripheral_dut(void);
extern void entrypoint_multilink_peripheral_central(void);
extern enum bst_result_t bst_result;

static void test_end_cb(void)
{
	/* This callback will fire right before the executable returns */
	if (bst_result != Passed) {
		TEST_PRINT("Test has not passed.");
	}
}

static const struct bst_test_instance entrypoints[] = {
	{
		.test_id = "l2cap/multilink_peripheral/dut",
		.test_delete_f = test_end_cb,
		.test_main_f = entrypoint_multilink_peripheral_dut,
	},
	{
		.test_id = "l2cap/multilink_peripheral/central",
		.test_delete_f = test_end_cb,
		.test_main_f = entrypoint_multilink_peripheral_central,
	},
	BSTEST_END_MARKER,
};

struct bst_test_list *
test_main_l2cap_multilink_peripheral_install(struct bst_test_list *tests)
{
	return bst_add_tests(tests, entrypoints);
};
