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

#include "srsgnb/adt/byte_buffer.h"
#include "srsgnb/adt/optional.h"
#include "srsgnb/asn1/e1ap/e1ap.h"
#include "srsgnb/cu_cp/cu_cp_types.h"
#include "srsgnb/e1/cu_cp/e1_cu_cp.h"
#include <string>
#include <vector>

namespace srsgnb {
namespace srs_cu_cp {

/// @brief Convert CU-CP s-NSSAI to E1AP s-NSSAI.
/// @param cu_cp_snssai The CU-CP s-NSSAI.
/// @return The E1AP s-NSSAI.
inline asn1::e1ap::snssai_s common_snssai_to_e1ap_snssai(srsgnb::srs_cu_cp::cu_cp_s_nssai cu_cp_snssai)
{
  asn1::e1ap::snssai_s snssai;
  snssai.sst.from_number(cu_cp_snssai.sst);
  if (cu_cp_snssai.sd.has_value()) {
    snssai.sd_present = true;
    snssai.sd.from_number(cu_cp_snssai.sd.value());
  }

  return snssai;
}

/// @brief Convert CU-CP PDU session type to E1AP PDU session type.
/// @param cu_cp_session_type The CU-CP PDU session type.
/// @return The E1AP PDU session type.
inline asn1::e1ap::pdu_session_type_e cu_cp_pdu_session_type_to_e1ap_pdu_session_type(std::string cu_cp_session_type)
{
  asn1::e1ap::pdu_session_type_e e1ap_session_type;

  if (cu_cp_session_type == "ethernet") {
    e1ap_session_type = asn1::e1ap::pdu_session_type_opts::options::ethernet;
    return e1ap_session_type;
  } else if (cu_cp_session_type == "ipv4") {
    e1ap_session_type = asn1::e1ap::pdu_session_type_opts::options::ipv4;
    return e1ap_session_type;
  } else if (cu_cp_session_type == "ipv4v6") {
    e1ap_session_type = asn1::e1ap::pdu_session_type_opts::options::ipv4v6;
    return e1ap_session_type;
  } else if (cu_cp_session_type == "ipv6") {
    e1ap_session_type = asn1::e1ap::pdu_session_type_opts::options::ipv6;
    return e1ap_session_type;
  } else {
    e1ap_session_type = asn1::e1ap::pdu_session_type_opts::options::nulltype;
    return e1ap_session_type;
  }
}

/// @brief Convert CU-CP UL NGU UP TNL Information to E1AP UP TNL Information.
/// @param ul_ngu_up_tnl_info The CU-CP UL NGU UP TNL Information.
/// @return The E1AP UP TNL Information.
inline asn1::e1ap::up_tnl_info_c cu_cp_ul_ngu_up_tnl_info_to_e1ap_up_tnl_info(cu_cp_gtp_tunnel ul_ngu_up_tnl_info)
{
  asn1::e1ap::up_tnl_info_c e1ap_up_tnl_info;

  e1ap_up_tnl_info.set_gtp_tunnel();
  auto& e1ap_gtp_tunnel = e1ap_up_tnl_info.gtp_tunnel();
  e1ap_gtp_tunnel.gtp_teid.from_number(ul_ngu_up_tnl_info.gtp_teid);
  e1ap_gtp_tunnel.transport_layer_address.from_string(ul_ngu_up_tnl_info.transport_layer_address);

  return e1ap_up_tnl_info;
}

/// @brief Convert E1AP Cause to CU-CP Cause.
/// @param e1ap_cause The E1AP Cause.
/// @return The CU-CP Cause.
inline cu_cp_cause_t e1ap_cause_to_cu_cp_cause(asn1::e1ap::cause_c e1ap_cause)
{
  cu_cp_cause_t cu_cp_cause;

  switch (e1ap_cause.type()) {
    case asn1::e1ap::cause_c::types_opts::radio_network:
      cu_cp_cause = cu_cp_cause_t::radio_network;
      return cu_cp_cause;
      break;
    case asn1::e1ap::cause_c::types_opts::transport:
      cu_cp_cause = cu_cp_cause_t::transport;
      return cu_cp_cause;
      break;
    case asn1::e1ap::cause_c::types_opts::protocol:
      cu_cp_cause = cu_cp_cause_t::protocol;
      return cu_cp_cause;
      break;
    default:
      cu_cp_cause = cu_cp_cause_t::misc;
      return cu_cp_cause;
      break;
      break;
  }
}

/// @brief Convert E1AP UP TNL Info to CU-CP GTP Tunnel.
/// @param up_tnl_info The E1AP UP TNL Info.
/// @return The CU-CP GTP Tunnel.
inline cu_cp_gtp_tunnel e1ap_up_tnl_info_to_cu_cp_gtp_tunnel(asn1::e1ap::up_tnl_info_c up_tnl_info)
{
  cu_cp_gtp_tunnel gtp_tunnel;
  gtp_tunnel.gtp_teid                = up_tnl_info.gtp_tunnel().gtp_teid.to_number();
  gtp_tunnel.transport_layer_address = up_tnl_info.gtp_tunnel().transport_layer_address.to_number();

  return gtp_tunnel;
}

/// @brief Convert E1AP NG DL UP Unchanged to its boolean representation
/// @param ng_dl_up_unchanged The E1AP NG DL UP Unchanged.
/// @return The boolean representation of E1AP NG DL UP Unchanged.
inline bool
e1ap_ng_dl_up_unchanged_to_bool(asn1::e1ap::pdu_session_res_setup_item_s::ng_dl_up_unchanged_e_ ng_dl_up_unchanged)
{
  return ng_dl_up_unchanged.value ==
         asn1::e1ap::pdu_session_res_setup_item_s::ng_dl_up_unchanged_opts::options::true_value;
}

} // namespace srs_cu_cp
} // namespace srsgnb