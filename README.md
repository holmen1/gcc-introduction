# An Introduction to GCC

A single invocation of GCC consists of the following stages:
1. preprocessing (to expand macros)
2. compilation (from source code to assembly language)
3. assembly (from assembly language to machine code)
4. linking (to create the final executable)

Below is my working notes based on *An Introduction to GCC* by Brian J. Gough (Network Theory Ltd, 2004).
Content includes quotes, paraphrased sections, and original examples/annotations.

Original book: Copyright © 2004 Network Theory Ltd.  
Licensed under the GNU Free Documentation License v1.2 or later.

> *"GCC is a programming tool, that's true—but it's also something more.  
> It is part of a 20-year campaign for freedom for computer users."*  
> 
> — Richard M. Stallman, Foreword



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

The final step in creating an executable file is to use `gcc` to link the object files together and fill in the missing addresses of external functions.
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
The file can be read like this: using the C compiler `gcc`, with compilation options '-std=c99 -Wall', build the target executable ```main```
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

By default, `gcc` searches the following directories for header files:
```/usr/local/include/:/usr/include/```
and the following directories for libraries: ```/usr/local/lib/:/usr/lib/```


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

#### Environment variables

Additional directories can be added to the include and link path using
environment variables C_INCLUDE_PATH and LIBRARY_PATH respectively:

```bash
$ C_INCLUDE_PATH=/tmp/gdbm-1.21/src
$ export C_INCLUDE_PATH
$ LIBRARY_PATH=/tmp/gdbm-1.21/src/.libs
$ export LIBRARY_PATH
```
`export` is needed to make the environment variable available to programs
outside the shell itself, such as the compiler.

Then, we can compile and link with
```bash
$ gcc -Wall dbmain.c -lgdbm
```

When environment and command-line options are used together the compiler
searches the directories in the following order:
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
```
```bash
$ ./a.out
Storing key-value pair... done.
```

Alternatively, static linking can be forced with the '-static' option
to avoid the use of the use of shared libraries.

```bash
$ gcc -Wall -static -I/tmp/gdbm-1.26/src -L/tmp/gdbm-1.26/src/.libs dbmain.c -lgdbm
$ ls -l
total 1148
-rwxr-xr-x 1 holmen1 holmen1 1169048 Dec 29 16:39 a.out
-rw-r--r-- 1 holmen1 holmen1     344 Dec 29 15:13 dbmain.c
```

Static linking produces a much larger executable (1.1 MB) because the entire library code is embedded in the binary. This eliminates the need for the `.so` file at runtime.

```bash
$ gcc -Wall -I/tmp/gdbm-1.26/src -L/tmp/gdbm-1.26/src/.libs dbmain.c -lgdbm
$ ls -l
total 20
-rwxr-xr-x 1 holmen1 holmen1 15600 Dec 29 16:38 a.out
-rw-r--r-- 1 holmen1 holmen1   344 Dec 29 15:13 dbmain.c
```

Shared linking keeps the executable small (15 KB) - only linking code is included, the actual library functions are loaded from `libgdbm.so.6` at runtime.

```bash
$ ls -l /tmp/gdbm-1.26/src/.libs/libgdbm*
-rw-r--r-- 1 holmen1 holmen1 586162 Dec 29 16:28 /tmp/gdbm-1.26/src/.libs/libgdbm.a
-rwxr-xr-x 1 holmen1 holmen1 337152 Dec 29 16:28 /tmp/gdbm-1.26/src/.libs/libgdbm.so.6.0.0
```

**Summary: Shared vs Static**

**Shared libraries (`.so`):**
- Small executable (15 KB) - code shared across processes
- Requires library at runtime - deployment dependency
- Library updates benefit all programs automatically
- Must set `LD_LIBRARY_PATH` if not in standard location

**Static libraries (`.a`):**
- Large executable (1.1 MB) - library embedded in binary
- Self-contained - no runtime dependencies
- Easier deployment but harder to update
- Must manually specify all transitive dependencies during linking

#### FreeBSD error
```bash
$ gcc -Wall -static dbmain.c -lgdbm
[...]
/tmp/gdbm-1.21/src/gdbmerrno.c:155:(.text+0x18f): undefined reference to `libintl_dgettext'
/usr/local/bin/ld: /tmp/gdbm-1.21/src/.libs/libgdbm.a(gdbmerrno.o):/tmp/gdbm-1.21/src/gdbmerrno.c:155: more undefined references to `libintl_dgettext' follow
collect2: error: ld returned 1 exit status
```
In this example, `libgdbm` depends on `libintl` (from GNU gettext for internationalization), which must be explicitly added with `-lintl` when using static linking.

```bash
$ gcc -Wall -static dbmain.c -lgdbm -lintl
```

### C language standards

#### Strict ANSI/ISO
The command-line option '-pedantic' in combination with '-ansi' will
cause `gcc` to reject all GNU C extensions, not just those that are incompatible with the
ANSI/ISO standard.

```bash
$ gcc -Wall -ansi -pedantic gnuarray.c
gnuarray.c: In function 'main':
gnuarray.c:4:5: warning: ISO C90 forbids variable length array 'x' [-Wvla]
    4 |     double x[n];
      |     ^~~~~~
```

#### Selecting specific standards
The specific language standard used by GCC can be controlled with the '-std' option.

```bash
$ gcc -std=c99 -Wall main.c
```

## Using the preprocessor

The preprocessor `cpp` expands macros in source files before they are compiled.

### Defining macros
The program [dtest.c](4_Preprocessor/dtest.c)
demonstrates the most common use of the C preprocessor. It uses the preprocessor conditional
`#ifdef` to check whether a macro is defined.
When the macro is defined, the preprocessor includes the corresponding code up to the closing
`#endif` command.

The `gcc` option '-D`NAME`' defines a preprocessor macro `NAME` from the command line.

```bash
$ gcc -Wall -DTEST dtest.c
$ ./a.out
Test mode
Running...
```
If the same program is compiled without the '-D' option then the "Test mode" message
is omitted from the source code after preprocessing, and the final executable does not include
the code for it:
```bash
$ gcc -Wall dtest.c
$ ./a.out
Running...
```

## Compiling for debuging

GCC provides the ‘-g’ debug option to store additional debugging information in object
files and executables. This debugging information allows errors to be traced back from a
specific machine instruction to the corresponding line in the original source file.
The execution of a program compiled with ‘-g’ can also be followed in a debugger, such as the GNU
Debugger `gdb`.

### Examining core files

When a program exits abnormally (i.e. crashes) the operating system can write out a
*core file* (usually named ‘core’) which contains the in-memory state of the program at the
time it crashed. This file is often referred to as a *core dump*.
Combined with information
from the symbol table produced by ‘-g’, the core dump can be used to find the line where
the program stopped, and the values of its variables at that point.


[null.c](5_Debugging/null.c)
is a simple program containing an invalid memory access bug, which we will use to
produce a core file.

```bash
$ gcc -Wall null.c
$ ./a.out
Segmentation fault         (core dumped) ./a.out
```

Whenever the error message ‘core dumped’ is displayed, the operating system should
produce a file called ‘core’ in the current directory. Variations ...

In order to be able to find the cause of the crash, we compile the
program with the ‘-g’ option: `$ gcc -Wall -g null.c`

Core files can be loaded into the GNU Debugger gdb with the following command:
`$ gdb EXECUTABLE-FILE CORE-FILE`

```bash
$ gdb a.out a.out.core
GNU gdb (GDB) 15.1 [GDB v15.1 for FreeBSD]
Copyright (C) 2024 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
Type "show copying" and "show warranty" for details.
This GDB was configured as "x86_64-portbld-freebsd15.0".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<https://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.

For help, type "help".
Type "apropos word" to search for commands related to "word"...
Reading symbols from a.out...
[New LWP 100876]
Core was generated by `./a.out'.
Program terminated with signal SIGSEGV, Segmentation fault.
Address not mapped to object.
#0  0x000000000040052c in foo (p=0x0) at null.c:11
11          int y = *p;
(gdb)
```

To investigate the cause of the crash, we display the value of the pointer p using the
debugger print command:
```bash
(gdb) print p
$1 = (int *) 0x0
(gdb) p *p
Cannot access memory at address 0x0
```
This shows that p is a null pointer (0x0) of type ‘int *’, so we know that dereferencing it
with the expression *p in this line has caused the crash.


### Displaying a backtrace
The debugger can also show the function calls and arguments up to the current point of
execution — this is called a stack backtrace and is displayed with the command `backtrace`
or shortcut `bt`.

```bash
(gdb) bt
#0  0x000000000040052c in foo (p=0x0) at null.c:11
#1  0x000000000040051e in main () at null.c:6
```

### Setting a breakpoint
To set a breakpoint on a specific function, use the command `b[reak]` *function-name*.
Since the main function is the first function to be executed in a C program the program
will stop immediately when it is run:

```bash
(gdb) b main
Breakpoint 1 at 0x40050a: file null.c, line 5.
(gdb) r
Starting program: /home/holmen1/repos/gcc-introduction/5_Debugging/a.out

Breakpoint 1, main () at null.c:5
5           int *p = 0; /* null pointer */
```

### Stepping through the program

The command `s[tep]` will follow the execution of the program interactively through any
functions that are called in the current line. If you want to move forward without tracing
these calls, use the command `n[ext]` instead.



### Modifying variables

Variables can be set to a specific value, or to the result of an expression, which may
include function calls. This powerful feature allows functions in a program to be tested
interactively through the debugger.
In this case we will interactively allocate some memory for the pointer p using the
function `malloc`, storing the value 255 in the resulting location.
If we now continue stepping through the program with the new value of p the previous
segmentation fault will not occur:

```bash
Breakpoint 1, main () at null.c:5
5           int *p = 0; /* null pointer */
(gdb) s
6           return foo(p);
(gdb) s
foo (p=0x0) at null.c:11
11          int y = *p;
(gdb) set variable p = malloc(sizeof(int))
(gdb) set variable *p = 255
(gdb) s
12          return y;
(gdb) p y
$1 = 255
(gdb) c
Continuing.
[Inferior 1 (process 5161) exited with code 0377]
```
Note that the exit code is shown in octal (0377 base 8 = 255 in base 10).


