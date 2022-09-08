# mprotect.m4 serial 2
dnl Copyright (C) 1993-2022 Free Software Foundation, Inc.
dnl This file is free software, distributed under the terms of the GNU
dnl General Public License as published by the Free Software Foundation;
dnl either version 2 of the License, or (at your option) any later version.
dnl As a special exception to the GNU General Public License, this file
dnl may be distributed as part of a program that contains a configuration
dnl script generated by Autoconf, under the same distribution terms as
dnl the rest of that program.

dnl Test whether mprotect() works.
dnl Sets gl_cv_func_mprotect_works and defines HAVE_WORKING_MPROTECT.

AC_DEFUN([gl_FUNC_MPROTECT_WORKS],
[
  AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles
  AC_REQUIRE([gl_FUNC_MMAP_ANON])

  AC_CHECK_FUNCS([mprotect])
  if test $ac_cv_func_mprotect = yes; then
    AC_CACHE_CHECK([for working mprotect], [gl_cv_func_mprotect_works],
      [if test $cross_compiling = no; then
         mprotect_prog='
           #include <sys/types.h>
           /* Declare malloc().  */
           #include <stdlib.h>
           /* Declare getpagesize().  */
           #if HAVE_UNISTD_H
            #include <unistd.h>
           #endif
           #ifdef __hpux
            extern
            #ifdef __cplusplus
            "C"
            #endif
            int getpagesize (void);
           #endif
           /* Declare mprotect().  */
           #include <sys/mman.h>
           char foo;
           int main ()
           {
             unsigned long pagesize = getpagesize ();
           #define page_align(address)  (char*)((unsigned long)(address) & -pagesize)
         '
         no_mprotect=
         AC_RUN_IFELSE(
           [AC_LANG_SOURCE([
              [$mprotect_prog
                 if ((pagesize - 1) & pagesize)
                   return 1;
                 return 0;
               }
              ]])
           ],
           [],
           [no_mprotect=1],
           [:])
         mprotect_prog="$mprotect_prog"'
           char* area = (char*) malloc (6 * pagesize);
           char* fault_address = area + pagesize*7/2;
         '
         if test -z "$no_mprotect"; then
           AC_RUN_IFELSE(
             [AC_LANG_SOURCE([
                GL_NOCRASH
                [$mprotect_prog
                   nocrash_init();
                   if (mprotect (page_align (fault_address), pagesize, PROT_NONE) < 0)
                     return 0;
                   foo = *fault_address; /* this should cause an exception or signal */
                   return 0;
                 }
                ]])
             ],
             [no_mprotect=1],
             [],
             [:])
         fi
         if test -z "$no_mprotect"; then
           AC_RUN_IFELSE(
             [AC_LANG_SOURCE([
                GL_NOCRASH
                [$mprotect_prog
                   nocrash_init();
                   if (mprotect (page_align (fault_address), pagesize, PROT_NONE) < 0)
                     return 0;
                   *fault_address = 'z'; /* this should cause an exception or signal */
                   return 0;
                 }
                ]])
             ],
             [no_mprotect=1],
             [],
             [:])
         fi
         if test -z "$no_mprotect"; then
           AC_RUN_IFELSE(
             [AC_LANG_SOURCE([
                GL_NOCRASH
                [$mprotect_prog
                   nocrash_init();
                   if (mprotect (page_align (fault_address), pagesize, PROT_READ) < 0)
                     return 0;
                   *fault_address = 'z'; /* this should cause an exception or signal */
                   return 0;
                 }
                ]])
             ],
             [no_mprotect=1],
             [],
             [:])
         fi
         if test -z "$no_mprotect"; then
           AC_RUN_IFELSE(
             [AC_LANG_SOURCE([
                GL_NOCRASH
                [$mprotect_prog
                   nocrash_init();
                   if (mprotect (page_align (fault_address), pagesize, PROT_READ) < 0)
                     return 1;
                   if (mprotect (page_align (fault_address), pagesize, PROT_READ | PROT_WRITE) < 0)
                     return 1;
                   *fault_address = 'z'; /* this should not cause an exception or signal */
                   return 0;
                 }
                ]])
             ],
             [],
             [no_mprotect=1],
             [:])
         fi
         if test -z "$no_mprotect"; then
           gl_cv_func_mprotect_works=yes
         else
           gl_cv_func_mprotect_works=no
         fi
       else
         dnl When cross-compiling, assume the known behaviour.
         case "$host_os" in
           dnl Guess yes on Linux systems, glibc systems,
           dnl macOS, BSD systems, AIX, HP-UX, IRIX, Solaris, Cygwin.
           linux-* | linux | *-gnu* | gnu* | \
           darwin* | freebsd* | dragonfly* | midnightbsd* | netbsd* | openbsd* | \
           aix* | hpux* | irix* | solaris* | cygwin*)
             gl_cv_func_mprotect_works="guessing yes" ;;
           mingw*)
             gl_cv_func_mprotect_works="guessing no" ;;
           *)
             dnl If we don't know, obey --enable-cross-guesses.
             gl_cv_func_mprotect_works="$gl_cross_guess_normal" ;;
         esac
       fi
      ])
    case "$gl_cv_func_mprotect_works" in
      *yes)
        AC_DEFINE([HAVE_WORKING_MPROTECT], [1],
          [have a working mprotect() function])
        ;;
    esac
  fi
])
