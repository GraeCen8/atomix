# Automatically finds all .c and .asm files and compiles them
build:
    #!/usr/bin/env bash
    set -e # Stop if an error occurs

    echo "Building Atomix OS..."

    # 1. Compile Assembly files
    for file in *.asm; do
        obj="${file%.asm}.asm.o" # Avoid clashing with similarly named C sources
        echo "Assembling $file -> $obj"
        nasm -f elf32 "$file" -o "$obj"
    done

    # 2. Compile C files
    for file in *.c; do
        obj="${file%.c}.c.o"
        echo "Compiling $file -> $obj"
        gcc -m32 -c "$file" -o "$obj"
    done

    # 3. Link all object files into the kernel
    echo "Linking..."
    ld -m elf_i386 -T link.ld -o kernel *.o

    echo "Done! Run 'just run' to boot."

# Runs the kernel in qemu
run:
    qemu-system-i386 -kernel kernel

# Cleans up all .o files (object files) and the kernel binary
clean:
    rm -f *.o

# The full pipeline: Build -> Clean (keep objs for linking?) -> Run
# We remove the 'clean' step from 'go' here because 'build' creates the .o files
# needed for linking. If you want to clean them after, you can do 'just clean' manually.
go:
    just build
    just clean
    just run
