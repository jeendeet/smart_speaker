//
// Created by ducnd on 03/05/2021.
//

#include "../common/defs.h"
#ifdef USE_KALDI

#ifndef VOSK_ANDROID_DEMO_ONLINE_ENDPOINT_BDI_H
#define VOSK_ANDROID_DEMO_ONLINE_ENDPOINT_BDI_H

#include "online2/online-endpoint.h"
#include "hmm/transition-model.h"

using namespace kaldi;

template <typename DEC>
int32 TrailingSilenceLengthBdi(const TransitionModel &tmodel,
                            const std::string &silence_phones,
                            const DEC &decoder);

/// This is a higher-level convenience function that works out the
/// arguments to the EndpointDetected function above, from the decoder.
template <typename DEC>
bool EndpointDetectedBdi(
        const OnlineEndpointConfig &config,
        const TransitionModel &tmodel,
        BaseFloat frame_shift_in_seconds,
        const DEC &decoder);

#endif //VOSK_ANDROID_DEMO_ONLINE_ENDPOINT_BDI_H

#endif
