# ConOS

## Requirements
* qemu-system-x86_64
* nasm
* gcc
* sudo

## Setup
1. Clone the project locally.
2. Select a gcc compiler to use by adding `export CONOS_CC=/path/to/gcc` to the end of ~/.bashrc. Be sure to restart the shell or run `source ~/.bashrc`.
3. `make run` to build the project.
4. `make debug` to debug the project using GDB. Use `target remote localhost:1234` in GDB to connect the debugger to the VM.
5. `make clean` to clean up build files.

## Commiting
Be sure to run make clean before pushing commits to the main repository!