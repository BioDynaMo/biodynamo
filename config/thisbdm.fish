# Source this script to set up the BDM build that this script is part of.
#
# This script is for the fish shell.
#
# Author: Fons Rademakers, 16/5/2019

function update_path -d "Remove argv[2]argv[3] from argv[1] if argv[2], and prepend argv[4]"
   # Assert that we got enough arguments
   if test (count $argv) -ne 4
      echo "update_path: needs 4 arguments but have " (count $argv)
      return 1
   end

   set var $argv[1]

   set newpath $argv[4]
   for el in $$var
      if test "$argv[2]" = ""; or not test "$el" = "$argv[2]$argv[3]"
         set newpath $newpath $el
      end
   end

   set -xg $var $newpath
end

if set -q BDMSYS
   set old_bdmsys $BDMSYS
end

set SOURCE (status -f)
# normalize path
set thisbdm (dirname $SOURCE)
set -xg BDMSYS (set oldpwd $PWD; cd $thisbdm/.. > /dev/null;pwd;cd $oldpwd; set -e oldpwd)

if not set -q MANPATH
   # Grab the default man path before setting the path to avoid duplicates
   if which manpath > /dev/null ^ /dev/null
      set -xg MANPATH (manpath)
   else
      set -xg MANPATH (man -w 2> /dev/null)
   end
end

update_path PATH "$old_bdmsys" "/bin" @bindir@
update_path LD_LIBRARY_PATH "$old_bdmsys" "/lib" @libdir@
update_path DYLD_LIBRARY_PATH "$old_bdmsys" "/lib" @libdir@
update_path MANPATH "$old_bdmsys" "/man" @mandir@
update_path CMAKE_PREFIX_PATH "$old_bdmsys" "" $BDMSYS

functions -e update_path
set -e old_bdmsys
set -e thisbdm
set -e SOURCE
