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

#include "ldpc_rate_dematcher_impl.h"

namespace srsran {

class ldpc_rate_dematcher_avx512_impl : public ldpc_rate_dematcher_impl
{
protected:
  void combine_softbits(span<log_likelihood_ratio>       out,
                        span<const log_likelihood_ratio> in0,
                        span<const log_likelihood_ratio> in1) const override;
};

} // namespace srsran
