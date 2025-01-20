#!/usr/bin/env bash
# Copyright (c) 2024 Nordic Semiconductor
# SPDX-License-Identifier: Apache-2.0

source ${ZEPHYR_BASE}/tests/bsim/sh_common.source

simulation_id=$(guess_test_long_name)
bsim_exe=./bs_${BOARD_TS}_$(guess_test_long_name)_prj_conf

cd ${BSIM_OUT_PATH}/bin

Execute "${bsim_exe}" -s=${simulation_id} -d=0 \
  -testid=l2cap/many_conns/central -rs=42
Execute "${bsim_exe}" -s=${simulation_id} -d=1 \
  -testid=l2cap/many_conns/peripheral -rs=1
Execute "${bsim_exe}" -s=${simulation_id} -d=2 \
  -testid=l2cap/many_conns/peripheral -rs=2
Execute "${bsim_exe}" -s=${simulation_id} -d=3 \
  -testid=l2cap/many_conns/peripheral -rs=3
Execute "${bsim_exe}" -s=${simulation_id} -d=4 \
  -testid=l2cap/many_conns/peripheral -rs=4

Execute ./bs_2G4_phy_v1 -s=${simulation_id} -D=5 -sim_length=10e6 $@

wait_for_background_jobs
