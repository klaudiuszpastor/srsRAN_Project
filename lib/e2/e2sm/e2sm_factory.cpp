/*
 *
 * Copyright 2021-2023 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "srsran/e2/e2sm/e2sm_factory.h"
#include "e2sm_kpm_asn1_packer.h"

using namespace srsran;

std::unique_ptr<e2sm_handler> srsran::create_e2sm(std::string e2sm_version)
{
  if (e2sm_version == "e2sm-kpm-v1") {
    return std::make_unique<e2sm_kpm_asn1_packer>();
  }
  return nullptr;
}
