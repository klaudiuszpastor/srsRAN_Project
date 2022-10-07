/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "dl_logical_channel_manager.h"
#include "harq_process.h"
#include "srsgnb/adt/stable_id_map.h"
#include "srsgnb/ran/du_types.h"
#include "srsgnb/scheduler/mac_scheduler.h"
#include "ue_configuration.h"
#include "ul_logical_channel_manager.h"

namespace srsgnb {

class ue_carrier
{
public:
  ue_carrier(du_ue_index_t                                ue_index,
             rnti_t                                       crnti_val,
             const cell_configuration&                    cell_cfg_common_,
             const serving_cell_ue_configuration_request& ue_serv_cell) :
    ue_index(ue_index),
    cell_index(ue_serv_cell.cell_index),
    harqs(crnti_val, 52, 16, srslog::fetch_basic_logger("MAC")),
    crnti_(crnti_val),
    ue_cfg(cell_cfg_common_, *ue_serv_cell.serv_cell_cfg)
  {
  }

  const du_ue_index_t   ue_index;
  const du_cell_index_t cell_index;

  rnti_t rnti() const { return crnti_; }

  bwp_id_t active_bwp_id() const { return to_bwp_id(0); }
  bool     is_active() const { return true; }

  const ue_cell_configuration& cfg() const { return ue_cfg; }

  harq_entity harqs;

private:
  rnti_t                crnti_;
  ue_cell_configuration ue_cfg;
};

class ue
{
public:
  ue(const cell_configuration& cell_cfg_common_, const sched_ue_creation_request_message& req) :
    ue_index(req.ue_index),
    crnti(req.crnti),
    cell_cfg_common(cell_cfg_common_),
    log_channels_configs(req.lc_config_list),
    sched_request_configs(req.sched_request_config_list)
  {
    for (unsigned i = 0; i != req.cells.size(); ++i) {
      carriers[i] = std::make_unique<ue_carrier>(ue_index, req.crnti, cell_cfg_common, req.cells[i]);
      ue_cc_list.push_back(carriers[i].get());
    }
  }
  ue(const ue&)            = delete;
  ue(ue&&)                 = delete;
  ue& operator=(const ue&) = delete;
  ue& operator=(ue&&)      = delete;

  const du_ue_index_t ue_index;
  const rnti_t        crnti;

  void slot_indication(slot_point sl_tx) {}

  ue_carrier* find_cc(du_cell_index_t cell_index)
  {
    srsgnb_assert(cell_index < MAX_CELLS, "Invalid cell_index={}", cell_index);
    return carriers[cell_index].get();
  }

  const ue_carrier* find_cc(du_cell_index_t cell_index) const
  {
    srsgnb_assert(cell_index < MAX_CELLS, "Invalid cell_index={}", cell_index);
    return carriers[cell_index].get();
  }

  span<ue_carrier*> ue_carriers() { return ue_cc_list; }

  span<ue_carrier* const> ue_carriers() const { return span<ue_carrier* const>(ue_cc_list.data(), ue_cc_list.size()); }

  bool is_ca_enabled() const { return ue_cc_list.size() > 1; }

  void activate_cells(bounded_bitset<MAX_NOF_DU_CELLS> activ_bitmap) {}

  /// \brief Handle received SR indication.
  void handle_sr_indication(const sr_indication_message& msg) { ul_lc_ch_mgr.handle_sr_indication(msg); }

  /// \brief Once an UL grant is given, the SR status of the UE must be reset.
  void reset_sr_indication() { ul_lc_ch_mgr.reset_sr_indication(); }

  /// \brief Handles received BSR indication by updating UE UL logical channel states.
  void handle_bsr_indication(const ul_bsr_indication_message& msg) { ul_lc_ch_mgr.handle_bsr_indication(msg); }

  /// \brief Handles MAC CE indication.
  void handle_dl_mac_ce_indication(const dl_mac_ce_indication& msg)
  {
    dl_lc_ch_mgr.handle_mac_ce_indication(msg.ce_lcid);
  }

  /// \brief Handles DL Buffer State indication.
  void handle_dl_buffer_state_indication(const dl_buffer_state_indication_message& msg)
  {
    dl_lc_ch_mgr.handle_dl_buffer_status_indication(msg.lcid, msg.bs);
  }

  void handle_reconfiguration_request(const sched_ue_reconfiguration_message& msg);

  /// \brief Computes the number of pending bytes to be allocated for the first time in UL for a given UE.
  unsigned pending_ul_newtx_bytes() const;

  /// UE DL Logical Channel Manager.
  dl_logical_channel_manager dl_lc_ch_mgr;

private:
  static const size_t MAX_CELLS = 4;

  /// Cell configuration. This is common to all UEs within the same cell.
  const cell_configuration& cell_cfg_common;

  /// List of \c mac-LogicalChannelConfig, TS 38.331; \ref sched_ue_creation_request_message.
  std::vector<logical_channel_config> log_channels_configs;
  /// \c schedulingRequestToAddModList, TS 38.331; \ref sched_ue_creation_request_message.
  std::vector<scheduling_request_to_addmod> sched_request_configs;

  /// List of UE carriers indexed by DU Cell Index.
  std::array<std::unique_ptr<ue_carrier>, MAX_CELLS> carriers;

  /// List of UE carriers indexed by UE component carrier index.
  static_vector<ue_carrier*, MAX_CELLS> ue_cc_list;

  /// UE UL Logical Channel Manager.
  ul_logical_channel_manager ul_lc_ch_mgr;
};

/// Container that stores all scheduler UEs.
using ue_list = stable_id_map<du_ue_index_t, ue, MAX_NOF_DU_UES>;

} // namespace srsgnb
