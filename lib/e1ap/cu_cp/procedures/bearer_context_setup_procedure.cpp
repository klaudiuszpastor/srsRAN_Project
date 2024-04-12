/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "bearer_context_setup_procedure.h"
#include "../e1ap_cu_cp_asn1_helpers.h"
#include "cu_cp/ue_context/e1ap_bearer_transaction_manager.h"

using namespace srsran;
using namespace srsran::srs_cu_cp;
using namespace asn1::e1ap;

bearer_context_setup_procedure::bearer_context_setup_procedure(const e1ap_message&              request_,
                                                               e1ap_bearer_transaction_manager& ev_mng_,
                                                               e1ap_ue_context_list&            ue_ctxt_list_,
                                                               e1ap_message_notifier&           e1ap_notif_,
                                                               e1ap_ue_logger&                  logger_) :
  request(request_), ev_mng(ev_mng_), ue_ctxt_list(ue_ctxt_list_), e1ap_notifier(e1ap_notif_), logger(logger_)
{
}

void bearer_context_setup_procedure::operator()(coro_context<async_task<e1ap_bearer_context_setup_response>>& ctx)
{
  CORO_BEGIN(ctx);

  logger.log_debug("\"{}\" initialized", name());

  // Subscribe to respective publisher to receive BEARER CONTEXT SETUP RESPONSE/FAILURE message.
  transaction_sink.subscribe_to(ev_mng.context_setup_outcome);

  // Send command to CU-UP.
  send_bearer_context_setup_request();

  // Await response.
  CORO_AWAIT(transaction_sink);

  // Handle response from CU-UP and return bearer index
  CORO_RETURN(create_bearer_context_setup_result());
}

void bearer_context_setup_procedure::send_bearer_context_setup_request()
{
  // send Bearer context setup request message
  e1ap_notifier.on_new_message(request);
}

e1ap_bearer_context_setup_response bearer_context_setup_procedure::create_bearer_context_setup_result()
{
  e1ap_bearer_context_setup_response res{};

  if (transaction_sink.successful()) {
    const asn1::e1ap::bearer_context_setup_resp_s& resp = transaction_sink.response();

    // Add CU-UP UE E1AP ID to UE context
    if (ue_ctxt_list.contains(int_to_gnb_cu_cp_ue_e1ap_id(resp->gnb_cu_cp_ue_e1ap_id))) {
      ue_ctxt_list.add_cu_up_ue_e1ap_id(int_to_gnb_cu_cp_ue_e1ap_id(resp->gnb_cu_cp_ue_e1ap_id),
                                        int_to_gnb_cu_up_ue_e1ap_id(resp->gnb_cu_up_ue_e1ap_id));
    }

    fill_e1ap_bearer_context_setup_response(res, resp);

    logger.log_debug("\"{}\" finalized", name());
  } else if (transaction_sink.failed()) {
    const asn1::e1ap::bearer_context_setup_fail_s& fail = transaction_sink.failure();
    logger.log_debug("Received BearerContextSetupFailure cause={}", get_cause_str(fail->cause));

    // Add CU-UP UE E1AP ID to UE context
    if (ue_ctxt_list.contains(int_to_gnb_cu_cp_ue_e1ap_id(fail->gnb_cu_cp_ue_e1ap_id))) {
      ue_ctxt_list.add_cu_up_ue_e1ap_id(int_to_gnb_cu_cp_ue_e1ap_id(fail->gnb_cu_cp_ue_e1ap_id),
                                        int_to_gnb_cu_up_ue_e1ap_id(fail->gnb_cu_up_ue_e1ap_id));
    }
    fill_e1ap_bearer_context_setup_response(res, fail);
  } else {
    logger.log_warning("BearerContextSetupResponse timeout");
    res.success = false;

    logger.log_error("\"{}\" failed", name());
  }

  return res;
}