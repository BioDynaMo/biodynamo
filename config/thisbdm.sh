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

if [ -n "${BDM_INSTALL_DIR}" ] ; then
   old_bdmsys_base=${BDM_INSTALL_DIR}
fi
if [ -n "${BDMSYS}" ] ; then
   old_bdmsys=${BDMSYS}
fi

BDM_INSTALL_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)
BDM_INSTALL_DIR=$(cd ${BDM_INSTALL_DIR}; cd ../../ > /dev/null; pwd); export BDM_INSTALL_DIR;
BDMSYS="${BDM_INSTALL_DIR}/biodynamo"; export BDMSYS;

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
   if [ -n "${BDM_CMAKE_DIR}" ]; then
      drop_from_path "$BDM_CMAKE_DIR" "${old_bdmsys}/share/cmake"
      BDM_CMAKE_DIR=$newpath
   fi
   if [ -n "${BDM_SRC_DIR}" ]; then
      drop_from_path "$BDM_SRC_DIR" "${old_bdmsys}/include"
      BDM_SRC_DIR=$newpath
   fi
fi

if [ -n "${old_bdmsys_base}" ] ; then
   if [ -n "${ParaView_DIR}" ]; then
      drop_from_path "$ParaView_DIR" "${old_bdmsys_base}/third_party/paraview/lib/cmake/paraview-5.6"
      ParaView_DIR=$newpath
   fi
   if [ -n "${ParaView_LIB_DIR}" ]; then
      drop_from_path "$ParaView_LIB_DIR" "${old_bdmsys_base}/third_party/paraview/lib"
      ParaView_LIB_DIR=$newpath
   fi
   if [ -n "${PYTHONPATH}" ]; then
      drop_from_path "$PYTHONPATH" "${old_bdmsys_base}/third_party/paraview/lib/python2.7/site-packages"
      PYTHONPATH=$newpath
   fi
   if [ -n "${PV_PLUGIN_PATH}" ]; then
      drop_from_path "$PV_PLUGIN_PATH" "${old_bdmsys_base}/biodynamo/lib/pv_plugin"
      PV_PLUGIN_PATH=$newpath
   fi
   if [ -n "${PATH}" ]; then
      drop_from_path "$PATH" "${old_bdmsys_base}/third_party/paraview/bin"
      PATH=$newpath
   fi
   if [ -n "${Qt5_DIR}" ]; then
      drop_from_path "$Qt5_DIR" "${old_bdmsys_base}/third_party/qt/lib/cmake/Qt5"
      Qt5_DIR=$newpath
   fi
   if [ -n "${QT_QPA_PLATFORM_PLUGIN_PATH}" ]; then
      drop_from_path "$QT_QPA_PLATFORM_PLUGIN_PATH" "${old_bdmsys_base}/third_party/qt/plugins"
      QT_QPA_PLATFORM_PLUGIN_PATH=$newpath
   fi
   if [ -n "${DYLD_LIBRARY_PATH}" ]; then
      drop_from_path "$DYLD_LIBRARY_PATH" "${old_bdmsys_base}/third_party/paraview/lib"
      DYLD_LIBRARY_PATH=$newpath
      drop_from_path "$DYLD_LIBRARY_PATH" "${old_bdmsys_base}/third_party/qt/lib"
      DYLD_LIBRARY_PATH=$newpath
   fi
   if [ -n "${LD_LIBRARY_PATH}" ]; then
      drop_from_path "$LD_LIBRARY_PATH" "${old_bdmsys_base}/third_party/paraview/lib"
      LD_LIBRARY_PATH=$newpath
      drop_from_path "$LD_LIBRARY_PATH" "${old_bdmsys_base}/third_party/qt/lib"
      LD_LIBRARY_PATH=$newpath
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
   PATH="${BDMSYS}/bin"; export PATH
else
   PATH="${BDMSYS}/bin":$PATH; export PATH
fi

if [ -z "${LD_LIBRARY_PATH}" ]; then
   LD_LIBRARY_PATH="${BDMSYS}/lib"; export LD_LIBRARY_PATH       # Linux, ELF HP-UX
else
   LD_LIBRARY_PATH="${BDMSYS}/lib":$LD_LIBRARY_PATH; export LD_LIBRARY_PATH
fi

if [ -z "${DYLD_LIBRARY_PATH}" ]; then
   DYLD_LIBRARY_PATH="${BDMSYS}/lib"; export DYLD_LIBRARY_PATH   # Mac OS X
else
   DYLD_LIBRARY_PATH="${BDMSYS}/lib":$DYLD_LIBRARY_PATH; export DYLD_LIBRARY_PATH
fi

if [ -z "${SHLIB_PATH}" ]; then
   SHLIB_PATH="${BDMSYS}/lib"; export SHLIB_PATH                 # legacy HP-UX
else
   SHLIB_PATH="${BDMSYS}/lib":$SHLIB_PATH; export SHLIB_PATH
fi

if [ -z "${LIBPATH}" ]; then
   LIBPATH="${BDMSYS}/lib"; export LIBPATH                       # AIX
else
   LIBPATH="${BDMSYS}/lib":$LIBPATH; export LIBPATH
fi

if [ -z "${MANPATH}" ]; then
   MANPATH=@mandir@:${default_manpath}; export MANPATH
else
   MANPATH=@mandir@:$MANPATH; export MANPATH
fi

##### CMake Specific Configurations #####
if [ -z "${CMAKE_PREFIX_PATH}" ]; then
   CMAKE_PREFIX_PATH="${BDMSYS}/cmake"; export CMAKE_PREFIX_PATH       # Linux, ELF HP-UX
else
   CMAKE_PREFIX_PATH="${BDMSYS}/cmake":$CMAKE_PREFIX_PATH; export CMAKE_PREFIX_PATH
fi

BDM_CMAKE_DIR="${BDMSYS}/share/cmake"; export BDM_CMAKE_DIR
BDM_SRC_DIR="${BDMSYS}/include"; export BDM_SRC_DIR
########

#### ROOT Specific Configurations ####
. ${BDM_INSTALL_DIR}/third_party/root/bin/thisroot.sh
export ROOT_INCLUDE_PATH="${ROOT_INCLUDE_PATH:+${ROOT_INCLUDE_PATH}:}${BDMSYS}/include"
########


#### ParaView Specific Configurations ####
if [ -z "${ParaView_DIR}" ]; then
   ParaView_DIR="${BDM_INSTALL_DIR}/third_party/paraview/lib/cmake/paraview-5.6"; export ParaView_DIR
else
   ParaView_DIR="${BDM_INSTALL_DIR}/third_party/paraview/lib/cmake/paraview-5.6":$ParaView_DIR; export ParaView_DIR
fi

if [ -z "${ParaView_LIB_DIR}" ]; then
   ParaView_LIB_DIR="${BDM_INSTALL_DIR}/third_party/paraview/lib"; export ParaView_LIB_DIR
else
   ParaView_LIB_DIR="${BDM_INSTALL_DIR}/third_party/paraview/lib":$ParaView_LIB_DIR; export ParaView_LIB_DIR
fi

if [ -z "${PYTHONPATH}" ]; then
   PYTHONPATH="${BDM_INSTALL_DIR}/third_party/paraview/lib/python2.7/site-packages"; export PYTHONPATH
else
   PYTHONPATH="${BDM_INSTALL_DIR}/third_party/paraview/lib/python2.7/site-packages":$PYTHONPATH; export PYTHONPATH
fi

if [ -z "${PV_PLUGIN_PATH}" ]; then
   PV_PLUGIN_PATH="${BDM_INSTALL_DIR}/biodynamo/lib/pv_plugin"; export PV_PLUGIN_PATH
else
   PV_PLUGIN_PATH="${BDM_INSTALL_DIR}/biodynamo/lib/pv_plugin":$PV_PLUGIN_PATH; export PV_PLUGIN_PATH
fi

if [ -z "${PATH}" ]; then
   PATH="${BDM_INSTALL_DIR}/third_party/paraview/bin"; export PATH
else
   PATH="${BDM_INSTALL_DIR}/third_party/paraview/bin":$PATH; export PATH
fi

if [ -z "${LD_LIBRARY_PATH}" ]; then
   LD_LIBRARY_PATH="${ParaView_LIB_DIR}"; export LD_LIBRARY_PATH
else
   LD_LIBRARY_PATH="${ParaView_LIB_DIR}":$LD_LIBRARY_PATH; export LD_LIBRARY_PATH
fi

if [ -z "${DYLD_LIBRARY_PATH}" ]; then
   DYLD_LIBRARY_PATH="${ParaView_LIB_DIR}"; export DYLD_LIBRARY_PATH
else
   DYLD_LIBRARY_PATH="${ParaView_LIB_DIR}":$DYLD_LIBRARY_PATH; export DYLD_LIBRARY_PATH
fi
########

#### Qt5 Specific Configurations ####
if [ -z "${Qt5_DIR}" ]; then
   Qt5_DIR="${BDM_INSTALL_DIR}/third_party/qt/lib/cmake/Qt5"; export Qt5_DIR
else
   Qt5_DIR="${BDM_INSTALL_DIR}/third_party/qt/lib/cmake/Qt5":$Qt5_DIR; export Qt5_DIR
fi

if [ -z "${QT_QPA_PLATFORM_PLUGIN_PATH}" ]; then
   QT_QPA_PLATFORM_PLUGIN_PATH="${BDM_INSTALL_DIR}/third_party/qt/plugins"; export QT_QPA_PLATFORM_PLUGIN_PATH
else
   QT_QPA_PLATFORM_PLUGIN_PATH="${BDM_INSTALL_DIR}/third_party/qt/plugins":$QT_QPA_PLATFORM_PLUGIN_PATH; export QT_QPA_PLATFORM_PLUGIN_PATH
fi

if [ -z "${LD_LIBRARY_PATH}" ]; then
   LD_LIBRARY_PATH="${BDM_INSTALL_DIR}/third_party/qt/lib"; export LD_LIBRARY_PATH
else
   LD_LIBRARY_PATH="${BDM_INSTALL_DIR}/third_party/qt/lib":$LD_LIBRARY_PATH; export LD_LIBRARY_PATH
fi

if [ -z "${DYLD_LIBRARY_PATH}" ]; then
   DYLD_LIBRARY_PATH="${BDM_INSTALL_DIR}/third_party/qt/lib"; export DYLD_LIBRARY_PATH
else
   DYLD_LIBRARY_PATH="${BDM_INSTALL_DIR}/third_party/qt/lib":$DYLD_LIBRARY_PATH; export DYLD_LIBRARY_PATH
fi

#######

#### MKDocs Specific Configurations ####
export PATH=$PATH:~/.local/bin
export PYTHONPATH=~/.local/lib/python2.7/site-packages:$PYTHONPATH
export PATH=$PATH:~/Library/Python/2.7/bin
########

if command -v g++-5 &>/dev/null; then
  export CC=gcc-5
  export CXX=g++-5
else
  export CC=gcc
  export CXX=g++
fi

# OpenMP
export OMP_PROC_BIND=true

#if [ "x`bdm-config --arch | grep -v win32gcc | grep -i win32`" != "x" ]; then
#  BDMSYS="`cygpath -w $BDM_INSTALL_DIR`"
#fi

# Apple specific
if [[ $(uname -s) == "Darwin"* ]]; then

    # Remove previous LLVM path
    if [ -n "${LLVMDIR}" ] ; then
        old_llvmdir=${LLVMDIR}
    fi
    if [ -n "${old_llvmdir}" ]; then
      if [ -n "${PATH}" ]; then
        drop_from_path "$PATH" "${old_llvmdir}/bin"
        PATH=$newpath
      fi
    fi
    unset old_llvmdir
    
    export LLVMDIR="/usr/local/opt/llvm"
    export CC=$LLVMDIR/bin/clang
    export CXX=$LLVMDIR/bin/clang++
    export CXXFLAGS=-I$LLVMDIR/include
    export LDFLAGS=-L$LLVMDIR/lib
    export PATH=$LLVMDIR/bin:$PATH
else
    # CentOs specifics
    if [ `lsb_release -si` == "CentOS" ]; then
        export MESA_GL_VERSION_OVERRIDE=3.3
        . scl_source enable devtoolset-7
        . scl_source enable rh-python36

        . /etc/profile.d/modules.sh
        module load mpi
    fi
fi

unset old_bdmsys
unset old_bdmsym_base
unset thisbdm
unset -f drop_from_path

echo "You have successfully sourced BioDynaMo's environment."
