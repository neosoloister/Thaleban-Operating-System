import sys
import os

def main():
    if len(sys.argv) < 3:
        print("Usage: python build_image.py <output_file> <input_file1> [input_file2 ...]")
        sys.exit(1)

    output_file = sys.argv[1]
    input_files = sys.argv[2:]

    try:
        with open(output_file, 'wb') as outfile:
            for input_file in input_files:
                if not os.path.exists(input_file):
                    print(f"Error: Input file '{input_file}' not found.")
                    sys.exit(1)
                
                with open(input_file, 'rb') as infile:
                    outfile.write(infile.read())
            
            # Pad to ensure it's at least 32KB (64 sectors * 512 bytes)
            # This matches the previous 'dd if=/dev/zero bs=512 count=64 >> $@' logic roughly,
            # but dd >> appends 32KB of zeros. 
            # The previous Makefile did: cat inputs > out; dd ... >> out.
            # So it appended 32KB of zeros *after* the content.
            
            outfile.write(b'\x00' * (64 * 512))
            
        print(f"Successfully created {output_file}")

    except IOError as e:
        print(f"Error creating image: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
