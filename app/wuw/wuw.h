//
// Created by ducnd on 18/11/2021.
//

#ifndef WAKEUP_WUW_H
#define WAKEUP_WUW_H

template<class FeatureType, class ResultType>
class WuW {
    virtual ResultType acceptWaveform(FeatureType&) = 0;
};

#endif //WAKEUP_WUW_H
