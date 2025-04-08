#!/usr/bin/env bash
# Copyright 2025 Xiaomi Corporation
# SPDX-License-Identifier: Apache-2.0

source $(dirname "${BASH_SOURCE[0]}")/../../_mesh_test.sh

# Test scenario:

RunTest mesh_adv_test_buf_unref \
    adv_buf_cli_msg_send adv_buf_srv_msg_recv
