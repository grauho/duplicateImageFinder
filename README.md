# DIF: Duplicate Image Finder program
This is a small toy program meant to demonstrate how hamming distances combined 
with simple image loading and resizing can be used to local similar images. 
This is a simplified version of how reverse image search works. 

# Synopsis

    ./difDemo [flags]... [images]...

# Building
A POSIX makefile has been included in this repository. To build the program
normally one need only invoke:

    make

Additionally, to list the various alternative targets and their information
one simply need invoke:

    make help

If on a non-POSIX compliant system the files may be built and linked together
manually at the command line. Be sure to disable the optional threading if
pthreads is not available through the use of the DIF\_DISABLE\_THREADING 
define. eg:

    cc -Wall -pedantic -O2 -DDIF_DISABLE_THREADING  -c -o main.o main.c
    cc -Wall -pedantic -O2 -DDIF_DISABLE_THREADING  -c -o stb_body.o stb_body.c
    cc -Wall -pedantic -O2 -DDIF_DISABLE_THREADING -o difDemo main.o stb_body.o -lpthread -lm


# Options

    -t, --threshold <NUM> : The threshold below which images are considered to 
        similar to one another. Allowed range is 1 to 64. Default is 5.

    -T, --threads <NUM>   : Number of threads the program should use for 
        loading and generating image fingerprints. Can be disabled by building
        the 'threadless' target. Default 5.

    -o, --output <PATH>   : Path to output found duplicate matches. Found 
        matches are always also printed to stdout so use of this flag is as
        if the program was redirected by a program like tee.

    -v, --verbose         : Enables extra output information. This extra info
        is printed to stdout and thus should not be used if one desires 
        strictly formatted output data.

    -h, --help            : Prints a short help message. 

# License
All the files in this repository unless otherwise noted are provided under the 
terms of the BSD 4-Clause License. A copy of this license can be found in the 
body of each of the header files and also in LICENSE.txt. 

The files in the 'thirdparty' directory are included under their own licenses.
Copies of which are found in the bodies of their respective files. 

# Bugs
Please report any bugs along with pertinent environmental information to the 
bugs section of this repository. Feature suggestions or requests are not bugs. 

# Authors
    Grauho <grauho@proton.me>

# Copyright 
Copyright (c) Grauho 2025, All Rights Reserved

# See Also
* [simpleHeaderLibraries] (https://github.com/grauho/simpleHeaderLibraries)
* [stb Image Libraries] (https://github.com/nothings/stb)
