/*
 *  Copyright (c) 2014 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_AUDIO_PROCESSING_SPLITTING_FILTER_H_
#define MODULES_AUDIO_PROCESSING_SPLITTING_FILTER_H_

#include <cstring>
#include <memory>
#include <vector>

#include "channel_buffer.h"
#include "three_band_filter_bank.h"

#ifdef __cplusplus
extern "C" {
#endif

void WebRtcSpl_AnalysisQMF(const int16_t *in_data,
                           size_t in_data_length,
                           int16_t *low_band,
                           int16_t *high_band,
                           int32_t *filter_state1,
                           int32_t *filter_state2);
void WebRtcSpl_SynthesisQMF(const int16_t *low_band,
                            const int16_t *high_band,
                            size_t band_length,
                            int16_t *out_data,
                            int32_t *filter_state1,
                            int32_t *filter_state2);
#ifdef __cplusplus
}
#endif  // __cplusplus
namespace webrtc {

    struct TwoBandsStates {
        TwoBandsStates() {
            memset(analysis_state1, 0, sizeof(analysis_state1));
            memset(analysis_state2, 0, sizeof(analysis_state2));
            memset(synthesis_state1, 0, sizeof(synthesis_state1));
            memset(synthesis_state2, 0, sizeof(synthesis_state2));
        }

        static const int kStateSize = 6;
        int analysis_state1[kStateSize]{};
        int analysis_state2[kStateSize]{};
        int synthesis_state1[kStateSize]{};
        int synthesis_state2[kStateSize]{};
    };

// Splitting filter which is able to split into and merge from 2 or 3 frequency
// bands. The number of channels needs to be provided at construction time.
//
// For each block, Analysis() is called to split into bands and then Synthesis()
// to merge these bands again. The input and output signals are contained in
// ChannelBuffers and for the different bands an array of ChannelBuffers is
// used.
    class SplittingFilter {
    public:
        SplittingFilter(size_t num_channels, size_t num_bands, size_t num_frames);

        ~SplittingFilter();

        void Analysis(const ChannelBuffer<float> *data, ChannelBuffer<float> *bands);

        void Synthesis(const ChannelBuffer<float> *bands, ChannelBuffer<float> *data);

    private:
        // Two-band analysis and synthesis work for 640 samples or less.
        void TwoBandsAnalysis(const ChannelBuffer<float> *data,
                              ChannelBuffer<float> *bands);

        void TwoBandsSynthesis(const ChannelBuffer<float> *bands,
                               ChannelBuffer<float> *data);

        void ThreeBandsAnalysis(const ChannelBuffer<float> *data,
                                ChannelBuffer<float> *bands);

        void ThreeBandsSynthesis(const ChannelBuffer<float> *bands,
                                 ChannelBuffer<float> *data);

        const size_t num_bands_;
        std::vector<TwoBandsStates> two_bands_states_;
        std::vector<ThreeBandFilterBank> three_band_filter_banks_;
    };

}  // namespace webrtc

#endif  // MODULES_AUDIO_PROCESSING_SPLITTING_FILTER_H_
