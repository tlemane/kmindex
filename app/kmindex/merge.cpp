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

  void main_merge(kmq_options_t opt)
  {
    kmq_merge_options_t o = std::static_pointer_cast<struct kmq_merge_options>(opt);

    sanity_check(o);

    spdlog::info("Merge [{}]", fmt::join(o->to_merge, ","));

    Timer time;

    index gindex(o->global_index_path);

    std::deque<std::string> q;
    std::vector<std::string> to_merge;

    for (auto& n : o->to_merge)
    {
      if ((gindex.get(n).nb_samples() % 8) == 0)
      {
        q.push_front(n);
      }
      else
      {
        q.push_back(n);
      }
    }

    for (auto& n : q)
    {
      to_merge.push_back(n);
    }

    auto merger = make_merger(
      &gindex, to_merge, o->new_path, o->name, o->mode, o->remove, gindex.get(to_merge[0]).bw());

    ThreadPool pool(o->nb_threads);
    merger->merge(pool);
    merger->rename(o->rename);

    spdlog::info("Done. ({})", time.formatted());
  }
}

