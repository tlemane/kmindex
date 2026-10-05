#!/usr/bin/env bash

kmindex_bin=$1
directory=$2

cd ${directory}

fail=0

# Test 1: --merge matrix output equals the concatenation of the per-index
# files, in sub-index order (lexicographic for --names all: abs, pa).
rm -rf out_sep out_merge

${kmindex_bin} query2 -i indexes/abs_global,indexes/pa_global \
                     -q datasets/abs_dataset/1.fasta \
                     -z 4 \
                     -f matrix \
                     -o out_sep -t 2 2> /dev/null

${kmindex_bin} query2 -i indexes/abs_global,indexes/pa_global \
                     -q datasets/abs_dataset/1.fasta \
                     -z 4 \
                     -f matrix \
                     -o out_merge --merge -t 2 2> /dev/null

cat out_sep/abs.tsv out_sep/pa.tsv > expected_concat.tsv
diff out_merge/merged.tsv expected_concat.tsv || { echo "FAIL: merged.tsv != concat of per-index files"; fail=1; }

# --merge must not also write the per-index files
[ -f out_merge/abs.tsv ] && { echo "FAIL: per-index file written with --merge"; fail=1; }
[ -f out_merge/pa.tsv ] && { echo "FAIL: per-index file written with --merge"; fail=1; }

# staging directory must be cleaned up on success
[ -d out_merge/.staging ] && { echo "FAIL: .staging not removed"; fail=1; }

# Test 2: --merge json output equals the checked-in fixture (key-union document)
rm -rf out_merge

${kmindex_bin} query2 -i indexes/abs_global,indexes/pa_global \
                     -q datasets/abs_dataset/1.fasta \
                     -z 4 \
                     -f json \
                     -o out_merge --merge -t 2 2> /dev/null

diff out_merge/merged.json outputs/q1_z4_merged.json || { echo "FAIL: merged.json differs from fixture"; fail=1; }

# Test 3: merged output is identical for a registry path and comma-separated
# individual index paths covering the same sub-indexes.
rm -rf out_merge out_merge_multi

${kmindex_bin} query2 -i indexes/index \
                     -q datasets/abs_dataset/1.fasta \
                     -z 4 \
                     -f json \
                     -o out_merge --merge -t 2 2> /dev/null

${kmindex_bin} query2 -i indexes/abs_global,indexes/pa_global \
                     -q datasets/abs_dataset/1.fasta \
                     -z 4 \
                     -f json \
                     -o out_merge_multi --merge -t 2 2> /dev/null

diff out_merge/merged.json out_merge_multi/merged.json || { echo "FAIL: registry vs multi-path merged.json differ"; fail=1; }

# Test 4: --merge jsonl output equals concatenated per-index files.
rm -rf out_sep out_merge

${kmindex_bin} query2 -i indexes/index \
                     -q datasets/abs_dataset/1.fasta \
                     -z 4 \
                     -f jsonl \
                     -o out_sep -t 2 2> /dev/null

${kmindex_bin} query2 -i indexes/index \
                     -q datasets/abs_dataset/1.fasta \
                     -z 4 \
                     -f jsonl \
                     -o out_merge --merge -t 2 2> /dev/null

cat out_sep/abs.jsonl out_sep/pa.jsonl > expected_concat.jsonl
diff out_merge/merged.jsonl expected_concat.jsonl || { echo "FAIL: merged.jsonl != concat of per-index files"; fail=1; }

# Test 5: --merge jsonl_vec (positions) — same concatenation property.
rm -rf out_sep out_merge

${kmindex_bin} query2 -i indexes/index \
                     -q datasets/abs_dataset/1.fasta \
                     -z 4 \
                     -f jsonl_vec \
                     -o out_sep -t 2 2> /dev/null

${kmindex_bin} query2 -i indexes/index \
                     -q datasets/abs_dataset/1.fasta \
                     -z 4 \
                     -f jsonl_vec \
                     -o out_merge --merge -t 2 2> /dev/null

cat out_sep/abs.jsonl out_sep/pa.jsonl > expected_concat.jsonl
diff out_merge/merged.jsonl expected_concat.jsonl || { echo "FAIL: merged.jsonl (jsonl_vec) != concat"; fail=1; }

# Test 6: --merge json_vec must equal the union of the per-index documents,
# including position arrays. Per-index files are {"name": <value>}; merged is
# {"abs":<value>,"pa":<value>} with identical values printed at a shallower
# indent. Minifying all whitespace makes the documents byte-comparable, so the
# expected merged text is "<" stripped of the outer braces joined by commas —
# no JSON parser needed. (Test data names contain no whitespace, so minifying
# cannot corrupt string values.)
rm -rf out_sep out_merge

${kmindex_bin} query2 -i indexes/index \
                     -q datasets/abs_dataset/1.fasta \
                     -z 4 \
                     -f json_vec \
                     -o out_sep -t 2 2> /dev/null

${kmindex_bin} query2 -i indexes/index \
                     -q datasets/abs_dataset/1.fasta \
                     -z 4 \
                     -f json_vec \
                     -o out_merge --merge -t 2 2> /dev/null

abs_min=$(tr -d '[:space:]' < out_sep/abs.json)
pa_min=$(tr -d '[:space:]' < out_sep/pa.json)
abs_inner=${abs_min#\{}; abs_inner=${abs_inner%\}}
pa_inner=${pa_min#\{}; pa_inner=${pa_inner%\}}
merged_min=$(tr -d '[:space:]' < out_merge/merged.json)
expected_min="{${abs_inner},${pa_inner}}"
[ "$merged_min" = "$expected_min" ] || { echo "FAIL: merged.json (json_vec) differs from per-index union"; fail=1; }
grep -q '"P"' out_merge/merged.json || { echo "FAIL: merged.json (json_vec) missing position data"; fail=1; }

# Test 7: --names subset restricts the merged output.
rm -rf out_merge

${kmindex_bin} query2 -i indexes/index \
                     -n pa \
                     -q datasets/pa_dataset/1.fasta \
                     -z 5 \
                     -f matrix \
                     -o out_merge --merge -t 2 2> /dev/null

diff out_merge/merged.tsv outputs/q1_z5_pa.tsv || { echo "FAIL: merged.tsv with -n pa differs"; fail=1; }

# Test 8: duplicate names in --names are queried/emitted once.
rm -rf out_merge out_merge_dup

${kmindex_bin} query2 -i indexes/index -n abs \
                     -q datasets/abs_dataset/1.fasta \
                     -z 4 -f json -o out_merge --merge -t 1 2> /dev/null

${kmindex_bin} query2 -i indexes/index -n abs,abs \
                     -q datasets/abs_dataset/1.fasta \
                     -z 4 -f json -o out_merge_dup --merge -t 1 2> /dev/null

diff out_merge/merged.json out_merge_dup/merged.json || { echo "FAIL: duplicated --names changed merged.json"; fail=1; }

# Test 9: merged output is deterministic across runs.
rm -rf out_merge out_merge_b

${kmindex_bin} query2 -i indexes/index \
                     -q datasets/abs_dataset/1.fasta \
                     -z 4 -f json -o out_merge --merge -t 2 2> /dev/null

${kmindex_bin} query2 -i indexes/index \
                     -q datasets/abs_dataset/1.fasta \
                     -z 4 -f json -o out_merge_b --merge -t 2 2> /dev/null

diff out_merge/merged.json out_merge_b/merged.json || { echo "FAIL: merged.json not deterministic"; fail=1; }

# Test 10: all queries too short -> every sub-index still appears with an
# empty object, matching the per-index output ({"<name>":{}} per file).
rm -rf out_merge out_sep
printf ">short\nACGT\n" > short_query.fa

${kmindex_bin} query2 -i indexes/index \
                     -q short_query.fa \
                     -z 4 -f json -o out_merge --merge -t 2 2> /dev/null

printf '{"abs":{}\n,"pa":{}\n}\n' > expected_empty.json
diff out_merge/merged.json expected_empty.json || { echo "FAIL: merged.json for empty results"; fail=1; }

${kmindex_bin} query2 -i indexes/index \
                     -q short_query.fa \
                     -z 4 -f json -o out_sep -t 2 2> /dev/null

# per-index json output must be valid (non-empty) json too
printf '{\n    "abs": {}\n}' > expected_abs_empty.json
diff out_sep/abs.json expected_abs_empty.json || { echo "FAIL: empty per-index abs.json is not {\"abs\":{}}"; fail=1; }

# Test 11: a sub-index that fails in a worker aborts the command with a
# non-zero exit and no merged.* file.
rm -rf broken_global out_fail
mkdir -p broken_global/pa
cp indexes/pa_global/index.json broken_global/index.json
cp -r indexes/pa_index/* broken_global/pa/
rm -rf broken_global/pa/matrices

${kmindex_bin} query2 -i indexes/abs_global,broken_global \
                     -q datasets/abs_dataset/1.fasta \
                     -z 4 -f matrix \
                     -o out_fail --merge -t 2 2> /dev/null

if [ $? -eq 0 ]; then
  echo "FAIL: expected non-zero exit on worker failure"; fail=1
fi
[ -f out_fail/merged.tsv ] && { echo "FAIL: merged.tsv left behind after worker failure"; fail=1; }

# Test 12: same failure without --merge must also exit non-zero.
rm -rf out_fail2

${kmindex_bin} query2 -i indexes/abs_global,broken_global \
                     -q datasets/abs_dataset/1.fasta \
                     -z 4 -f matrix \
                     -o out_fail2 -t 2 2> /dev/null

if [ $? -eq 0 ]; then
  echo "FAIL: expected non-zero exit on worker failure (per-index mode)"; fail=1
fi

# Test 13: --threads 0 is rejected by the CLI checker instead of hanging on
# an empty worker pool. (The ctest TIMEOUT property bounds this test in case
# the rejection regresses to a hang.)
rm -rf out_t0
# parse-time BCliError goes to stdout: the stderr logger is only installed
# after cli.parse() succeeds.
${kmindex_bin} query2 -i indexes/index \
                     -q datasets/abs_dataset/1.fasta \
                     -z 4 -f matrix -o out_t0 --merge -t 0 > t0_err.log 2>/dev/null
grep -q "Not in range" t0_err.log || { echo "FAIL: --threads 0 not rejected by CLI checker"; fail=1; }
[ -d out_t0 ] && { echo "FAIL: --threads 0 produced output"; fail=1; }

# Test 14: a sub-index name containing a path traversal is rejected before
# any output file is written.
# malicious_global registers "../evil_index", which would make staged output
# escape .staging into the output directory.
rm -rf malicious_global evil_index out_evil
mkdir -p malicious_global
sed 's/"pa"/"..\/evil_index"/' indexes/pa_global/index.json > malicious_global/index.json
cp -r indexes/pa_index evil_index

${kmindex_bin} query2 -i malicious_global \
                     -q datasets/abs_dataset/1.fasta \
                     -z 4 -f matrix -o out_evil --merge -t 1 2> evil_err.log
if [ $? -eq 0 ]; then
  echo "FAIL: traversal sub-index name was accepted"; fail=1
fi
grep -q "Invalid sub-index name" evil_err.log || { echo "FAIL: expected 'Invalid sub-index name' error"; fail=1; }
[ -e out_evil/evil_index.tsv ] && { echo "FAIL: staged file escaped .staging"; fail=1; }
[ -f out_evil/merged.tsv ] && { echo "FAIL: merged.tsv produced from traversal registry"; fail=1; }

# Test 15: explicit --names order is preserved in merged output (pa first,
# even though abs sorts first lexicographically).
rm -rf out_merge

${kmindex_bin} query2 -i indexes/index -n pa,abs \
                     -q datasets/abs_dataset/1.fasta \
                     -z 4 -f json -o out_merge --merge -t 2 2> /dev/null

# Compare byte offsets of the top-level keys rather than line layout so the
# check is independent of the exact JSON pretty-printing.
pa_pos=$(grep -bo '"pa":{' out_merge/merged.json | head -1 | cut -d: -f1)
abs_pos=$(grep -bo '"abs":{' out_merge/merged.json | head -1 | cut -d: -f1)
{ [ -n "$pa_pos" ] && [ -n "$abs_pos" ] && [ "$pa_pos" -lt "$abs_pos" ]; } || { echo "FAIL: --names order not preserved in merged.json"; fail=1; }

# Test 16: @names file. A valid file behaves exactly like the equivalent
# comma-separated list; a missing or empty file must fail rather than
# silently querying nothing and emitting an empty result.
rm -rf out_merge_b out_names
printf 'pa\nabs\n' > names_file.txt

${kmindex_bin} query2 -i indexes/index -n @names_file.txt \
                     -q datasets/abs_dataset/1.fasta \
                     -z 4 -f json -o out_merge_b --merge -t 2 2> /dev/null

diff out_merge/merged.json out_merge_b/merged.json || { echo "FAIL: @names file output differs from -n pa,abs"; fail=1; }

${kmindex_bin} query2 -i indexes/index -n @missing_names.txt \
                     -q datasets/abs_dataset/1.fasta \
                     -z 4 -f json -o out_names --merge -t 2 2> names_err.log
if [ $? -eq 0 ]; then
  echo "FAIL: missing names file accepted"; fail=1
fi
grep -q "Cannot open names file" names_err.log || { echo "FAIL: expected names-file open error"; fail=1; }
[ -e out_names/merged.json ] && { echo "FAIL: merged.json produced for missing names file"; fail=1; }

printf '' > names_empty.txt
${kmindex_bin} query2 -i indexes/index -n @names_empty.txt \
                     -q datasets/abs_dataset/1.fasta \
                     -z 4 -f json -o out_names --merge -t 2 2> names_err.log
if [ $? -eq 0 ]; then
  echo "FAIL: empty names file accepted"; fail=1
fi
grep -q "No sub-index names" names_err.log || { echo "FAIL: expected empty names-file error"; fail=1; }
[ -e out_names/merged.json ] && { echo "FAIL: merged.json produced for empty names file"; fail=1; }

${kmindex_bin} query2 -i indexes/index -n @ \
                     -q datasets/abs_dataset/1.fasta \
                     -z 4 -f json -o out_names --merge -t 2 2> names_err.log
if [ $? -eq 0 ]; then
  echo "FAIL: bare @ accepted"; fail=1
fi
grep -q "Empty file path" names_err.log || { echo "FAIL: expected bare-@ error"; fail=1; }

# Test 17: an empty name in the --names list (",abs") is rejected cleanly —
# previously the empty first entry reached unchecked [0] access.
rm -rf out_badname

${kmindex_bin} query2 -i indexes/index -n ,abs \
                     -q datasets/abs_dataset/1.fasta \
                     -z 4 -f json -o out_badname --merge -t 2 2> badname_err.log
if [ $? -eq 0 ]; then
  echo "FAIL: empty sub-index name accepted"; fail=1
fi
grep -q "Invalid sub-index name" badname_err.log || { echo "FAIL: expected 'Invalid sub-index name' error"; fail=1; }
[ -d out_badname ] && { echo "FAIL: output dir created for empty name"; fail=1; }

rm -rf out_sep out_merge out_merge_multi out_merge_dup out_merge_b \
       out_fail out_fail2 out_t0 out_names out_badname \
       broken_global malicious_global evil_index out_evil \
       expected_concat.tsv expected_concat.jsonl expected_empty.json \
       expected_abs_empty.json short_query.fa t0_err.log evil_err.log \
       names_file.txt names_empty.txt names_err.log badname_err.log

if [ $fail -eq 0 ]; then
  echo "All query2 merge tests passed."
  exit 0
else
  exit 1
fi
