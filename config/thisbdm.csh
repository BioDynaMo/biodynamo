# Source this script to set up the BDM build that this script is part of.
#
# Conveniently an alias like this can be defined in ~/.cshrc:
#   alias thisbdm "source bin/thisbdm.sh"
#
# This script if for the csh like shells, see thisbdm.sh for bash like shells.
#
# Author: Fons Rademakers, 18/8/2006

if ($?BDMSYS) then
   if ($BDMSYS != "") then
     set old_bdmsys="$BDMSYS"
   endif
endif

# $_ should be source .../thisbdm.csh
set ARGS=($_)

set LSOF=`env PATH=/usr/sbin:${PATH} which lsof`
set thisfile="`${LSOF} -w +p $$ | grep -oE '/.*thisbdm.csh'  `"
if ( "$thisfile" == "" ) then
#   set thisfile=/does/not/exist
endif
if ( "$thisfile" != "" && -e ${thisfile} ) then
   # We found it, didn't we.
   set thisbdm="`dirname ${thisfile}`"
else if ("$ARGS" != "") then
   set thisbdm="`dirname ${ARGS[2]}`"
else
   # But $_ might not be set if the script is source non-interactively.
   # In [t]csh the sourced file is inserted 'in place' inside the
   # outer script, so we need an external source of information
   # either via the current directory or an extra parameter.
   if ( -e thisbdm.csh ) then
      set thisbdm=${PWD}
   else if ( -e bin/thisbdm.csh ) then
      set thisbdm=${PWD}/bin
   else if ( "$1" != "" ) then
      if ( -e ${1}/bin/thisbdm.csh ) then
         set thisbdm=${1}/bin
      else if ( -e ${1}/thisbdm.csh ) then
         set thisbdm=${1}
      else
         echo "thisbdm.csh: ${1} does not contain a BDM installation"
      endif
   else
      echo 'Error: The call to "source where_bdm_is/bin/thisbdm.csh" can not determine the location of the BDM installation'
      echo "because it was embedded another script (this is an issue specific to csh)."
      echo "Use either:"
      echo "   cd where_bdm_is; source bin/thisbdm.csh"
      echo "or"
      echo "   source where_bdm_is/bin/thisbdm.csh where_bdm_is"
   endif
endif

if ($?thisbdm) then

setenv BDMSYS "`(cd ${thisbdm}/..;pwd)`"

if ($?old_bdmsys) then
   setenv PATH `set DOLLAR='$'; echo $PATH | sed -e "s;:$old_bdmsys/bin:;:;g" \
                                 -e "s;:$old_bdmsys/bin${DOLLAR};;g"   \
                                 -e "s;^$old_bdmsys/bin:;;g"   \
                                 -e "s;^$old_bdmsys/bin${DOLLAR};;g"`
   if ($?LD_LIBRARY_PATH) then
      setenv LD_LIBRARY_PATH `set DOLLAR='$'; echo $LD_LIBRARY_PATH | \
                             sed -e "s;:$old_bdmsys/lib:;:;g" \
                                 -e "s;:$old_bdmsys/lib${DOLLAR};;g"   \
                                 -e "s;^$old_bdmsys/lib:;;g"   \
                                 -e "s;^$old_bdmsys/lib${DOLLAR};;g"`
   endif
   if ($?DYLD_LIBRARY_PATH) then
      setenv DYLD_LIBRARY_PATH `set DOLLAR='$'; echo $DYLD_LIBRARY_PATH | \
                             sed -e "s;:$old_bdmsys/lib:;:;g" \
                                 -e "s;:$old_bdmsys/lib${DOLLAR};;g"   \
                                 -e "s;^$old_bdmsys/lib:;;g"   \
                                 -e "s;^$old_bdmsys/lib${DOLLAR};;g"`
   endif
   if ($?SHLIB_PATH) then
      setenv SHLIB_PATH `set DOLLAR='$'; echo $SHLIB_PATH | \
                             sed -e "s;:$old_bdmsys/lib:;:;g" \
                                 -e "s;:$old_bdmsys/lib${DOLLAR};;g"   \
                                 -e "s;^$old_bdmsys/lib:;;g"   \
                                 -e "s;^$old_bdmsys/lib${DOLLAR};;g"`
   endif
   if ($?LIBPATH) then
      setenv LIBPATH `set DOLLAR='$'; echo $LIBPATH | \
                             sed -e "s;:$old_bdmsys/lib:;:;g" \
                                 -e "s;:$old_bdmsys/lib${DOLLAR};;g"   \
                                 -e "s;^$old_bdmsys/lib:;;g"   \
                                 -e "s;^$old_bdmsys/lib${DOLLAR};;g"`
   endif
   if ($?MANPATH) then
      setenv MANPATH `set DOLLAR='$'; echo $MANPATH | \
                             sed -e "s;:$old_bdmsys/man:;:;g" \
                                 -e "s;:$old_bdmsys/man${DOLLAR};;g"   \
                                 -e "s;^$old_bdmsys/man:;;g"   \
                                 -e "s;^$old_bdmsys/man${DOLLAR};;g"`
   endif
   if ($?CMAKE_PREFIX_PATH) then
      setenv CMAKE_PREFIX_PATH `set DOLLAR='$'; echo $CMAKE_PREFIX_PATH | \
                             sed -e "s;:${old_bdmsys}:;:;g" \
                                 -e "s;:${old_bdmsys}${DOLLAR};;g"   \
                                 -e "s;^${old_bdmsys}:;;g"   \
                                 -e "s;^${old_bdmsys}${DOLLAR};;g"`
   endif
endif


if ($?MANPATH) then
# Nothing to do
else
   # Grab the default man path before setting the path to avoid duplicates
   if ( -X manpath ) then
      set default_manpath = `manpath`
   else
      set default_manpath = `man -w`
   endif
endif

set path = (@bindir@ $path)

if ($?LD_LIBRARY_PATH) then
   setenv LD_LIBRARY_PATH @libdir@:$LD_LIBRARY_PATH      # Linux, ELF HP-UX
else
   setenv LD_LIBRARY_PATH @libdir@
endif

if ($?DYLD_LIBRARY_PATH) then
   setenv DYLD_LIBRARY_PATH @libdir@:$DYLD_LIBRARY_PATH  # Mac OS X
else
   setenv DYLD_LIBRARY_PATH @libdir@
endif

if ($?SHLIB_PATH) then
   setenv SHLIB_PATH @libdir@:$SHLIB_PATH                # legacy HP-UX
else
   setenv SHLIB_PATH @libdir@
endif

if ($?LIBPATH) then
   setenv LIBPATH @libdir@:$LIBPATH                      # AIX
else
   setenv LIBPATH @libdir@
endif

if ($?MANPATH) then
   setenv MANPATH `dirname @mandir@`:$MANPATH
else
   setenv MANPATH `dirname @mandir@`:$default_manpath
endif

if ($?CMAKE_PREFIX_PATH) then
   setenv CMAKE_PREFIX_PATH ${BDMSYS}:$CMAKE_PREFIX_PATH
else
   setenv CMAKE_PREFIX_PATH ${BDMSYS}
endif

endif # if ("$thisbdm" != "")

set thisbdm=
set old_bdmsys=

