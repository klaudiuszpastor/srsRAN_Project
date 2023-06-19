/*
 *
 * Copyright 2021-2023 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "iq_compression_bfp_neon.h"
#include "neon_helpers.h"
#include "packing_utils_neon.h"
#include "quantizer.h"

using namespace srsran;
using namespace ofh;

#define shift_right_x3vector(src, dst, exponent)                                                                       \
  (dst).val[0] = vshrq_n_s16((src).val[0], (exponent));                                                                \
  (dst).val[1] = vshrq_n_s16((src).val[1], (exponent));                                                                \
  (dst).val[2] = vshrq_n_s16((src).val[2], (exponent));

void iq_compression_bfp_neon::compress(span<srsran::ofh::compressed_prb>         output,
                                       span<const srsran::cf_t>                  input,
                                       const srsran::ofh::ru_compression_params& params)
{
  // Auxiliary arrays used for float to fixed point conversion of the input data.
  std::array<int16_t, NOF_SAMPLES_PER_PRB* MAX_NOF_PRBS> input_quantized = {};

  span<const float> float_samples_span(reinterpret_cast<const float*>(input.data()), input.size() * 2U);
  span<int16_t>     input_quantized_span(input_quantized.data(), input.size() * 2U);

  // Performs conversion of input complex float values to signed 16bit integers.
  quantize_input(input_quantized_span, float_samples_span);

  // Check whether NEON packing utils support requested bit width.
  bool iq_width_supported = neon::iq_width_packing_supported(params.data_width);

  /// Compression algorithm implemented according to Annex A.1.2 in O-RAN.WG4.CUS.
  unsigned sample_idx = 0, rb = 0;

  // One NEON register can store 8 16bit samples. A PRB is comprised of 24 16bit IQ samples, thus we need three NEON
  // registers to process one PRB.
  //
  // The loop below processes four resource blocks at a time.
  for (size_t rb_index_end = (output.size() / 4) * 4; iq_width_supported && (rb != rb_index_end); rb += 4) {
    // Load samples.
    int16x8x3_t vec_s16x3_0 = vld1q_s16_x3(&input_quantized[sample_idx]);
    int16x8x3_t vec_s16x3_1 = vld1q_s16_x3(&input_quantized[sample_idx + NOF_SAMPLES_PER_PRB]);
    int16x8x3_t vec_s16x3_2 = vld1q_s16_x3(&input_quantized[sample_idx + NOF_SAMPLES_PER_PRB * 2]);
    int16x8x3_t vec_s16x3_3 = vld1q_s16_x3(&input_quantized[sample_idx + NOF_SAMPLES_PER_PRB * 3]);

    // Determine exponents.
    uint8_t exponent_0 = neon::determine_bfp_exponent(vec_s16x3_0, params.data_width);
    uint8_t exponent_1 = neon::determine_bfp_exponent(vec_s16x3_1, params.data_width);
    uint8_t exponent_2 = neon::determine_bfp_exponent(vec_s16x3_2, params.data_width);
    uint8_t exponent_3 = neon::determine_bfp_exponent(vec_s16x3_3, params.data_width);

    // Shift original IQ samples right.
    int16x8x3_t shifted_data_0, shifted_data_1, shifted_data_2, shifted_data_3;
    shift_right_x3vector(vec_s16x3_0, shifted_data_0, exponent_0);
    shift_right_x3vector(vec_s16x3_1, shifted_data_1, exponent_1);
    shift_right_x3vector(vec_s16x3_2, shifted_data_2, exponent_2);
    shift_right_x3vector(vec_s16x3_3, shifted_data_3, exponent_3);

    // Pack compressed samples of the PRB using utility function and save the exponent.
    neon::pack_prb_big_endian(output[rb], shifted_data_0, params.data_width);
    output[rb].set_compression_param(exponent_0);
    neon::pack_prb_big_endian(output[rb + 1], shifted_data_1, params.data_width);
    output[rb + 1].set_compression_param(exponent_1);
    neon::pack_prb_big_endian(output[rb + 2], shifted_data_2, params.data_width);
    output[rb + 2].set_compression_param(exponent_2);
    neon::pack_prb_big_endian(output[rb + 3], shifted_data_3, params.data_width);
    output[rb + 3].set_compression_param(exponent_3);

    sample_idx += (4 * NOF_SAMPLES_PER_PRB);
  }
  // The loop below processes two resource blocks at a time.
  for (size_t rb_index_end = (output.size() / 2) * 2; iq_width_supported && (rb != rb_index_end); rb += 2) {
    // Load samples.
    int16x8x3_t vec_s16x3_0 = vld1q_s16_x3(&input_quantized[sample_idx]);
    int16x8x3_t vec_s16x3_1 = vld1q_s16_x3(&input_quantized[sample_idx + NOF_SAMPLES_PER_PRB]);

    // Determine exponents.
    uint8_t exponent_0 = neon::determine_bfp_exponent(vec_s16x3_0, params.data_width);
    uint8_t exponent_1 = neon::determine_bfp_exponent(vec_s16x3_1, params.data_width);

    // Shift original IQ samples right.
    int16x8x3_t shifted_data_0, shifted_data_1;
    shift_right_x3vector(vec_s16x3_0, shifted_data_0, exponent_0);
    shift_right_x3vector(vec_s16x3_1, shifted_data_1, exponent_1);

    // Pack compressed samples of the PRB using utility function.
    neon::pack_prb_big_endian(output[rb], shifted_data_0, params.data_width);
    // Save exponent.
    output[rb].set_compression_param(exponent_0);

    neon::pack_prb_big_endian(output[rb + 1], shifted_data_1, params.data_width);
    output[rb + 1].set_compression_param(exponent_1);

    sample_idx += (2 * NOF_SAMPLES_PER_PRB);
  }
  // Process the last resource block.
  if (iq_width_supported && (rb != output.size())) {
    // Load samples.
    int16x8x3_t vec_s16x3 = vld1q_s16_x3(&input_quantized[sample_idx]);

    // Determine exponent.
    uint8_t exponent = neon::determine_bfp_exponent(vec_s16x3, params.data_width);

    // Shift original IQ samples right.
    int16x8x3_t shifted_data;
    shift_right_x3vector(vec_s16x3, shifted_data, exponent);

    // Pack compressed samples of the PRB using utility function.
    neon::pack_prb_big_endian(output[rb], shifted_data, params.data_width);
    // Save exponent.
    output[rb].set_compression_param(exponent);

    sample_idx += NOF_SAMPLES_PER_PRB;
    ++rb;
  }

  // Use generic implementation if the requested compression width is not supported by neon utils.
  for (; rb != output.size(); ++rb) {
    const auto* start_it = input_quantized.begin() + sample_idx;
    compress_prb_generic(output[rb], {start_it, NOF_SAMPLES_PER_PRB}, params.data_width);
    sample_idx += NOF_SAMPLES_PER_PRB;
  }
}

void iq_compression_bfp_neon::decompress(span<srsran::cf_t>                        output,
                                         span<const srsran::ofh::compressed_prb>   input,
                                         const srsran::ofh::ru_compression_params& params)
{
  // Quantizers.
  quantizer q_in(params.data_width);
  quantizer q_out(Q_BIT_WIDTH);

  unsigned out_idx = 0;
  for (const compressed_prb& c_prb : input) {
    bool simd_support_for_iq_width = neon::iq_width_packing_supported(params.data_width);

    // Use generic implementation if NEON utils don't support requested bit width.
    if (!simd_support_for_iq_width) {
      span<cf_t> out_rb_samples = output.subspan(out_idx, NOF_SUBCARRIERS_PER_RB);
      // Decompress resource block.
      decompress_prb_generic(out_rb_samples, c_prb, q_in, params.data_width);
      out_idx += NOF_SUBCARRIERS_PER_RB;
      continue;
    }
    // Compute scaling factor.
    uint8_t exponent = c_prb.get_compression_param();
    int16_t scaler   = 1 << exponent;

    // Determine array size so that NEON store operation doesn't write the data out of array bounds.
    constexpr size_t neon_size_iqs = 8;
    constexpr size_t arr_size      = divide_ceil(NOF_SAMPLES_PER_PRB, neon_size_iqs) * neon_size_iqs;
    alignas(64) std::array<int16_t, arr_size> unpacked_iq_data = {};
    // Unpack resource block.
    neon::unpack_prb_big_endian(unpacked_iq_data, c_prb.get_packed_data(), params.data_width);

    span<cf_t>    output_span = output.subspan(out_idx, NOF_SUBCARRIERS_PER_RB);
    span<int16_t> unpacked_span(unpacked_iq_data.data(), NOF_SUBCARRIERS_PER_RB * 2);

    // Convert to complex samples.
    q_out.to_float(output_span, unpacked_span, scaler);
    out_idx += NOF_SUBCARRIERS_PER_RB;
  }
}