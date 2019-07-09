#!/usr/bin/env bash
# Source this script to set up the BioDynaMo build that this script is part of.
#
# Conveniently an alias like this can be defined in .bashrc:
#   alias thisbdm=". bin/thisbdm.sh"
#
# This script if for the bash like shells, see thisbdm.csh for csh like shells.
#
# Author: Fons Rademakers, 16/5/2019

drop_from_path()
{
   # Assert that we got enough arguments
   if test $# -ne 2 ; then
      echo "drop_from_path: needs 2 arguments"
      return 1
   fi

   local p=$1
   local drop=$2

   newpath=`echo $p | sed -e "s;:${drop}:;:;g" \
                          -e "s;:${drop}\$;;g"   \
                          -e "s;^${drop}:;;g"   \
                          -e "s;^${drop}\$;;g"`
}

if [ -n "${BDMSYS}" ] ; then
   old_bdmsys=${BDMSYS}
fi

SOURCE=${BASH_ARGV[0]}
if [ "x$SOURCE" = "x" ]; then
    SOURCE=${(%):-%N} # for zsh
fi

if [ "x${SOURCE}" = "x" ]; then
    if [ -f bin/thisbdm.sh ]; then
        BDMSYS="$PWD"; export BDMSYS
    elif [ -f ./thisbdm.sh ]; then
        BDMSYS=$(cd ..  > /dev/null; pwd); export BDMSYS
    else
        echo ERROR: must "cd where/bdm/is" before calling ". bin/thisbdm.sh" for this version of bash!
        BDMSYS=; export BDMSYS
        return 1
    fi
else
    # get param to "."
    thisbdm=$(dirname ${SOURCE})
    BDMSYS=$(cd ${thisbdm}/.. > /dev/null;pwd); export BDMSYS
fi

# Clear the env from previously set BioDynaMo paths.
if [ -n "${old_bdmsys}" ] ; then
   if [ -n "${PATH}" ]; then
      drop_from_path "$PATH" "${old_bdmsys}/bin"
      PATH=$newpath
   fi
   if [ -n "${LD_LIBRARY_PATH}" ]; then
      drop_from_path "$LD_LIBRARY_PATH" "${old_bdmsys}/lib"
      LD_LIBRARY_PATH=$newpath
   fi
   if [ -n "${DYLD_LIBRARY_PATH}" ]; then
      drop_from_path "$DYLD_LIBRARY_PATH" "${old_bdmsys}/lib"
      DYLD_LIBRARY_PATH=$newpath
   fi
   if [ -n "${SHLIB_PATH}" ]; then
      drop_from_path "$SHLIB_PATH" "${old_bdmsys}/lib"
      SHLIB_PATH=$newpath
   fi
   if [ -n "${LIBPATH}" ]; then
      drop_from_path "$LIBPATH" "${old_bdmsys}/lib"
      LIBPATH=$newpath
   fi
   if [ -n "${MANPATH}" ]; then
      drop_from_path "$MANPATH" "${old_bdmsys}/man"
      MANPATH=$newpath
   fi
   if [ -n "${CMAKE_PREFIX_PATH}" ]; then
      drop_from_path "$CMAKE_PREFIX_PATH" "${old_bdmsys}"
      CMAKE_PREFIX_PATH=$newpath
   fi
fi

if [ -z "${MANPATH}" ]; then
   # Grab the default man path before setting the path to avoid duplicates
   if command -v manpath >/dev/null; then
      default_manpath=`manpath`
   elif command -v man >/dev/null; then
      default_manpath=`man -w 2> /dev/null`
   else
      default_manpath=""
   fi
fi

if [ -z "${PATH}" ]; then
   PATH=@CMAKE_INSTALL_BINDIR@; export PATH
else
   PATH=@CMAKE_INSTALL_BINDIR@:$PATH; export PATH
fi

if [ -z "${LD_LIBRARY_PATH}" ]; then
   LD_LIBRARY_PATH=@CMAKE_INSTALL_LIBDIR@; export LD_LIBRARY_PATH       # Linux, ELF HP-UX
else
   LD_LIBRARY_PATH=@CMAKE_INSTALL_LIBDIR@:$LD_LIBRARY_PATH; export LD_LIBRARY_PATH
fi

if [ -z "${DYLD_LIBRARY_PATH}" ]; then
   DYLD_LIBRARY_PATH=@CMAKE_INSTALL_LIBDIR@; export DYLD_LIBRARY_PATH   # Mac OS X
else
   DYLD_LIBRARY_PATH=@CMAKE_INSTALL_LIBDIR@:$DYLD_LIBRARY_PATH; export DYLD_LIBRARY_PATH
fi

if [ -z "${SHLIB_PATH}" ]; then
   SHLIB_PATH=@CMAKE_INSTALL_LIBDIR@; export SHLIB_PATH                 # legacy HP-UX
else
   SHLIB_PATH=@CMAKE_INSTALL_LIBDIR@:$SHLIB_PATH; export SHLIB_PATH
fi

if [ -z "${LIBPATH}" ]; then
   LIBPATH=@CMAKE_INSTALL_LIBDIR@; export LIBPATH                       # AIX
else
   LIBPATH=@CMAKE_INSTALL_LIBDIR@:$LIBPATH; export LIBPATH
fi

if [ -z "${MANPATH}" ]; then
   MANPATH=@mandir@:${default_manpath}; export MANPATH
else
   MANPATH=@mandir@:$MANPATH; export MANPATH
fi

if [ -z "${CMAKE_PREFIX_PATH}" ]; then
   CMAKE_PREFIX_PATH=$BDMSYS; export CMAKE_PREFIX_PATH       # Linux, ELF HP-UX
else
   CMAKE_PREFIX_PATH=$BDMSYS:$CMAKE_PREFIX_PATH; export CMAKE_PREFIX_PATH
fi

if [ "x`bdm-config --arch | grep -v win32gcc | grep -i win32`" != "x" ]; then
  BDMSYS="`cygpath -w $BDMSYS`"
fi

unset old_bdmsys
unset thisbdm
unset -f drop_from_path
