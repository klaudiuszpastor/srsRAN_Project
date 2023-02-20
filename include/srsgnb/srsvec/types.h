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

#include "srsgnb/adt/complex.h"
#include "srsgnb/adt/span.h"
#include "srsgnb/support/srsgnb_assert.h"
#include <cstdint>

// The supported vector data types are:
// - srsran::span<cf_t>: For complex float vectors
// - srsran::span<float>: For float vectors
// - srsran::span<int16_t>: For signed 16 bit integer vectors
// - srsran::span<int8_t>: For signed 8 bit integer vectors
// - srsran::span<uint8_t>: For unsigned 8 bit integer vectors
//
// To make the span constant use srsran::span<const T>.

#define srsgnb_srsvec_assert_size(X, Y)                                                                                \
  srsgnb_assert(                                                                                                       \
      X.size() == Y.size(), "The size " #X " (i.e., {}) and " #Y " (i.e., {}) must be equal.", X.size(), Y.size())
