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

    # 2. Compile C files (including subdirectories like drivers/)
    for file in $(find . -name '*.c' -print); do
        clean="${file#./}"
        obj="${clean//\//_}"
        obj="${obj%.c}.c.o"
        echo "Compiling $clean -> $obj"
        gcc -m32 -ffreestanding -fno-stack-protector -fno-pic -I. -c "$clean" -o "$obj"
    done

    # 3. Link all object files into the kernel
    echo "Linking..."
    ld -m elf_i386 -T link.ld -o kernel *.o

    echo "Done! Run 'just run' to boot."

    # 4. clean up all object files
    just clean

# Runs the kernel in qemu
run:
    qemu-system-i386 -kernel kernel

# Cleans up all .o files (object files) and the kernel binary
clean:
    rm -f *.o

# Build and run without forced clean to keep iteration fast.
go:
    just build
    just run
