#ifndef DISTANCE_MATRIX_H
#define DISTANCE_MATRIX_H

#include <stdexcept>
#include <vector>
#include <cstdlib>
#include <utils.h>

namespace bms
{
    class DistanceMatrix
    {
        public:
            DistanceMatrix() = default;
            DistanceMatrix(std::size_t size);

            void resize(std::size_t size);
            std::size_t width() const;
            double get(std::size_t x, std::size_t y) const;
            void set(std::size_t x, std::size_t y, double distance);
            void reset();
        private:
            std::size_t index(std::size_t x, std::size_t y) const;
            std::vector<double> matrix;
            std::size_t _size;
    };
};
#endif