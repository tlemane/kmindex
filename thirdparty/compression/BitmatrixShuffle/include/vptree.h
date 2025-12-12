#ifndef VPTREE_H
#define VPTREE_H

#include <algorithm> //Sort algorithms
#include <functional> //Function prototyping
#include <iostream> //Print
#include <vector>

#include <fast_median.h>
#include <rng.h>
#include <utils.h>

namespace bms 
{

    template <class T>
    struct DistanceFunctions
    {
        std::function<double(T, T)> compute;
        std::function<double(T, T)> get;
        std::function<void(T, T, double)> store;
    };

    template <class T>
    class VPTree
    {
        private:
            VPTree* left = nullptr;
            VPTree* right = nullptr;
            
            bool skip = false;
            T pivot; //Vertex (or vertex identifier) that is used to split space in two parts
            double threshold; //Median of pivot distances from other vertices
            DistanceFunctions<T>* distFunc;        
        
            void init(const std::vector<T>& vertices);
        public:
            VPTree(const std::vector<T>& vertices, DistanceFunctions<T>* distFunc);

            ~VPTree();

            static DistanceFunctions<T> bind_distance_functions(std::function<double(T, T)> computeDistFunc, std::function<double(T, T)> getDistFunc, std::function<void(T, T, double)> setDistFunc);

            void get_unvisited_nearest_neighbor(T query, const std::vector<bool>& alreadyAdded, double* tau, T* currentResult);
    };
}

#include <vptree_impl.h>

#endif