/*
In the C and C++ programming languages, an #include guard, sometimes called a macro guard, header guard or file guard, 
is a particular construct used to avoid the problem of double inclusion when dealing with the include directive.

The C preprocessor processes directives of the form #include <file> in a source file by locating the associated file on 
disk and transcluding ("including") its contents into a copy of the source file known as the translation unit, replacing 
the include directive in the process. The files included in this regard are generally header files, which typically contain 
declarations of functions and classes or structs.

If an #include directive for a given file appears multiple times during compilation (i.e. because it appeared in multiple 
other headers), the file is processed again each time. However, if certain C or C++ language constructs are defined twice, 
the resulting translation unit is invalid. #include guards prevent this erroneous construct from arising by defining a 
preprocessor macro when a header is first included, and detecting its presence to skip the file's contents on subsequent inclusions.

The addition of #include guards to a header file is one way to make that file idempotent. Another construct to combat 
double inclusion is #pragma once, which is non-standard but nearly universally supported among C and C++ compilers.

from: https://en.wikipedia.org/wiki/Include_guard

*/


#ifndef MAIN_H
#define MAIN_H

#include <stm32f091xc.h>


#endif /* !MAIN_H */