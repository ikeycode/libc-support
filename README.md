### libc-support

This project aims to give portable solutions to our current problem: missing
core tooling in the musl distribution.

#### getconf

getconf is used throughout many shell scripts and utilities to obtain system
configuration values and paths. It is present on almost every \*nix compatible
OS.

#### getent

getent extracts values from system databases, such as shadow, files and hosts.

**NOTE**: This tool is still in development and is not shipping in the tarball.

#### mDNS support

This won't be our immediate focus, but we will need mDNS support for our use
case and even upstream musl has expressed interest in a future mDNS solution.

#### Note

While many variants of `getconf` and `getent` do exist in various BSD distributions,
and indeed, resourced for a handful of Linux distributions, the age, history and
quality can not all be assessed.

We will seek clean-room implementations with verifiable git history, proper tagging,
documentation and static analysis.

### Authors

Original author Jouni Roivas

Copyright © 2020 Serpent OS Developers

Available under the terms of the MIT license

