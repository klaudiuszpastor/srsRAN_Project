/*
 *
 * Copyright 2021-2023 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

// This file was generated using the following MATLAB class on 22-Nov-2022:
//   + "srsTBSCalculatorUnittest.m"

#include "lib/scheduler/support/tbs_calculator.h"
#include <vector>

namespace srsran {

struct test_case_t {
  tbs_calculator_configuration config;
  unsigned                     tbs;
};

static const std::vector<test_case_t> tbs_calculator_test_data = {
    // clang-format off
  {{12, 6, 0, {modulation_scheme::QPSK, 102.4}, 1, 0, 6}, 160},
  {{12, 6, 0, {modulation_scheme::QPSK, 102.4}, 1, 2, 6}, 40},
  {{12, 6, 12, {modulation_scheme::QPSK, 102.4}, 1, 0, 6}, 144},
  {{12, 6, 12, {modulation_scheme::QPSK, 102.4}, 1, 2, 6}, 32},
  {{12, 6, 0, {modulation_scheme::QPSK, 921.6}, 1, 0, 6}, 1544},
  {{12, 6, 0, {modulation_scheme::QPSK, 921.6}, 1, 2, 6}, 368},
  {{12, 6, 12, {modulation_scheme::QPSK, 921.6}, 1, 0, 6}, 1416},
  {{12, 6, 12, {modulation_scheme::QPSK, 921.6}, 1, 2, 6}, 336},
  {{12, 36, 0, {modulation_scheme::QPSK, 102.4}, 1, 0, 6}, 128},
  {{12, 36, 0, {modulation_scheme::QPSK, 102.4}, 1, 2, 6}, 32},
  {{12, 36, 12, {modulation_scheme::QPSK, 102.4}, 1, 0, 6}, 112},
  {{12, 36, 12, {modulation_scheme::QPSK, 102.4}, 1, 2, 6}, 24},
  {{12, 36, 0, {modulation_scheme::QPSK, 921.6}, 1, 0, 6}, 1160},
  {{12, 36, 0, {modulation_scheme::QPSK, 921.6}, 1, 2, 6}, 288},
  {{12, 36, 12, {modulation_scheme::QPSK, 921.6}, 1, 0, 6}, 1032},
  {{12, 36, 12, {modulation_scheme::QPSK, 921.6}, 1, 2, 6}, 256},
  {{12, 6, 0, {modulation_scheme::QPSK, 102.4}, 1, 0, 11}, 304},
  {{12, 6, 0, {modulation_scheme::QPSK, 102.4}, 1, 2, 11}, 72},
  {{12, 6, 12, {modulation_scheme::QPSK, 102.4}, 1, 0, 11}, 272},
  {{12, 6, 12, {modulation_scheme::QPSK, 102.4}, 1, 2, 11}, 64},
  {{12, 6, 0, {modulation_scheme::QPSK, 921.6}, 1, 0, 11}, 2728},
  {{12, 6, 0, {modulation_scheme::QPSK, 921.6}, 1, 2, 11}, 704},
  {{12, 6, 12, {modulation_scheme::QPSK, 921.6}, 1, 0, 11}, 2472},
  {{12, 6, 12, {modulation_scheme::QPSK, 921.6}, 1, 2, 11}, 640},
  {{12, 36, 0, {modulation_scheme::QPSK, 102.4}, 1, 0, 11}, 240},
  {{12, 36, 0, {modulation_scheme::QPSK, 102.4}, 1, 2, 11}, 56},
  {{12, 36, 12, {modulation_scheme::QPSK, 102.4}, 1, 0, 11}, 208},
  {{12, 36, 12, {modulation_scheme::QPSK, 102.4}, 1, 2, 11}, 48},
  {{12, 36, 0, {modulation_scheme::QPSK, 921.6}, 1, 0, 11}, 2152},
  {{12, 36, 0, {modulation_scheme::QPSK, 921.6}, 1, 2, 11}, 528},
  {{12, 36, 12, {modulation_scheme::QPSK, 921.6}, 1, 0, 11}, 1928},
  {{12, 36, 12, {modulation_scheme::QPSK, 921.6}, 1, 2, 11}, 480},
  {{12, 6, 0, {modulation_scheme::QPSK, 102.4}, 4, 0, 6}, 672},
  {{12, 6, 0, {modulation_scheme::QPSK, 102.4}, 4, 2, 6}, 160},
  {{12, 6, 12, {modulation_scheme::QPSK, 102.4}, 4, 0, 6}, 608},
  {{12, 6, 12, {modulation_scheme::QPSK, 102.4}, 4, 2, 6}, 144},
  {{12, 6, 0, {modulation_scheme::QPSK, 921.6}, 4, 0, 6}, 5888},
  {{12, 6, 0, {modulation_scheme::QPSK, 921.6}, 4, 2, 6}, 1544},
  {{12, 6, 12, {modulation_scheme::QPSK, 921.6}, 4, 0, 6}, 5376},
  {{12, 6, 12, {modulation_scheme::QPSK, 921.6}, 4, 2, 6}, 1416},
  {{12, 36, 0, {modulation_scheme::QPSK, 102.4}, 4, 0, 6}, 528},
  {{12, 36, 0, {modulation_scheme::QPSK, 102.4}, 4, 2, 6}, 128},
  {{12, 36, 12, {modulation_scheme::QPSK, 102.4}, 4, 0, 6}, 456},
  {{12, 36, 12, {modulation_scheme::QPSK, 102.4}, 4, 2, 6}, 112},
  {{12, 36, 0, {modulation_scheme::QPSK, 921.6}, 4, 0, 6}, 4608},
  {{12, 36, 0, {modulation_scheme::QPSK, 921.6}, 4, 2, 6}, 1160},
  {{12, 36, 12, {modulation_scheme::QPSK, 921.6}, 4, 0, 6}, 4096},
  {{12, 36, 12, {modulation_scheme::QPSK, 921.6}, 4, 2, 6}, 1032},
  {{12, 6, 0, {modulation_scheme::QPSK, 102.4}, 4, 0, 11}, 1224},
  {{12, 6, 0, {modulation_scheme::QPSK, 102.4}, 4, 2, 11}, 304},
  {{12, 6, 12, {modulation_scheme::QPSK, 102.4}, 4, 0, 11}, 1128},
  {{12, 6, 12, {modulation_scheme::QPSK, 102.4}, 4, 2, 11}, 272},
  {{12, 6, 0, {modulation_scheme::QPSK, 921.6}, 4, 0, 11}, 11016},
  {{12, 6, 0, {modulation_scheme::QPSK, 921.6}, 4, 2, 11}, 2728},
  {{12, 6, 12, {modulation_scheme::QPSK, 921.6}, 4, 0, 11}, 9992},
  {{12, 6, 12, {modulation_scheme::QPSK, 921.6}, 4, 2, 11}, 2472},
  {{12, 36, 0, {modulation_scheme::QPSK, 102.4}, 4, 0, 11}, 984},
  {{12, 36, 0, {modulation_scheme::QPSK, 102.4}, 4, 2, 11}, 240},
  {{12, 36, 12, {modulation_scheme::QPSK, 102.4}, 4, 0, 11}, 848},
  {{12, 36, 12, {modulation_scheme::QPSK, 102.4}, 4, 2, 11}, 208},
  {{12, 36, 0, {modulation_scheme::QPSK, 921.6}, 4, 0, 11}, 8456},
  {{12, 36, 0, {modulation_scheme::QPSK, 921.6}, 4, 2, 11}, 2152},
  {{12, 36, 12, {modulation_scheme::QPSK, 921.6}, 4, 0, 11}, 7552},
  {{12, 36, 12, {modulation_scheme::QPSK, 921.6}, 4, 2, 11}, 1928},
  {{12, 6, 0, {modulation_scheme::QAM16, 102.4}, 1, 0, 6}, 336},
  {{12, 6, 0, {modulation_scheme::QAM16, 102.4}, 1, 2, 6}, 80},
  {{12, 6, 12, {modulation_scheme::QAM16, 102.4}, 1, 0, 6}, 304},
  {{12, 6, 12, {modulation_scheme::QAM16, 102.4}, 1, 2, 6}, 72},
  {{12, 6, 0, {modulation_scheme::QAM16, 921.6}, 1, 0, 6}, 2976},
  {{12, 6, 0, {modulation_scheme::QAM16, 921.6}, 1, 2, 6}, 768},
  {{12, 6, 12, {modulation_scheme::QAM16, 921.6}, 1, 0, 6}, 2728},
  {{12, 6, 12, {modulation_scheme::QAM16, 921.6}, 1, 2, 6}, 704},
  {{12, 36, 0, {modulation_scheme::QAM16, 102.4}, 1, 0, 6}, 256},
  {{12, 36, 0, {modulation_scheme::QAM16, 102.4}, 1, 2, 6}, 64},
  {{12, 36, 12, {modulation_scheme::QAM16, 102.4}, 1, 0, 6}, 224},
  {{12, 36, 12, {modulation_scheme::QAM16, 102.4}, 1, 2, 6}, 56},
  {{12, 36, 0, {modulation_scheme::QAM16, 921.6}, 1, 0, 6}, 2408},
  {{12, 36, 0, {modulation_scheme::QAM16, 921.6}, 1, 2, 6}, 576},
  {{12, 36, 12, {modulation_scheme::QAM16, 921.6}, 1, 0, 6}, 2088},
  {{12, 36, 12, {modulation_scheme::QAM16, 921.6}, 1, 2, 6}, 528},
  {{12, 6, 0, {modulation_scheme::QAM16, 102.4}, 1, 0, 11}, 608},
  {{12, 6, 0, {modulation_scheme::QAM16, 102.4}, 1, 2, 11}, 144},
  {{12, 6, 12, {modulation_scheme::QAM16, 102.4}, 1, 0, 11}, 552},
  {{12, 6, 12, {modulation_scheme::QAM16, 102.4}, 1, 2, 11}, 136},
  {{12, 6, 0, {modulation_scheme::QAM16, 921.6}, 1, 0, 11}, 5504},
  {{12, 6, 0, {modulation_scheme::QAM16, 921.6}, 1, 2, 11}, 1416},
  {{12, 6, 12, {modulation_scheme::QAM16, 921.6}, 1, 0, 11}, 4992},
  {{12, 6, 12, {modulation_scheme::QAM16, 921.6}, 1, 2, 11}, 1256},
  {{12, 36, 0, {modulation_scheme::QAM16, 102.4}, 1, 0, 11}, 480},
  {{12, 36, 0, {modulation_scheme::QAM16, 102.4}, 1, 2, 11}, 112},
  {{12, 36, 12, {modulation_scheme::QAM16, 102.4}, 1, 0, 11}, 432},
  {{12, 36, 12, {modulation_scheme::QAM16, 102.4}, 1, 2, 11}, 104},
  {{12, 36, 0, {modulation_scheme::QAM16, 921.6}, 1, 0, 11}, 4224},
  {{12, 36, 0, {modulation_scheme::QAM16, 921.6}, 1, 2, 11}, 1064},
  {{12, 36, 12, {modulation_scheme::QAM16, 921.6}, 1, 0, 11}, 3824},
  {{12, 36, 12, {modulation_scheme::QAM16, 921.6}, 1, 2, 11}, 984},
  {{12, 6, 0, {modulation_scheme::QAM16, 102.4}, 4, 0, 6}, 1320},
  {{12, 6, 0, {modulation_scheme::QAM16, 102.4}, 4, 2, 6}, 336},
  {{12, 6, 12, {modulation_scheme::QAM16, 102.4}, 4, 0, 6}, 1224},
  {{12, 6, 12, {modulation_scheme::QAM16, 102.4}, 4, 2, 6}, 304},
  {{12, 6, 0, {modulation_scheme::QAM16, 921.6}, 4, 0, 6}, 11784},
  {{12, 6, 0, {modulation_scheme::QAM16, 921.6}, 4, 2, 6}, 2976},
  {{12, 6, 12, {modulation_scheme::QAM16, 921.6}, 4, 0, 6}, 10760},
  {{12, 6, 12, {modulation_scheme::QAM16, 921.6}, 4, 2, 6}, 2728},
  {{12, 36, 0, {modulation_scheme::QAM16, 102.4}, 4, 0, 6}, 1032},
  {{12, 36, 0, {modulation_scheme::QAM16, 102.4}, 4, 2, 6}, 256},
  {{12, 36, 12, {modulation_scheme::QAM16, 102.4}, 4, 0, 6}, 928},
  {{12, 36, 12, {modulation_scheme::QAM16, 102.4}, 4, 2, 6}, 224},
  {{12, 36, 0, {modulation_scheme::QAM16, 921.6}, 4, 0, 6}, 9224},
  {{12, 36, 0, {modulation_scheme::QAM16, 921.6}, 4, 2, 6}, 2408},
  {{12, 36, 12, {modulation_scheme::QAM16, 921.6}, 4, 0, 6}, 8192},
  {{12, 36, 12, {modulation_scheme::QAM16, 921.6}, 4, 2, 6}, 2088},
  {{12, 6, 0, {modulation_scheme::QAM16, 102.4}, 4, 0, 11}, 2408},
  {{12, 6, 0, {modulation_scheme::QAM16, 102.4}, 4, 2, 11}, 608},
  {{12, 6, 12, {modulation_scheme::QAM16, 102.4}, 4, 0, 11}, 2216},
  {{12, 6, 12, {modulation_scheme::QAM16, 102.4}, 4, 2, 11}, 552},
  {{12, 6, 0, {modulation_scheme::QAM16, 921.6}, 4, 0, 11}, 22032},
  {{12, 6, 0, {modulation_scheme::QAM16, 921.6}, 4, 2, 11}, 5504},
  {{12, 6, 12, {modulation_scheme::QAM16, 921.6}, 4, 0, 11}, 19968},
  {{12, 6, 12, {modulation_scheme::QAM16, 921.6}, 4, 2, 11}, 4992},
  {{12, 36, 0, {modulation_scheme::QAM16, 102.4}, 4, 0, 11}, 1928},
  {{12, 36, 0, {modulation_scheme::QAM16, 102.4}, 4, 2, 11}, 480},
  {{12, 36, 12, {modulation_scheme::QAM16, 102.4}, 4, 0, 11}, 1736},
  {{12, 36, 12, {modulation_scheme::QAM16, 102.4}, 4, 2, 11}, 432},
  {{12, 36, 0, {modulation_scheme::QAM16, 921.6}, 4, 0, 11}, 16896},
  {{12, 36, 0, {modulation_scheme::QAM16, 921.6}, 4, 2, 11}, 4224},
  {{12, 36, 12, {modulation_scheme::QAM16, 921.6}, 4, 0, 11}, 15112},
  {{12, 36, 12, {modulation_scheme::QAM16, 921.6}, 4, 2, 11}, 3824},
  {{12, 6, 0, {modulation_scheme::QAM64, 102.4}, 1, 0, 6}, 504},
  {{12, 6, 0, {modulation_scheme::QAM64, 102.4}, 1, 2, 6}, 120},
  {{12, 6, 12, {modulation_scheme::QAM64, 102.4}, 1, 0, 6}, 456},
  {{12, 6, 12, {modulation_scheme::QAM64, 102.4}, 1, 2, 6}, 112},
  {{12, 6, 0, {modulation_scheme::QAM64, 921.6}, 1, 0, 6}, 4480},
  {{12, 6, 0, {modulation_scheme::QAM64, 921.6}, 1, 2, 6}, 1128},
  {{12, 6, 12, {modulation_scheme::QAM64, 921.6}, 1, 0, 6}, 4032},
  {{12, 6, 12, {modulation_scheme::QAM64, 921.6}, 1, 2, 6}, 1032},
  {{12, 36, 0, {modulation_scheme::QAM64, 102.4}, 1, 0, 6}, 384},
  {{12, 36, 0, {modulation_scheme::QAM64, 102.4}, 1, 2, 6}, 96},
  {{12, 36, 12, {modulation_scheme::QAM64, 102.4}, 1, 0, 6}, 352},
  {{12, 36, 12, {modulation_scheme::QAM64, 102.4}, 1, 2, 6}, 80},
  {{12, 36, 0, {modulation_scheme::QAM64, 921.6}, 1, 0, 6}, 3496},
  {{12, 36, 0, {modulation_scheme::QAM64, 921.6}, 1, 2, 6}, 888},
  {{12, 36, 12, {modulation_scheme::QAM64, 921.6}, 1, 0, 6}, 3104},
  {{12, 36, 12, {modulation_scheme::QAM64, 921.6}, 1, 2, 6}, 808},
  {{12, 6, 0, {modulation_scheme::QAM64, 102.4}, 1, 0, 11}, 928},
  {{12, 6, 0, {modulation_scheme::QAM64, 102.4}, 1, 2, 11}, 224},
  {{12, 6, 12, {modulation_scheme::QAM64, 102.4}, 1, 0, 11}, 848},
  {{12, 6, 12, {modulation_scheme::QAM64, 102.4}, 1, 2, 11}, 208},
  {{12, 6, 0, {modulation_scheme::QAM64, 921.6}, 1, 0, 11}, 8192},
  {{12, 6, 0, {modulation_scheme::QAM64, 921.6}, 1, 2, 11}, 2088},
  {{12, 6, 12, {modulation_scheme::QAM64, 921.6}, 1, 0, 11}, 7424},
  {{12, 6, 12, {modulation_scheme::QAM64, 921.6}, 1, 2, 11}, 1864},
  {{12, 36, 0, {modulation_scheme::QAM64, 102.4}, 1, 0, 11}, 736},
  {{12, 36, 0, {modulation_scheme::QAM64, 102.4}, 1, 2, 11}, 176},
  {{12, 36, 12, {modulation_scheme::QAM64, 102.4}, 1, 0, 11}, 640},
  {{12, 36, 12, {modulation_scheme::QAM64, 102.4}, 1, 2, 11}, 152},
  {{12, 36, 0, {modulation_scheme::QAM64, 921.6}, 1, 0, 11}, 6400},
  {{12, 36, 0, {modulation_scheme::QAM64, 921.6}, 1, 2, 11}, 1608},
  {{12, 36, 12, {modulation_scheme::QAM64, 921.6}, 1, 0, 11}, 5632},
  {{12, 36, 12, {modulation_scheme::QAM64, 921.6}, 1, 2, 11}, 1480},
  {{12, 6, 0, {modulation_scheme::QAM64, 102.4}, 4, 0, 6}, 2024},
  {{12, 6, 0, {modulation_scheme::QAM64, 102.4}, 4, 2, 6}, 504},
  {{12, 6, 12, {modulation_scheme::QAM64, 102.4}, 4, 0, 6}, 1864},
  {{12, 6, 12, {modulation_scheme::QAM64, 102.4}, 4, 2, 6}, 456},
  {{12, 6, 0, {modulation_scheme::QAM64, 921.6}, 4, 0, 6}, 17928},
  {{12, 6, 0, {modulation_scheme::QAM64, 921.6}, 4, 2, 6}, 4480},
  {{12, 6, 12, {modulation_scheme::QAM64, 921.6}, 4, 0, 6}, 16392},
  {{12, 6, 12, {modulation_scheme::QAM64, 921.6}, 4, 2, 6}, 4032},
  {{12, 36, 0, {modulation_scheme::QAM64, 102.4}, 4, 0, 6}, 1608},
  {{12, 36, 0, {modulation_scheme::QAM64, 102.4}, 4, 2, 6}, 384},
  {{12, 36, 12, {modulation_scheme::QAM64, 102.4}, 4, 0, 6}, 1416},
  {{12, 36, 12, {modulation_scheme::QAM64, 102.4}, 4, 2, 6}, 352},
  {{12, 36, 0, {modulation_scheme::QAM64, 921.6}, 4, 0, 6}, 14088},
  {{12, 36, 0, {modulation_scheme::QAM64, 921.6}, 4, 2, 6}, 3496},
  {{12, 36, 12, {modulation_scheme::QAM64, 921.6}, 4, 0, 6}, 12552},
  {{12, 36, 12, {modulation_scheme::QAM64, 921.6}, 4, 2, 6}, 3104},
  {{12, 6, 0, {modulation_scheme::QAM64, 102.4}, 4, 0, 11}, 3624},
  {{12, 6, 0, {modulation_scheme::QAM64, 102.4}, 4, 2, 11}, 928},
  {{12, 6, 12, {modulation_scheme::QAM64, 102.4}, 4, 0, 11}, 3368},
  {{12, 6, 12, {modulation_scheme::QAM64, 102.4}, 4, 2, 11}, 848},
  {{12, 6, 0, {modulation_scheme::QAM64, 921.6}, 4, 0, 11}, 32776},
  {{12, 6, 0, {modulation_scheme::QAM64, 921.6}, 4, 2, 11}, 8192},
  {{12, 6, 12, {modulation_scheme::QAM64, 921.6}, 4, 0, 11}, 29704},
  {{12, 6, 12, {modulation_scheme::QAM64, 921.6}, 4, 2, 11}, 7424},
  {{12, 36, 0, {modulation_scheme::QAM64, 102.4}, 4, 0, 11}, 2856},
  {{12, 36, 0, {modulation_scheme::QAM64, 102.4}, 4, 2, 11}, 736},
  {{12, 36, 12, {modulation_scheme::QAM64, 102.4}, 4, 0, 11}, 2536},
  {{12, 36, 12, {modulation_scheme::QAM64, 102.4}, 4, 2, 11}, 640},
  {{12, 36, 0, {modulation_scheme::QAM64, 921.6}, 4, 0, 11}, 25608},
  {{12, 36, 0, {modulation_scheme::QAM64, 921.6}, 4, 2, 11}, 6400},
  {{12, 36, 12, {modulation_scheme::QAM64, 921.6}, 4, 0, 11}, 23040},
  {{12, 36, 12, {modulation_scheme::QAM64, 921.6}, 4, 2, 11}, 5632},
  {{12, 6, 0, {modulation_scheme::QAM256, 102.4}, 1, 0, 6}, 672},
  {{12, 6, 0, {modulation_scheme::QAM256, 102.4}, 1, 2, 6}, 160},
  {{12, 6, 12, {modulation_scheme::QAM256, 102.4}, 1, 0, 6}, 608},
  {{12, 6, 12, {modulation_scheme::QAM256, 102.4}, 1, 2, 6}, 144},
  {{12, 6, 0, {modulation_scheme::QAM256, 921.6}, 1, 0, 6}, 5888},
  {{12, 6, 0, {modulation_scheme::QAM256, 921.6}, 1, 2, 6}, 1544},
  {{12, 6, 12, {modulation_scheme::QAM256, 921.6}, 1, 0, 6}, 5376},
  {{12, 6, 12, {modulation_scheme::QAM256, 921.6}, 1, 2, 6}, 1416},
  {{12, 36, 0, {modulation_scheme::QAM256, 102.4}, 1, 0, 6}, 528},
  {{12, 36, 0, {modulation_scheme::QAM256, 102.4}, 1, 2, 6}, 128},
  {{12, 36, 12, {modulation_scheme::QAM256, 102.4}, 1, 0, 6}, 456},
  {{12, 36, 12, {modulation_scheme::QAM256, 102.4}, 1, 2, 6}, 112},
  {{12, 36, 0, {modulation_scheme::QAM256, 921.6}, 1, 0, 6}, 4608},
  {{12, 36, 0, {modulation_scheme::QAM256, 921.6}, 1, 2, 6}, 1160},
  {{12, 36, 12, {modulation_scheme::QAM256, 921.6}, 1, 0, 6}, 4096},
  {{12, 36, 12, {modulation_scheme::QAM256, 921.6}, 1, 2, 6}, 1032},
  {{12, 6, 0, {modulation_scheme::QAM256, 102.4}, 1, 0, 11}, 1224},
  {{12, 6, 0, {modulation_scheme::QAM256, 102.4}, 1, 2, 11}, 304},
  {{12, 6, 12, {modulation_scheme::QAM256, 102.4}, 1, 0, 11}, 1128},
  {{12, 6, 12, {modulation_scheme::QAM256, 102.4}, 1, 2, 11}, 272},
  {{12, 6, 0, {modulation_scheme::QAM256, 921.6}, 1, 0, 11}, 11016},
  {{12, 6, 0, {modulation_scheme::QAM256, 921.6}, 1, 2, 11}, 2728},
  {{12, 6, 12, {modulation_scheme::QAM256, 921.6}, 1, 0, 11}, 9992},
  {{12, 6, 12, {modulation_scheme::QAM256, 921.6}, 1, 2, 11}, 2472},
  {{12, 36, 0, {modulation_scheme::QAM256, 102.4}, 1, 0, 11}, 984},
  {{12, 36, 0, {modulation_scheme::QAM256, 102.4}, 1, 2, 11}, 240},
  {{12, 36, 12, {modulation_scheme::QAM256, 102.4}, 1, 0, 11}, 848},
  {{12, 36, 12, {modulation_scheme::QAM256, 102.4}, 1, 2, 11}, 208},
  {{12, 36, 0, {modulation_scheme::QAM256, 921.6}, 1, 0, 11}, 8456},
  {{12, 36, 0, {modulation_scheme::QAM256, 921.6}, 1, 2, 11}, 2152},
  {{12, 36, 12, {modulation_scheme::QAM256, 921.6}, 1, 0, 11}, 7552},
  {{12, 36, 12, {modulation_scheme::QAM256, 921.6}, 1, 2, 11}, 1928},
  {{12, 6, 0, {modulation_scheme::QAM256, 102.4}, 4, 0, 6}, 2664},
  {{12, 6, 0, {modulation_scheme::QAM256, 102.4}, 4, 2, 6}, 672},
  {{12, 6, 12, {modulation_scheme::QAM256, 102.4}, 4, 0, 6}, 2408},
  {{12, 6, 12, {modulation_scheme::QAM256, 102.4}, 4, 2, 6}, 608},
  {{12, 6, 0, {modulation_scheme::QAM256, 921.6}, 4, 0, 6}, 24072},
  {{12, 6, 0, {modulation_scheme::QAM256, 921.6}, 4, 2, 6}, 5888},
  {{12, 6, 12, {modulation_scheme::QAM256, 921.6}, 4, 0, 6}, 21504},
  {{12, 6, 12, {modulation_scheme::QAM256, 921.6}, 4, 2, 6}, 5376},
  {{12, 36, 0, {modulation_scheme::QAM256, 102.4}, 4, 0, 6}, 2088},
  {{12, 36, 0, {modulation_scheme::QAM256, 102.4}, 4, 2, 6}, 528},
  {{12, 36, 12, {modulation_scheme::QAM256, 102.4}, 4, 0, 6}, 1864},
  {{12, 36, 12, {modulation_scheme::QAM256, 102.4}, 4, 2, 6}, 456},
  {{12, 36, 0, {modulation_scheme::QAM256, 921.6}, 4, 0, 6}, 18432},
  {{12, 36, 0, {modulation_scheme::QAM256, 921.6}, 4, 2, 6}, 4608},
  {{12, 36, 12, {modulation_scheme::QAM256, 921.6}, 4, 0, 6}, 16392},
  {{12, 36, 12, {modulation_scheme::QAM256, 921.6}, 4, 2, 6}, 4096},
  {{12, 6, 0, {modulation_scheme::QAM256, 102.4}, 4, 0, 11}, 4872},
  {{12, 6, 0, {modulation_scheme::QAM256, 102.4}, 4, 2, 11}, 1224},
  {{12, 6, 12, {modulation_scheme::QAM256, 102.4}, 4, 0, 11}, 4360},
  {{12, 6, 12, {modulation_scheme::QAM256, 102.4}, 4, 2, 11}, 1128},
  {{12, 6, 0, {modulation_scheme::QAM256, 921.6}, 4, 0, 11}, 44040},
  {{12, 6, 0, {modulation_scheme::QAM256, 921.6}, 4, 2, 11}, 11016},
  {{12, 6, 12, {modulation_scheme::QAM256, 921.6}, 4, 0, 11}, 39936},
  {{12, 6, 12, {modulation_scheme::QAM256, 921.6}, 4, 2, 11}, 9992},
  {{12, 36, 0, {modulation_scheme::QAM256, 102.4}, 4, 0, 11}, 3824},
  {{12, 36, 0, {modulation_scheme::QAM256, 102.4}, 4, 2, 11}, 984},
  {{12, 36, 12, {modulation_scheme::QAM256, 102.4}, 4, 0, 11}, 3368},
  {{12, 36, 12, {modulation_scheme::QAM256, 102.4}, 4, 2, 11}, 848},
  {{12, 36, 0, {modulation_scheme::QAM256, 921.6}, 4, 0, 11}, 33816},
  {{12, 36, 0, {modulation_scheme::QAM256, 921.6}, 4, 2, 11}, 8456},
  {{12, 36, 12, {modulation_scheme::QAM256, 921.6}, 4, 0, 11}, 30216},
  {{12, 36, 12, {modulation_scheme::QAM256, 921.6}, 4, 2, 11}, 7552},
    // clang-format on
};

} // namespace srsran
