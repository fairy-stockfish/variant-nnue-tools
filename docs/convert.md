# Convert Commands

The conversion functionality has been split into three separate commands: `convert_bin`, `convert_plain`, and `convert_epd`. Each command serves a specific conversion purpose for training data.

As with all commands in stockfish, these can be invoked either from the command line (as `stockfish.exe convert_bin ...`) or in the interactive prompt.

Note that you will need to set the correct variant with the `UCI_Variant` option before converting data for games other than chess. 

## convert_bin

Converts plain text training data into binary `.bin` format (PackedSfenValue).

**Purpose**: Converts text files containing position data (in FEN format with move, score, ply, and result information) into the binary format used for training.

**Syntax**:
```
convert_bin [options]
```

**Common Options**:
- `targetfile <file>` - Specifies an input file to convert (can be specified multiple times)
- `targetdir <dir>` - Specifies a directory containing input files
- `basedir <dir>` - Base directory for relative paths
- `output_file_name <output>` - Path to the output binary file (default: `shuffled_sfen.bin`)
- `ply_minimum <n>` - Minimum ply number to include (default: 0)
- `ply_maximum <n>` - Maximum ply number to include (default: 114514)
- `check_invalid_fen <0|1>` - Filter out invalid FEN positions (default: 0)
- `check_illegal_move <0|1>` - Filter out illegal moves (default: 0)
- `interpolate_eval <n>` - Interpolate evaluation scores (default: 0)
- `src_score_min_value <n>` - Minimum value in source score range (default: 0.0)
- `src_score_max_value <n>` - Maximum value in source score range (default: 1.0)
- `dest_score_min_value <n>` - Minimum value in destination score range (default: 0.0)
- `dest_score_max_value <n>` - Maximum value in destination score range (default: 1.0)

**Example**:
```
convert_bin targetfile training_data.txt output_file_name training.bin
```

**Input Format**: Plain text with each position represented as:
```
fen <fen-string>
move <move>
score <score>
ply <ply-number>
result <result>
e
```

## convert_plain

Converts binary `.bin` format (PackedSfenValue) back into plain text format.

**Purpose**: Converts the binary training data format back into human-readable text format for inspection or further processing.

**Syntax**:
```
convert_plain [options]
```

**Options**:
- `targetfile <file>` - Specifies an input binary file to convert (can be specified multiple times)
- `targetdir <dir>` - Specifies a directory containing input binary files
- `basedir <dir>` - Base directory for relative paths
- `output_file_name <output>` - Path to the output text file (default: `shuffled_sfen.bin`)

**Example**:
```
convert_plain targetfile training.bin output_file_name training_data.txt
```

**Output Format**: Plain text with each position represented as:
```
fen <fen-string>
move <move>
score <score>
ply <ply-number>
result <result>
e
```

## convert_epd

Converts binary `.bin` format (PackedSfenValue) into EPD format (FEN positions only).

**Purpose**: Exports positions from binary training data as EPD format, which contains only FEN strings (one per line), suitable for position databases or testing tools.

**Syntax**:
```
convert_epd [options]
```

**Options**:
- `targetfile <file>` - Specifies an input binary file to convert (can be specified multiple times)
- `targetdir <dir>` - Specifies a directory containing input binary files
- `basedir <dir>` - Base directory for relative paths
- `output_file_name <output>` - Path to the output EPD file (default: `shuffled_sfen.bin`)

**Example**:
```
convert_epd targetfile training.bin output_file_name positions.epd
```

**Output Format**: EPD format - one FEN string per line:
```
rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1
...
```

## Migration Note

The old unified `convert` command has been removed. If you were previously using:
```
convert from_path to_path
```

You should now use:
- `convert_bin` to convert from plain text to binary format
- `convert_plain` to convert from binary to plain text format
- `convert_epd` to export positions as EPD format
