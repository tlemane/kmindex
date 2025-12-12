#include <fast_median.h>

namespace bms
{
    //https://rcoh.me/posts/linear-time-median-finding/

    //Find median by sorting O(n.log(n))
    //But assumed to be O(1) for n < 5
    double nlogn_median(std::vector<double>& distances)
    {
        std::sort(distances.begin(), distances.end());
        if(distances.size() % 2 == 1)
            return distances[distances.size() / 2]; //Median

        //Average of two medians
        return 0.5 * (distances[distances.size() / 2 - 1] + distances[distances.size() / 2]); 
    }

    double quickselect_median(std::vector<double>& distances)
    {
        if(distances.size() % 2 == 1)
            return quickselect(distances, distances.size() / 2);
        
        //Average of two medians
        return 0.5 * (quickselect(distances, distances.size() / 2 - 1) + quickselect(distances, distances.size() / 2));
    }


    //Used to find median in O(n)
    double approximate_median(std::vector<double>& distances)
    {
        if(distances.size() < 5)
            return nlogn_median(distances);

        std::vector<double> medians;
        medians.reserve(distances.size() / 5);

        for(auto i = distances.begin(); i < distances.end() - 4; i += 5)
        {
            std::sort(i, i+5);
            medians.push_back(*(i+2)); //Get middle among 5 --> 2-th element
        }

        return quickselect_median(medians);
    }

    //Return the k-th largest element from vector
    double quickselect(std::vector<double>& distances, unsigned k)
    {
        if(distances.size() == 1)
            return distances[0];

        //Approximate median as pivot to prevent QuickSelect worst-case to occur (which is n^2)
        double pivot = approximate_median(distances);

        //Lows
        std::vector<double> lows;
        for(unsigned i = 0; i < distances.size(); ++i)
            if(distances[i] < pivot)
                lows.push_back(distances[i]);

        if(k < lows.size())
            return quickselect(lows, k);

        //Pivots
        std::vector<double> pivots;
        for(unsigned i = 0; i < distances.size(); ++i)
            if(distances[i] == pivot)
                pivots.push_back(distances[i]);

        if(k < lows.size() + pivots.size()) //We got lucky and guessed the median
            return pivots[0];

        //Highs
        std::vector<double> highs;
        highs.reserve(distances.size() - pivots.size() - lows.size());

        for(unsigned i = 0; i < distances.size(); ++i)
            if(distances[i] > pivot)
                highs.push_back(distances[i]);

        return quickselect(highs, k - lows.size() - pivots.size());
    }
};