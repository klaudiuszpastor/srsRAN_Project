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
#include "resource_grid_dimensions.h"
#include "srsran/phy/support/resource_grid_reader.h"

namespace srsran {

/// Implements the resource grid reader interface.
class resource_grid_reader_impl : public resource_grid_reader
{
public:
  using storage_type = tensor<static_cast<unsigned>(resource_grid_dimensions::all), cf_t, resource_grid_dimensions>;

  /// Constructs a resource grid reader implementation from a tensor.
  resource_grid_reader_impl(const storage_type& data_, span<const bool> empty_) : data(data_), empty(empty_) {}

  // See interface for documentation.
  unsigned get_nof_ports() const override;

  // See interface for documentation.
  unsigned get_nof_subc() const override;

  // See interface for documentation.
  unsigned get_nof_symbols() const override;

  // See interface for documentation.
  bool is_empty(unsigned port) const override;

  // See interface for documentation.
  span<cf_t> get(span<cf_t> symbols, unsigned port, unsigned l, unsigned k_init, span<const bool> mask) const override;

  // See interface for documentation.
  span<cf_t> get(span<cf_t>                          symbols,
                 unsigned                            port,
                 unsigned                            l,
                 unsigned                            k_init,
                 const bounded_bitset<MAX_RB * NRE>& mask) const override;

  // See interface for documentation.
  void get(span<cf_t> symbols, unsigned port, unsigned l, unsigned k_init) const override;

private:
  const storage_type& data;
  span<const bool>    empty;
};

} // namespace srsran