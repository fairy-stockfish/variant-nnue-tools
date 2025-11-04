# generate_puzzles

`generate_puzzles` command allows generation of puzzle positions from self-play games. Puzzles are tactical positions that can be used for training, testing, or creating puzzle databases. The generator introduces random moves to diversify positions, uses fixed depth evaluation, and applies filters to ensure quality puzzle positions.

As all commands in stockfish `generate_puzzles` can be invoked either from command line (as `stockfish.exe generate_puzzles ...`, but this is not recommended because it's not possible to specify UCI options before `generate_puzzles` executes) or in the interactive prompt.

It is recommended to keep the `EnableTranspositionTable` UCI option at the default `true` value as it will make the generation process faster without noticeably harming the uniformity of the data.

`generate_puzzles` takes named parameters in the form of `generate_puzzles param_1_name param_1_value param_2_name param_2_value ...`.

A very minimal example is `generate_puzzles count 100 output_file_name test.epd`.

The reported puzzles only fulfill a very basic set of requirements. It is highly recommended to further validate and filter them using [ianfab/chess-variant-puzzler](https://github.com/ianfab/chess-variant-puzzler).

Currently the following options are available:

`set_recommended_uci_options` - this is a modifier not a parameter, no value follows it. If specified then some UCI options are set to recommended values.

`depth` - sets minimum and maximum depth of evaluation of each position. Default: 3.

`min_depth` - minimum depth of evaluation of each position. If not specified then the same as `depth`.

`max_depth` - maximum depth of evaluation of each position. If not specified then the same as `depth`.

`puzzle_depth` - if set to a value greater than 0, enables mate puzzle validation. Positions will be searched to this depth to verify they contain a forced mate. Only positions with mates longer than `mate_ply` will be kept. Default: 7.

`nodes` - the number of nodes to use for evaluation of each position. This number is multiplied by the number of PVs of the current search. This does NOT override the depth options. If specified then whichever of depth or nodes limit is reached first applies. Default: 0 (no node limit).

`count` - the number of puzzle positions to generate. 1 entry == 1 position. Default: 10000.

`output_file_name` - the name of the file to output to. If the extension is not present or doesn't match the selected data format the right extension will be appended. Default: puzzles

`material_limit` - maximum allowed material advantage. Positions with material imbalance exceeding this value are filtered out. Default: 64000 (effectively disabled).

`final_material_limit` - maximum allowed material imbalance after playing the validation PV. Helps avoid puzzles where the mating line ends with overwhelming extra material. Default: 64000 (effectively disabled).

`material_diff_limit` - maximum allowed absolute change in material imbalance between the candidate position and the final position reached by the validation PV. Default: 64000 (effectively disabled).

`mate_ply` - only keep positions with mate in at least this many plies. Filters out mates that are too short. When using `puzzle_depth` it applies to this validation search, otherwise uses the regular search result. Default: 1.

`second_pv_limit` - uses multipv 2 during puzzle verification and filters out positions where the second best line's evaluation is greater than or equal to this value (in centipawns). This helps ensure puzzle positions have a clear best solution by filtering out positions with strong alternative moves. Set to a value >= 32000 (VALUE_MATE) to disable this check. Default: 1000.

`second_pv_nonroot_limit` - when validating the PV recursively, re-search positions where the attacking side is to move again. If the second-best move at any of those points evaluates above this limit, the puzzle is rejected. The search depth shrinks as the PV progresses. Set to a value >= 32000 (VALUE_MATE) to disable. Default: VALUE_MATE_IN_MAX_PLY.

`random_move_min_ply` - the minimal ply at which a random move may be executed instead of a move chosen by search. Default: 1.

`random_move_max_ply` - the maximal ply at which a random move may be executed instead of a move chosen by search. Default: 150.

`random_move_count` - maximum number of random moves in a single self-play game. Default: 10.

`random_multi_pv` - the number of PVs used for determining the random move. If zero then a truly random move will be chosen. If non-zero then a multiPV search will be performed and the random move will be one of the moves chosen by the search. Default: 0.

`random_multi_pv_diff` - Makes the multiPV random move selection consider only moves that are at most `random_multi_pv_diff` worse than the next best move. Default: 300.

`random_multi_pv_depth` - the depth to use for multiPV search for random move. Default: `max_depth`.

`write_min_ply` - minimum ply for which the puzzle position will be emitted. Default: 5.

`write_max_ply` - maximum ply for which the puzzle position will be emitted. Default: 150.

`book` - a path to an opening book to use for the starting positions. Currently only .epd format is supported. If not specified then the starting position is always the variant starting position.

`save_every` - the number of puzzle entries per file. If not specified then there will be always one file. If specified there may be more than one file generated (each having at most `save_every` puzzle entries) and each file will have a unique number attached.

`report_stats_every` - the number of positions between progress reports. Default: 10.

`random_file_name` - if specified then the output filename will be chosen randomly. Overrides `output_file_name`.

`adjudicate_draws_by_score` - either 0 or 1. If 1 then drawn games will be adjudicated when the score remains 0 for at least 8 plies after ply 80. Default: 1.

`adjudicate_draws_by_insufficient_mating_material` - either 0 or 1. If 1 then positions with insufficient material will be adjudicated as draws. Default: 1.

`data_format` - format of the puzzle data to use. Supported formats are `bin` (binary PackedSfenValue format) and `epd` (Extended Position Description, FEN strings). Default: `epd`.

`seed` - seed for the PRNG. Can be either a number or a string. If it's a string then its hash will be used. If not specified then the current time will be used.
