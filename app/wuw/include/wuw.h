//
// Created by ducnd on 18/11/2021.
//

#ifndef WAKEUP_WUW_H
#define WAKEUP_WUW_H

#include <vector>

template<class FeatureType, class ResultType>
class WuW {
    virtual ResultType acceptWaveform(FeatureType&) = 0;
};

// template class WuW<float*, bool>;
// template class WuW<std::tuple<short*, size_t>, std::vector<float*>>;
// template class WuW<std::tuple<short*, size_t>, bool>;

#endif //WAKEUP_WUW_H
