#!/usr/bin/env bash
# -----------------------------------------------------------------------------
#
# Copyright (C) The BioDynaMo Project.
# All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
#
# See the LICENSE file distributed with this work for details.
# See the NOTICE file distributed with this work for additional information
# regarding copyright ownership.
#
# -----------------------------------------------------------------------------
# Source this script to set up the BioDynaMo build that this script is part of.
#
# Conveniently an alias like this can be defined in .bashrc or .zshrc:
#   alias thisbdm="source path/to/biodynamo/bin/thisbdm.sh -q"
#
# This script is for bash like shells, see thisbdm.fish for fish.
#
# Author: Fons Rademakers, 16/5/2019

_bdm_quiet=false

_bdm_err()
{
  echo -e "\e[31m$*\e[0m"
}

_bdm_ok()
{
  $_bdm_quiet || echo -e "\e[32m$*\e[0m"
}

_drop_bdm_from_path()
{
  # Assert that we got enough arguments
  if test $# -ne 2 ; then
    _bdm_err "[ERR] drop_bdm_from_path: needs 2 arguments"
    return 1
  fi

  local p=$1
  local drop=$2

  _newpath=$(echo "$p" | sed -e "s;:${drop}:;:;g"\
                             -e "s;:${drop}\$;;g"\
                             -e "s;^${drop}:;;g" \
                             -e "s;^${drop}\$;;g")
}

_bdm_define_command()
{
  unset -f "$1" >/dev/null 2>&1 || true
  unalias  "$1" >/dev/null 2>&1 || true

  if [ -n  "$ZSH_VERSION" ]; then
    autoload -Uz "$1"
  else
    # shellcheck disable=SC1090
    source "${BDMSYS}/bin/sh_functions/$1"
    export -f "${1?}"
  fi
}

_bdm_unset()
{
  unset "$@" >/dev/null 2>&1 || true
}

_thisbdm_cleanup() {
  _bdm_unset thisbdm
  _bdm_unset _bdm_quiet
  _bdm_unset _newpath
  _bdm_unset -f _drop_bdm_from_path
  _bdm_unset -f _source_thisbdm
  _bdm_unset -f _bdm_define_command
  _bdm_unset -f _bdm_err
  _bdm_unset -f _bdm_ok
  unset -f _bdm_unset >/dev/null 2>&1 || true
  unset -f _thisbdm_cleanup >/dev/null 2>&1 || true
}

_source_thisbdm()
{
  ### Detect bash-like shell, and act accordingly ###
  local bdm_shell
  if [ -n "$BASH_VERSION" ]; then
    bdm_shell="bash"
  elif [ -n "$ZSH_VERSION" ]; then
    bdm_shell="zsh"
    # The reason why this script is wrapped in this god function
    # is to enable bash compatibility *only* inside its respective
    # scope. Local vars are a nice bonus.
    emulate -LR sh
  else
    # In case we encounter a shell (e.g., ksh) that can run this script up to here.
    # In reality, most shells (csh, fish, etc.) will fail immediately due to parse errors.
    _bdm_err "[ERR] Your shell is not supported, please use bash or zsh."
    _bdm_err "      For fish, please source 'thisbdm.fish' instead."
    return 1
  fi
  ########

  ### Arguments ###
  while [[ $# -gt 0 ]]; do
    local key="$1"
    case $key in
        -q|--quiet)
        _bdm_quiet=true
        shift # next key
        ;;
        *)
        _bdm_err "[ERR] Unknown option $key"
        return 1
        ;;
    esac
  done
  ########

  if [ -n "${BDM_INSTALL_DIR}" ]; then
     local old_bdmsys_base=${BDM_INSTALL_DIR}
  fi
  if [ -n "${BDMSYS}" ]; then
     local old_bdmsys=${BDMSYS}
  fi

  case $bdm_shell in
    bash)
      BDM_INSTALL_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
      ;;
    zsh)
      # The zsh equivalent of ${BASH_SOURCE[0]} is ${(%):-%x}
      # shellcheck disable=SC2154
      BDM_INSTALL_DIR=$(cd "$(dirname "${(%):-%x}")/.." && pwd)
      ;;
  esac

  export BDM_INSTALL_DIR
  export BDMSYS="${BDM_INSTALL_DIR}"

  # Clear the env from previously set BioDynaMo paths.
  if [ -n "${old_bdmsys}" ] ; then
     if [ -n "${PATH}" ]; then
      _drop_bdm_from_path "$PATH" "${old_bdmsys}/bin"
      PATH=$_newpath
     fi
     if [ -n "${LD_LIBRARY_PATH}" ]; then
      _drop_bdm_from_path "$LD_LIBRARY_PATH" "${old_bdmsys}/lib"
      LD_LIBRARY_PATH=$_newpath
     fi
     if [ -n "${DYLD_LIBRARY_PATH}" ]; then
      _drop_bdm_from_path "$DYLD_LIBRARY_PATH" "${old_bdmsys}/lib"
      DYLD_LIBRARY_PATH=$_newpath
     fi
     if [ -n "${SHLIB_PATH}" ]; then
      _drop_bdm_from_path "$SHLIB_PATH" "${old_bdmsys}/lib"
      SHLIB_PATH=$_newpath
     fi
     if [ -n "${LIBPATH}" ]; then
      _drop_bdm_from_path "$LIBPATH" "${old_bdmsys}/lib"
      LIBPATH=$_newpath
     fi
     if [ -n "${MANPATH}" ]; then
      _drop_bdm_from_path "$MANPATH" "${old_bdmsys}/man"
      MANPATH=$_newpath
     fi
     if [ -n "${CMAKE_PREFIX_PATH}" ]; then
      _drop_bdm_from_path "$CMAKE_PREFIX_PATH" "${old_bdmsys}"
      CMAKE_PREFIX_PATH=$_newpath
     fi
  fi

  if [ -n "${old_bdmsys_base}" ]; then
     if [ -n "${ParaView_DIR}" ]; then
      _drop_bdm_from_path "$ParaView_DIR" "${old_bdmsys_base}/third_party/paraview/lib/cmake/paraview-5.8"
      ParaView_DIR=$_newpath
     fi
     if [ -n "${ParaView_LIB_DIR}" ]; then
      _drop_bdm_from_path "$ParaView_LIB_DIR" "${old_bdmsys_base}/third_party/paraview/lib"
      ParaView_LIB_DIR=$_newpath
     fi
     if [ -n "${PV_PLUGIN_PATH}" ]; then
      _drop_bdm_from_path "$PV_PLUGIN_PATH" "${old_bdmsys_base}/biodynamo/lib/pv_plugin"
      PV_PLUGIN_PATH=$_newpath
     fi
     if [ -n "${PATH}" ]; then
      _drop_bdm_from_path "$PATH" "${old_bdmsys_base}/third_party/paraview/bin"
      PATH=$_newpath
     fi
     if [ -n "${Qt5_DIR}" ]; then
      _drop_bdm_from_path "$Qt5_DIR" "${old_bdmsys_base}/third_party/qt/lib/cmake/Qt5"
      Qt5_DIR=$_newpath
     fi
     if [ -n "${QT_QPA_PLATFORM_PLUGIN_PATH}" ]; then
      _drop_bdm_from_path "$QT_QPA_PLATFORM_PLUGIN_PATH" "${old_bdmsys_base}/third_party/qt/plugins"
      QT_QPA_PLATFORM_PLUGIN_PATH=$_newpath
     fi
     if [ -n "${DYLD_LIBRARY_PATH}" ]; then
      _drop_bdm_from_path "$DYLD_LIBRARY_PATH" "${old_bdmsys_base}/third_party/paraview/lib"
      DYLD_LIBRARY_PATH=$_newpath
      _drop_bdm_from_path "$DYLD_LIBRARY_PATH" "${old_bdmsys_base}/third_party/qt/lib"
      DYLD_LIBRARY_PATH=$_newpath
     fi
     if [ -n "${LD_LIBRARY_PATH}" ]; then
      _drop_bdm_from_path "$LD_LIBRARY_PATH" "${old_bdmsys_base}/third_party/paraview/lib"
      LD_LIBRARY_PATH=$_newpath
      _drop_bdm_from_path "$LD_LIBRARY_PATH" "${old_bdmsys_base}/third_party/qt/lib"
      LD_LIBRARY_PATH=$_newpath
     fi
  fi
  #########

  if [ -z "${MANPATH}" ]; then
     local default_manpath
     # Grab the default man path before setting the path to avoid duplicates
     if command -v manpath >/dev/null; then
      default_manpath=$(manpath)
     elif command -v man >/dev/null; then
      default_manpath=$(man -w 2> /dev/null)
     else
      default_manpath=''
     fi
  fi

  if [ -z "${PATH}" ]; then
    PATH="${BDMSYS}/bin"
  else
    PATH="${BDMSYS}/bin":$PATH
  fi
  export PATH

  if [ -z "${LD_LIBRARY_PATH}" ]; then
    LD_LIBRARY_PATH="${BDMSYS}/lib" # Linux, ELF HP-UX
  else
    LD_LIBRARY_PATH="${BDMSYS}/lib":$LD_LIBRARY_PATH
  fi
  export LD_LIBRARY_PATH

  if [ -z "${DYLD_LIBRARY_PATH}" ]; then
    DYLD_LIBRARY_PATH="${BDMSYS}/lib"; # Mac OS X
  else
    DYLD_LIBRARY_PATH="${BDMSYS}/lib":$DYLD_LIBRARY_PATH
  fi
  export DYLD_LIBRARY_PATH

  if [ -z "${SHLIB_PATH}" ]; then
    SHLIB_PATH="${BDMSYS}/lib" # legacy HP-UX
  else
    SHLIB_PATH="${BDMSYS}/lib":$SHLIB_PATH
  fi
  export SHLIB_PATH

  if [ -z "${LIBPATH}" ]; then
    LIBPATH="${BDMSYS}/lib" # AIX
  else
    LIBPATH="${BDMSYS}/lib":$LIBPATH
  fi
  export LIBPATH

  if [ -z "${MANPATH}" ]; then
    MANPATH="${BDMSYS}/man":${default_manpath}
  else
    MANPATH="${BDMSYS}/man":$MANPATH
  fi
  export MANPATH

  ##### Python Specific Configurations #####
  export PYENV_ROOT=@pyenvroot@
  if [ -z "${PYENV_ROOT}" ]; then
    export PYENV_ROOT="$HOME/.pyenv"
  fi
  export PATH="$PYENV_ROOT/bin:$PATH"

  # FIXME Some paths are (ap/pre)pended n times for n calls to thisbdm.*sh
  # due to https://github.com/pyenv/pyenv/issues/969
  eval "$(pyenv init -)" || return 1
  pyenv shell @pythonvers@ || return 1

  # Location of jupyter executable (installed with `pip install --user` command)
  if [ -n "${PYTHONUSERBASE}" ]; then
    export PATH="$PYTHONUSERBASE/.local/bin:$PATH"
  else
    export PATH="$HOME/.local/bin:$PATH"
  fi
  export LD_LIBRARY_PATH="$PYENV_ROOT/versions/@pythonvers@/lib":$LD_LIBRARY_PATH
  ########

  ##### CMake Specific Configurations #####
  if [ -z "${CMAKE_PREFIX_PATH}" ]; then
    CMAKE_PREFIX_PATH="${BDMSYS}/share/cmake" # Linux, ELF HP-UX
  else
    CMAKE_PREFIX_PATH="${BDMSYS}/share/cmake":$CMAKE_PREFIX_PATH
  fi
  export CMAKE_PREFIX_PATH
  ########

  #### ROOT Specific Configurations ####
  if [ -z "${BDM_ROOT_DIR}" ] && [ -z "${ROOTSYS}" ]; then
    BDM_ROOT_DIR=${BDMSYS}/third_party/root
    if ! [ -d "$BDM_ROOT_DIR" ]; then
      _bdm_err "[ERR] We are unable to source ROOT! Please make sure ROOT is installed on your system!"
      _bdm_err "      You can specify manually its location by executing 'export BDM_ROOT_DIR=path/to/root'"
      _bdm_err "      before running cmake."
      return 1
    fi
  else
    # ROOTSYS has precedence over the BDM_ROOT_DIR custom configuration
    if [ -n "${ROOTSYS}" ]; then
     local orvers="@rootvers@"
     local crvers
     crvers="$("$ROOTSYS"/bin/root-config --version)"
     if [ "$crvers" = "$orvers" ]; then
      BDM_ROOT_DIR=${ROOTSYS}
     else
      _bdm_err "[ERR] ROOTSYS points to ROOT version $crvers, while BDM was build with version $orvers."
      _bdm_err "      Make sure that ROOTSYS points to the right version of ROOT."
      return 1
     fi
    fi
  fi

  export BDM_ROOT_DIR
  # shellcheck disable=SC1090
  . "${BDM_ROOT_DIR}"/bin/thisroot.sh || return 1
  _bdm_define_command root
  ########

  #### ParaView Specific Configurations ####
  local with_paraview=@with_paraview@
  if [ "$with_paraview" = 'ON' ]; then
     if [ -z "${ParaView_DIR}" ]; then
      ParaView_DIR=${BDMSYS}/third_party/paraview; export ParaView_DIR
      if ! [ -d "$ParaView_DIR" ]; then
       _bdm_err "[ERR] We are unable to find ParaView! Please make sure it is installed in your system!"
       _bdm_err "      You can specify manually its location by executing 'export ParaView_DIR=path/to/paraview'"
       _bdm_err "      together with 'export Qt5_DIR=path/to/qt' before running cmake."
       return 1
      fi
     fi

     if [ -z "${ParaView_LIB_DIR}" ]; then
       ParaView_LIB_DIR="${ParaView_DIR}/lib"
     else
       ParaView_LIB_DIR="${ParaView_DIR}/lib":$ParaView_LIB_DIR
     fi
     export ParaView_LIB_DIR

     if [ -z "${PV_PLUGIN_PATH}" ]; then
      PV_PLUGIN_PATH="${BDMSYS}/lib/pv_plugin"
     else
      PV_PLUGIN_PATH="${BDMSYS}/lib/pv_plugin":$PV_PLUGIN_PATH
     fi
     export PV_PLUGIN_PATH

     # We don't add the ParaView site-packages path to PYTHONPATH, because pip in the
     # pyenv environment will not function anymore: ModuleNotFoundError: No module named 'pip._internal'
     _bdm_define_command paraview
     _bdm_define_command pvpython
     _bdm_define_command pvbatch

     if [ -z "${LD_LIBRARY_PATH}" ]; then
       LD_LIBRARY_PATH="${ParaView_LIB_DIR}"
     else
       LD_LIBRARY_PATH="${ParaView_LIB_DIR}":$LD_LIBRARY_PATH
     fi
     export LD_LIBRARY_PATH

     if [ -z "${DYLD_LIBRARY_PATH}" ]; then
       DYLD_LIBRARY_PATH="${ParaView_LIB_DIR}"
     else
       DYLD_LIBRARY_PATH="${ParaView_LIB_DIR}":$DYLD_LIBRARY_PATH
     fi
     export DYLD_LIBRARY_PATH
     ########

     #### Qt5 Specific Configurations ####
     if [ -z "${Qt5_DIR}" ]; then
      Qt5_DIR=${BDMSYS}/third_party/qt; export Qt5_DIR
      if ! [ -d "$Qt5_DIR" ]; then
       _bdm_err "[ERR] We are unable to find Qt! Please make sure it is installed in your system!"
       _bdm_err "      You can specify manually its location by executing 'export Qt5_DIR=path/to/qt'"
       _bdm_err "      together with 'export ParaView_DIR=path/to/paraview' before running cmake."
       return 1
      fi
     fi

     if [ -z "${QT_QPA_PLATFORM_PLUGIN_PATH}" ]; then
       QT_QPA_PLATFORM_PLUGIN_PATH="${Qt5_DIR}/plugins"
     else
       QT_QPA_PLATFORM_PLUGIN_PATH="${Qt5_DIR}/plugins":$QT_QPA_PLATFORM_PLUGIN_PATH
     fi
     export QT_QPA_PLATFORM_PLUGIN_PATH

     if [ -z "${LD_LIBRARY_PATH}" ]; then
       LD_LIBRARY_PATH="${Qt5_DIR}/lib"
     else
       LD_LIBRARY_PATH="${Qt5_DIR}/lib":$LD_LIBRARY_PATH
     fi
     export LD_LIBRARY_PATH

     if [ -z "${DYLD_LIBRARY_PATH}" ]; then
       DYLD_LIBRARY_PATH="${Qt5_DIR}/lib"
     else
       DYLD_LIBRARY_PATH="${Qt5_DIR}/lib":$DYLD_LIBRARY_PATH
     fi
     export DYLD_LIBRARY_PATH
  fi
  #######

  # OpenMP
  export OMP_PROC_BIND=true

  ###### Platform-specific Configuration
  # Apple specific
  if [ "$(uname)" = 'Darwin' ]; then
    # Nothing for now
    true
  else # GNU/Linux
    # CentOS specifics
    local os_id
    os_id=$(grep -oP '(?<=^ID=).+' /etc/os-release | tr -d '"')
    if [ "$os_id" = 'centos' ]; then
        export MESA_GL_VERSION_OVERRIDE=3.3
        if [ -z "${CXX}" ] && [ -z "${CC}" ] ; then
            . scl_source enable devtoolset-7 || return 1
        fi
        . /etc/profile.d/modules.sh || return 1
        module load mpi || return 1

        # load llvm 6 required for libroadrunner
        if [ -d "${BDMSYS}"/third_party/libroadrunner ]; then
          . scl_source enable llvm-toolset-6.0 || return 1
        fi
    fi
  fi
  #######

  if [ -n "$ZSH_VERSION" ]; then
    # for autoload
    if [ -n "${old_bdmsys}" ]; then
      _drop_bdm_from_path "$FPATH" "${old_bdmsys}/bin/sh_functions"
      FPATH=$_newpath
    fi

    FPATH="${BDMSYS}/bin/sh_functions:$FPATH"
    export FPATH

    ### Enable commands in child shells (like in bash) ###
    local ld_root='if [ -d "${BDM_ROOT_DIR}" ]; then autoload -Uz root; fi;'
    local ld_pv='if [ -d "${ParaView_DIR}" ]; then autoload -Uz paraview pvpython pvbatch; fi;'
    local marker=' # >>thisbdm<<'
    local zshenv_line='if [ -d "${BDMSYS}" ]; then; '"${ld_root} ""${ld_pv}"' ; fi;'"${marker}"
    local zsh_config_dir="$HOME"
    if [ -n "$ZDOTDIR" ]; then
      zsh_config_dir="$ZDOTDIR"
    fi

    local zshenv_file="${zsh_config_dir}/.zshenv"
    if ! [ -f "$zshenv_file" ]; then
      _bdm_ok "[INFO] creating .zshenv file"
      touch "$zshenv_file"
    fi

    # ensure the above is only called once in .zshrc
    sed -i.bak '/^.*'"$marker"'$/,$d' "$zshenv_file" && rm "${zshenv_file}.bak" || return 1
    echo "$zshenv_line" >> "$zshenv_file"
  fi

  ### Environment Indicator ###
  if [ "$_bdm_quiet" = false ]; then
    local bdm_major_minor='';
    bdm_major_minor=$(bdm-config --version | sed -n  '1 s/.*v\([0-9]*.[0-9]*\).*/\1/p')
    if [ -z "$__bdm_sh_prompt_original" ]; then
      __bdm_sh_prompt_original=$PS1
    fi
    # NB: overrides prompt for current session only
    export PS1="[bdm-$bdm_major_minor] $__bdm_sh_prompt_original"
  fi

  return 0
}

# Run
if _source_thisbdm "$@"; then
  _bdm_ok "[OK] You have successfully sourced BioDynaMo's environment."
  _thisbdm_cleanup
  return 0
else
  _bdm_err "[ERR] BioDynaMo's environment could not be sourced."
  # traps don't particularly work well (for this use-case)
  # in zsh, so we'll repeat ourselves just this once
  _thisbdm_cleanup
  return 1
fi
