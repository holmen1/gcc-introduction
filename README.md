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
produce a core file which can be used for debugging.

**Note on Arch Linux / systemd:** Core dumps are handled by systemd and stored in `/var/lib/systemd/coredump/` 
by default. To instead write core dumps to the current working directory, use:
```bash
$ ulimit -c 1000
$ sudo sysctl -w kernel.core_pattern=%e.%p.core
```
The first command sets the core dump size limit to 1000 blocks of 512 bytes each (512 KB). 
The second configures the kernel to write core files directly to the current working directory 
with the naming format `executable.pid.core` (e.g., `a.out.1234.core`).
To view systemd-managed core dumps, use `coredumpctl` command.

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

## Compiling with optimization

GCC is an optimizing compiler. It provides a wide range of options which aim to increase
the speed, or reduce the size, of the executable files it generates.

### Source-level optimization

The first form of optimization used by GCC occurs at the source-code level, and does not require any knowledge of the machine instructions.

#### Common subexpression elimination

One method of source-level optimization which is easy to understand involves computing an
expression in the source code with fewer instructions, by reusing already-computed results.
For example, the following assignment:
```c
x = cos(v)*(1+sin(u/2)) + sin(w)*(1-sin(u/2))
```
can be rewritten with a temporary variable t to eliminate an unnecessary extra evaluation
of the term sin(u/2):
```c
t = sin(u/2)
x = cos(v)*(1+t) + sin(w)*(1-t)
```
This rewriting is called common subexpression elimination (CSE).

#### Function inlining

Whenever a function is used, a certain amount of extra time is required for the CPU
to carry out the call: it must store the function arguments in the appropriate registers
and memory locations, jump to the start of the function (bringing the appropriate virtual
memory pages into physical memory or the CPU cache if necessary), begin executing the
code, and then return to the original point of execution when the function call is complete.
This additional work is referred to as function-call overhead. Function inlining eliminates
this overhead by replacing calls to a function by the code of the function itself.

The following function sq(x) is a typical example of a function that would benefit from
being inlined. It computes the square of its argument x:
```c
double sq (double x)
{
  return x * x;
}
```
If this function is used inside a loop, such as the one below, then the function-call overhead would become substantial:
```c
for (i = 0; i < 1000000; i++)
{
  sum += sq (i + 0.5);
}
```
Optimization with inlining replaces the inner loop of the program with the body of the
function, giving the following code:
```c
for (i = 0; i < 1000000; i++)
{
double t = (i + 0.5); /* temporary variable */
sum += t * t;
}
```


### Speed-space tradeoffs

While some forms of optimization, such as common subexpression elimination, are able
to increase the speed and reduce the size of a program simultaneously, other types of
optimization produce faster code at the expense of increasing the size of the executable.

#### Loop unrolling

The following loop from 0 to 7 tests the condition i < 8 on each iteration:
```c
for (i = 0; i < 8; i++)
{
  y[i] = i;
}
```

A more efficient way to write the same code is simply to unroll the loop and execute the
assignments directly:
```c
y[0] = 0;
y[1] = 1;
y[2] = 2;
y[3] = 3;
y[4] = 4;
y[5] = 5;
y[6] = 6;
y[7] = 7;
```
This form of the code does not require any tests, and executes at maximum speed. Since
each assignment is independent, it also allows the compiler to use parallelism on processors
that support it.

### Scheduling

The lowest level of optimization is scheduling, in which the compiler determines the best
ordering of individual instructions.

Scheduling improves the speed of an executable without increasing its size, but requires
additional memory and time in the compilation process itself (due to its complexity).

### Optimization levels

An optimization level is chosen with the command line option ‘-OLEVEL’, where LEVEL
is a number from 0 to 3. The effects of the different optimization levels are described below:

`-O0` or no option (default)  
At this optimization level GCC does not perform any optimization and compiles
the source code in the most straightforward way possible.  
**This is the best option to use when debugging** a program and is the default
if no optimization level option is specified.

`-O1` or `-O`  
This level turns on the most common forms of optimization that do not require
any speed-space tradeoffs. With this option the resulting executables should be
smaller and faster than with ‘-O0’.

`-O2`  
These additional optimizations include instruction scheduling.  
**This option is generally the best choice for deployment** of a program,
because it provides maximum optimization without increasing the executable size.

`-O3`  
This option turns on more expensive optimizations, such as function inlining.

`-funroll-loops`  
This option turns on loop-unrolling, and is independent of the other optimization options.

`-Os`  
This option selects optimizations which reduce the size of an executable. The
aim of this option is to produce the smallest possible executable, for systems
constrained by memory or disk space.  
It is important to remember that the benefit of optimization at the highest levels must be
weighed against the cost.


### Examples

```bash
$ lscpu | grep -E "Model name|MHz"
Model name:                              11th Gen Intel(R) Core(TM) i5-1135G7 @ 2.40GHz
$ lscpu | grep -E "L1|L2|L3"
L1d cache:                               192 KiB (4 instances)
L1i cache:                               128 KiB (4 instances)
L2 cache:                                5 MiB (4 instances)
L3 cache:                                8 MiB (1 instance)
```
Here are some results for the program [test.c](6_Optimization/test.c),
using GCC 15.2.1 on a GNU/Linux system:

```bash
$ gcc -Wall test.c
time ./a.out

real    0m4.717s
user    0m4.710s
sys     0m0.000s


$ gcc -Wall -O1 test.c
$ time ./a.out

real    0m1.432s
user    0m1.427s
sys     0m0.003s


$ gcc -Wall -O2 test.c
$ time ./a.out

real    0m1.227s
user    0m1.224s
sys     0m0.002s


$ gcc -Wall -O3 test.c
$ time ./a.out

real    0m0.985s
user    0m0.983s
sys     0m0.001s


$ gcc -Wall -O3 -funroll-loops test.c
$ time ./a.out

real    0m0.962s
user    0m0.960s
sys     0m0.001s
```


```bash
# Compile with different flags explicitly
gcc -Wall -O0 test.c -o test-O0
gcc -Wall -O3 -funroll-loops test.c -o test-O3

# Check actual code section sizes
size test-O0 test-O3
   text    data     bss     dec     hex filename
   1587     584       8    2179     883 test-O0
   1994     584       8    2586     a1a test-O3

# See the assembly to verify optimization
gcc -Wall -O0 -S test.c -o test-O0.s
gcc -Wall -O3 -funroll-loops -S test.c -o test-O3.s
wc -l test-O0.s test-O3.s
 113 test-O0.s
 237 test-O3.s
 350 total
```
Bottom line: The -O3 -funroll-loops version has 26% more actual code, but you can't see it in the total file size because the code is a small fraction of the binary. Always use size to see the real difference!

### Optimization and debugging

With GCC it is possible to use optimization in combination with the debugging option ‘-g’.

When a program crashes unexpectedly, any debugging information is better
than none—so the use of ‘-g’ is recommended for optimized programs.

### Optimization and compiler warnings

As part of the optimization process, the compiler examines the use of all variables and
their initial values —this is referred to as *data-flow analysis*.
The ‘-Wuninitialized’ option (which is included in ‘-Wall’) warns about variables
that are read without being initialized.
The following function contains an example of such a variable:
```c
int sign (int x)
{
  int s;
  if (x > 0)
    s = 1;
  else if (x < 0)
    s = -1;
  return s;
}
```

The function works correctly for most arguments, but has a bug when x is zero—in this
case the return value of the variable s will be undefined.

To produce a warning, the program must be compiled with ‘-Wall’ and optimization simultaneously:
```bash
$ gcc -Wall -O2 -c uninit.c
uninit.c: In function ‘sign’:
uninit.c:4: warning: ‘s’ might be used uninitialized in this function
```
This correctly detects the possibility of the variable s being used without being defined.


## Troubleshooting

GCC provides several help and diagnostic options to assist in troubleshooting problems
with the compilation process.

### Help for command-line options

To display a complete list of options for gcc and its associated programs, such as the GNU
Linker and GNU Assembler, use the help option above with the verbose (‘-v’) option:
```$ gcc -v --help```

### Version numbers

You can find the version number of gcc using the version option:
```bash
$ gcc --version
gcc (GCC) 15.2.1 20260103
```
The version number is important when investigating compilation problems, since older
versions of GCC may be missing some features that a program uses.

More details about the version can be found using ‘-v’:
```bash
$ gcc -v
Using built-in specs.
COLLECT_GCC=gcc
COLLECT_LTO_WRAPPER=/usr/lib/gcc/x86_64-pc-linux-gnu/15.2.1/lto-wrapper
Target: x86_64-pc-linux-gnu
Configured with: /build/gcc/src/gcc/configure --enable-languages=ada,c,c++,d,fortran,go,lto,m2,objc,obj-c++,rust,cobol --enable-bootstrap --prefix=/usr --libdir=/usr/lib --libexecdir=/usr/lib --mandir=/usr/share/man --infodir=/usr/share/info --with-bugurl=https://gitlab.archlinux.org/archlinux/packaging/packages/gcc/-/issues --with-build-config=bootstrap-lto --with-linker-hash-style=gnu --with-system-zlib --enable-__cxa_atexit --enable-cet=auto --enable-checking=release --enable-clocale=gnu --enable-default-pie --enable-default-ssp --enable-gnu-indirect-function --enable-gnu-unique-object --enable-libstdcxx-backtrace --enable-link-serialization=1 --enable-linker-build-id --enable-lto --enable-multilib --enable-plugin --enable-shared --enable-threads=posix --disable-libssp --disable-libstdcxx-pch --disable-werror
Thread model: posix
Supported LTO compression algorithms: zlib zstd
gcc version 15.2.1 20260103 (GCC) 
```

### Verbose compilation

The ‘-v’ option can also be used to display detailed information about the exact sequence
of commands used to compile and link a program:
```bash
9_Troubleshooting $ gcc -v -std=c99 -Wall hello.c
Using built-in specs.
COLLECT_GCC=gcc
COLLECT_LTO_WRAPPER=/usr/lib/gcc/x86_64-pc-linux-gnu/15.2.1/lto-wrapper
Target: x86_64-pc-linux-gnu
Configured with: /build/gcc/src/gcc/configure --enable-languages=ada,c,c++,d,fortran,go,lto,m2,objc,obj-c++,rust,cobol --enable-bootstrap --prefix=/usr --libdir=/usr/lib --libexecdir=/usr/lib --mandir=/usr/share/man --infodir=/usr/share/info --with-bugurl=https://gitlab.archlinux.org/archlinux/packaging/packages/gcc/-/issues --with-build-config=bootstrap-lto --with-linker-hash-style=gnu --with-system-zlib --enable-__cxa_atexit --enable-cet=auto --enable-checking=release --enable-clocale=gnu --enable-default-pie --enable-default-ssp --enable-gnu-indirect-function --enable-gnu-unique-object --enable-libstdcxx-backtrace --enable-link-serialization=1 --enable-linker-build-id --enable-lto --enable-multilib --enable-plugin --enable-shared --enable-threads=posix --disable-libssp --disable-libstdcxx-pch --disable-werror
Thread model: posix
Supported LTO compression algorithms: zlib zstd
gcc version 15.2.1 20260103 (GCC)
COLLECT_GCC_OPTIONS='-v' '-std=c99' '-Wall' '-mtune=generic' '-march=x86-64' '-dumpdir' 'a-'
 /usr/lib/gcc/x86_64-pc-linux-gnu/15.2.1/cc1 -quiet -v hello.c -quiet -dumpdir a- -dumpbase hello.c -dumpbase-ext .c -mtune=generic -march=x86-64 -Wall -std=c99 -version -o /tmp/ccY3Sv8a.s
GNU C99 (GCC) version 15.2.1 20260103 (x86_64-pc-linux-gnu)
        compiled by GNU C version 15.2.1 20260103, GMP version 6.3.0, MPFR version 4.2.2, MPC version 1.3.1, isl version isl-0.27-GMP

GGC heuristics: --param ggc-min-expand=100 --param ggc-min-heapsize=131072
ignoring nonexistent directory "/usr/lib/gcc/x86_64-pc-linux-gnu/15.2.1/../../../../x86_64-pc-linux-gnu/include"
#include "..." search starts here:
#include <...> search starts here:
 /usr/lib/gcc/x86_64-pc-linux-gnu/15.2.1/include
 /usr/local/include
 /usr/lib/gcc/x86_64-pc-linux-gnu/15.2.1/include-fixed
 /usr/include
End of search list.
Compiler executable checksum: 2aa0ead74273ed721e313ee5d9153840
COLLECT_GCC_OPTIONS='-v' '-std=c99' '-Wall' '-mtune=generic' '-march=x86-64' '-dumpdir' 'a-'
 as -v --64 -o /tmp/ccOExV7p.o /tmp/ccY3Sv8a.s
GNU assembler version 2.45.1 (x86_64-pc-linux-gnu) using BFD version (GNU Binutils) 2.45.1
COMPILER_PATH=/usr/lib/gcc/x86_64-pc-linux-gnu/15.2.1/:/usr/lib/gcc/x86_64-pc-linux-gnu/15.2.1/:/usr/lib/gcc/x86_64-pc-linux-gnu/:/usr/lib/gcc/x86_64-pc-linux-gnu/15.2.1/:/usr/lib/gcc/x86_64-pc-linux-gnu/
LIBRARY_PATH=/usr/lib/gcc/x86_64-pc-linux-gnu/15.2.1/:/usr/lib/gcc/x86_64-pc-linux-gnu/15.2.1/../../../../lib/:/lib/../lib/:/usr/lib/../lib/:/usr/lib/gcc/x86_64-pc-linux-gnu/15.2.1/../../../:/lib/:/usr/lib/
COLLECT_GCC_OPTIONS='-v' '-std=c99' '-Wall' '-mtune=generic' '-march=x86-64' '-dumpdir' 'a.'
 /usr/lib/gcc/x86_64-pc-linux-gnu/15.2.1/collect2 -plugin /usr/lib/gcc/x86_64-pc-linux-gnu/15.2.1/liblto_plugin.so -plugin-opt=/usr/lib/gcc/x86_64-pc-linux-gnu/15.2.1/lto-wrapper -plugin-opt=-fresolution=/tmp/cciEhdsP.res -plugin-opt=-pass-through=-lgcc -plugin-opt=-pass-through=-lgcc_s -plugin-opt=-pass-through=-lc -plugin-opt=-pass-through=-lgcc -plugin-opt=-pass-through=-lgcc_s --build-id --eh-frame-hdr --hash-style=gnu -m elf_x86_64 -dynamic-linker /lib64/ld-linux-x86-64.so.2 -pie /usr/lib/gcc/x86_64-pc-linux-gnu/15.2.1/../../../../lib/Scrt1.o /usr/lib/gcc/x86_64-pc-linux-gnu/15.2.1/../../../../lib/crti.o /usr/lib/gcc/x86_64-pc-linux-gnu/15.2.1/crtbeginS.o -L/usr/lib/gcc/x86_64-pc-linux-gnu/15.2.1 -L/usr/lib/gcc/x86_64-pc-linux-gnu/15.2.1/../../../../lib -L/lib/../lib -L/usr/lib/../lib -L/usr/lib/gcc/x86_64-pc-linux-gnu/15.2.1/../../.. -L/lib -L/usr/lib /tmp/ccOExV7p.o -lgcc --push-state --as-needed -lgcc_s --pop-state -lc -lgcc --push-state --as-needed -lgcc_s --pop-state /usr/lib/gcc/x86_64-pc-linux-gnu/15.2.1/crtendS.o /usr/lib/gcc/x86_64-pc-linux-gnu/15.2.1/../../../../lib/crtn.o
COLLECT_GCC_OPTIONS='-v' '-std=c99' '-Wall' '-mtune=generic' '-march=x86-64' '-dumpdir' 'a.'
```

The output produced by ‘-v’ can be useful whenever there is a problem with the compilation
process itself. It displays the full directory paths used to search for header files and libraries,
the predefined preprocessor symbols, and the object files and libraries used for linking.

### Stopping a program in an infinite loop

A program which goes into an infinite loop or “hangs” can be difficult to debug.
A sophisticated
approach is to attach to the running process with a debugger and inspect it interactively.
For example, here is a simple
[program](9_Troubleshooting/loop.c)
with an infinite loop:

```bash
$ gcc -Wall loop.c
$ ./a.out
(program hangs)
```

Once the executable is running we need to find its process id (pid). This can be done from
another session with the command ps x:
```bash
$ ps x | grep a.out
   6226 pts/0    R+     0:56 ./a.out
```

In this case the process id is 6226, and we can now attach to it with gdb. The debugger
should be started in the directory containing the executable and its source code.

**Note:** On modern Linux systems with ptrace restrictions (`ptrace_scope=1`), you'll need to
use `sudo` to attach to a running process. See the note below for alternative approaches.

```bash
$ sudo gdb -p 6226
GNU gdb (GDB) 17.1
Copyright (C) 2025 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
Type "show copying" and "show warranty" for details.
This GDB was configured as "x86_64-pc-linux-gnu".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<https://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.

For help, type "help".
Type "apropos word" to search for commands related to "word".
Attaching to process 6226
Reading symbols from /home/holmen1/repos/gcc-introduction/9_Troubleshooting/a.out...
Reading symbols from /usr/lib/libc.so.6...
(No debugging symbols found in /usr/lib/libc.so.6)
Reading symbols from /lib64/ld-linux-x86-64.so.2...
(No debugging symbols found in /lib64/ld-linux-x86-64.so.2)
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/usr/lib/libthread_db.so.1".
0x000055ae1d900128 in main () at loop.c:6
6               i++;
(gdb) p i
$1 = 3748150416
(gdb) kill
Kill the program being debugged? (y or n) y
[Inferior 1 (process 6625) killed]
```

```bash
$ gdb ./a.out
GNU gdb (GDB) 17.1
Copyright (C) 2025 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
Type "show copying" and "show warranty" for details.
This GDB was configured as "x86_64-pc-linux-gnu".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<https://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.

For help, type "help".
Type "apropos word" to search for commands related to "word"...
Reading symbols from ./a.out...
(gdb) run
Starting program: /home/holmen1/repos/gcc-introduction/9_Troubleshooting/a.out
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/usr/lib/libthread_db.so.1".
^C
Program received signal SIGINT, Interrupt.
0x0000555555555128 in main () at loop.c:6
6               i++;
(gdb) p i
$1 = 3053629673
```

**Modern Linux Systems (ptrace restrictions):**

On modern Linux distributions, the Yama security module restricts ptrace operations by default.
If you encounter `ptrace: Operation not permitted` when trying to attach GDB to a running process,
check the current restriction level:
```bash
$ cat /proc/sys/kernel/yama/ptrace_scope
```
- `0` = classic ptrace (unrestricted)
- `1` = restricted (only parent processes can trace) - **default on most systems**
- `2` = admin-only
- `3` = no ptrace allowed

**The examples above demonstrate two common workarounds:**

1. **Run GDB with sudo** (first example above):
   ```bash
   $ sudo gdb -p <pid>
   ```

2. **Start the program under GDB** (second example above, recommended for debugging):
   ```bash
   $ gdb ./a.out
   (gdb) run
   # Press Ctrl-C when it hangs to interrupt
   ```

3. **Temporarily allow ptrace** (until reboot):
   ```bash
   $ sudo sysctl -w kernel.yama.ptrace_scope=0
   # Now you can attach to running processes without sudo
   ```

### Preventing excessive memory usage

Sometimes a programming error will cause a process to allocate huge amounts of memory, consuming all the ram on a system. To prevent this, the GNU Bash command ulimit -v
limit can be used to restrict the amount of virtual memory available to each process.
For example,
```bash
$ ulimit -v 4096
```
will limit subsequent processes to 4 megabytes of virtual memory (4096k).

An artificially low limit can be used to simulate running out of memory
— a well-written program should not crash in this case.

## Compiler-related tools

### Creating a library with the GNU archiver

The GNU archiver **ar** combines a collection of object files into a single archive file, also
known as a library.

The first object file will be generated from the source code for the hello function, in
the file [hello_fn.c](10_Tools/hello_fn.c) seen earlier.
The second object file will be generated from the source file
[bye_fn.c](10_Tools/bye_fn.c), which contains the new function bye.

Both functions use the header file [hello.h](10_Tools/hello.h), now with a prototype for the function **bye()**.

The source code can be compiled to the object files ‘hello_fn.o’ and ‘bye_fn.o’ using the
commands:
```bash
$ gcc -Wall -c hello_fn.c
$ gcc -Wall -c bye_fn.c
```
These object files can be combined into a static library using the following command line:
```bash
$ ar cr libhello.a hello_fn.o bye_fn.o
```
The option ‘cr’ stands for “create and replace”.

The archiver **ar** also provides a “table of contents” option ‘t’ to list the object files in an existing library:
```bash
$ ar t libhello.a
hello_fn.o
bye_fn.o
```
Note that when a library is distributed, the header files for the public functions and variables
it provides should also be made available, so that the end-user can include them and obtain the correct prototypes.

We can now write [main.c](10_Tools/main.c) using the functions in the newly created library.

This file can be compiled with the following command line,
assuming the library ‘libhello.a’ is stored in the current directory:
```bash
$ gcc -Wall main.c libhello.a -o hello
```

The short-cut library linking option '-l' can also be used to link the program, without needing to specify the full filename of the library explicitly:
```bash
$ gcc -Wall -L. main.c -lhello -o hello
```
The option '-L.' is needed to add the current directory to the library search path.

The resulting executable can be run as usual:
```bash
$ ./hello
Hello, world!
Goodbye!
```

### Using the profiler gprof
The GNU profiler **gprof** is a useful tool for measuring the performance
of a program - it records the number of calls to each function and the amount of time spent there, on a perfunction basis.

We will use **gprof** to examine the performance of a small numerical program which computes the lengths of sequences occurring in the unsolved Collatz conjecture in mathematics.
The Collatz conjecture involves sequences defined by the rule:
$$
x_{n+1} \leftarrow
\begin{cases}
x_n / 2 & \text{if } x_n \text{ even} \\
3x_n + 1 & \text{if } x_n \text{ odd}
\end{cases}
$$

The sequence is iterated from an initial value x0 until it terminates with the value 1.
According to the conjecture, all sequences do terminate eventually - the program below displays the longest sequences as x0 increases.
The source file [collatz.c](10_Tools/collatz.c)
contains three functions: main, nseq and step.

To use profiling, the program must be compiled and linked with the '-pg' profiling option:
```bash
$ gcc -Wall -c -pg collatz.c
$ gcc -Wall -pg collatz.o
```
This creates an instrumented executable which contains additional instructions that record
the time spent in each function.

The executable must be run to create the profiling data:
```bash
$ ./a.out
(normal program output is displayed)
```
Profiling data is silently written to a file
‘gmon.out’ in the current directory. It can be analyzed with gprof by giving the name
of the executable as an argument:
```bash
$ gprof a.out
Flat profile:

Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total
 time   seconds   seconds    calls  ns/call  ns/call  name
 50.41      1.24     1.24 740343580     1.67     1.67  step
 34.55      2.09     0.85  5000000   170.00   418.00  stepn
 12.60      2.40     0.31                             _init
  2.44      2.46     0.06                             main
```
The first column of the data shows that the program spends most of its time
in the function step (50%), and 35% in stepn. Consequently efforts to decrease the run-time of
the program should concentrate on these hot functions.

#### Optimizing based on the profile

Guided by the profiling data, I created an optimized version [collatz2.c](10_Tools/collatz2.c) with the following changes:

1. **Simplified the loop condition in `stepn`** — changed `while (x != 1 && x != 0)` to `while (x != 1)`, eliminating one comparison per iteration. Since this loop body runs ~740 million times, removing a branch from the inner loop is significant.

2. **Moved boundary handling out of `stepn` into `main`** — the checks for `x0 == 0` and `x0 == 1` were removed from `stepn` and instead `main` handles these two trivial cases with direct `printf` calls. This removes a branch that was evaluated on every one of the 5 million calls to `stepn`.

3. **Used a ternary expression in `step`** — replaced the `if/else` with a ternary `(x % 2 == 0) ? x / 2 : 3 * x + 1`. This is a stylistic change; the compiler likely generates equivalent code.

```bash
$ gprof a.out
Flat profile:

Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total
 time   seconds   seconds    calls  ns/call  ns/call  name
 59.17      1.29     1.29 740343580     1.74     1.74  step
 22.48      1.78     0.49  4999998    98.00   356.00  stepn
 14.22      2.09     0.31                             _init
  4.13      2.18     0.09                             main
```

**Key takeaways:**

- The biggest win came from simplifying the `stepn` inner loop condition. Removing the redundant `x != 0` check saved ~0.36 s — nearly 42% of `stepn`'s original time. This makes sense: the check ran on every one of the ~740 million iterations of the inner loop.
- The number of `step` calls is identical (740,343,580) because the actual Collatz sequences for $k \geq 2$ are unchanged — we only changed *how* we loop, not *what* we compute.
- After optimization, `step` now dominates even more (59% vs 50%). The profile tells us that further improvement would require optimizing `step` itself — for example, replacing the modulo operation with a bitwise test (`x & 1`), which the compiler may or may not already do.
- Profiling first, then optimizing the measured bottleneck, is far more effective than guessing. The book's advice to "concentrate on the former" (the hottest function) is validated here.

#### Trying to outsmart the compiler

Following the profile's hint, I created [collatz3.c](10_Tools/collatz3.c) with `static inline` on `step` and bitwise ops (`x & 1`, `x >> 1`) instead of `%` and `/`:

| Build | collatz2 | collatz3 | Speedup |
|---|---|---|---|
| `-O0` | 2.03 s | 1.97 s | ~3% |
| `-O2` | 1.22 s | 1.16 s | ~5% |

At `-O2` the difference nearly vanishes — the compiler already replaces `x % 2` with a bitwise test and `x / 2` with a shift, and inlines small functions on its own. Old fact: don't try to be smart, the compiler is smarter.


### Coverage testing with gcov

The GNU coverage testing tool **gcov** analyses the number of times each line of a program
is executed during a run.

We will use [cov.c](10_Tools/cov.c) to demonstrate gcov. This program loops overs
the integers 1 to 9 and tests their divisibility with the modulus (%) operator.

To enable coverage testing the program must be compiled with the following options (without optimization):
```bash
$ gcc -Wall -c --coverage cov.c
$ gcc -Wall --coverage cov.o
```
The `--coverage` flag is needed at both stages: the compile step (`-c`) instruments the code
and generates a `.gcno` graph file describing the program structure, while the link step adds
the `gcov` runtime library that writes coverage data on exit.
The executable must then be run to create the coverage
data:
```bash
$ ./a.out
3 is divisable by 3
6 is divisable by 3
9 is divisable by 3
```
The data from the run is written to a `.gcda` (data) file in the current directory.
Together with the `.gcno` (graph) file produced at compile time, it records arc transition
counts, value profile counts, and some summary information.
This data can be analyzed using the gcov command and the name of a source file:
```bash
$ gcov cov.c
File 'cov.c'
Lines executed:85.71% of 7
Creating 'cov.c.gcov'

Lines executed:85.71% of 7
```
The gcov command produces an annotated version of the original source file, with the file
extension ‘.gcov’, containing counts of the number of times each line was executed:
```bash
$ cat cov.c.gcov
        -:    0:Source:cov.c
        -:    0:Graph:cov.gcno
        -:    0:Data:cov.gcda
        -:    0:Runs:1
        -:    1:#include <stdio.h>
        -:    2:
        1:    3:int main(void)
        -:    4:{
       10:    5:    for (int i = 1; i < 10; i++) {
        9:    6:        if (i % 3 == 0)
        3:    7:            printf("%d is divisable by 3\n", i);
        9:    8:        if (i % 11 == 0)
    #####:    9:            printf("%d is divisable by 11\n", i);
        -:   10:    }
        1:   11:    return 0;
        -:   12:}
```
The line counts can be seen in the first column of the output. Lines which were not executed
are marked with hashes ‘######’.

## How the compiler works

This chapter describes in more detail how GCC transforms source files to an executable file.
Compilation is a multi-stage process involving several tools, including the GNU Compiler
itself, the GNU Assembler as, and the GNU Linker ld.

### An overview of the compilation process

The sequence of commands executed by a single invocation of GCC consists of the following stages:
* preprocessing (to expand macros)
* compilation (from source code to assembly language)
* assembly (from assembly language to machine code)
* linking (to create the final executable)

As an example, we will examine these compilation stages individually using the Hello World
program [hello.c](11_Compiler/hello.c).

Although the Hello World program is very simple it uses external header files and libraries, and so exercises all the major steps of the compilation process.

### The preprocessor

The first stage of the compilation process is the use of the preprocessor to expand macros
and included header files. To perform this stage, GCC executes the following command:
```bash
$ cpp hello.c > hello.i
```
The result is a file [hello.i](11_Compiler/hello.i) which contains the source code with all macros expanded.
In practice, the preprocessed file is not saved to disk unless the
‘-save-temps’ option is used.

### The compiler

The next stage of the process is the actual compilation of preprocessed source code to
assembly language, for a specific processor. The command-line option ‘-S’ instructs gcc to
convert the preprocessed C source code to assembly language without creating an object
file:
```bash
$ gcc -Wall -S hello.i
```
The resulting assembly language is stored in the file ‘hello.s’. Here is what the Hello
World assembly language for an Intel x86 (i5-1135G7) processor looks like:
```bash
$ cat hello.s
        .file   "hello.c"
        .text
        .section        .rodata
.LC0:
        .string "Hello World!"
        .text
        .globl  main
        .type   main, @function
main:
.LFB0:
        .cfi_startproc
        pushq   %rbp
        .cfi_def_cfa_offset 16
        .cfi_offset 6, -16
        movq    %rsp, %rbp
        .cfi_def_cfa_register 6
        leaq    .LC0(%rip), %rax
        movq    %rax, %rdi
        call    puts@PLT
        movl    $0, %eax
        popq    %rbp
        .cfi_def_cfa 7, 8
        ret
        .cfi_endproc
.LFE0:
        .size   main, .-main
        .ident  "GCC: (GNU) 15.2.1 20260103"
        .section        .note.GNU-stack,"",@progbits
```
Note that the assembly language contains a call to the external function puts (GCC automatically
replaces `printf("...\n")` with the simpler `puts("...")` when no format specifiers are used).

### The assembler

The purpose of the assembler is to convert assembly language into machine code and generate an object file. When there are calls to external functions in the assembly source file,
the assembler leaves the addresses of the external functions undefined, to be filled in later
by the linker. The assembler can be invoked with the following command line:
```bash
$ as hello.s -o hello.o
```
As with GCC, the output file is specified with the ‘-o’ option. The resulting file ‘hello.o’
contains the machine instructions for the Hello World program, with an undefined reference
to printf.

### The linker

The final stage of compilation is the linking of object files to create an executable. In
practice, an executable requires many external functions from system and C run-time (crt)
libraries. Consequently, the actual link commands used internally by GCC are complicated.
For example, on a modern 64-bit system (GCC 15, x86-64):
```bash
$ ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 -pie \
    /usr/lib/Scrt1.o /usr/lib/crti.o \
    /usr/lib/gcc/x86_64-pc-linux-gnu/15.2.1/crtbeginS.o \
    -L/usr/lib/gcc/x86_64-pc-linux-gnu/15.2.1 -L/usr/lib \
    hello.o \
    -lgcc --as-needed -lgcc_s --no-as-needed -lc \
    /usr/lib/gcc/x86_64-pc-linux-gnu/15.2.1/crtendS.o \
    /usr/lib/crtn.o -o hello
```
Notable differences: 64-bit dynamic linker (`ld-linux-x86-64.so.2`), PIE startup files
(`Scrt1.o`, `crtbeginS.o`) because `--enable-default-pie` is now standard, and `-pie` flag
for position-independent executables.

Fortunately there is never any need to type the command above directly—the entire linking
process is handled transparently by gcc when invoked as follows:
```bash
$ gcc hello.o
```
This links the object file ‘hello.o’ to the C standard library, and produces an executable
file ‘a.out’:
```bash
$ ./a.out
Hello World!
```








