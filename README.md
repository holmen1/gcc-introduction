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

By default, gcc searches the fallowing directories for header files:
```/usr/local/include/:/usr/include/```
and the fallowing directories for libraries: ```/usr/local/lib/:/usr/lib/```


When additional libraries are installed in other directories it is necessary to extend the search paths.

The compiler options '-I' and '-L' add new directories to the beginning of the include path and library search path respectively. 

#### GDBM example

[dbmain.c](3_Options/dbmain.c)

Accidentally uninstalled gdbm from standard directories :/

[download](https://www.gnu.org.ua/software/gdbm/download.html) and make new


Trying compile 
```bash
$ gcc -Wall dbmain.c -lgdbm
dbmain.c:1:10: fatal error: gdbm.h: No such file or directory
    1 | #include <gdbm.h>
      |          ^~~~~~~~
compilation terminated.
```

```bash
$ find /tmp/gdbm-1.21/ -name gdbm.h
/tmp/gdbm-1.21/src/gdbm.h
```

Adding the appropriate directory to the include path allows the program to be compiled but not linked
```bash
$ gcc -Wall -I/tmp/gdbm-1.21/src dbmain.c -lgdbm
/usr/local/bin/ld: cannot find -lgdbm: No such file or directory
collect2: error: ld returned 1 exit status
```

```bash
$ find /tmp/gdbm-1.21/ -name libgdbm.a
/tmp/gdbm-1.21/src/.libs/libgdbm.a
```

Add library to link path
```bash
$ gcc -Wall -I/tmp/gdbm-1.21/src -L/tmp/gdbm-1.21/src/.libs dbmain.c -lgdbm
```
Compliles and links but won't execute :/ (explained below).

### Environment variables

Additional directories can be added to the include and link path using
environment variables C_INCLUDE_PATH and LIBRARY_PATH respectively:

```bash
$ C_INCLUDE_PATH=/tmp/gdbm-1.21/src
$ export C_INCLUDE_PATH
$ LIBRARY_PATH=/tmp/gdbm-1.21/src/.libs
$ export LIBRARY_PATH
```
*export* is needed to make the environment variable available to programs
outside the shell itself, such as the compiler.

Then, we can compile and link with
```bash
$ gcc -Wall dbmain.c -lgdbm
```

When environment and command-line options are used together the compiler
searches the directories in the fallowing order:
1. command-line options '-I' and '-L', from left to right
2. environment variables C_INCLUDE_PATH and LIBRARY_PATH
3. default system directories

### Shared vs static libraries

Attempt to start the executable directly
```bash
$ ./a.out
ld-elf.so.1: Shared object "libgdbm.so.6" not found, required by "a.out"
```
This is because the GBDM package provides a *shared library*.
This type of library requires special treatment - it must be loaded from disc before the executable will run.

Whenever a static library 'libNAME.a' would be used for linking with the option
'-lNAME' the compiler first checks for an alternative with the same name and a '.so' extension.

Simple solution is setting load path through environment variable
```bash
$ LD_LIBRARY_PATH=/tmp/gdbm-1.21/src/.libs
$ export LD_LIBRARY_PATH
$ ./a.out
Storing key-value pair... done.
```

Alternatively, static linking can be forced with the '-static' option
to avoid the use of the use of shared libraries.


```bash
$ gcc -Wall -static dbmain.c -lgdbm
/usr/local/bin/ld: /tmp/gdbm-1.21/src/.libs/libgdbm.a(bucket.o): in function `_gdbm_write_bucket':
/tmp/gdbm-1.21/src/bucket.c:553:(.text+0x14d): undefined reference to `libintl_dgettext'
/usr/local/bin/ld: /tmp/gdbm-1.21/src/.libs/libgdbm.a(bucket.o): in function `_gdbm_get_bucket':
/tmp/gdbm-1.21/src/bucket.c:282:(.text+0x5ca): undefined reference to `libintl_dgettext'
/usr/local/bin/ld: /tmp/gdbm-1.21/src/.libs/libgdbm.a(bucket.o): in function `_gdbm_split_bucket':
/tmp/gdbm-1.21/src/bucket.c:421:(.text+0xca4): undefined reference to `libintl_dgettext'
/usr/local/bin/ld: /tmp/gdbm-1.21/src/.libs/libgdbm.a(bucket.o): in function `_gdbm_fetch_data':
/tmp/gdbm-1.21/src/bucket.c:662:(.text+0xffe): undefined reference to `libintl_dgettext'
/usr/local/bin/ld: /tmp/gdbm-1.21/src/.libs/libgdbm.a(gdbmerrno.o): in function `gdbm_strerror':
/tmp/gdbm-1.21/src/gdbmerrno.c:155:(.text+0x18f): undefined reference to `libintl_dgettext'
/usr/local/bin/ld: /tmp/gdbm-1.21/src/.libs/libgdbm.a(gdbmerrno.o):/tmp/gdbm-1.21/src/gdbmerrno.c:155: more undefined references to `libintl_dgettext' follow
collect2: error: ld returned 1 exit status
```


