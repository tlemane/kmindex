#ifndef FAST_MEDIAN_H
#define FAST_MEDIAN_H

#include <vector>
#include <algorithm>

namespace bms
{
    //O(n.log(n)) exact: sort & take middle value
    double nlogn_median(std::vector<double>& distances);

    //O(n) approximate: quickselect using median of medians
    double quickselect(std::vector<double>& distances, unsigned k);
    double quickselect_median(std::vector<double>& distances);
    double approximate_median(std::vector<double>& distances);
};
#endif