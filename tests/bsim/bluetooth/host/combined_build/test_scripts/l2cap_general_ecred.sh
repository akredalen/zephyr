#!/usr/bin/env bash
# Copyright (c) 2022 Nordic Semiconductor
# SPDX-License-Identifier: Apache-2.0

source ${ZEPHYR_BASE}/tests/bsim/sh_common.source

# EATT test
simulation_id=$(guess_test_long_name)
bsim_exe=./bs_${BOARD_TS}_$(guess_test_long_name)_prj_conf
verbosity_level=2
EXECUTE_TIMEOUT=120

cd ${BSIM_OUT_PATH}/bin

Execute "${bsim_exe}" -v=${verbosity_level} -s=${simulation_id} -d=0 \
  -testid=l2cap/general/ecred/central -rs=43

Execute "${bsim_exe}" -v=${verbosity_level} -s=${simulation_id} -d=1 \
  -testid=l2cap/general/ecred/peripheral -rs=42

Execute ./bs_2G4_phy_v1 -v=${verbosity_level} -s=${simulation_id} \
  -D=2 -sim_length=60e6 $@

wait_for_background_jobs
