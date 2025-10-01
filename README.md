# fmtlog (patched)

This repository maintains modified `fmtlog` source for server
type projects that have a few additional requirements for
source locations and log message format.

This repository is not maintained on a regular basis and
is updated only when, and if, the underlying projects require
new `fmtlog` functionality. Some of the options are updated
for completeness, but are not tested much.

See the upstream project for more information on how to use
`fmtlog`.

https://github.com/MengRao/fmtlog

# Changes

This section describes notable changes to the upstream `fmtlog`
source.

* Ability to log arbitrary source code locations.

  For example, an exception with source location may be thrown
  with a source location and logged to reflect where in the code
  the error occurred, rather than report the line in the catch
  statement:

  ```c++
  try {
    if(cond() == error)
      throw my_error_t(__FILE__, __LINE__, error, details);
  }
  catch (const my_err_t& error) {
      FMTLOG_ONCE_LOCATION(fmtlog::ERR,
            error.location(),
            "{:d} {:s}", error.code(), error.what());
  }
  ```

  A more complete change would be to modify logging macros,
  such as `logio`, but this would create a larger and more
  conflict-prone patch. See these macros in `fmtlog.h`, if
  you want to implement this change to keep code more
  consistent with `fmtlog`'s original syntax.

* Ability to rewrite formatted log messages to avoid generating
  malformed log files if log messages contain special characters,
  such as line breaks.

  For example, if a log message with line breaks are logged
  without filtering formatted log messages, such as shown below:

  ```c++
  logi("My message: {:s}", "ABC\n\nXYZ");
  ```

  , the log file will contain line breaks in a way that cannot
  be analyzed reliably or at all, such as this.

  ```
  10:54:33.002566 log_test.cc:112  INF[18900 ] My message: ABC

  XYZ
  ```

  Log message arguments may come from different components,
  which makes it hard to intercept these arguments when they
  are generated. A rewrite callback function may be used
  instead to rewrite formatted log messages.
  
  A rewrite callback function may be installed via `fmtlog::setMsgCB`
  and is expected to have the following signature.

  ```c++
  std::optional<std::string_view>
          msgCB(std::string_view msg,
                std::string& msgCBStr,
                void* userData);
  ```

  `msg` contains the current formatted log message. It is not
  null-terminated.

  `msgCBStr` is a string instance that is used as a buffer if
  the log message is to be rewritten. Its content is never
  evaluated directly and may contain arbitrary characters.
  Only the returned string view is evaluated, which may point
  to the string buffer or any other persistent string.
  
  `userData` is a user-provided pointer, which typically will
  be used to pass in some context, such as the logger instance.

  The function should return `std::nullopt` if the formatted
  log message can be used as-is. Otherwise it should contain
  a string view with the new log message.

  See `log_test.cpp` for a sample callback function. Make sure
  to avoid constructing string view temporaries in return values,
  which may result in invalid memory being passed to `fmtlog`.

* Ability to use externally-supplied `fmt` library, which may
  be installed with a package manager, such as `dnf` or `apt`,
  or manually to a shared 3rd-party source location.

  Header includes referencing `fmt` within `fmtlog` source
  were changed to use angle brackets, so locally-installed
  `fmt` source is not considered when `#include` statements
  are processed.

  See build instructions further in this file for additional
  information about using `fmt` with this project.

# Using `fmt`

The upstream repository utilizes `fmt` as a Git submodule and
may conflict with a standalone `fmt` installation, depending
on how include search paths are set up.

Another consideration is that `fmt` is a popular library and
most likely will be used by many projects on its own, so even
if include search paths are sorted out, having two versions
of `fmt` is harder to maintain.

The option to use the `fmt` source bundled with `fmtlog` was
retained for this repository, but it should be avoided in
favor of setting up `fmt` on its own.

There are two script files in this repository, `get-fmt.bat`
and `get-fmt.sh`, which can be used for experimentation and
as a starting point to set up more robust scripts to obtain
`fmt` for your projects.

Each script downloads `fmt` of the same version that `fmtlog`
references as the submodule (or compatible), builds it with
minimum options necessary for `fmtlog` and installs `fmt`
locally.

For your DevOps scripts, you will need to change the
installation location to the one appropriate for your project,
such as `/usr/local` on Linux and a shared 3rd-party source
location on Windows.

# CMake Options

Following CMake options may be specified when building
`fmtlog`.

* `-DFMTLOG_BUNDLED_FMT=ON/OFF`

  Indicates whether there is `fmt` directory in the `fmtlog`
  source, which may be restored as a Git submodule or as a
  standalone download.

  Using this option is not recommended. If you do use it,
  you may want to revert the change in how `fmt` headers are
  included within `fmtlog` source and use double quotes, so
  `fmt` headers in the `fmtlog` source tree are used.

* `-DFMTLOG_BUILD_TEST=ON/OFF`

  Indicates whether to build `fmtlog` test applications or
  not.

* `-DFMTLOG_FULL_BUFFER_BLOCK=ON/OFF`

  Indicates whether the `FMTLOG_BLOCK` is defined as `1`
  or `0`, which controls how `fmtlog` handles filling up
  the log buffer.

  If `FMTLOG_BLOCK` is defined as `1`, `fmtlog` will block
  until the buffer is flushed. If defined as `0`, which is
  the default, log messages will be discarded.

  **NOTE**: This flag is being evaluated in the inline method
  in `fmtlog` and must be defined for projects using `fmtlog`
  exactly the same way as this flag indicates.

* `-DBUILD_SHARED_LIBS=ON/OFF`

  This is standard CMake flag that builds a shared library
  instead of a static library. This flag is not supported
  for Windows builds because `fmtlog` source does not use
  export declarations for its methods and cannot be linked
  as a DLL.

* `-DCMAKE_BUILD_TYPE=Debug/Release`

  This is standard CMake flag that builds a debug or a
  release configuration for single-configuration build
  systems, such as Makefiles on Linux.

* `-DFMTLOG_FMT_INCLUDE_DIR=path/to/fmt/include`

  Points to the location of `fmt` header files. This option
  must be defined on Windows to point to the 3rd-party shared
  source location where `fmt` is installed.

  Projects built on Linux do not need this option defined
  if `fmt` is installed in one of standard locations, such
  as `/usr/include` or `/usr/local/include`.

* `-DFMTLOG_FMT_LIB_DIR=/path/to/fmt/lib`

  Points to the location of `fmt` library files. Everything
  described for the include files path option is applicable
  for this option as well.

# CMake Build

## Linux

Build and install `fmt` using options in `get-fmt.sh`, but
remove the `--prefix $BLD_DIR/fmt` to install it into the
default location under `/usr/local`.

Change into `fmtlog` source directory and use these commands
to build `fmtlog` as a static library for `Debug` and `Release`
configurations.

```bash
cmake -S . -B x64/Debug -DCMAKE_BUILD_TYPE=Debug
cmake --build x64/Debug
cmake --install x64/Debug

cmake -S . -B x64/Release -DCMAKE_BUILD_TYPE=Release
cmake --build x64/Release
cmake --install x64/Release
```

Copy `install_manifest.txt` from each build location to make
it easier to delete installed files later.

## Windows

For quick experiments, use the bundled `fmt` source either
by expanding Git submodule or downloading `fmt` source of
the right version manually and extracting it into the `fmt`
directory under `fmtlog`.

Use these commands to build using Git submodule commands.

```cmd
git submodule init fmt
git submodule update fmt
cmake -S . -B x64\Debug -DFMTLOG_BUILD_TEST=ON -DFMTLOG_BUNDLED_FMT=ON
cmake --build x64\Debug
```

For external `fmt` source, use these commands. `get-fmt.bat`
may be used as a sample for writing a more robust script.

Note that `fmt` paths should be absolute paths to work
reliably.

```cmd
cmake -S . -B x64 ^
  -DFMTLOG_BUILD_TEST=ON ^
  -DFMTLOG_FMT_INCLUDE_DIR=c:\3rd-party\fmt\include ^
  -DFMTLOG_FMT_LIB_DIR=c:\3rd-party\fmt\lib

cmake --build x64 --config Debug
cmake --build x64 --config Release
```

CMake installs generated output into `Program Files (x86)`
on Windows, which is not a very good idea because it requires
elevated permissions. Use explicit `--prefix` command to
install generated output into shared 3rd-party source
location, similar to this.

```
cmake --install x64 --config Debug --prefix c:\3rd-party\fmtlog
cmake --install x64 --config Release --prefix c:\3rd-party\fmtlog
```

# `fmtlog` Patching

While this repository can be used to build a patched version
of `fmtlog`, this is not a very robust way of maintaining
dependencies. A better way is to create a patch from this
repository and apply it against the source from the matching
tag in the upstream repository.

This repository is structured to maintain a patch branch
for specific tags in the upstream repository. For example,
a branch `v2-2-1-patches` contains changes against the tag
`v2.2.1`.

In order to produce a patch, run this command from the
source directory.

```
git diff v2.3.0 v2-3-0-patches
```

If you know which changes are relevant for your project,
you can list them explicitly. For example, you may not need
the GitHub workflow YAML file in your project.

```
git diff v2.3.0 v2-3-0-patches -- CMakeLists.txt fmtlog.h fmtlog-inl.h
```

This patch should be stored in your project's repository
and applied against the upstream source at the matching tag
name.

The upstream source may be downloaded directly from GitHub,
but a better option is to save it in your storage, such as a
file share or a cloud blob, and download it from there with
appropriate tools, such as `azcopy`.

This example uses GitHub to keep it simple.

On Windows the commands should be launched in the Visual Studio
developer's command prompt. `patch.exe` is available under
`%PROGRAMFILES%\Git\usr\bin\`. Replace Bash line continuation
character with `^`.

```
curl --location --output fmtlog-2.3.0.tar.gz \
    https://github.com/MengRao/fmtlog/archive/refs/tags/v2.3.0.tar.gz

tar -xzf fmtlog-2.3.0.tar.gz
cd fmtlog-2.3.0
patch --strip=1 --input=fmtlog-2.3.0.patch
```

After applying the patch, use CMake to build `fmtlog` libraries
for your project.
