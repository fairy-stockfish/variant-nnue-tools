import argparse

RECORD_SIZE = 40
SFEN_SIZE = 32

def process_files(input_files, output_file):
    seen_sfens = set()
    total_records_read = 0
    unique_records_written = 0

    with open(output_file, 'wb') as f_out:
        for input_path in input_files:
            print(f"Processing input file: {input_path}")
            try:
                with open(input_path, 'rb') as f_in:
                    while True:
                        record = f_in.read(RECORD_SIZE)
                        if not record:
                            break  # End of file
                        
                        if len(record) < RECORD_SIZE:
                            print(f"Warning: Malformed record in {input_path}. Expected {RECORD_SIZE} bytes, got {len(record)}. Skipping.")
                            continue
                        
                        total_records_read += 1
                        sfen_data = record[:SFEN_SIZE]
                        
                        if sfen_data not in seen_sfens:
                            seen_sfens.add(sfen_data)
                            f_out.write(record)
                            unique_records_written += 1
            except FileNotFoundError:
                print(f"Error: Input file not found: {input_path}")
            except Exception as e:
                print(f"An error occurred while processing {input_path}: {e}")
    
    print("\n--- Summary ---")
    print(f"Total records read: {total_records_read}")
    print(f"Total unique records written: {unique_records_written}")
    print(f"Output file: {output_file}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Merge and deduplicate records from binary files.")
    parser.add_argument('input_files', nargs='+', help="Paths to input binary files.")
    parser.add_argument('output_file', help="Path to the output binary file.")
    
    args = parser.parse_args()
    
    process_files(args.input_files, args.output_file)
