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

#include "srsgnb/cu_cp/du_processor.h"
#include "srsgnb/support/async/async_task.h"
#include "srsgnb/support/async/eager_async_task.h"

namespace srsran {
namespace srs_cu_cp {

/// \brief Handles the setup of PDU session resources from the RRC viewpoint.
/// TODO Add seqdiag
class pdu_session_resource_setup_routine
{
public:
  pdu_session_resource_setup_routine(const cu_cp_pdu_session_resource_setup_request& setup_msg_,
                                     const srsran::security::sec_as_config&          security_cfg_,
                                     du_processor_e1ap_control_notifier&             e1ap_ctrl_notif_,
                                     du_processor_f1ap_ue_context_notifier&          f1ap_ue_ctxt_notif_,
                                     du_processor_rrc_ue_control_message_notifier&   rrc_ue_notifier_,
                                     drb_manager&                                    rrc_ue_drb_manager_,
                                     srslog::basic_logger&                           logger_);

  void operator()(coro_context<async_task<cu_cp_pdu_session_resource_setup_response>>& ctx);

  static const char* name() { return "PDU Session Creation Routine"; }

private:
  void fill_e1ap_bearer_context_setup_request(e1ap_bearer_context_setup_request& e1ap_request);
  void fill_e1ap_bearer_context_modification_request(e1ap_bearer_context_modification_request& e1ap_request);

  cu_cp_pdu_session_resource_setup_response handle_pdu_session_resource_setup_result(bool success);

  const cu_cp_pdu_session_resource_setup_request setup_msg;
  const srsran::security::sec_as_config          security_cfg;

  std::vector<drb_id_t> drb_to_add_list; // list of DRBs to be added

  du_processor_e1ap_control_notifier&           e1ap_ctrl_notifier;    // to trigger bearer context setup at CU-UP
  du_processor_f1ap_ue_context_notifier&        f1ap_ue_ctxt_notifier; // to trigger UE context modification at DU
  du_processor_rrc_ue_control_message_notifier& rrc_ue_notifier;       // to trigger RRC Reconfiguration at UE
  drb_manager&                                  rrc_ue_drb_manager;    // to get RRC DRB config
  srslog::basic_logger&                         logger;

  // (sub-)routine requests
  e1ap_bearer_context_setup_request           bearer_context_setup_request;
  cu_cp_ue_context_modification_request       ue_context_mod_request;
  e1ap_bearer_context_modification_request    bearer_context_modification_request;
  cu_cp_rrc_reconfiguration_procedure_request rrc_reconfig_args;

  // (sub-)routine results
  cu_cp_pdu_session_resource_setup_response response_msg;
  e1ap_bearer_context_setup_response        bearer_context_setup_response; // to initially setup the DRBs at the CU-UP
  cu_cp_ue_context_modification_response    ue_context_modification_response; // to inform DU about the new DRBs
  e1ap_bearer_context_modification_response
       bearer_context_modification_response; // to inform CU-UP about the new TEID for UL F1u traffic
  bool rrc_reconfig_result = false;          // the final UE reconfiguration
};

} // namespace srs_cu_cp
} // namespace srsran
