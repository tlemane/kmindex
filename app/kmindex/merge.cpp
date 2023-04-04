#include "merge.hpp"

#include <set>

#include <kmindex/index/index.hpp>
#include <kmindex/index/merge.hpp>
#include <kmindex/utils.hpp>
#include <kmindex/exceptions.hpp>

#include <spdlog/spdlog.h>

namespace kmq {

  kmq_options_t kmq_merge_cli(parser_t parser, kmq_merge_options_t options)
  {
    auto cmd = parser->add_command("merge", "Merge sub-indexes.");


    cmd->add_param("-i/--index", "Global index path.")
       ->meta("STR")
       ->checker(bc::check::is_dir)
       ->setter(options->global_index_path);

    cmd->add_param("-n/--new-name", "Name of the new index.")
       ->meta("STR")
       ->setter(options->name);

    cmd->add_param("-p/--new-path", "Output path.")
       ->meta("STR")
       ->setter(options->new_path);

    auto merge_setter = [options](const std::string& v) {
      options->to_merge = bc::utils::split(v, ',');
    };

    cmd->add_param("-m/--to-merge", "Sub-indexes to merge, comma separated.")
       ->meta("LIST[STR]")
       ->setter_c(merge_setter);

    cmd->add_param("-d/--delete-old", "Delete old sub-index files.")
       ->as_flag()
       ->setter(options->remove);

    const std::string rename_help =
                          "Rename sample ids.\n"
      "                      - A sub-index cannot contain samples with similar identifiers.\n"
      "                        Sub-indexes containing identical identifiers cannot be merged, the\n"
      "                        identifiers must be renamed.\n"
      "                      - Renaming can be done in three different ways:\n\n"
      "                        1. Using identifier files (one per line).\n"
      "                           For example, if you want to merge three sub-indexes:\n"
      "                             '-r f:id1.txt,id2.txt,id3.txt'\n\n"
      "                        2. Using a format string ('{}' is replaced by an integer in [0, nb_samples)).\n"
      "                             '-r \"s:id_{}\"'\n\n"
      "                        3. Manually (not recommended).\n"
      "                           Identifiers can be changed in 'kmtricks.fof' files in sub-index directories.";
    cmd->add_param("-r/--rename", rename_help)
       ->meta("STR")
       ->def("")
       ->setter(options->rename);

    add_common_options(cmd, options, true);

    return options;
  }

  bool are_mergeable(const std::vector<std::string>& names, const index& gindex)
  {
    std::set<std::string> shash;

    for (auto& n : names)
      shash.insert(gindex.get(n).sha1());

    if (shash.size() > 1)
      return false;

    return true;
  }

  std::set<std::string> id_from_fof(const std::string& path)
  {
    std::ifstream fof(path, std::ios::in);
    check_fstream_good(path, fof);

    std::set<std::string> ids;
    for (std::string line; std::getline(fof, line);)
    {
      if (bc::utils::trim(line).empty())
        continue;
      ids.insert(bc::utils::trim(bc::utils::split(line, ':')[0]));
    }

    return ids;
  }

  bool share_sample_ids(const std::vector<std::string>& paths)
  {
    std::size_t sum = 0;
    std::set<std::string> all;
    for (auto& fof_path : paths)
    {
      auto ids = id_from_fof(fof_path);
      sum += ids.size();
      all.insert(std::begin(ids), std::end(ids));
    }

    if (all.size() < sum)
      return true;

    return false;

  }

  bool share_sample_ids(const std::vector<std::string>& names, const index& gindex)
  {
    std::vector<std::string> paths; paths.reserve(names.size());

    for (auto& n : names)
      paths.push_back(fmt::format("{}/kmtricks.fof", gindex.get(n).path()));

    return share_sample_ids(paths);
  }

  std::string same_nb_samples(const std::vector<std::string>& names,
                       const std::vector<std::string>& ids,
                       const index& gindex)
  {
    for (std::size_t i = 0; i < names.size(); ++i)
    {
      auto s1 = id_from_fof(fmt::format("{}/kmtricks.fof", gindex.get(names[i]).path()));
      auto s2 = id_from_fof(ids[i]);

      if (s1 != s2)
      {
        return fmt::format(
          "{} and {} do not contain the same number of identifiers.", names[i], ids[i]);
      }
    }

    return "";
  }

  void sanity_check(kmq_merge_options_t opt)
  {
    index gindex(opt->global_index_path);

    if (!are_mergeable(opt->to_merge, gindex))
    {
      throw kmq_error(fmt::format("[{}] are not mergeable.", fmt::join(opt->to_merge, ",")));
    }

    if (share_sample_ids(opt->to_merge, gindex) && opt->rename.empty())
    {
      throw kmq_error(
        fmt::format(
          "[{}] share some sample identifiers. See --rename.", fmt::join(opt->to_merge, ",")));
    }

    if (!opt->rename.empty())
    {
      auto ren = opt->rename.substr(2);
      if (opt->rename[0] == 'f')
      {
        opt->mode = rename_mode::file;
        auto sren = bc::utils::split(ren, ',');
        if (opt->to_merge.size() != sren.size())
        {
          throw kmq_error(
              "--rename: the number of renaming files must be equal to"
              "the number of indexes to merge.");
        }
        auto err = same_nb_samples(opt->to_merge, sren, gindex);
        if (!err.empty())
        {
          throw kmq_error(
            fmt::format("--rename: {}", err));
        }
        if (share_sample_ids(sren))
        {
          throw kmq_error(
            fmt::format(
              "--rename: [{}] share some sample identifiers.",
              fmt::join(sren, ",")));
        }
        opt->rename = ren;
      }
      if (opt->rename[0] == 's' && !bc::utils::contains(ren, "{}"))
      {
        throw kmq_error(
          fmt::format("Invalid format string ({}).", ren));
      }
      else if (opt->rename[0] == 's')
      {
        opt->mode = rename_mode::format;
        opt->rename = ren;
      }
    }
  }

  //std::string new_fof(kmq_merge_options_t opt, index& gindex)
  //{
  //  std::stringstream ss;

  //  auto npaths = bc::utils::split(opt->rename, ',');

  //  std::size_t s = 0;

  //  for (std::size_t i = 0; i < opt->to_merge.size(); ++i)
  //  {
  //    switch (opt->mode)
  //    {
  //      case rename_mode::none:
  //        break;
  //      case rename_mode::file:
  //        gindex.get(opt->to_merge[i]).rename_samples(npaths[i]);
  //        break;
  //      case rename_mode::format:
  //        s = gindex.get(opt->to_merge[i]).rename_samples(opt->rename, s);
  //        break;
  //    }
  //    ss << gindex.get(opt->to_merge[i]).fof_str();
  //  }
  //  return ss.str();
  //}

  //void update_kmtricks_options(const std::string& opt, const std::string new_opt)
  //{
  //  unused(opt); unused(new_opt);
  //}

  //void copy_kmtricks_tree(const std::string& path, const std::string& new_path)
  //{
  //  fs::copy(fmt::format("{}/options.txt", path), new_path, fs::copy_options::skip_existing);
  //  fs::copy(fmt::format("{}/hash.info", path), new_path, fs::copy_options::skip_existing);
  //  fs::copy(fmt::format("{}/config_gatb", path),
  //           fmt::format("{}/config_gatb", new_path),
  //           fs::copy_options::skip_existing | fs::copy_options::recursive);
  //  fs::copy(fmt::format("{}/repartition_gatb", path),
  //           fmt::format("{}/repartition_gatb", new_path),
  //           fs::copy_options::skip_existing | fs::copy_options::recursive);
  //}

  //void copy_kmtricks_trees(const index& gindex,
  //                         const std::vector<std::string>& sub,
  //                         const std::string& new_path,
  //                         const std::string& fof)
  //{
  //  fs::create_directory(new_path);

  //  for (auto& i : sub)
  //    copy_kmtricks_tree(fmt::format("{}/{}", gindex.path(), i), new_path);

  //  fs::create_directory(fmt::format("{}/matrices", new_path));
  //  fs::create_directory(fmt::format("{}/repartition_gatb", new_path));
  //  fs::create_directory(fmt::format("{}/config_gatb", new_path));

  //  {
  //    std::ofstream out(fmt::format("{}/kmtricks.fof", new_path), std::ios::out);
  //    out << fof;
  //  }
  //}

  //void merge_partition(std::size_t p, const index& gindex, kmq_merge_options_t opt, std::size_t n)
  //{
  //  std::vector<std::pair<mio::mmap_source, std::size_t>> partitions;
  //  partitions.reserve(opt->to_merge.size());

  //  std::size_t nb_samples = 0;
  //  spdlog::info("AAAA");
  //  for (auto& i : opt->to_merge)
  //  {
  //    auto& ii = gindex.get(i);
  //    nb_samples += ii.nb_samples();

  //    spdlog::info("NB {}", ii.nb_samples());
  //    partitions.push_back(
  //      std::make_pair(
  //        mio::mmap_source(fmt::format("{}/matrices/matrix_{}.cmbf", fmt::format("{}/{}", gindex.path(), i), p), 49),
  //        ii.nb_samples()
  //      )
  //    );
  //  }

  //  spdlog::info("AAAA");
  //  std::vector<std::uint8_t> buffer((nb_samples + 7) / 8, 0);
  //  spdlog::info(buffer.size());
  //  std::ofstream out(fmt::format("{}/matrices/matrix_{}.cmbf", opt->new_path, p));

  //  spdlog::info("AAAA");
  //  spdlog::info(n);
  //  spdlog::info(buffer.size());
  //  for (std::size_t i = 0; i < n; ++i)
  //  {
  //    std::memcpy(&buffer[0],
  //                partitions[0].first.begin() + (((partitions[0].second + 7) / 8) * i),
  //                buffer.size());


  //    std::size_t current = partitions[0].second;
  //    spdlog::info("current {}, i {}", current, i);

  //    for (std::size_t j = 1; j < partitions.size(); ++j)
  //    {
  //      auto& p = partitions[j];

  //      for (std::size_t k = 0; k < p.second; ++k)
  //      {
  //        spdlog::info("k {}", k);
  //        if (BITCHECK(p.first, k + ((p.second + 7) / 8) * i * 8))
  //        {
  //          spdlog::info("TRUE");
  //          BITSET(buffer, current);
  //        }
  //        current++;
  //      }

  //      current += p.second;
  //    }
  //    out.write(reinterpret_cast<char*>(buffer.data()), buffer.size());

  //    std::fill(buffer.begin(), buffer.end(), 0);
  //  }
  //}

  //void merge_partitions(const index& gindex, kmq_merge_options_t opt)
  //{
  //  auto& ii = gindex.get(opt->to_merge[0]);

  //  std::size_t n = ii.bloom_size() / ii.nb_partitions();

  //  for (std::size_t p = 0; p < ii.nb_partitions(); ++p)
  //  {
  //    merge_partition(p, gindex, opt, n);
  //    break;
  //  }
  //}


  void main_merge(kmq_options_t opt)
  {
    kmq_merge_options_t o = std::static_pointer_cast<struct kmq_merge_options>(opt);

    sanity_check(o);

    spdlog::info("Merge [{}]", fmt::join(o->to_merge, ","));

    Timer time;

    index gindex(o->global_index_path);

    bool is_pa = gindex.get(o->to_merge[0]).bw() == 1;
    
    if (!is_pa)
    {
      throw kmq_error("Only presence/absence indexes can be merged. Support for abundance indexes is coming soon.");
    }

    auto merger = make_merger(
      &gindex, o->to_merge, o->new_path, o->name, o->mode, o->remove, is_pa);

    ThreadPool pool(o->nb_threads);
    merger->merge(pool);
    merger->rename(o->rename);

    spdlog::info("Done. ({})", time.formatted());
  }
}

