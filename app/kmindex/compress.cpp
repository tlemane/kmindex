#include <iostream>

#include "compress.hpp"
#include <kmindex/index/index.hpp>
#include <kmindex/exceptions.hpp>
#include <spdlog/spdlog.h>

#include <zstd/BlockCompressorZSTD.h>

namespace kmq {

  kmq_options_t kmq_compress_cli(parser_t parser, kmq_compress_options_t options)
  {
    auto cmd = parser->add_command("compress", "Compress index.");

    cmd->add_param("-i/--global-index", "Global index path.")
      ->meta("STR")
      ->setter(options->global_index_path);

    cmd->add_param("-n/--name", "Index name.")
       ->meta("STR")
       ->setter(options->index_name);

    cmd->add_param("-d/--delete", "Delete uncompressed index after compressing.")
       ->as_flag()
       ->setter(options->delete_old);

    cmd->add_param("-b/--block-size", "Size of uncompressed blocks, in megabytes.")
       ->meta("INT")
       ->def("8")
       ->setter(options->block_size);

    add_common_options(cmd, options);

    return options;
  }

  void main_compress(kmq_options_t opt)
  {
    Timer gtime;
    kmq_compress_options_t o = std::static_pointer_cast<struct kmq_compress_options>(opt);
    index i(o->global_index_path);

    auto sub = i.get(o->index_name);
    
    if (sub.is_compressed_index())
    {
      throw kmq_error(fmt::format("Index '{}' is already compressed.", o->index_name));
      return;
    }

    auto bits_per_block = (o->block_size * 1024 * 1024 * 8);
    auto entry_per_block = bits_per_block / sub.nb_samples();

    auto config_path = sub.get_compression_config() + ".tmp";
    {
      std::ofstream config_file(config_path, std::ios::out);
      config_file << "samples = " << sub.nb_samples() << "\n";
      config_file << "bitvectorsperblock = " << entry_per_block << "\n";
      config_file << "preset = 6" << std::endl;
    }

    spdlog::info("Index '{}' has {} partitions, {} samples.", o->index_name, sub.nb_partitions(), sub.nb_samples());
    spdlog::info("Compressing index '{}' using {}MB blocks ({} bit vectors per block).", o->index_name, o->block_size, entry_per_block);

    for (std::size_t i = 0; i < sub.nb_partitions(); ++i)
    {
      std::string part_path = sub.get_partition(i);

      std::string out = fmt::format("{}/matrices/blocks{}", sub.get_directory(), i);
      std::string out_ef = out + ".ef";
      BlockCompressorZSTD bc(out, out_ef, config_path);
      bc.compress_file(sub.get_partition(i), 49);
      bc.close();
    }

    fs::copy_file(config_path, sub.get_compression_config(), fs::copy_options::overwrite_existing);
    fs::remove(config_path);

    sub.set_compress(true);

    if (o->delete_old)
    {
      for (std::size_t i = 0; i < sub.nb_partitions(); ++i)
      {
        auto part_path = sub.get_partition(i);
        fs::remove(part_path);
      }
    }
    spdlog::info("Index '{}' compressed. ({})", o->index_name, gtime.formatted());
  }
}
