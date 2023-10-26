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

#include "rlc_sdu_queue.h"
#include "rlc_tx_entity.h"
#include "srsran/support/executors/task_executor.h"

namespace srsran {

class rlc_tx_tm_entity : public rlc_tx_entity
{
private:
  rlc_sdu_queue sdu_queue;

  task_executor& pcell_executor;

  pcap_rlc_pdu_context pcap_context;

  /// This atomic_flag indicates whether a buffer state update task has been queued but not yet run by pcell_executor.
  /// It helps to avoid queuing of redundant notification tasks in case of frequent changes of the buffer status.
  /// If the flag is set, no further notification needs to be scheduled, because the already queued task will pick the
  /// latest buffer state upon execution.
  std::atomic_flag pending_buffer_state = ATOMIC_FLAG_INIT;

public:
  rlc_tx_tm_entity(du_ue_index_t                        du_index,
                   rb_id_t                              rb_id,
                   rlc_tx_upper_layer_data_notifier&    upper_dn_,
                   rlc_tx_upper_layer_control_notifier& upper_cn_,
                   rlc_tx_lower_layer_notifier&         lower_dn_,
                   task_executor&                       pcell_executor_,
                   pcap_rlc&                            pcap_);

  // Interfaces for higher layers
  void handle_sdu(rlc_sdu sdu) override;
  void discard_sdu(uint32_t pdcp_sn) override;

  // Interfaces for lower layers
  byte_buffer_chain pull_pdu(uint32_t grant_len) override;

  uint32_t get_buffer_state() override;

private:
  /// Called whenever the buffer state has been changed by upper layers (new SDUs or SDU discard) so that lower layers
  /// need to be informed about the new buffer state. This function defers the actual notification \c
  /// handle_changed_buffer_state to pcell_executor. The notification is discarded if another notification is already
  /// queued for execution. This function should not be called from \c pull_pdu, since the lower layer accounts for the
  /// amount of extracted data itself.
  ///
  /// Safe execution from: Any executor
  void handle_changed_buffer_state();

  /// Immediately informs the lower layer of the current buffer state. This function is called from pcell_executor and
  /// its execution is queued by \c handle_changed_buffer_state.
  ///
  /// Safe execution from: pcell_executor
  void update_mac_buffer_state();
};

} // namespace srsran
