#ifndef BITMATRIXSHUFFLE_H
#define BITMATRIXSHUFFLE_H

#include <cstring>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <random>
#include <vector>

#include <utils.h>
#include <tsp.h>
#include <zstd/BlockCompressorZSTD.h>

#define BMS_REGRESSION_SLOPE 2.961897441
#define BMS_REGRESSION_INTERCEPT 0.816400508

namespace bms
{
    std::size_t target_block_nb_rows(const std::size_t NB_COLS, const std::size_t BLOCK_TARGET_SIZE);
    
    std::size_t target_block_size(const std::size_t NB_COLS, const std::size_t BLOCK_TARGET_SIZE);

    //Return how much the compression will be improved according to metric returned by 'compute_order_from_matrix_columns'
    
    constexpr double predict_metric_from_threshold(double threshold)
    {
        return threshold * BMS_REGRESSION_SLOPE + BMS_REGRESSION_INTERCEPT;
    }

    constexpr double predict_threshold_from_metric(double metric)
    {
        return (metric - BMS_REGRESSION_INTERCEPT) / BMS_REGRESSION_SLOPE;
    }

    //Start multiple path TSP instances to be solved using Nearest-Neighbor, need 
    double compute_order_from_matrix_columns(const std::string& MATRIX_PATH, const unsigned HEADER, const std::size_t NB_COLS, const std::size_t NB_ROWS, const std::size_t GROUPSIZE, const std::size_t SUBSAMPLED_ROWS, std::vector<std::uint64_t>& order);

    //Reorder matrix columns (bit-swapping on memory-mapped file)
    void reorder_matrix_columns(const std::string& MATRIX_PATH, const unsigned HEADER, const std::size_t NB_COLS, const std::size_t NB_ROWS, const std::vector<std::uint64_t>& ORDER, const std::size_t BLOCK_TARGET_SIZE);

    //Reorder matrix columns (bit-swapping on memory-mapped file)
    void reorder_matrix_columns_and_compress(const std::string& MATRIX_PATH, const std::string& OUTPUT_PATH, const std::string& OUTPUT_EF_PATH, const std::string& CONFIG_PATH, const unsigned HEADER, const std::size_t NB_COLS, const std::size_t NB_ROWS, const std::vector<std::uint64_t>& ORDER, const std::size_t BLOCK_TARGET_SIZE);

    //Reorder matrix rows (row-swapping on memory-mapped file)
    void reorder_matrix_rows(char* mapped_file, const unsigned HEADER, const std::size_t ROW_LENGTH, const std::vector<std::uint64_t>& ORDER);

    //Get an order that can be used to retrieve original matrix
    void reverse_order(const std::vector<std::uint64_t>& ORDER, std::vector<std::uint64_t>& reversed_order);
};   

#endif