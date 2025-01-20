#!/usr/bin/env bash
# Copyright (c) 2024 Nordic Semiconductor
# SPDX-License-Identifier: Apache-2.0

set -eu

source ${ZEPHYR_BASE}/tests/bsim/sh_common.source

simulation_id=$(guess_test_long_name)
bsim_exe=./bs_${BOARD_TS}_$(guess_test_long_name)_prj_conf
verbosity_level=2

SIM_LEN_US=$((1 * 1000 * 1000))

cd ${BSIM_OUT_PATH}/bin

Execute "${bsim_exe}" -v=${verbosity_level} -s=${simulation_id} -d=0 -rs=420 -testid=l2cap/ecred/dut
Execute "${bsim_exe}" -v=${verbosity_level} -s=${simulation_id} -d=1 -rs=69 -testid=l2cap/ecred/peer

Execute ./bs_2G4_phy_v1 -v=${verbosity_level} -s=${simulation_id} -D=2 -sim_length=${SIM_LEN_US} $@

wait_for_background_jobs
