/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "../../lib/rlc/rlc_um_entity.h"
#include <gtest/gtest.h>
#include <queue>

using namespace srsgnb;

/// Mocking class of the surrounding layers invoked by the RLC.
class rlc_test_frame : public rlc_rx_upper_layer_data_notifier,
                       public rlc_tx_upper_layer_data_notifier,
                       public rlc_tx_upper_layer_control_notifier,
                       public rlc_tx_lower_layer_notifier
{
public:
  std::queue<byte_buffer_slice_chain> sdu_queue;
  uint32_t                            sdu_counter = 0;
  std::list<uint32_t>                 transmitted_pdcp_count_list;
  uint32_t                            bsr       = 0;
  uint32_t                            bsr_count = 0;

  // rlc_rx_upper_layer_data_notifier interface
  void on_new_sdu(byte_buffer_slice_chain sdu) override
  {
    sdu_queue.push(std::move(sdu));
    sdu_counter++;
  }

  // rlc_tx_upper_layer_data_notifier interface
  void on_delivered_sdu(uint32_t pdcp_count) override
  {
    // store in list
    transmitted_pdcp_count_list.push_back(pdcp_count);
  }

  // rlc_tx_upper_layer_control_notifier interface
  void on_protocol_failure() override {}
  void on_max_retx() override {}

  // rlc_tx_buffer_state_update_notifier interface
  void on_buffer_state_update(unsigned bsr) override
  {
    this->bsr = bsr;
    this->bsr_count++;
  }
};

/// Fixture class for RLC UM tests
/// It requires TEST_P() and INSTANTIATE_TEST_SUITE_P() to create/spawn tests for each supported SN size
class rlc_um_test : public ::testing::Test, public ::testing::WithParamInterface<rlc_um_sn_size>
{
protected:
  void SetUp() override
  {
    // init test's logger
    srslog::init();
    logger.set_level(srslog::basic_levels::debug);

    // init RLC logger
    srslog::fetch_basic_logger("RLC", false).set_level(srslog::basic_levels::debug);
    srslog::fetch_basic_logger("RLC", false).set_hex_dump_max_size(100);
  }

  void TearDown() override
  {
    // flush logger after each test
    srslog::flush();
  }

  /// \brief Initializes fixture according to size sequence number size
  /// \param sn_size_ size of the sequence number
  void init(rlc_um_sn_size sn_size_)
  {
    logger.info("Creating RLC UM ({} bit)", to_number(sn_size));

    sn_size = sn_size_;

    // Set Rx config
    config.rx.sn_field_length = sn_size;
    config.rx.t_reassembly_ms = 5;

    // Set Tx config
    config.tx.sn_field_length = sn_size;

    // Create RLC entities
    rlc1 = std::make_unique<rlc_um_entity>(
        du_ue_index_t::MIN_DU_UE_INDEX, lcid_t::LCID_SRB0, config, tester1, tester1, tester1, tester1, timers);
    rlc2 = std::make_unique<rlc_um_entity>(
        du_ue_index_t::MIN_DU_UE_INDEX, lcid_t::LCID_SRB0, config, tester2, tester2, tester2, tester2, timers);

    // Bind interfaces
    rlc1_rx_lower = rlc1->get_rx_lower_layer_interface();
    rlc1_tx_upper = rlc1->get_tx_upper_layer_data_interface();
    rlc1_tx_lower = rlc1->get_tx_lower_layer_interface();
    rlc2_rx_lower = rlc2->get_rx_lower_layer_interface();
    rlc2_tx_upper = rlc2->get_tx_upper_layer_data_interface();
    rlc2_tx_lower = rlc2->get_tx_lower_layer_interface();
  }

  void tx_with_pdu_duplicates(uint32_t hold_back_pdu)
  {
    uint32_t buffer_state = 0;

    const uint32_t num_sdus = 1;
    const uint32_t sdu_size = 100;

    // Push SDUs into RLC1
    byte_buffer sdu_bufs[num_sdus];
    for (uint32_t i = 0; i < num_sdus; i++) {
      sdu_bufs[i] = byte_buffer();
      // Write the index into the buffer
      for (uint32_t k = 0; k < sdu_size; ++k) {
        sdu_bufs[i].append(i + k);
      }

      // write SDU into upper end
      rlc_sdu sdu = {0, sdu_bufs[i].deep_copy()}; // no std::move - keep local copy for later comparison
      rlc1_tx_upper->handle_sdu(std::move(sdu));
    }
    buffer_state = rlc1_tx_lower->get_buffer_state();
    EXPECT_EQ(num_sdus * (sdu_size + 1), buffer_state);

    // Read PDUs from RLC1 with grant of 25 Bytes each
    const uint32_t          payload_len  = 25;
    const uint32_t          max_num_pdus = 10;
    uint32_t                num_pdus     = 0;
    byte_buffer_slice_chain pdu_bufs[max_num_pdus];

    buffer_state = rlc1_tx_lower->get_buffer_state();
    while (buffer_state > 0 && num_pdus < max_num_pdus) {
      pdu_bufs[num_pdus] = rlc1_tx_lower->pull_pdu(payload_len);

      if (pdu_bufs[num_pdus].empty()) {
        break;
      }
      // TODO: write PCAP
      num_pdus++;
      buffer_state = rlc1_tx_lower->get_buffer_state();
    }
    EXPECT_EQ(0, buffer_state);

    // Write all PDUs twice into RLC2 (except for hold_back_pdu)
    for (uint32_t k = 0; k < 2; k++) {
      for (uint32_t i = 0; i < num_pdus; i++) {
        if (i != hold_back_pdu) {
          byte_buffer pdu;
          for (const byte_buffer_slice& slice : pdu_bufs[i].slices()) {
            pdu.append(slice);
          }
          rlc2_rx_lower->handle_pdu(std::move(pdu));
        }
      }
    }
    buffer_state = rlc2_tx_lower->get_buffer_state();
    EXPECT_EQ(0, buffer_state);

    // Write the skipped PDU into RLC2
    {
      byte_buffer pdu;
      for (const byte_buffer_slice& slice : pdu_bufs[hold_back_pdu].slices()) {
        pdu.append(slice);
      }
      rlc2_rx_lower->handle_pdu(std::move(pdu));
    }

    // Read SDUs from RLC2's upper layer
    EXPECT_EQ(num_sdus, tester2.sdu_counter);
    for (uint32_t i = 0; i < num_sdus; i++) {
      EXPECT_TRUE(tester2.sdu_queue.empty() == false);
      byte_buffer_slice_chain& rx_sdu = tester2.sdu_queue.front();
      EXPECT_EQ(sdu_size, rx_sdu.length());
      EXPECT_TRUE(sdu_bufs[i] == rx_sdu);
      tester2.sdu_queue.pop();
    }
  }

  srslog::basic_logger&              logger = srslog::fetch_basic_logger("TEST", false);
  rlc_um_sn_size                     sn_size;
  rlc_um_config                      config;
  timer_manager                      timers;
  rlc_test_frame                     tester1, tester2;
  std::unique_ptr<rlc_um_entity>     rlc1, rlc2;
  rlc_rx_lower_layer_interface*      rlc1_rx_lower = nullptr;
  rlc_tx_upper_layer_data_interface* rlc1_tx_upper = nullptr;
  rlc_tx_lower_layer_interface*      rlc1_tx_lower = nullptr;
  rlc_rx_lower_layer_interface*      rlc2_rx_lower = nullptr;
  rlc_tx_upper_layer_data_interface* rlc2_tx_upper = nullptr;
  rlc_tx_lower_layer_interface*      rlc2_tx_lower = nullptr;
};

TEST_P(rlc_um_test, create_new_entity)
{
  init(GetParam());
  EXPECT_NE(rlc1_rx_lower, nullptr);
  EXPECT_NE(rlc1_tx_upper, nullptr);
  ASSERT_NE(rlc1_tx_lower, nullptr);
  EXPECT_NE(rlc2_rx_lower, nullptr);
  EXPECT_NE(rlc2_tx_upper, nullptr);
  ASSERT_NE(rlc2_tx_lower, nullptr);

  EXPECT_EQ(rlc1_tx_lower->get_buffer_state(), 0);
  EXPECT_EQ(tester1.bsr, 0);
  EXPECT_EQ(tester1.bsr_count, 0);
  EXPECT_EQ(rlc2_tx_lower->get_buffer_state(), 0);
  EXPECT_EQ(tester2.bsr, 0);
  EXPECT_EQ(tester2.bsr_count, 0);
}

TEST_P(rlc_um_test, tx_without_segmentation)
{
  init(GetParam());

  const uint32_t num_sdus = 5;
  const uint32_t num_pdus = 5;
  const uint32_t sdu_size = 1;

  // Push SDUs into RLC1
  byte_buffer sdu_bufs[num_sdus];
  for (uint32_t i = 0; i < num_sdus; i++) {
    sdu_bufs[i] = byte_buffer();
    for (uint32_t k = 0; k < sdu_size; ++k) {
      sdu_bufs[i].append(i + k);
    }

    // write SDU into upper end
    rlc_sdu sdu = {i + 13, sdu_bufs[i].deep_copy()}; // no std::move - keep local copy for later comparison
    rlc1_tx_upper->handle_sdu(std::move(sdu));
  }
  EXPECT_EQ(rlc1_tx_lower->get_buffer_state(), num_sdus * (sdu_size + 1));
  EXPECT_EQ(tester1.bsr, num_sdus * (sdu_size + 1));
  EXPECT_EQ(tester1.bsr_count, num_sdus);

  // Read PDUs from RLC1
  byte_buffer_slice_chain pdu_bufs[num_pdus];
  const int               payload_len = 1 + sdu_size; // 1 bytes for header + payload
  for (uint32_t i = 0; i < num_pdus; i++) {
    pdu_bufs[i] = rlc1_tx_lower->pull_pdu(payload_len);
    EXPECT_EQ(payload_len, pdu_bufs[i].length());
    EXPECT_EQ(i, pdu_bufs[i][payload_len - 1]); // check if last payload item corresponds with index

    // Verify transmit notification
    EXPECT_EQ(1, tester1.transmitted_pdcp_count_list.size());
    EXPECT_EQ(i + 13, tester1.transmitted_pdcp_count_list.front());
    tester1.transmitted_pdcp_count_list.pop_front();

    // TODO: write PCAP
  }
  EXPECT_EQ(rlc1_tx_lower->get_buffer_state(), 0);
  EXPECT_EQ(tester1.bsr, 0);
  EXPECT_EQ(tester1.bsr_count, num_sdus + num_pdus);

  // Write PDUs into RLC2
  for (uint32_t i = 0; i < num_pdus; i++) {
    byte_buffer pdu;
    for (const byte_buffer_slice& slice : pdu_bufs[i].slices()) {
      pdu.append(slice);
    }
    rlc2_rx_lower->handle_pdu(std::move(pdu));
  }
  EXPECT_EQ(rlc2_tx_lower->get_buffer_state(), 0);
  EXPECT_EQ(tester2.bsr, 0);
  EXPECT_EQ(tester2.bsr_count, 0);

  // Read SDUs from RLC2's upper layer
  EXPECT_EQ(num_sdus, tester2.sdu_counter);
  for (uint32_t i = 0; i < num_sdus; i++) {
    EXPECT_TRUE(tester2.sdu_queue.empty() == false);
    byte_buffer_slice_chain& rx_sdu = tester2.sdu_queue.front();
    EXPECT_EQ(sdu_size, rx_sdu.length());
    EXPECT_TRUE(sdu_bufs[i] == rx_sdu);
    tester2.sdu_queue.pop();
  }
}

TEST_P(rlc_um_test, tx_with_segmentation)
{
  init(GetParam());

  const uint32_t num_sdus = 1;
  const uint32_t sdu_size = 100;

  // Push SDUs into RLC1
  byte_buffer sdu_bufs[num_sdus];
  for (uint32_t i = 0; i < num_sdus; i++) {
    sdu_bufs[i] = byte_buffer();
    // Write the index into the buffer
    for (uint32_t k = 0; k < sdu_size; ++k) {
      sdu_bufs[i].append(i + k);
    }

    // write SDU into upper end
    rlc_sdu sdu = {i, sdu_bufs[i].deep_copy()}; // no std::move - keep local copy for later comparison
    rlc1_tx_upper->handle_sdu(std::move(sdu));
  }
  EXPECT_EQ(rlc1_tx_lower->get_buffer_state(), num_sdus * (sdu_size + 1));
  EXPECT_EQ(tester1.bsr, num_sdus * (sdu_size + 1));
  EXPECT_EQ(tester1.bsr_count, num_sdus);

  // Read PDUs from RLC1 with grant of 25 Bytes each
  const uint32_t          payload_len  = 25;
  const uint32_t          max_num_pdus = 10;
  uint32_t                num_pdus     = 0;
  byte_buffer_slice_chain pdu_bufs[max_num_pdus];

  while (rlc1_tx_lower->get_buffer_state() > 0 && num_pdus < max_num_pdus) {
    pdu_bufs[num_pdus] = rlc1_tx_lower->pull_pdu(payload_len);

    if (num_pdus % ((sdu_size / payload_len) + 1) == 0) {
      // Verify transmit notification
      EXPECT_EQ(1, tester1.transmitted_pdcp_count_list.size());
      EXPECT_EQ(num_pdus, tester1.transmitted_pdcp_count_list.front());
      tester1.transmitted_pdcp_count_list.pop_front();
    }

    if (pdu_bufs[num_pdus].empty()) {
      break;
    }
    // TODO: write PCAP
    num_pdus++;
  }
  EXPECT_EQ(rlc1_tx_lower->get_buffer_state(), 0);
  EXPECT_EQ(tester1.bsr, 0);
  EXPECT_EQ(tester1.bsr_count, num_sdus + num_pdus);

  // Verify there are no multiple transmit notifications
  EXPECT_EQ(0, tester1.transmitted_pdcp_count_list.size());

  // Write PDUs into RLC2
  // receive PDUs in order
  for (uint32_t i = 0; i < num_pdus; i++) {
    byte_buffer pdu;
    for (const byte_buffer_slice& slice : pdu_bufs[i].slices()) {
      pdu.append(slice);
    }
    rlc2_rx_lower->handle_pdu(std::move(pdu));
  }
  EXPECT_EQ(rlc2_tx_lower->get_buffer_state(), 0);
  EXPECT_EQ(tester2.bsr, 0);
  EXPECT_EQ(tester2.bsr_count, 0);

  // Read SDUs from RLC2's upper layer
  EXPECT_EQ(num_sdus, tester2.sdu_counter);
  for (uint32_t i = 0; i < num_sdus; i++) {
    EXPECT_TRUE(tester2.sdu_queue.empty() == false);
    byte_buffer_slice_chain& rx_sdu = tester2.sdu_queue.front();
    EXPECT_EQ(sdu_size, rx_sdu.length());
    EXPECT_TRUE(sdu_bufs[i] == rx_sdu);
    tester2.sdu_queue.pop();
  }
}

TEST_P(rlc_um_test, tx_with_segmentation_reverse_rx)
{
  init(GetParam());

  const uint32_t num_sdus = 1;
  const uint32_t sdu_size = 100;

  // Push SDUs into RLC1
  byte_buffer sdu_bufs[num_sdus];
  for (uint32_t i = 0; i < num_sdus; i++) {
    sdu_bufs[i] = byte_buffer();
    // Write the index into the buffer
    for (uint32_t k = 0; k < sdu_size; ++k) {
      sdu_bufs[i].append(i + k);
    }

    // write SDU into upper end
    rlc_sdu sdu = {i, sdu_bufs[i].deep_copy()}; // no std::move - keep local copy for later comparison
    rlc1_tx_upper->handle_sdu(std::move(sdu));
  }
  EXPECT_EQ(rlc1_tx_lower->get_buffer_state(), num_sdus * (sdu_size + 1));
  EXPECT_EQ(tester1.bsr, num_sdus * (sdu_size + 1));
  EXPECT_EQ(tester1.bsr_count, num_sdus);

  // Read PDUs from RLC1 with grant of 25 Bytes each
  const uint32_t          payload_len  = 25;
  const uint32_t          max_num_pdus = 10;
  uint32_t                num_pdus     = 0;
  byte_buffer_slice_chain pdu_bufs[max_num_pdus];

  while (rlc1_tx_lower->get_buffer_state() > 0 && num_pdus < max_num_pdus) {
    pdu_bufs[num_pdus] = rlc1_tx_lower->pull_pdu(payload_len);

    if (num_pdus % ((sdu_size / payload_len) + 1) == 0) {
      // Verify transmit notification
      EXPECT_EQ(1, tester1.transmitted_pdcp_count_list.size());
      EXPECT_EQ(num_pdus, tester1.transmitted_pdcp_count_list.front());
      tester1.transmitted_pdcp_count_list.pop_front();
    }

    if (pdu_bufs[num_pdus].empty()) {
      break;
    }
    // TODO: write PCAP
    num_pdus++;
  }
  EXPECT_EQ(rlc1_tx_lower->get_buffer_state(), 0);
  EXPECT_EQ(tester1.bsr, 0);
  EXPECT_EQ(tester1.bsr_count, num_sdus + num_pdus);

  // Verify there are no multiple transmit notifications
  EXPECT_EQ(0, tester1.transmitted_pdcp_count_list.size());

  // Write PDUs into RLC2
  // receive PDUs in reverse order
  for (uint32_t i = 0; i < num_pdus; i++) {
    byte_buffer pdu;
    for (const byte_buffer_slice& slice : pdu_bufs[num_pdus - i - 1].slices()) {
      pdu.append(slice);
    }
    rlc2_rx_lower->handle_pdu(std::move(pdu));
  }
  EXPECT_EQ(rlc2_tx_lower->get_buffer_state(), 0);
  EXPECT_EQ(tester2.bsr, 0);
  EXPECT_EQ(tester2.bsr_count, 0);

  // Read SDUs from RLC2's upper layer
  ASSERT_EQ(num_sdus, tester2.sdu_counter);
  for (uint32_t i = 0; i < num_sdus; i++) {
    EXPECT_TRUE(tester2.sdu_queue.empty() == false);
    byte_buffer_slice_chain& rx_sdu = tester2.sdu_queue.front();
    EXPECT_EQ(sdu_size, rx_sdu.length());
    EXPECT_TRUE(sdu_bufs[i] == rx_sdu);
    tester2.sdu_queue.pop();
  }
}

TEST_P(rlc_um_test, tx_multiple_SDUs_with_segmentation)
{
  init(GetParam());

  const uint32_t num_sdus = 2;
  const uint32_t sdu_size = 100;

  // Push SDUs into RLC1
  byte_buffer sdu_bufs[num_sdus];
  for (uint32_t i = 0; i < num_sdus; i++) {
    sdu_bufs[i] = byte_buffer();
    // Write the index into the buffer
    for (uint32_t k = 0; k < sdu_size; ++k) {
      sdu_bufs[i].append(i + k);
    }

    // write SDU into upper end
    rlc_sdu sdu = {0, sdu_bufs[i].deep_copy()}; // no std::move - keep local copy for later comparison
    rlc1_tx_upper->handle_sdu(std::move(sdu));
  }
  EXPECT_EQ(rlc1_tx_lower->get_buffer_state(), num_sdus * (sdu_size + 1));
  EXPECT_EQ(tester1.bsr, num_sdus * (sdu_size + 1));
  EXPECT_EQ(tester1.bsr_count, num_sdus);

  // Read PDUs from RLC1 with grant of 25 Bytes each
  const uint32_t          payload_len  = 25;
  const uint32_t          max_num_pdus = 20;
  uint32_t                num_pdus     = 0;
  byte_buffer_slice_chain pdu_bufs[max_num_pdus];

  while (rlc1_tx_lower->get_buffer_state() > 0 && num_pdus < max_num_pdus) {
    pdu_bufs[num_pdus] = rlc1_tx_lower->pull_pdu(payload_len);

    if (pdu_bufs[num_pdus].empty()) {
      break;
    }
    // TODO: write PCAP
    num_pdus++;
  }
  EXPECT_EQ(rlc1_tx_lower->get_buffer_state(), 0);
  EXPECT_EQ(tester1.bsr, 0);
  EXPECT_EQ(tester1.bsr_count, num_sdus + num_pdus);

  // Write PDUs into RLC2 (except 1 and 6)
  for (uint32_t i = 0; i < num_pdus; i++) {
    if (i != 1 && i != 6) {
      byte_buffer pdu;
      for (const byte_buffer_slice& slice : pdu_bufs[i].slices()) {
        pdu.append(slice);
      }
      rlc2_rx_lower->handle_pdu(std::move(pdu));
    }
  }

  // write remaining two PDUs in reverse-order (so SN=1 is received first)
  {
    byte_buffer pdu;
    for (const byte_buffer_slice& slice : pdu_bufs[6].slices()) {
      pdu.append(slice);
    }
    rlc2_rx_lower->handle_pdu(std::move(pdu));
  }
  {
    byte_buffer pdu;
    for (const byte_buffer_slice& slice : pdu_bufs[1].slices()) {
      pdu.append(slice);
    }
    rlc2_rx_lower->handle_pdu(std::move(pdu));
  }

  EXPECT_EQ(rlc2_tx_lower->get_buffer_state(), 0);
  EXPECT_EQ(tester2.bsr, 0);
  EXPECT_EQ(tester2.bsr_count, 0);

  // Read SDUs from RLC2's upper layer
  // Upper layer receives SDUs in order of completion (i.e. here in reverse order)
  // Therefore compare with initial testvectors in reverse order
  ASSERT_EQ(num_sdus, tester2.sdu_counter);
  for (uint32_t i = 0; i < num_sdus; i++) {
    EXPECT_TRUE(tester2.sdu_queue.empty() == false);
    byte_buffer_slice_chain& rx_sdu = tester2.sdu_queue.front();
    EXPECT_EQ(sdu_size, rx_sdu.length());
    EXPECT_TRUE(sdu_bufs[num_sdus - 1 - i] == rx_sdu);
    tester2.sdu_queue.pop();
  }
}

TEST_P(rlc_um_test, tx_with_PDU_duplicates_hold_0)
{
  init(GetParam());
  tx_with_pdu_duplicates(0);
}

TEST_P(rlc_um_test, tx_with_PDU_duplicates_hold_1)
{
  init(GetParam());
  tx_with_pdu_duplicates(1);
}

TEST_P(rlc_um_test, tx_with_PDU_duplicates_hold_2)
{
  init(GetParam());
  tx_with_pdu_duplicates(2);
}

TEST_P(rlc_um_test, tx_with_PDU_duplicates_hold_3)
{
  init(GetParam());
  tx_with_pdu_duplicates(3);
}

TEST_P(rlc_um_test, tx_with_PDU_duplicates_hold_4)
{
  init(GetParam());
  tx_with_pdu_duplicates(4);
}

TEST_P(rlc_um_test, reassembly_window_wrap_around)
{
  init(GetParam());

  const uint32_t num_sdus = cardinality(to_number(sn_size)); // 2 * UM_Window_Size
  const uint32_t sdu_size = 10;

  const uint32_t          payload_len  = 8;
  const uint32_t          max_num_pdus = num_sdus * 2; // we need 2 PDUs for each SDU
  uint32_t                num_pdus     = 0;
  byte_buffer_slice_chain pdu_bufs[max_num_pdus];

  // Push SDUs into RLC1 and read PDUs from RLC1 with grant of 8 Bytes each
  byte_buffer sdu_bufs[num_sdus];
  for (uint32_t i = 0; i < num_sdus; i++) {
    sdu_bufs[i] = byte_buffer();
    // Write the index into the buffer
    for (uint32_t k = 0; k < sdu_size; ++k) {
      sdu_bufs[i].append(i + k);
    }

    // write SDU into upper end
    rlc_sdu sdu = {0, sdu_bufs[i].deep_copy()}; // no std::move - keep local copy for later comparison
    rlc1_tx_upper->handle_sdu(std::move(sdu));

    // check buffer state
    EXPECT_EQ(rlc1_tx_lower->get_buffer_state(), sdu_size + 1);
    EXPECT_EQ(tester1.bsr, sdu_size + 1);
    EXPECT_EQ(tester1.bsr_count, i + 1 + num_pdus);

    // read PDUs from lower end
    while (rlc1_tx_lower->get_buffer_state() > 0 && num_pdus < max_num_pdus) {
      pdu_bufs[num_pdus] = rlc1_tx_lower->pull_pdu(payload_len);

      if (pdu_bufs[num_pdus].empty()) {
        break;
      }
      // TODO: write PCAP
      num_pdus++;
    }
  }
  EXPECT_EQ(rlc1_tx_lower->get_buffer_state(), 0);
  EXPECT_EQ(tester1.bsr, 0);
  EXPECT_EQ(tester1.bsr_count, num_sdus + num_pdus);

  // Write PDUs into RLC2 and read SDUs from RLC2's upper layer
  uint32_t rx_sdu_idx = 0;
  for (uint32_t i = 0; i < num_pdus; i++) {
    // Prepare and write PDU
    byte_buffer pdu;
    for (const byte_buffer_slice& slice : pdu_bufs[i].slices()) {
      pdu.append(slice);
    }
    rlc2_rx_lower->handle_pdu(std::move(pdu));

    // Read SDU and check content
    while (!tester2.sdu_queue.empty() && rx_sdu_idx < num_sdus) {
      byte_buffer_slice_chain& rx_sdu = tester2.sdu_queue.front();
      EXPECT_EQ(sdu_size, rx_sdu.length());
      EXPECT_TRUE(sdu_bufs[rx_sdu_idx] == rx_sdu);
      tester2.sdu_queue.pop();
      rx_sdu_idx++;
    }
  }

  // Check number of received SDUs
  EXPECT_EQ(num_sdus, tester2.sdu_counter);

  EXPECT_EQ(rlc2_tx_lower->get_buffer_state(), 0);
  EXPECT_EQ(tester2.bsr, 0);
  EXPECT_EQ(tester2.bsr_count, 0);
}

TEST_P(rlc_um_test, lost_PDU_outside_reassembly_window)
{
  init(GetParam());

  const uint32_t num_sdus = cardinality(to_number(sn_size)); // 2 * UM_Window_Size
  const uint32_t sdu_size = 10;

  const uint32_t          payload_len  = 8;
  const uint32_t          max_num_pdus = num_sdus * 2; // we need 2 PDUs for each SDU
  uint32_t                num_pdus     = 0;
  byte_buffer_slice_chain pdu_bufs[max_num_pdus];

  // Push SDUs into RLC1 and read PDUs from RLC1 with grant of 8 Bytes each
  byte_buffer sdu_bufs[num_sdus];
  for (uint32_t i = 0; i < num_sdus; i++) {
    sdu_bufs[i] = byte_buffer();
    // Write the index into the buffer
    for (uint32_t k = 0; k < sdu_size; ++k) {
      sdu_bufs[i].append(i + k);
    }

    // write SDU into upper end
    rlc_sdu sdu = {0, sdu_bufs[i].deep_copy()}; // no std::move - keep local copy for later comparison
    rlc1_tx_upper->handle_sdu(std::move(sdu));

    // check buffer state
    EXPECT_EQ(rlc1_tx_lower->get_buffer_state(), sdu_size + 1);
    EXPECT_EQ(tester1.bsr, sdu_size + 1);
    EXPECT_EQ(tester1.bsr_count, i + 1 + num_pdus);

    // read PDUs from lower end
    while (rlc1_tx_lower->get_buffer_state() > 0 && num_pdus < max_num_pdus) {
      pdu_bufs[num_pdus] = rlc1_tx_lower->pull_pdu(payload_len);

      if (pdu_bufs[num_pdus].empty()) {
        break;
      }
      // TODO: write PCAP
      num_pdus++;
    }
  }
  EXPECT_EQ(rlc1_tx_lower->get_buffer_state(), 0);
  EXPECT_EQ(tester1.bsr, 0);
  EXPECT_EQ(tester1.bsr_count, num_sdus + num_pdus);

  // Write PDUs into RLC2 (except 11th) and read SDUs from RLC2's upper layer
  uint32_t rx_sdu_idx = 0;
  for (uint32_t i = 0; i < num_pdus; i++) {
    if (i != 10) {
      // Prepare and write PDU
      byte_buffer pdu;
      for (const byte_buffer_slice& slice : pdu_bufs[i].slices()) {
        pdu.append(slice);
      }
      rlc2_rx_lower->handle_pdu(std::move(pdu));
    } else {
      // increment rx_sdu_idx to skip content check of the SDU that will be lost
      rx_sdu_idx++;
    }

    // Read SDU and check content
    while (!tester2.sdu_queue.empty() && rx_sdu_idx < num_sdus) {
      byte_buffer_slice_chain& rx_sdu = tester2.sdu_queue.front();
      EXPECT_EQ(sdu_size, rx_sdu.length());
      EXPECT_TRUE(sdu_bufs[rx_sdu_idx] == rx_sdu);
      tester2.sdu_queue.pop();
      rx_sdu_idx++;
    }
  }

  // Check number of received SDUs
  EXPECT_EQ(num_sdus - 1, tester2.sdu_counter);

  EXPECT_EQ(rlc2_tx_lower->get_buffer_state(), 0);
  EXPECT_EQ(tester2.bsr, 0);
  EXPECT_EQ(tester2.bsr_count, 0);

  // let t-reassembly expire
  while (timers.nof_running_timers() != 0) {
    timers.tick_all();
  }

  rlc_bearer_metrics_container rlc2_metrics = rlc2->get_metrics();
  EXPECT_TRUE(rlc2_metrics.rx.num_lost_pdus == 1);
}

TEST_P(rlc_um_test, lost_segment_outside_reassembly_window)
{
  init(GetParam());

  const uint32_t num_sdus = 10;
  const uint32_t sdu_size = 10;

  // Push SDUs into RLC1
  byte_buffer sdu_bufs[num_sdus];
  for (uint32_t i = 0; i < num_sdus; i++) {
    sdu_bufs[i] = byte_buffer();
    // Write the index into the buffer
    for (uint32_t k = 0; k < sdu_size; ++k) {
      sdu_bufs[i].append(i + k);
    }

    // write SDU into upper end
    rlc_sdu sdu = {0, sdu_bufs[i].deep_copy()}; // no std::move - keep local copy for later comparison
    rlc1_tx_upper->handle_sdu(std::move(sdu));
  }
  EXPECT_EQ(rlc1_tx_lower->get_buffer_state(), num_sdus * (sdu_size + 1));
  EXPECT_EQ(tester1.bsr, num_sdus * (sdu_size + 1));
  EXPECT_EQ(tester1.bsr_count, num_sdus);

  // Read PDUs from RLC1 with grant of 8 Bytes each
  const uint32_t          payload_len  = 8;
  const uint32_t          max_num_pdus = num_sdus * 2; // we need 2 PDUs for each SDU
  uint32_t                num_pdus     = 0;
  byte_buffer_slice_chain pdu_bufs[max_num_pdus];

  while (rlc1_tx_lower->get_buffer_state() > 0 && num_pdus < max_num_pdus) {
    pdu_bufs[num_pdus] = rlc1_tx_lower->pull_pdu(payload_len);

    if (pdu_bufs[num_pdus].empty()) {
      break;
    }
    // TODO: write PCAP
    num_pdus++;
  }
  EXPECT_EQ(rlc1_tx_lower->get_buffer_state(), 0);
  EXPECT_EQ(tester1.bsr, 0);
  EXPECT_EQ(tester1.bsr_count, num_sdus + num_pdus);

  // Write PDUs into RLC2 (except 2nd)
  for (uint32_t i = 0; i < num_pdus; i++) {
    if (i != 2) {
      byte_buffer pdu;
      for (const byte_buffer_slice& slice : pdu_bufs[i].slices()) {
        pdu.append(slice);
      }
      rlc2_rx_lower->handle_pdu(std::move(pdu));
    }
  }
  EXPECT_EQ(rlc2_tx_lower->get_buffer_state(), 0);
  EXPECT_EQ(tester2.bsr, 0);
  EXPECT_EQ(tester2.bsr_count, 0);

  // let t-reassembly expire
  while (timers.nof_running_timers() != 0) {
    timers.tick_all();
  }

  // Read SDUs from RLC2's upper layer
  EXPECT_EQ(num_sdus - 1, tester2.sdu_counter);
  for (uint32_t i = 0; i < num_sdus; i++) {
    if (i != 1) {
      EXPECT_TRUE(tester2.sdu_queue.empty() == false);
      byte_buffer_slice_chain& rx_sdu = tester2.sdu_queue.front();
      EXPECT_EQ(sdu_size, rx_sdu.length());
      EXPECT_TRUE(sdu_bufs[i] == rx_sdu);
      tester2.sdu_queue.pop();
    }
  }

  rlc_bearer_metrics_container rlc2_metrics = rlc2->get_metrics();
  EXPECT_TRUE(rlc2_metrics.rx.num_lost_pdus == 1);
}

TEST_P(rlc_um_test, out_of_order_segments_across_SDUs)
{
  init(GetParam());

  const uint32_t num_sdus = 2;
  const uint32_t sdu_size = 20;

  // Push SDUs into RLC1
  byte_buffer sdu_bufs[num_sdus];
  for (uint32_t i = 0; i < num_sdus; i++) {
    sdu_bufs[i] = byte_buffer();
    // Write the index into the buffer
    for (uint32_t k = 0; k < sdu_size; ++k) {
      sdu_bufs[i].append(i + k);
    }

    // write SDU into upper end
    rlc_sdu sdu = {0, sdu_bufs[i].deep_copy()}; // no std::move - keep local copy for later comparison
    rlc1_tx_upper->handle_sdu(std::move(sdu));
  }
  EXPECT_EQ(rlc1_tx_lower->get_buffer_state(), num_sdus * (sdu_size + 1));
  EXPECT_EQ(tester1.bsr, num_sdus * (sdu_size + 1));
  EXPECT_EQ(tester1.bsr_count, num_sdus);

  // Read PDUs from RLC1 with grant smaller than SDU size
  const uint32_t          payload_len  = 10;
  const uint32_t          max_num_pdus = 10;
  uint32_t                num_pdus     = 0;
  byte_buffer_slice_chain pdu_bufs[max_num_pdus];

  while (rlc1_tx_lower->get_buffer_state() > 0 && num_pdus < max_num_pdus) {
    pdu_bufs[num_pdus] = rlc1_tx_lower->pull_pdu(payload_len);

    if (pdu_bufs[num_pdus].empty()) {
      break;
    }
    // TODO: write PCAP
    num_pdus++;
  }
  EXPECT_EQ(6, num_pdus);
  EXPECT_EQ(rlc1_tx_lower->get_buffer_state(), 0);
  EXPECT_EQ(tester1.bsr, 0);
  EXPECT_EQ(tester1.bsr_count, num_sdus + num_pdus);

  // Write all PDUs such that the middle section of SN=0 is received after the start section of SN=1
  //                    +------------------- skip 2nd PDU which is the middle section of SN=0
  //                    |      +------------ now feed the middle part of SN=0
  //                    |      |  +--------- and the rest of SN=1
  //                    V      V  V
  uint32_t order[] = {0, 2, 3, 1, 4, 5};

  // Write PDUs into RLC2 (except 2nd)
  for (uint32_t i = 0; i < num_pdus; i++) {
    byte_buffer pdu;
    for (const byte_buffer_slice& slice : pdu_bufs[order[i]].slices()) {
      pdu.append(slice);
    }
    rlc2_rx_lower->handle_pdu(std::move(pdu));
  }
  EXPECT_EQ(rlc2_tx_lower->get_buffer_state(), 0);
  EXPECT_EQ(tester2.bsr, 0);
  EXPECT_EQ(tester2.bsr_count, 0);

  // Read SDUs from RLC2's upper layer
  EXPECT_EQ(num_sdus, tester2.sdu_counter);
  for (uint32_t i = 0; i < num_sdus; i++) {
    EXPECT_TRUE(tester2.sdu_queue.empty() == false);
    byte_buffer_slice_chain& rx_sdu = tester2.sdu_queue.front();
    EXPECT_EQ(sdu_size, rx_sdu.length());
    EXPECT_TRUE(sdu_bufs[i] == rx_sdu);
    tester2.sdu_queue.pop();
  }

  rlc_bearer_metrics_container rlc2_metrics = rlc2->get_metrics();
  EXPECT_TRUE(rlc2_metrics.rx.num_lost_pdus == 0);

  EXPECT_EQ(0, timers.nof_running_timers());
}

///////////////////////////////////////////////////////////////////////////////
// Finally, instantiate all testcases for each supported SN size
///////////////////////////////////////////////////////////////////////////////

std::string test_param_info_to_string(const ::testing::TestParamInfo<rlc_um_sn_size>& info)
{
  fmt::memory_buffer buffer;
  fmt::format_to(buffer, "{}bit", to_number(info.param));
  return fmt::to_string(buffer);
}

INSTANTIATE_TEST_SUITE_P(rlc_um_test_each_sn_size,
                         rlc_um_test,
                         ::testing::Values(rlc_um_sn_size::size6bits, rlc_um_sn_size::size12bits),
                         test_param_info_to_string);
