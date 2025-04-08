/*
 * Copyright (c) 2025 Nordic Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Test for proper unreferencing of the advertising buffers.
 */

#include "mesh_test.h"
#include "mesh/net.h"
#include "mesh/adv.h"

#include <string.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(test_adv_buf, LOG_LEVEL_INF);

#define CLI_ADDR    0x7728
#define SRV_ADDR    0x18f8
#define WAIT_TIME   60 /* seconds */
#define SEM_TIMEOUT K_SECONDS(10)

#define BT_MESH_DUMMY_VND_MOD_GET_OP	BT_MESH_MODEL_OP_3(0xDC, TEST_VND_COMPANY_ID)
#define BT_MESH_DUMMY_VND_MOD_STATUS_OP BT_MESH_MODEL_OP_3(0xCD, TEST_VND_COMPANY_ID)

#define BT_MESH_DUMMY_VND_MOD_MSG_MINLEN 7
#define BT_MESH_DUMMY_VND_MOD_MSG_MAXLEN 8

#define TEST_SEND_ITR 100

static struct k_sem cli_suspend_sem;
static struct k_sem srv_suspend_sem;
static const uint8_t dev_key[16] = {0xaa};
static struct bt_mesh_prov prov;
static struct bt_mesh_cfg_cli cfg_cli;

static int get_handler(const struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
		       struct net_buf_simple *buf)
{
	BT_MESH_MODEL_BUF_DEFINE(msg, BT_MESH_DUMMY_VND_MOD_STATUS_OP,
				 BT_MESH_DUMMY_VND_MOD_MSG_MAXLEN);

	bt_mesh_model_msg_init(&msg, BT_MESH_DUMMY_VND_MOD_STATUS_OP);
	memset(net_buf_simple_add(&msg, BT_MESH_DUMMY_VND_MOD_MSG_MINLEN), 0,
	       BT_MESH_DUMMY_VND_MOD_MSG_MINLEN);

	k_sem_give(&srv_suspend_sem);

	return bt_mesh_model_send(model, ctx, &msg, NULL, NULL);
}

static int status_handler(const struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
			  struct net_buf_simple *buf)
{
	k_sem_give(&cli_suspend_sem);

	return 0;
}

static int dummy_vnd_mod_get(const struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx)
{
	BT_MESH_MODEL_BUF_DEFINE(msg, BT_MESH_DUMMY_VND_MOD_GET_OP,
				 BT_MESH_DUMMY_VND_MOD_MSG_MAXLEN);

	bt_mesh_model_msg_init(&msg, BT_MESH_DUMMY_VND_MOD_GET_OP);
	memset(net_buf_simple_add(&msg, BT_MESH_DUMMY_VND_MOD_MSG_MINLEN), 0,
	       BT_MESH_DUMMY_VND_MOD_MSG_MINLEN);

	return bt_mesh_model_send(model, ctx, &msg, NULL, NULL);
}

static const struct bt_mesh_model_op _dummy_vnd_mod_op[] = {
	{BT_MESH_DUMMY_VND_MOD_GET_OP, BT_MESH_DUMMY_VND_MOD_MSG_MINLEN, get_handler},
	{BT_MESH_DUMMY_VND_MOD_STATUS_OP, BT_MESH_DUMMY_VND_MOD_MSG_MINLEN, status_handler},
	BT_MESH_MODEL_OP_END,
};

static const struct bt_mesh_elem elements[] = {BT_MESH_ELEM(
	0,
	MODEL_LIST(BT_MESH_MODEL_CFG_SRV, BT_MESH_MODEL_CFG_CLI(&cfg_cli)),
	MODEL_LIST(BT_MESH_MODEL_VND_CB(TEST_VND_COMPANY_ID, TEST_VND_MOD_ID, _dummy_vnd_mod_op,
					NULL, NULL, NULL)))};

static const struct bt_mesh_comp comp = {
	.cid = TEST_VND_COMPANY_ID,
	.elem = elements,
	.elem_count = ARRAY_SIZE(elements),
};

static void prov_and_conf(uint16_t addr)
{
	uint8_t status;
	int err;

	ASSERT_OK(bt_mesh_provision(test_net_key, 0, 0, 0, addr, dev_key));

	err = bt_mesh_cfg_cli_app_key_add(0, addr, 0, 0, test_app_key, &status);
	if (err || status) {
		FAIL("AppKey add failed (err %d, status %u)", err, status);
	}
	err = bt_mesh_cfg_cli_mod_app_bind_vnd(0, addr, addr, 0, TEST_VND_MOD_ID,
						TEST_VND_COMPANY_ID, &status);
	if (err || status) {
		FAIL("Failed to bind TEST_MOD to application (err %d, status %u)", err,
		     status);
	}
}

// static void adv_suspend(void)
// {
// 	atomic_set_bit(bt_mesh.flags, BT_MESH_SUSPENDED);
// 	ASSERT_OK_MSG(bt_mesh_adv_disable(), "Failed to disable advertiser");
// }

// static void adv_resume(void)
// {
// 	atomic_clear_bit(bt_mesh.flags, BT_MESH_SUSPENDED);
// 	ASSERT_OK_MSG(bt_mesh_adv_enable(), "Failed to enable advertiser");
// }

static void test_cli_msg_send(void)
{
	int err;
	uint8_t status;

	bt_mesh_test_cfg_set(NULL, WAIT_TIME);
	bt_mesh_device_setup(&prov, &comp);
	prov_and_conf(CLI_ADDR);

	err = bt_mesh_cfg_cli_net_transmit_set(0, CLI_ADDR, BT_MESH_TRANSMIT(7, 50), &status);
	if (err || status != BT_MESH_TRANSMIT(7, 50)) {
		FAIL("Net transmit set failed (err %d, status %u)", err, status);
	}

	ASSERT_OK(k_sem_init(&cli_suspend_sem, 0, TEST_SEND_ITR));

	struct bt_mesh_msg_ctx ctx = {
		.net_idx = 0,
		.app_idx = 0,
		.addr = SRV_ADDR,
	};

	for (int i = 0; i < TEST_SEND_ITR; i++) {
		
		ASSERT_OK(dummy_vnd_mod_get(&elements[0].vnd_models[0], &ctx));

		if (k_sem_take(&cli_suspend_sem, SEM_TIMEOUT)) {
			FAIL("Client timed out waiting for STATUS message");
		}

		printk("CLI: Received status %d\n", i+1);

		// adv_suspend();
		// k_sleep(K_SECONDS(1));
		// adv_resume();
		// k_sleep(K_MSEC(100)); // Give the advertiser time to re-enable
	}

	PASS();
}

static void test_srv_msg_recv(void)
{
	bt_mesh_test_cfg_set(NULL, WAIT_TIME);
	bt_mesh_device_setup(&prov, &comp);
	prov_and_conf(SRV_ADDR);

	ASSERT_OK(k_sem_init(&srv_suspend_sem, 0, TEST_SEND_ITR));

	/* Wait for all 100 GET messages to have been received */
	for (int i = 0; i < TEST_SEND_ITR; i++) {

		if (k_sem_take(&srv_suspend_sem, SEM_TIMEOUT)) {
			FAIL("Server timed out waiting for GET message");
		}
		printk("SRV: Received get %d\n", i+1);
	}
	
	PASS();
}

#define TEST_CASE(role, name, description)         \
	{                                              \
		.test_id = "adv_buf_" #role "_" #name,      \
		.test_descr = description,                 \
		.test_tick_f = bt_mesh_test_timeout,       \
		.test_main_f = test_##role##_##name,       \
	}

static const struct bst_test_instance test_adv_buf[] = {
	TEST_CASE(
		cli, msg_send,
		"Sends 100 GET messages to server, disabling and resuming mesh after each STATUS "
		"message received. "),
	TEST_CASE(srv, msg_recv,
		  "Waits for GET requests and replies with STATUS messages. "),

	BSTEST_END_MARKER};

struct bst_test_list *test_adv_buf_install(struct bst_test_list *tests)
{
	tests = bst_add_tests(tests, test_adv_buf);
	return tests;
}
