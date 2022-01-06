[TOC]

# CMake Tutorial

> https://cliutils.gitlab.io/modern-cmake/

## Introduction

### Make/Ninja

**buildsystem** 

it drives the compiler and other build tools to build your code.

### CMake

a **generator of buildsystems**.

It can produce Makefiles, it can produce Ninja build files, it can produce KDEvelop or Xcode projects, it can produce Visual Studio solutions. From the same starting point, the same CMakeLists.txt file.

## *Basic* CMakeLists.txt

You can add [comments](https://cmake.org/cmake/help/latest/manual/cmake-language.7.html#comments) with the `#` character.

### cmake_minimum_required()

```cmake
cmake_minimum_required(VERSION <min>[...<policy_max>])

e.g:
cmake_minimum_required(VERSION 3.1)
or
cmake_minimum_required(VERSION 3.1...3.15)
```

Here's the first line of every `CMakeLists.txt`.

This command will set the value of the [`CMAKE_MINIMUM_REQUIRED_VERSION`](https://cmake.org/cmake/help/latest/variable/CMAKE_MINIMUM_REQUIRED_VERSION.html#variable:CMAKE_MINIMUM_REQUIRED_VERSION) variable to `<min>`.

The command name [`cmake_minimum_required`](https://cmake.org/cmake/help/latest/command/cmake_minimum_required.html) is **case insensitive**, so the common practice is to **use lower case**.

>  https://cmake.org/cmake/help/latest/manual/cmake-policies.7.html



### project()

Now, every **top-level CMake file** will have the next line:

```cmake
project(<PROJECT-NAME>
        [VERSION <major>[.<minor>[.<patch>[.<tweak>]]]]
        [DESCRIPTION <project-description-string>]
        [HOMEPAGE_URL <url-string>]
        [LANGUAGES <language-name>...])
        
e.g:
project(MyProject VERSION 1.0
                  DESCRIPTION "Very nice project"
                  LANGUAGES CXX)
```

Sets the name of the project, and stores it in the variable [`PROJECT_NAME`](https://cmake.org/cmake/help/latest/variable/PROJECT_NAME.html#variable:PROJECT_NAME).

Also sets the variables:

- [`PROJECT_SOURCE_DIR`](https://cmake.org/cmake/help/latest/variable/PROJECT_SOURCE_DIR.html#variable:PROJECT_SOURCE_DIR), [`_SOURCE_DIR`](https://cmake.org/cmake/help/latest/variable/PROJECT-NAME_SOURCE_DIR.html#variable:_SOURCE_DIR)
    - Absolute path to the source directory for the project.
    - This is the source directory of the last call to the [`project()`](https://cmake.org/cmake/help/latest/command/project.html#command:project) command made in the current directory scope or one of its parents. 

- [`PROJECT_BINARY_DIR`](https://cmake.org/cmake/help/latest/variable/PROJECT_BINARY_DIR.html#variable:PROJECT_BINARY_DIR), [`_BINARY_DIR`](https://cmake.org/cmake/help/latest/variable/PROJECT-NAME_BINARY_DIR.html#variable:_BINARY_DIR)
    - Absolute path to the binary directory for the project.



### add_executable() / add_library()

```cmake
add_executable(<name> 
               [source1] [source2 ...])
               
add_library(<name> [STATIC | SHARED | MODULE]
            [<source>...])
            
add_library(<name> ALIAS <target>)

e.g:
add_library(lib1 lib1.cpp)
add_library(Upstream::lib1 ALIAS lib1)
```

The `<name>` corresponds to the logical target name and must be **globally unique within a project**. 

The headers will be, for most intents and purposes, ignored; the only reason to list them is to get them to show up in IDEs.



Adds an **executable target** called `<name>` to be built from the source files listed in the command invocation.

------

Adds a **library target** called `<name>` to be built from the source files listed in the command invocation. ( `lib<name>.a` or `<name>.lib`)

- `STATIC` libraries are archives of object files for use when linking other targets. 

- `SHARED` libraries are linked dynamically and loaded at runtime. 

- `MODULE` libraries are plugins that are not linked into other targets but may be loaded dynamically at runtime using dlopen-like functionality. 

- If no type is given explicitly the type is `STATIC` or `SHARED` based on whether the current value of the variable [`BUILD_SHARED_LIBS`](https://cmake.org/cmake/help/latest/variable/BUILD_SHARED_LIBS.html#variable:BUILD_SHARED_LIBS) is `ON`.

### add information about *Targets*

#### target_include_directories()

Now we've specified a target, how do we add information about it? For example, maybe it needs an include directory:

```cmake
target_include_directories(<target>
  <INTERFACE|PUBLIC|PRIVATE> [items1...]
  [<INTERFACE|PUBLIC|PRIVATE> [items2...] ...])
  
e.g:
target_include_directories(one PUBLIC include/)
```

**Specifies include directories to use when compiling a given target.** 

The named `<target>` must have been created by a command such as [`add_executable()`](https://cmake.org/cmake/help/latest/command/add_executable.html#command:add_executable) or [`add_library()`](https://cmake.org/cmake/help/latest/command/add_library.html#command:add_library) and must not be an [ALIAS target](https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html#alias-targets).

- `PRIVATE` and `PUBLIC` items will populate the [`INCLUDE_DIRECTORIES`](https://cmake.org/cmake/help/latest/prop_tgt/INCLUDE_DIRECTORIES.html#prop_tgt:INCLUDE_DIRECTORIES) property of `<target>`. 
- `PUBLIC` and `INTERFACE` items will populate the [`INTERFACE_INCLUDE_DIRECTORIES`](https://cmake.org/cmake/help/latest/prop_tgt/INTERFACE_INCLUDE_DIRECTORIES.html#prop_tgt:INTERFACE_INCLUDE_DIRECTORIES) property of `<target>`. 

`PUBLIC` doesn't mean much for an executable; for a library it lets CMake know that any targets that link to this target must also need that include directory. 

Other options are `PRIVATE` (only affect the current target, not dependencies), and `INTERFACE` (only needed for dependencies).



#### target_link_libraries()

We can then chain targets:

```cmake
e.g:
add_library(another STATIC another.cpp another.h)
target_link_libraries(another PUBLIC one)
# If no target of that name (one) exists, then it adds a link to a library called one on your path (hence the name of the command). Or you can give it a full path to a library. Or a linker flag.

target_link_libraries(<target>
                      <PRIVATE|PUBLIC|INTERFACE> <item>...
                     [<PRIVATE|PUBLIC|INTERFACE> <item>...]...)
```

Libraries and targets following `PUBLIC` are linked to, and are made part of the link interface. Libraries and targets following `PRIVATE` are linked to, but are not made part of the link interface. Libraries following `INTERFACE` are appended to the link interface and are not used for linking `<target>`.

Each `<item>` may be:

- **A library target name**
- **A full path to a library file**
    - `/usr/lib/libfoo.so`
- **A plain library name**
    -  `-lfoo` or `foo.lib`
- **A link flag**
- ...

#### target_compile_features()

```cmake
target_compile_features(<target> <PRIVATE|PUBLIC|INTERFACE> <feature> [...])
```

Add expected compiler features to a target.

`PRIVATE` and `PUBLIC` items will populate the [`COMPILE_FEATURES`](https://cmake.org/cmake/help/latest/prop_tgt/COMPILE_FEATURES.html#prop_tgt:COMPILE_FEATURES) property of `<target>`. `PUBLIC` and `INTERFACE` items will populate the [`INTERFACE_COMPILE_FEATURES`](https://cmake.org/cmake/help/latest/prop_tgt/INTERFACE_COMPILE_FEATURES.html#prop_tgt:INTERFACE_COMPILE_FEATURES) property of `<target>`. Repeated calls for the same `<target>` append items.

>  https://cmake.org/cmake/help/latest/prop_gbl/CMAKE_C_KNOWN_FEATURES.html#prop_gbl:CMAKE_C_KNOWN_FEATURES
>
> https://cmake.org/cmake/help/latest/prop_gbl/CMAKE_CXX_KNOWN_FEATURES.html#prop_gbl:CMAKE_CXX_KNOWN_FEATURES

### EXAMPLE 1

```cmake
cmake_minimum_required(VERSION 3.8)

project(Calculator LANGUAGES CXX)

add_library(calclib STATIC src/calclib.cpp include/calc/lib.hpp)
target_include_directories(calclib PUBLIC include)
target_compile_features(calclib PUBLIC cxx_std_11)

add_executable(calc apps/calc.cpp)
target_link_libraries(calc PUBLIC calclib)
```



### Variables

> https://cmake.org/cmake/help/latest/manual/cmake-commands.7.html#scripting-commands
>
> https://cmake.org/cmake/help/latest/command/string.html
>
> https://cmake.org/cmake/help/latest/command/list.html

#### Local Variables

**CMake has the concept of scope.**

```cmake
set(MY_VARIABLE "value")

set(MY_LIST "one" "two")  # equals to
set(MY_LIST "one;two")    # which internally become ; separated values.
```

The names of variables are **usually all caps**, and the value follows. 

You access a variable by using `${}`, such as `${MY_VARIABLE}`.

Note that an **unquoted value in CMake is the same as a quoted one if there are no spaces in it**.
This allows you to skip the quotes most of the time when working with value that you know could not contain spaces. (never write `${MY_PATH}`, always should be `"${MY_PATH}"`).

#### Cache Variables

....

The cache is actually just a text file, `CMakeCache.txt`, that gets created in the build directory when you run CMake. This is how CMake remembers anything you set, so you don't have to re-list your options every time you rerun CMake.



#### Environment variables

...



#### Properties

The other way CMake stores information is in properties. This is like a variable, but it is attached to some other item, like a directory or a target.

```cmake
set (CMAKE_CXX_STANDARD 17)

set_property(TARGET TargetName
             PROPERTY CXX_STANDARD 11)

set_target_properties(TargetName PROPERTIES
                      CXX_STANDARD 11)
```

Many target properties are initialized from a matching variable with `CMAKE_` at the front. So setting `CMAKE_CXX_STANDARD`, for example, will mean that all new targets created will have `CXX_STANDARD` set to that when they are created. 

> https://cmake.org/cmake/help/latest/manual/cmake-properties.7.html



## Programming in CMake

### if()

```cmake
if(variable)
    # If variable is `ON`, `YES`, `TRUE`, `Y`, or non zero number
else()
    # If variable is `0`, `OFF`, `NO`, `FALSE`, `N`, `IGNORE`, `NOTFOUND`, `""`, or ends in `-NOTFOUND`
endif()
# If variable does not expand to one of the above, CMake will expand it then try again
```

as long as the minimum version of CMake is **3.1+**, you can do:

```cmake
if("${variable}")
    # True if variable is not false-like
else()
    # Note that undefined variables would be `""` thus false
endif()
```

### generator-expressions

Most CMake commands happen at configure time, include the if statements seen above. But what if you need logic to occur at build time or even install time? Generator expressions were added for this purpose.[1](https://cliutils.gitlab.io/modern-cmake/chapters/basics/functions.html#fn_1) They are evaluated in target properties.



some example:

```cmake
# If you want to put a compile flag only for the DEBUG configuration, for example, you could do:
target_compile_options(MyTarget PRIVATE "$<$<CONFIG:Debug>:--my-flag>")


```

> https://cmake.org/cmake/help/latest/manual/cmake-generator-expressions.7.html



### Macros and Functions

The only difference between a function and a macro is scope; macros don't have one. 

An example of a simple function is as follows:

```cmake
function(SIMPLE REQUIRED_ARG)
    message(STATUS "Simple arguments: ${REQUIRED_ARG}, followed by ${ARGN}")
    set(${REQUIRED_ARG} "From SIMPLE" PARENT_SCOPE)
endfunction()

simple(This Foo Bar)
message("Output: ${This}")
```

The output would be:

```cmake
-- Simple arguments: This, followed by Foo;Bar
Output: From SIMPLE
```

If you want positional arguments, they are listed **explicitly**, and all other arguments are collected in `ARGN` (`ARGV` holds all arguments, even the ones you list).

不加`PARENT_SCOPE`是值传递,加了是引用传递. 大致可以这么理解吧.

## How to structure your project

```
- project
  - .gitignore
  - README.md
  - LICENCE.md
  - CMakeLists.txt
  - cmake
    - FindSomeLib.cmake
    - something_else.cmake
  - include
    - project
      - lib.hpp
  - src
    - CMakeLists.txt
    - lib.cpp
  - apps
    - CMakeLists.txt
    - app.cpp
  - tests
    - CMakeLists.txt
    - testlib.cpp
  - docs
    - CMakeLists.txt
  - extern
    - googletest
  - scripts
    - helper.py
```

1. 使`include/`保持干净, 能够直接拷贝到类似`/usr/include/`的目录中,and not have any extra files or cause any conflicts
2. the `CMakeLists.txt` files are split up over all source directories, and are not in the include directories.
3. `extern` folder should contain git submodules almost exclusively. 
4. `/build* `in your `.gitignore`

> choose your license here.  https://ufal.github.io/public-license-selector/

#### 

## EXAMPLE 2

### A simple example

```cmake
# Almost all CMake files should start with this
# You should always specify a range with the newest
# and oldest tested versions of CMake. This will ensure
# you pick up the best policies.
cmake_minimum_required(VERSION 3.1...3.22)

# This is your project statement. You should always list languages;
# Listing the version is nice here since it sets lots of useful variables
project(
  ModernCMakeExample
  VERSION 1.0
  LANGUAGES CXX)

# If you set any CMAKE_ variables, that can go here.
# (But usually don't do this, except maybe for C++ standard)

# Find packages go here.

# You should usually split this into folders, but this is a simple example

# This is a "default" library, and will match the *** variable setting.
# Other common choices are STATIC, SHARED, and MODULE
# Including header files here helps IDEs but is not required.
# Output libname matches target name, with the usual extensions on your system
add_library(MyLibExample simple_lib.cpp simple_lib.hpp)

# Link each target with other targets or add options, etc.

# Adding something we can run - Output name matches target name
add_executable(MyExample simple_example.cpp)

# Make sure you link your targets with this command. It can also link libraries and
# even flags, so linking a target that does not exist will not give a configure-time error.
target_link_libraries(MyExample PRIVATE MyLibExample)
```



### An extended example

> https://gitlab.com/CLIUtils/modern-cmake/tree/master/examples/extended-project

```
.
├── apps
│   ├── app.cpp
│   └── CMakeLists.txt
├── cmake
│   ├── add_FetchContent_MakeAvailable.cmake
│   └── FindSomeLib.cmake
├── CMakeLists.txt
├── docs
│   ├── CMakeLists.txt
│   └── mainpage.md
├── include
│   └── modern
│       └── lib.hpp
├── README.md
├── extern
│   └── googletest
├── src
│   ├── CMakeLists.txt
│   └── lib.cpp
└── tests
    ├── CMakeLists.txt
    └── testlib.cpp
```



##### ./CMakeLists.txt

```cmake
# Works with 3.11 and tested through 3.22
cmake_minimum_required(VERSION 3.11...3.22)

# Project name and a few useful settings. Other commands can pick up the results
project(
  ModernCMakeExample
  VERSION 0.1
  DESCRIPTION "An example project with CMake"
  LANGUAGES CXX)

# Only do these if this is the main project, and not if it is included through add_subdirectory
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

  # Optionally set things like CMAKE_CXX_STANDARD, CMAKE_POSITION_INDEPENDENT_CODE here

  # Let's ensure -std=c++xx instead of -std=g++xx
  set(CMAKE_CXX_EXTENSIONS OFF)

  # Let's nicely support folders in IDEs
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  # Testing only available if this is the main app
  # Note this needs to be done in the main CMakeLists
  # since it calls enable_testing, which must be in the
  # main CMakeLists.
  include(CTest)

  # Docs only available if this is the main app
  find_package(Doxygen)
  if(Doxygen_FOUND)
    add_subdirectory(docs)
  else()
    message(STATUS "Doxygen not found, not building docs")
  endif()
endif()

# FetchContent added in CMake 3.11, downloads during the configure step
include(FetchContent)
# FetchContent_MakeAvailable was not added until CMake 3.14; use our shim
if(${CMAKE_VERSION} VERSION_LESS 3.14)
  include(cmake/add_FetchContent_MakeAvailable.cmake)
endif()

# Accumulator library
# This is header only, so could be replaced with git submodules or FetchContent
find_package(Boost REQUIRED)
# Adds Boost::boost

# Formatting library
FetchContent_Declare(
  fmtlib
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 5.3.0)
FetchContent_MakeAvailable(fmtlib)
# Adds fmt::fmt

# The compiled library code is here
add_subdirectory(src)

# The executable code is here
add_subdirectory(apps)

# Testing only available if this is the main app
# Emergency override MODERN_CMAKE_BUILD_TESTING provided as well
if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR MODERN_CMAKE_BUILD_TESTING)
   AND BUILD_TESTING)
  add_subdirectory(tests)
endif()

```

> [The difference is that `CMAKE_PROJECT_NAME` is the name from the last `project` call from the root CMakeLists.txt, while `PROJECT_NAME` is from the last `project` call, regardless from the location of the file containing the command.](https://stackoverflow.com/a/38940669/11803107)



##### ./apps/CMakeLists.txt

```cmake
add_executable(app app.cpp)
target_compile_features(app PRIVATE cxx_std_17)

target_link_libraries(app PRIVATE modern_library fmt::fmt)
```

##### ./src/CMakeLists.txt

```cmake
# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

# Optionally glob, but only for CMake 3.12 or later:
# file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${ModernCMakeExample_SOURCE_DIR}/include/modern/*.hpp")
set(HEADER_LIST "${ModernCMakeExample_SOURCE_DIR}/include/modern/lib.hpp")

# Make an automatic library - will be static or dynamic based on user setting
add_library(modern_library lib.cpp ${HEADER_LIST})

# We need this directory, and users of our library will need it too
target_include_directories(modern_library PUBLIC ../include)

# This depends on (header only) boost
target_link_libraries(modern_library PRIVATE Boost::boost)

# All users of this library will need at least C++11
target_compile_features(modern_library PUBLIC cxx_std_11)

# IDEs should put the headers in a nice place
source_group(
  TREE "${PROJECT_SOURCE_DIR}/include"
  PREFIX "Header Files"
  FILES ${HEADER_LIST})
```

##### ./tests/CMakeLists.txt

```cmake
# Testing library
FetchContent_Declare(
  catch
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v2.13.6)
FetchContent_MakeAvailable(catch)
# Adds Catch2::Catch2

# Tests need to be added as executables first
add_executable(testlib testlib.cpp)

# I'm using C++17 in the test
target_compile_features(testlib PRIVATE cxx_std_17)

# Should be linked to the main library, as well as the Catch2 testing library
target_link_libraries(testlib PRIVATE modern_library Catch2::Catch2)

# If you register a test, then ctest and make test will run it.
# You can also run examples and check the output, as well.
add_test(NAME testlibtest COMMAND testlib) # Command can be a target

```



##### ./docs/CMakeLists.txt

```cmake
set(DOXYGEN_EXTRACT_ALL YES)
set(DOXYGEN_BUILTIN_STL_SUPPORT YES)

doxygen_add_docs(docs modern/lib.hpp "${CMAKE_CURRENT_SOURCE_DIR}/mainpage.md"
                 WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/include")
```



##### ./cmake/add_FetchContent_MakeAvailable.cmake

```cmake
macro(FetchContent_MakeAvailable NAME)
  FetchContent_GetProperties(${NAME})
  if(NOT ${NAME}_POPULATED)
    FetchContent_Populate(${NAME})
    add_subdirectory(${${NAME}_SOURCE_DIR} ${${NAME}_BINARY_DIR})
  endif()
endmacro()
```





## Set C++ Standards

```cmake
set(CMAKE_CXX_STANDARD 11 CACHE STRING "The C++ standard to use")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
```

The first line sets a C++ standard level, 
and the second tells CMake to use it,
and the final line is optional and ensures `-std=c++11` vs. something like `-std=g++11`. 

This method isn't bad for a final package, but **shouldn't be used by a library**. You should always set this as a cached variable, so you can override it to try a new version easily (or if this gets used as a library, this is the only way to override it - but again, don't use this for libraries). 

```cmake
set_target_properties(myTarget PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
```

Which is better, but still doesn't have the sort of explicit control that compiler features have for populating `PRIVATE` and `INTERFACE` properties, so it really is only useful on final targets.



> Don't set manual flags yourself. You'll then become responsible for mainting correct flags for every release of every compiler, error messages for unsupported compilers won't be useful, and some IDEs might not pick up the manual flags.



## Submodule

> https://git-scm.com/book/en/v2/Git-Tools-Submodules

`git submodule add ../../owner/repo.git extern/repog`

e.g:

`git submodule add --depth 1 git@github.com:gabime/spdlog.git extern/spdlog `

```cmake
# ...
add_subdirectory(extern/spdlog)
target_link_libraries(sqlight spdlog::spdlog)
```



