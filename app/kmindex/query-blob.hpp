#ifndef QUERY_HPPA_ZDULHSN1
#define QUERY_HPPA_ZDULHSN1

#include <vector>
#include <kmindex/query/format.hpp>
#include "common.hpp"

namespace kmq {

  struct kmq_queryb_options : kmq_options
  {
    std::string index;
    std::vector<std::string> index_names;
    std::string output;
    std::string input;
    enum format format;
    std::string single;
    std::size_t z {0};
    double sk_threshold {0};
    std::size_t batch_size {0};
    bool cache {false};
    bool blob_mode {false};
    bool aggregate {false};
  };

  using kmq_queryb_options_t = std::shared_ptr<struct kmq_queryb_options>;

  kmq_options_t kmq_queryblob_cli(parser_t parser, kmq_queryb_options_t options);

  void main_queryblob(kmq_options_t opt);
}

#endif /* end of include guard: QUERY_HPP_ZDULHSN1 */
