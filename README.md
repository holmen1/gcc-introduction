# An Introduction to GCC

GCC is a programming tool, that's true-but it's also something more.  
It is part of a 20-year campaign for freedom for computer users.

Richard M. Stallman  
February 2004

## Compiling a C program
### Compiling a simple C program
```
gcc -Wall hello.c -o hello
```
The option '-Wall' turns on all the most commonly-used compiler warnings -**it is recommended that you always use this option!**


### Compiling multiple source files
```
gcc -Wall main.c hello_fn.c -o newhello
```


### Compiling files independently
The command-line option '-c' is used to compile a source file to an object file.
```
gcc -Wall -c main.c
```
This produces an object file 'main.o' containing the machine code for the main function.

The final step in creating an executable file is to use ```gcc``` to link the object files together and fill in the missing addresses of external functions.
```
gcc main.o hello_fn.o -o hello
```

### A simple makefile
A makefile specifies a set of compilation rules in terms of *targets* and their *dependencies*.  
Implicit rules are defined in terms of *make variables*, such as ```CC``` and ```CFLAGS```.


```Makefile
CC=gcc
CFLAGS=-std=c99 -Wall

main: main.o hello_fn.o

clean:
	rm -f main main.o hello_fn.o
```
The file can be read like this: using the C compiler gcc, with compilation options '-std=c99 -Wall', build the target executable ```main```
from the object files 'main.o' and 'hello_fn.o' (these in turn, will be built via implicit rules from 'main.c' and 'hello_fn.c').

To use the makefile, type ```make```.

```bash
$ make
gcc -std=c99 -Wall   -c -o main.o main.c
gcc -std=c99 -Wall   -c -o hello_fn.o hello_fn.c
gcc   main.o hello_fn.o   -o main
$ ./main
Hello, world!
```

Finally, to remove the generated files, type ```make clean```.

```bash
$ make clean
rm -f main main.o hello_fn.o
```

### Linking with external libraries

A library is a collection of precompiled object files which can be linked into programs.  
Libraries are typically stored in special *archive files* with the extension '.a',
referred to as *static libraries*.

The standard system libraries are usually found in the directories '/usr/lib' and '/lib'.

To enable the compiler to link a compiled function to a program we need to supply the library.
For example: ```$ gcc calc.c /usr/lib/libm.a -o calc```.  
The compiler provides a short-cut option '-l' for linking against libraries.
So ```$ gcc calc.c -lm -o calc``` is equivalent to above.

In general, the compiler option '-lNAME' will attempt to link object files with a library file
'libNAME.a' in the standard library directories.


## Compilation options

### Setting search paths

When additional libraries are installed in other directories it is necessary to extend the search paths.

The compiler options '-I' and '-L' add new directories to the beginning of the include path and library search path respectively. 

Accidentally uninstalled gdbm :/
```bash
$ gcc -Wall dbmain.c -lgdbm
dbmain.c:1:10: fatal error: gdbm.h: No such file or directory
    1 | #include <gdbm.h>
      |          ^~~~~~~~
compilation terminated.
```
(download)[https://www.gnu.org.ua/software/gdbm/download.html] and make new

Adding the appropriate directory to the include path allows the program to be compiled but not linked
```bash
$ gcc -Wall -I/home/holmen1/Downloads/gdbm-1.21/src dbmain.c -lgdbm
/usr/local/bin/ld: cannot find -lgdbm: No such file or directory
collect2: error: ld returned 1 exit status
```

Add library to link path
```bash
$ gcc -Wall -I/home/holmen1/Downloads/gdbm-1.21/src -L/home/holmen1/Downloads/gdbm-1.21/src/.libs dbmain.c -lgdbm
```

But
```bash
$ ./a.out
ld-elf.so.1: Shared object "libgdbm.so.6" not found, required by "a.out"
```
This is because the GBDM package provides a *shared library*.
This type of library requires special treatment - it must be loaded from disc before the executable will run.

```bash
$ LD_LIBRARY_PATH=/home/holmen1/Downloads/gdbm-1.21/src/.libs
$ export LD_LIBRARY_PATH
$ ./a.out
Storing key-value pair... done.
```

