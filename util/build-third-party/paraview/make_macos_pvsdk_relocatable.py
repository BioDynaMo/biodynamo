#!/usr/bin/env python3
'''
Make macOS UNIX-like ParaView SDK bundles relocatable (using rpaths).

Kludgy, but necessary at the moment. As of Dec. 2020,
ParaView-Superbuild (SB) can only generate macOS ``.app``-style
relocatable bundles. While the functionality to make Unix-like SDK bundles
exists in SB through the use of cmake macro calls to ``fixup_bundle.apple.py``,
seemingly large modifications are necessary for our use-case. Making these
changes is certainly the long-term solution, but this is best left to
the upstream developers. This script essentially avoids modifying SB
cmake files by replicating multiple macro calls to ``fixup_bundle.apple.py``.

Large portions of this script are adapted from:
https://gitlab.kitware.com/paraview/common-superbuild/cmake/scripts/fixup_bundle.apple.py

Unused bits of the script above are still kept in case we end up needing them.

Example usage:
```
python3 make_macos_pvsdk_relocatable.py \
  --source $HOME/bdm-build-third-party/paraview/build/install \
  --dest $HOME/bdm-build-third-party/paraview/paraview \
  --third-party $HOME/bdm-build-third-party/paraview qt \
  --pv 5.9 --py 3.9
```

Prerequisites:
  * Python 3.8+
  * The build output of 'util/build-third-party/build-paraview.sh'.
    The directory structure should resemble a UNIX build, with the
    exception of '<source>/Applications/paraview.app'.
  * Any non-system third-party libraries to link against this bundle should
    be placed in ``--third-party``. This is to facilitate bundles distributed
    in the manner below:
    ```
    . (--third-party)
    ├── paraview (--dest)
    │   ├── bin
    │   │   └── paraview
    │   │         $ otool -l paraview
    │   │             @rpath/some_qt_lib.dylib ...
    │   │             @rpath/some_pv_lib.dylib ...
    │   │         where @rpath="@loader_path/../lib;@loader_path/../../qt/lib"
    │   ├── lib
    │   └── ...
    └── qt
        ├── bin
        ├── lib
        └── ...
    ```
'''
import json
import os
import platform
import re
import shutil
import subprocess
import operator
from functools import reduce

dry_run: bool = True
is_verbose: bool = False
install_source: str = None
install_dest: str = None
third_party_path: str = None
third_party_libs: list[str] = None

python_v: str = '3.9'
paraview_v: str = '5.9'


def vprint(*args, **kwargs):
    '''print if ``is_verbose`` global is true'''
    if is_verbose:
        print(*args, *kwargs)


def startswith_any(_str: str, _list: list[str]):
    for prefix in _list:
        if _str.startswith(prefix):
            return True
    return False


def os_makedirs(path):
    '''
    A function to fix up the fact that os.makedirs
    chokes if the path already exists.
    '''
    if os.path.exists(path):
        return
    os.makedirs(path)


def copy_tree(src, dst, symlinks=False, ignore=None, copy_function=shutil.copy2,
              ignore_dangling_symlinks=False, dirs_exist_ok=True, skip_symlinks=False):
    '''
    Modified version of ``shutil.copytree`` for copying directory trees and symlinks 
    *without* overwriting or raising exceptions on existing files, dirs and symlinks
    in ``dst``. You may skip all actions on symlinks using ``skip_symlinks=True``.
    '''
    with os.scandir(src) as itr:
        entries = list(itr)
    return _copy_tree(entries=entries, src=src, dst=dst, symlinks=symlinks,
                      ignore=ignore, copy_function=copy_function,
                      ignore_dangling_symlinks=ignore_dangling_symlinks,
                      dirs_exist_ok=dirs_exist_ok, skip_symlinks=skip_symlinks)


def _copy_tree(entries, src, dst, symlinks, ignore, copy_function,
               ignore_dangling_symlinks, dirs_exist_ok=False, skip_symlinks=False):
    if ignore is not None:
        ignored_names = ignore(os.fspath(src), [x.name for x in entries])
    else:
        ignored_names = set()

    os.makedirs(dst, exist_ok=dirs_exist_ok)
    errors = []
    use_srcentry = copy_function is shutil.copy2 or copy_function is shutil.copy

    for srcentry in entries:
        if srcentry.name in ignored_names:
            continue
        srcname = os.path.join(src, srcentry.name)
        dstname = os.path.join(dst, srcentry.name)
        srcobj = srcentry if use_srcentry else srcname
        try:
            is_symlink = srcentry.is_symlink()
            if is_symlink and os.name == 'nt':
                # Special check for directory junctions, which appear as
                # symlinks but we want to recurse.
                lstat = srcentry.stat(follow_symlinks=False)
                if lstat.st_reparse_tag == os.stat.IO_REPARSE_TAG_MOUNT_POINT:
                    is_symlink = False
            if is_symlink:
                linkto = os.readlink(srcname)
                if skip_symlinks:
                    vprint(f"<symlink> skip {linkto}")
                else:
                    if symlinks:
                        # We can't just leave it to `copy_function` because legacy
                        # code with a custom `copy_function` may rely on copytree
                        # doing the right thing.
                        if not os.path.exists(dstname):
                            vprint(f"<symlink> {linkto} -> {dstname}")
                            os.symlink(linkto, dstname)
                            shutil.copystat(srcobj, dstname, follow_symlinks=not symlinks)
                    else:
                        # ignore dangling symlink if the flag is on
                        if not os.path.exists(linkto) and ignore_dangling_symlinks:
                            continue
                        # otherwise let the copy occur. copy2 will raise an error
                        if srcentry.is_dir():
                            vprint(srcname, dstname)
                            copy_tree(srcobj, dstname, symlinks, ignore,
                                      copy_function, dirs_exist_ok=dirs_exist_ok)
                        else:
                            if not os.path.exists(dstname):
                                vprint(srcname, dstname)
                                copy_function(srcobj, dstname)
            elif srcentry.is_dir():
                vprint(srcname, dstname)
                copy_tree(srcobj, dstname, symlinks, ignore, copy_function,
                          dirs_exist_ok=dirs_exist_ok)
            else:
                vprint(srcname, dstname)
                # Will raise a SpecialFileError for unsupported file types
                if not os.path.exists(dstname):
                    copy_function(srcobj, dstname)
        # catch the Error from the recursive copytree so that we can
        # continue with other files
        except shutil.Error as err:
            errors.extend(err.args[0])
        except OSError as why:
            errors.append((srcname, dstname, str(why)))
    try:
        shutil.copystat(src, dst)
    except OSError as why:
        # Copying file access times may fail on Windows
        if getattr(why, 'winerror', None) is None:
            errors.append((src, dst, str(why)))
    if errors:
        raise shutil.Error(errors)
    return dst


class Pipeline(object):
    '''
    A simple class to handle a list of shell commands
    which need to pass input to each other.
    '''

    def __init__(self, *commands):
        if not commands:
            raise RuntimeError('Pipeline: at least one command must be given')

        self._commands = commands

    def __call__(self):
        # Use /dev/null as the input for the first command.
        last_input = open(os.devnull, 'r')
        command = None
        for command_args in self._commands:
            command = subprocess.Popen(
                command_args, stdin=last_input, stdout=subprocess.PIPE)
            last_input.close()
            last_input = command.stdout

        stdout, stderr = command.communicate()
        if command.returncode:
            raise RuntimeError('failed to execute pipeline:\n%s' % stderr)
        return stdout.decode('utf-8')
    
    def call_non_dry(self, fatal=True):
        try:
            return self.__call__() if not dry_run else ''
        except RuntimeError as e:
            if fatal:
                raise e
            else:
                vprint(f'<pipe-fail> {e}')


class Library(object):
    '''
    A representation of a library.

    This class includes information that a runtime loader needs in order to
    perform its job. It tries to implement the behavior of ``dyld(1)`` as
    closely as possible.

    Known Issues
    ------------

    ``@rpath/`` and ``DYLD_LIBRARY_PATH``
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    When a library contains a reference to a library like
    ``@rpath/libname.dylib``, if ``DYLD_LIBRARY_PATH`` is set to contain a path
    which has a ``libname.dylib``, ``dyld(1)`` will find it even if no
    ``LC_RPATH`` commands are present. This behavior is not documented and it
    only seems to work if a library is directly underneath a
    ``DYLD_LIBRARY_PATH`` path. If the library reference is
    ``@rpath/dir/libname.dylib``, even if a ``dir/libname.dylib`` exists in a
    ``DYLD_LIBRARY_PATH`` path, it will still not be found. It is unknown
    whether this behavior is expected or not due to the lack of documentation.
    The logic in this script includes neither of these behaviors.
    '''

    def __init__(self, path, parent=None, search_paths=None, ignores=None):
        # This is the actual path to a physical file
        self._path = os.path.normpath(path)

        if search_paths is None:
            self._search_paths = []
        else:
            self._search_paths = search_paths

        if ignores is None:
            self._ignores = []
        else:
            self._ignores = ignores
    
        self._parent = parent
        self._symlinks = None
        self._framework_info = None
        self._executable_path = None
        self._dependencies = None
        self._rpaths = None
        self._raw_rpaths = None
        self._raw_trans_rpaths = None
        self._pending_rpaths: set[str] = set()
        self._installed_id = None

    def __hash__(self):
        return self._path.__hash__()

    def __eq__(self, other):
        return self._path == other._path

    def __repr__(self):
        return 'Library(%s : %s)' % (self._installed_id, self.path)

    @property
    def path(self):
        '''The absolute path to the library.'''
        return self._path

    @property
    def parent(self):
        '''The binary which loaded the library.'''
        return self._parent

    @property
    def name(self):
        '''The name of the library.'''
        return os.path.basename(self.path)

    @property
    def ignores(self):
        '''Regular expressions of IDs to ignore from this library.'''
        if self.parent is None:
            return self._ignores
        return self.parent.ignores

    @property
    def installed_id(self):
        '''
        The ID of the library.

        This is the string by which the library will be referenced by other
        binaries in the installation.
        '''
        return self._installed_id

    @property
    def pending_rpaths(self):
        return self._pending_rpaths
        
    def set_installed_id(self, installed_id):
        '''Set the ID of the library as it is installed as.'''
        self._installed_id = installed_id

    def extend_pending_rpaths(self, pending_rpaths: set[str]):
        self._pending_rpaths = self._pending_rpaths.union(pending_rpaths)

    @property
    def dependent_reference(self):
        '''
        The prefix to use for a library loaded by the library.

        This is used as the prefix for IDs for libraries loaded by the library.
        It is based on the initial binary which loaded the library. For
        example, executables use ``@executable_path`` and plugins use
        ``@loader_path``. In a chain of loaded libraries, the loader (parent)
        of a library determines the prefix.
        '''
        # Refer to libraries the same way that the library which is loading it
        # references it.
        if self.parent is None:
            raise RuntimeError('Unable to get a reference')
        return self.parent.dependent_reference

    @property
    def symlinks(self):
        '''
        A list of symlinks to the library.

        Symlinks are looked for only beside the library and the names of these
        files are returned, not their full paths.
        '''
        if self._symlinks is None:
            realpath = os.path.realpath(self.path)
            dirname = os.path.dirname(realpath)
            symlinks = set(Pipeline([
                'find', '-L', dirname,
                '-depth', '1',
                '-samefile', realpath,
            ])().split())

            symlink_bases = []
            for symlink in symlinks:
                symlink_dir, symlink_base = os.path.split(symlink)
                if not symlink_dir == dirname:
                    continue
                symlink_bases.append(symlink_base)
            if self.name in symlink_bases:
                symlink_bases.remove(self.name)
            self._symlinks = symlink_bases

        return self._symlinks

    @property
    def executable_path(self):
        '''The path to the loading executable (if available).'''
        if self._parent is not None:
            return self._parent.executable_path
        return self._executable_path

    @property
    def loader_path(self):
        '''The path to use for ``@loader_path`` references from the library.'''
        return os.path.dirname(self.path)

    @property
    def loader_paths(self):
        '''
        A list of paths to look for libraries due to where the loading
        libraries look.
        '''
        loader_paths = [self.loader_path]
        if self.parent is not None:
            loader_paths.extend(self.parent.loader_paths)
        return loader_paths

    @property
    def is_framework(self):
        '''Whether the library is a framework or not.'''
        return self.path.count('.framework')

    @property
    def framework_info(self):
        '''
        Information for frameworks.

        The return value is a tuple of path (where the framework is located),
        name (the ``NAME.framework`` part of its path), and associated library
        (the path under the ``.framework`` directory which contains the actual
        library binary).

        See the ``framework_path``, ``framework_name``, and
        ``framework_library`` properties.
        '''
        if self._framework_info is None:
            if not self.is_framework:
                self._framework_info = (None, None, None)
            else:
                name = None
                library = []

                path = self.path
                while path:
                    path, component = os.path.split(path)
                    if component.endswith('.framework'):
                        name = component
                        break
                    library.append(component)

                if name is None:
                    raise RuntimeError('%s is not a framework?' % self.path)

                self._framework_info = (
                    os.path.join(path),
                    name,
                    os.path.join(*reversed(library)),
                )
        return self._framework_info

    @property
    def framework_path(self):
        '''
        The path which contains the ``.framework`` for the library.

        ``None`` if the library is not a framework.
        '''
        return self.framework_info[0]

    @property
    def framework_name(self):
        '''
        The name of the framework containing the library.

        ``None`` if the library is not a framework.
        '''
        return self.framework_info[1]

    @property
    def framework_library(self):
        '''
        The path to the library under the ``.framework`` directory.

        ``None`` if the library is not a framework.
        '''
        return self.framework_info[2]
    
    @property 
    def raw_rpaths(self) -> set[str]:
        '''Unresolved ``LC_RPATH`` load commands of ``self``.'''
        if self._raw_rpaths is None:
            get_rpaths = Pipeline([
                'otool',
                '-l',
                self.path,
            ], [
                'awk',
                '''
                    $1 == "cmd" {
                        cmd = $2
                    }

                    $1 == "path" {
                        if (cmd == "LC_RPATH") {
                            print $2
                        }
                    }
                ''',
            ])
            self._raw_rpaths = set(get_rpaths().split('\n'))
            self._raw_rpaths.discard('')

        return self._raw_rpaths

    @property
    def raw_trans_rpaths(self) -> set[str]:
        '''
        Unresolved ``LC_RPATH`` load commands of ``self`` *and parents*.

        In addition to the ``LC_RPATH`` load commands contained within the
        library, rpaths in the binaries which loaded the library are
        referenced. These are included in the property.
        '''
        if self._raw_trans_rpaths is None:
            self._raw_trans_rpaths = self.raw_rpaths \
                if self._parent is None \
                else self._parent.raw_rpaths.union(self.raw_rpaths)
            self._raw_trans_rpaths.discard('')
        return self._raw_trans_rpaths

    @property
    def rpaths(self):
        '''
        The list of rpaths used when resolving ``@rpath/`` references in the
        library.

        In addition to the ``LC_RPATH`` load commands contained within the
        library, rpaths in the binaries which loaded the library are
        referenced. These are included in the property.
        '''
        if self._rpaths is None:
            # rpaths may contain magic ``@`` references. This property only
            # contains full paths, so we resolve them now.
            resolved_rpaths = []
            for rpath in self.raw_trans_rpaths:
                if rpath.startswith('@executable_path'):
                    # If the loader does not have an executable path, it is a plugin or
                    # a framework and we trust the executable which loads the plugin to
                    # provide the library instead.
                    if self.executable_path is None:
                        continue
                    resolved_rpaths.append(rpath.replace(
                        '@executable_path', self.executable_path))
                elif rpath.startswith('@loader_path'):
                    resolved_rpaths.append(rpath.replace(
                        '@loader_path', self.loader_path))
                elif rpath:
                    resolved_rpaths.append(rpath)

            self._rpaths = resolved_rpaths
        return self._rpaths

    def _get_dependencies(self):
        '''Get the dependent library IDs of the library.'''
        pipe = Pipeline([
            'otool',
            '-L',
            self.path,
        ], [
            'sed',
            '-n',
            '-e', '/compatibility version/s/ (compatibility.*)//p',
        ])
        return pipe().split()

    @property
    def dependencies(self):
        '''Dependent libraries of the library.'''
        if self._dependencies is None:
            collection = {}
            for dep in self._get_dependencies():
                deplib = Library.from_reference(dep, self)
                if deplib is not None \
                    and not deplib.path == self.path:
                    collection[dep] = deplib
            self._dependencies = collection
        return self._dependencies

    def _find_library(self, ref):
        '''
        Find a library using search paths.

        Use of this method to find a dependent library indicates that the
        library dependencies are not properly specified. As such, it warns
        when it is used.
        '''
        print('WARNING: dependency from %s to %s requires a search path' %
              (self.path, ref))
        for loc in self._search_paths:
            path = os.path.join(loc, ref)
            if os.path.exists(path):
                return path
        return ref

    @classmethod
    def from_reference(cls, ref, loader):
        '''Create a library representation given an ID and a loading binary.'''
        paths = [ref]
        if ref.startswith('@executable_path/'):
            # If the loader does not have an executable path, it is a plugin or
            # a framework and we trust the executable which loads the plugin to
            # provide this library instead.
            if loader.executable_path is None:
                return None
            paths.append(ref.replace('@executable_path', loader.executable_path))
        elif ref.startswith('@loader_path/'):
            paths.append(ref.replace('@loader_path', loader.loader_path))
        elif ref.startswith('@rpath/'):
            for rpath in loader.rpaths:
                paths.append(ref.replace('@rpath', rpath))
        paths.append(os.path.join(os.path.dirname(loader.path), ref))
        for path in paths:
            if os.path.exists(path):
                return cls.from_path(os.path.realpath(path), parent=loader)
        if loader.ignores:
            for ignore in loader.ignores:
                if ignore.match(ref):
                    return None
        if ref.startswith('/System/Library/Frameworks/') or \
           ref.startswith('/usr/lib/'):
            # These files do not exist on-disk as of macOS 11. This is Apple
            # magic and assumed to be a system library.
            return None
        search_path = loader._find_library(ref)
        if os.path.exists(search_path):
            return cls.from_path(os.path.realpath(search_path), parent=loader)
        raise RuntimeError(
            'Unable to find the %s library from %s' % (ref, loader.path))

    __cache = {}

    @classmethod
    def from_path(cls, path, parent=None, _search_paths=None):
        '''Create a library representation from a path.'''
        if not os.path.exists(path):
            raise RuntimeError('%s does not exist' % path)

        path = os.path.normpath(path)
        if path not in cls.__cache:
            search_paths = _search_paths
            if search_paths is None and parent is not None:
                search_paths = parent._search_paths

            cls.__cache[path] = Library(path, parent=parent,
                                        search_paths=search_paths)

        return cls.__cache[path]

    @classmethod
    def from_manifest(cls, path, installed_id):
        '''Create a library representation from a cached manifest entry.'''
        if path in cls.__cache:
            raise RuntimeError('There is already a library for %s' % path)

        library = Library(path)
        library.set_installed_id(installed_id)
        library._dependencies = {}
        library._symlinks = []
        library._is_cached = True

        cls.__cache[path] = library
        return cls.__cache[path]


class Plugin(Library):
    '''
    A plugin library.

    These libraries are expected to be loaded by an executable and use
    ``@executable_path/`` references where possible, but for any libraries
    required by the plugin and not otherwise provided, ``@loader_path/`` is
    used instead.

    Some plugins may require to be considered as their own ``@executable_path``
    reference. This may indicate errors in the building of the plugin.
    '''

    def __init__(self, path, fake_exe_path=False, **kwargs):
        super(Plugin, self).__init__(path, None, **kwargs)

        if fake_exe_path:
            self._executable_path = os.path.dirname(path)

    # @property
    # def bundle_location(self):
    #     return 'Contents/Plugins'

    @property
    def dependent_reference(self):
        return '@loader_path/..'


class Module(Library):
    '''
    A library loaded programmatically at runtime.

    Modules are usually used by interpreted languages (as opposed to compiled
    languages) and loaded at runtime. They may live at any depth in the installation.

    Currently it is assumed that the only executables which will load these
    modules is a binary in the same installation. It is unknown if this
    assumption is actually valid and documentation is scarce.

    Some modules may require to be considered as their own ``@executable_path``
    reference. This may indicate errors in the building of the module.
    '''

    def __init__(self, path, fake_exe_path=False, **kwargs):
        super(Module, self).__init__(path, None, **kwargs)

        self._bundle_location = os.path.normpath(
            os.path.dirname(os.path.relpath(path, install_source)))
        if fake_exe_path:
            self._executable_path = path
            for _ in range(self.bundle_location.count('/')):
                self._executable_path = os.path.dirname(self._executable_path)

        parent_parts = ['..'] * self.bundle_location.count('/')
        self._dependent_reference = os.path.join('@loader_path', *parent_parts)

    @property
    def bundle_location(self):
        return self._bundle_location

    @property
    def dependent_reference(self):
        return '@executable_path/..'
        # XXX(modules): is this right? should modules ever not be loaded by
        # their owning application?
        # return self._dependent_reference


def copy_library(root_dest: str, library: Library, sub_dest: str='lib'):
    '''Copy a library into the ``.app`` bundle.'''
    if library.is_framework:
        # Frameworks go into Contents/<framework_dest>.

        app_dest = os.path.join(root_dest, sub_dest)
        binary = os.path.join(
            app_dest, library.framework_name, library.framework_library)

        print('<copy> %s/%s -> %s' % (library.framework_path, library.framework_name,
                                      os.path.join(sub_dest, library.framework_name, 
                                      library.framework_library)))

        library.set_installed_id(os.path.join(
            '@executable_path', '..', sub_dest, 
            library.framework_name, library.framework_library))
        root_dest = os.path.join(app_dest, library.framework_name)

        if not dry_run:
            # TODO: This could be optimized to only copy the particular version.
            if os.path.exists(root_dest):
                shutil.rmtree(root_dest)
            os_makedirs(app_dest)
            shutil.copytree(os.path.join(library.framework_path,
                                         library.framework_name), root_dest, symlinks=True)

            # We need to make sure the copied libraries are writable.
            chmod = Pipeline([
                'chmod',
                '-R',
                'u+w',
                root_dest,
            ])
            chmod()
    else:
        app_dest = os.path.join(root_dest, sub_dest)
        binary = os.path.join(app_dest, library.name)
        print('<copy> %s -> %s' %
              (library.path, os.path.join(sub_dest, library.name)))

        # FIXME(plugins, frameworks): fix the installed id of the library based
        # on what drags it in.
        library.set_installed_id(os.path.join('@rpath', library.name))
        root_dest = app_dest

        if not dry_run:
            os_makedirs(app_dest)
            shutil.copy2(library.path, root_dest)

            # We need to make the library after copying it.
            chmod = Pipeline([
                'chmod',
                'u+w',
                os.path.join(root_dest, os.path.basename(library.path)),
            ])
            chmod()

        # Create any symlinks we found for the library as well.
        for symlink in library.symlinks:
            print('<symlink> %s/%s -> %s' %
                  (sub_dest, library.name, os.path.join(sub_dest, symlink)))
            if not dry_run:
                symlink_path = os.path.join(app_dest, symlink)
                if os.path.exists(symlink_path):
                    vprint('exists, removing %s ' % symlink_path)
                    os.remove(symlink_path)
                ln = Pipeline([
                    'ln',
                    '-s',
                    library.name,
                    symlink_path,
                ])
                ln()

    return binary


def _arg_parser():
    import argparse

    # https://stackoverflow.com/questions/4194948/python-argparse-is-there-a-way-to-specify-a-range-in-nargs
    def required_length(nmin, nmax=999):
        class RequiredLength(argparse.Action):
            def __call__(self, parser, args, values, option_string=None):
                if not nmin <= len(values) <= nmax:
                    msg = 'argument "{f}" requires between {nmin} and {nmax} arguments'.format(
                        f=self.dest, nmin=nmin, nmax=nmax)
                    raise argparse.ArgumentTypeError(msg)
                setattr(args, self.dest, values)

        return RequiredLength

    parser = argparse.ArgumentParser(
        description='Make macOS UNIX-like ParaView SDK bundles relocatable')
    parser.add_argument('-S', '--source', metavar='SOURCE', required=True,
                        help='location of the PV SDK superbuild install'),
    parser.add_argument('-O', '--dest', metavar='DEST', required=True,
                        help='destination of the relocatable PV SDK'),
    parser.add_argument('-T', '--third-party', metavar='PATH', nargs='+',
                        action=required_length(2), required=True,
                        help='Third-party libs that will be loaded relative to --dest.'
                             'Syntax: <third-party-dir> <lib-subdir> { <lib-subdir> }'),
    parser.add_argument('-n', '--dry-run', action='store_true',
                        help='do not actually modify filesystem', default=False)
    parser.add_argument('-v', '--verbose', action='store_true',
                        help='enable verbose output', default=False)
    parser.add_argument('--py', metavar="MAJOR.MINOR",
                        help='Python 3 version', default='3.8')
    parser.add_argument('--pv', metavar="MAJOR.MINOR",
                        help='ParaView version', default='5.9')

    return parser


def update_manifest(manifest, installed):
    '''Update the manifest file with a set of newly installed binaries.'''
    for input_path, binary_info in installed.items():
        binary, _ = binary_info
        manifest[input_path] = binary.installed_id


def path_in_dest(path: str):
    subdir = os.path.relpath(path, install_source)
    return os.path.join(install_dest, subdir)


def pend_rpaths_from_deps(binary: Library, is_excluded, rpath_suffixes):
    '''
    Build rpath entries of the form ``@loader_path ['/' <suffix>]``
    using rpath_suffixes. Mark them to be added by fix_installed_binaries.
    '''
    deps: list[Library] = list(binary.dependencies.values())
    loader_to_dep_relpaths: set[str] = set()

    for dep in deps:
        if is_excluded(dep.path):
            continue
        dep_relpath = ''
        
        if dep.path.startswith(install_source):
                dep_relpath = \
                os.path.relpath(dep.path, binary.loader_path)
        elif startswith_any(dep.path, third_party_libs):
            # 3rd party deps need to be viewed relative 
            # to the current binaries final destination
            dep_relpath = \
                os.path.relpath(dep.path, path_in_dest(binary.loader_path))

        loader_to_dep_relpaths.add(os.path.split(dep_relpath)[0])
    
    vprint(f'<dep-relpath> {loader_to_dep_relpaths}')
    if len(loader_to_dep_relpaths) > 0:
        pend_rpaths = set()
        # dependency in one of the suffixes
        for dep_relp in loader_to_dep_relpaths:
            for suffix in rpath_suffixes:
                if dep_relp == suffix or dep_relp.startswith(suffix):
                    to_add = os.path.join('@loader_path', suffix)
                    # make all suffixes end with trailing slash
                    to_add = to_add if to_add.endswith('/') else f'{to_add}/'
                    pend_rpaths.add(to_add)
                    break
        # dependency in @loader_path
        if '' in loader_to_dep_relpaths:
            pend_rpaths.add('@loader_path')

        vprint(f"<rpath-pend> {binary.path}\n\t<-{pend_rpaths}")
        binary.extend_pending_rpaths(pend_rpaths)


def install_binary(binary: Library, is_excluded, bundle_dest, 
                   installed, manifest, rpath_suffixes, sub_dest='lib'):
    '''Install the main binary into the package.'''
    print(f"<install> {os.path.basename(binary.path)}")
    pend_rpaths_from_deps(binary, is_excluded, rpath_suffixes)
    # Start looking at our main executable's dependencies.
    deps = list(binary.dependencies.values())
    while deps:
        dep = deps.pop(0)

        # Ignore dependencies which the bundle already provides.
        if dep.path in manifest:
            continue

        # Ignore dependencies we don't care about.
        if is_excluded(dep.path):
            vprint(f"<dep-abs> {dep.path}")
            continue

        # If we've already installed this dependency
        # for some other library, skip it.
        if dep.path in installed:
            continue
        
        # Add this dependency's dependencies to the pile.
        deps.extend(dep.dependencies.values())
        if dep.path.startswith(install_source):
            dep_sub_dest = os.path.normpath(
                os.path.dirname(os.path.relpath(dep.path, install_source)))
            vprint(f"<dep> {dep.path}")
            pend_rpaths_from_deps(dep, is_excluded, rpath_suffixes)
            # Remember what we installed and where.
            installed[dep.path] = \
                (dep, copy_library(bundle_dest, dep, sub_dest=dep_sub_dest))
        elif startswith_any(dep.path, third_party_libs): 
            vprint(f"<dep-third-party> {dep.path}")
            installed[dep.path] = \
                (dep, os.path.relpath(dep.path, install_source))
        else:
            # This call resulted in an error on arm64 and ParaView-5.10 and was
            # therefore turned into an exception.
            try:
                vprint(f"<?> {dep.paths}")
            except Exception as ex:
                print(ex)
                vprint(f"'Library' object has no attribute 'paths'")


    # Install the main executable itself.
    app_dest = os.path.join(bundle_dest, sub_dest)
    binary_destination = os.path.join(app_dest, os.path.basename(binary.path))
    installed[binary.path] = (binary, binary_destination)
    binary.set_installed_id(os.path.join(sub_dest, binary.name))
    
    print('<copy> %s -> %s' % (binary.path, sub_dest))
    if not dry_run:
        os_makedirs(app_dest)
        shutil.copy2(binary.path, app_dest)


_fin_rpaths: dict[str, list[str]] = dict()


def fix_installed_binaries(installed: dict[str, (Library, str)]):
    '''
    This function updates all of the installed binaries to use consistent
    library IDs when referring to each other.
    '''
    # Go through all of the binaries installed and fix up references to other things.
    for binary_info in installed.values():
        binary, installed_path = binary_info
        print(f'<fix> {binary.path}')
        if binary.path.endswith('.a'):
            # nothing to fix
            continue

        vprint(f'\t<rpath-pend>\n\t\t{binary.pending_rpaths.difference(binary.raw_rpaths)}')

        if binary.installed_id:
            # Set the ID on the binary.
            vprint(
                f"<set-id> {binary.installed_id}")
            Pipeline([
                'install_name_tool',
                '-id', binary.installed_id,
                installed_path,
            ]).call_non_dry()

        changes = []
        for old_name, library in binary.dependencies.items():
            if library.installed_id is not None \
                    and not old_name == library.installed_id:
                if startswith_any(old_name, ['@loader_path', '@rpath']):
                    pass
                else:
                    changes.extend(['-change', old_name, library.installed_id])
            
        # Fix up the library names.
        if changes:
            install_name_tool = \
                ['install_name_tool'] + changes + [installed_path]
            vprint(' '.join(install_name_tool))
            Pipeline(install_name_tool).call_non_dry()
        
        # Add pending rpaths to binary.
        if not installed_path in _fin_rpaths:
            rp_diff = binary.pending_rpaths.difference(binary.raw_rpaths)
            rp_isect = binary.pending_rpaths.intersection(binary.raw_rpaths)
            _fin_rpaths[installed_path] = list(rp_diff.union(rp_isect))
            rpaths_to_add = []
            for pending in rp_diff:
                rpaths_to_add.extend(['-add_rpath', pending])
            if rpaths_to_add:
                install_name_tool = \
                    ['install_name_tool'] + rpaths_to_add + [installed_path]
                vprint(' '.join(install_name_tool))
                Pipeline(install_name_tool).call_non_dry()


def _is_excluded(path):
    # System libraries
    ## Apple
    if path.startswith('/System/Library'):
        return True
    if path.startswith('/usr/lib'):
        return True

    ## Homebrew
    if path.startswith('/usr/local/lib'):
        return True
    if path.startswith('/usr/local/Cellar'):
        return True
    if path.startswith('/opt/homebrew'):
        return True

    ## Macports
    if path.startswith('/opt/local/lib'):
        return True

    return False


def is_binary(fpath):
    finfo = Pipeline(['file', '-Ib', fpath])().splitlines()[0]
    return re.match(r"application/(x-mach-binary|x-archive).*", finfo) is not None


def find_bins_in(path):
    pred='( -type f ) -and ( -perm +111 -or -name *.dylib -or -name *.so -or -name *.a )'
    bin_candidates = Pipeline(f'find {path} {pred}'.split())().split()
    return [cand for cand in bin_candidates if is_binary(cand)]


_manifest = {}
_installed = {}


class BinaryGroup(object):
    def __init__(self,
                 path_or_bins,
                 rpath_suffixes,
                 search_paths=None,
                 preproc_fn=lambda x: x,
                 exclude_fn=_is_excluded,
                 typ='lib',
                 rpath_map=None,
                 installed_map=None,
                 bin_manifest=None):
        '''
        A ``BinaryGroup`` contains a collection of binaries along
        with instructions for how to copy over and install them 
        from ``--source`` to ``--dest``.

        Args:
        path_or_bins (Union[str, list[str]]): 
            Path containing binaries (will be recursively searched by 
            ``find``) or a list of paths to each binary in this group.
            NB: All paths passed in must be relative to ``--source``.
        rpath_suffixes (list[str]):
            List of suffixes to match against each binaries' relative paths 
            to its dependencies . Matches will be used to build rpath entries 
            of the form ``@loader_path ['/' <suffix>]``.
        search_paths (Optional[list[str]]): 
            List of paths (default ``install_source/{bin,lib}``)
            to search for dependent libraries.
        preproc_fn (Callable[[str], Any]):
            Callback called just before the creation of the path's corresponding
            ``Library`` (sub)class. This callback will be applied to the binary 
            in ``--dest``, so make sure its side-effect is idempotent, or that 
            it at least doesn't break anything.
        exclude_fn (Callable[[str], bool]): 
            Callback to determine which binary files to exclude.
        typ (str): 
            One of {'lib', 'module', 'plugin'} (default 'lib'). Determines
            which subclass of ``Library`` the binaries in this group will be.
            The behavior of ``self.install()`` depends on this type.
        rpath_map (Optional[dict[str, str]]): 
            For each binary 'b' in this group, and each key-value pair 
            'k', 'v' in `rpath_map`; if 'k' matches an rpath entry in 'b',
            replace it with 'v', or if 'v' is None, then delete the entry.

        Returns:
        ``BinaryGroup`` ready to be ``self.install()``ed.
        '''
        _default_searchpaths = \
            [os.path.join(install_source, _dir) for _dir in ['bin', 'lib']]

        self.installed_map = \
            _installed if installed_map is None else installed_map
        self.bin_manifest = \
            _manifest if bin_manifest is None else bin_manifest
        self.rpath_suffixes = rpath_suffixes
        vprint(path_or_bins)
        if isinstance(path_or_bins, str):
            self.bin_relpaths = find_bins_in(path_or_bins)
        elif isinstance(path_or_bins, list):
            self.bin_relpaths = \
                [cand for cand in path_or_bins if is_binary(cand)]
        else:
            raise RuntimeError('path_or_bins is either list[str] or str')

        self.exclude_fn = exclude_fn
        self.bin_preproc_fn = preproc_fn
        self._is_installed = False
        self.typ = typ
        self.rpath_map = {} if rpath_map is None else rpath_map
        self.search_paths = \
            _default_searchpaths if search_paths is None else search_paths

    def install(self):
        if self._is_installed:
            vprint("WARNING: This group is already installed!")
            return

        (bin_relpaths, search_paths, typ) = \
            (self.bin_relpaths, self.search_paths, self.typ)

        for bin_relpath in bin_relpaths:
            bin_path = os.path.abspath(bin_relpath)

            try:
                self.bin_preproc_fn(bin_path)
            except RuntimeError:
                pass
            
            _bin: Library = None
            if typ == 'plugin':
                _bin = Plugin.from_path(bin_path,
                                        _search_paths=search_paths)
            elif typ == 'module':
                _bin = Module.from_path(bin_path,
                                        _search_paths=search_paths)
            else:
                _bin = Library.from_path(bin_path,
                                         _search_paths=search_paths)

            _sub_dest = os.path.normpath(os.path.dirname(bin_relpath))
            install_binary(_bin, self.exclude_fn, install_dest,
                           self.installed_map, self.bin_manifest,
                           self.rpath_suffixes, sub_dest=_sub_dest)

            # Remap rpaths from any maching keys in rpath_map to 
            # their values, if the value is not None, delete otherwise.
            bin_abs_dest = os.path.join(
                os.path.join(install_dest, _sub_dest), 
                os.path.basename(_bin.path))
            for key, value in self.rpath_map.items():
                if key in _bin.raw_trans_rpaths:
                    if value is not None:
                        vprint(f"<rpath-remap> {key} -> {value}")
                        Pipeline([
                            'install_name_tool',
                            '-rpath', key, value, bin_abs_dest
                        ]).call_non_dry(fatal=False)
                    else:
                        vprint(f"<rpath-del> {key}")
                        Pipeline([
                            'install_name_tool',
                            '-delete_rpath', key, bin_abs_dest
                        ]).call_non_dry(fatal=False)

            if typ == 'module':
                _bin.set_installed_id(os.path.join('@rpath', _bin.name))

        update_manifest(_manifest, _installed)
        self._is_installed = True


def main(args):
    # Check if we are running on an ARM-based Apple device
    system_info = platform.uname()
    is_arm_based = (system_info.system == "Darwin" and 
                    system_info.machine == "arm64")
    if is_arm_based:
        print("Detected M1 machine, support experimental ..")

    # Parse and initialize arguments.
    parser = _arg_parser()
    opts = parser.parse_args(args)

    global dry_run, is_verbose, install_source, install_dest, \
        third_party_path, third_party_libs, paraview_v, python_v

    install_source, install_dest = (opts.source, opts.dest)
    third_party_path = opts.third_party[0]
    third_party_libs = \
        [os.path.join(third_party_path, ldir) for ldir in set(opts.third_party[1:])]
    dry_run, is_verbose = (opts.dry_run, opts.verbose)
    paraview_v, python_v = (opts.pv, opts.py)

    # Shell commands will be run relative to install_source.
    os.chdir(install_source)
    
    # Copy paraview binary from dummy .app to bin.
    os_makedirs(os.path.dirname(f'{install_dest}/bin'))
    shutil.copy2(f'{install_source}/Applications/paraview.app/Contents/MacOS/paraview', f'{install_dest}/bin')
    
    # The order is important. Match longest first.
    # Some paths may have higher precedence than others.
    # E.g., ../qt/lib < ../root/lib
    rp_suffixes = [
        '../../../../../qt/lib',
        '../../../..',
        '../../qt/lib',
        '../../..',
        '../lib'
    ]
    # Add variants with unnecessary trailing slash.
    rp_suffixes = \
        reduce(operator.concat, [[s+'/', s] for s in rp_suffixes])

    # Remove the old bundle.
    if os.path.exists(install_dest):
        vprint(f"rm -rf {install_dest}")
        if not dry_run:
            shutil.rmtree(install_dest, ignore_errors=True)

    def _preprocess_uni_bin(bin_path):
        # libs in lib/universal require special treatment
        # where their @loader_path is added as an rpath
        # entry *before* creating their Library objects
        Pipeline([
            'install_name_tool',
            '-add_rpath', '@loader_path',
            bin_path,
        ]).call_non_dry()

    # Qt is always a dependency. Make sure it's in --third-party.
    qt_lib_dir = os.path.join(third_party_path, 'qt', 'lib')

    # For 5.9, Qt5 is not installed via brew, thus, the explicit check.
    if paraview_v == '5.9':
        if not os.path.isdir(qt_lib_dir):
            raise RuntimeError(f"./qt/lib directory does not exist in {third_party_path}")
    
    # Mark undesirable for deletion with None.
    rpath_map = { qt_lib_dir: None, '@executable_path/': None, '@executable_path/../lib': None }

    print("==> Install binary groups")
    bin_groups = \
        [
            BinaryGroup("./bin", rp_suffixes, rpath_map=rpath_map),
            BinaryGroup(f"./lib/python{python_v}", rp_suffixes, rpath_map=rpath_map),
            BinaryGroup(f"./lib/paraview-{paraview_v}", rp_suffixes, rpath_map=rpath_map),
        ]
    # The following BinaryGroup was not found on the tested M1 machine and
    # caused problems. We therefore remove it if we're not building the outdated
    # ParaView 5.9
    if paraview_v == '5.9':
        bin_groups.append(BinaryGroup("./lib/universal", 
                          rp_suffixes, 
                          preproc_fn=_preprocess_uni_bin, 
                          rpath_map=rpath_map))

    for bin_group in bin_groups:
        bin_group.install()

    print("==> Installing remaining (likely runtime loaded) binaries as modules")
    rem_set = set(find_bins_in('./lib')).difference(_installed.keys())
    
    remaining = BinaryGroup(list(rem_set), rp_suffixes, typ='module', rpath_map=rpath_map)
    remaining.install()
    
    print("==> Fixing installed binary groups")
    fix_installed_binaries(_installed)

    print("==> Dump manifest.json to --source")
    # Dump manifest to --source, like a sort of receipt.
    with open('manifest.json', 'w') as fout:
        json.dump(_manifest, fout)
    
    print("==> Dump final_rpaths.json to --source")
    # For manually checking if any crazy rpaths are generated.
    with open('final_rpaths.json', 'w') as fout:
        json.dump(_fin_rpaths, fout)

    # Copy dummy .app too, just to keep 'clever' cmake files happy.
    print("==> Copying remaining files in --source to --dest")
    if not dry_run:
        for _path in ['Applications', 'bin', 'include', 'lib', 'materials', 'share']:
            # First copy over tree without symlinks, then just the 
            # symlinks to make sure everything is copied correctly.
            for symlinks, skip_symlinks in [(False, True), (True, False)]:
                copy_tree(os.path.join(install_source, _path),
                        os.path.join(install_dest, _path),
                        symlinks=symlinks,
                        skip_symlinks=skip_symlinks,
                        dirs_exist_ok=True)

    # Clean references to source install path in misc. files.
    # This is all manual, I'm afraid. When upgrading, whoever
    # maintains this script is advised to run a search for
    # references to --source in a dummy install (e.g., with ripgrep),
    # then write some clever find&replace commands to remove them.
    if not dry_run:
        os.chdir(install_dest)

        print("==> Cleaning references to --source in --dest")

        print("<clean> include/ospray/SDK/**/*_ispc.h")
        os.system("find ./include/ospray/SDK -name '*_ispc.h' -exec sed -i '' 2d {} \\;")

        print(f"<clean> include/paraview-{paraview_v}/vtkCPConfig.h")
        os.system("sed -i '' "
                  "'s|#define PARAVIEW_BINARY_DIR.*|#define PARAVIEW_BINARY_DIR \"./bin\"|g; "
                  "s|#define PARAVIEW_INSTALL_DIR.*|#define PARAVIEW_INSTALL_DIR \"./\"|g;' "
                  f'include/paraview-{paraview_v}/vtkCPConfig.h'
                  )

        print("<clean> lib/cmake/**/*.cmake")
        os.system(f"find {install_dest}/lib/cmake -name '*.cmake' -exec sed -E -i '' "
                  f'\'s|([[:space:]]*)(IMPORTED_SONAME_RELEASE)([[:space:]]*)"{install_source}/(.*)"|\\1\\2\\3"\\4"|g\''
                  ' {} +'
                  )

        print("<clean> lib/pkgconfig/nlohmann_json.pc")
        os.system(f"sed -i '' 's|Cflags:.*|Cflags: |g;' lib/pkgconfig/nlohmann_json.pc")
    
    print('==> Looking for broken or circular symlinks')
    if not dry_run:
        broken_links = \
            Pipeline((f'find {install_dest} -type l -exec test ! -e' + ' {} ; -print').split())().split()
        if len(broken_links) > 0:
            print('WARNING: Found broken or circular links:')
            for link in broken_links:
                print(link)
        else:
            print('==> None found')
    
    print('==> Done.')


if __name__ == '__main__':
    import sys

    main(sys.argv[1:])
