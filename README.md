### libc-support

This project will eventually host some compatibility tooling making it easier
to integrate `musl` into Serpent OS, as a modern Linux distribution.

Upstream musl currently lacks some tooling and support that we eventually plan
to include:

#### getconf

getconf is used throughout many shell scripts and utilities to obtain system
configuration values and paths. It is present on almost every \*nix compatible
OS.

#### getent

getent extracts values from system databases, such as shadow, files and hosts.

#### mDNS support

This won't be our immediate focus, but we will need mDNS support for our use
case and even upstream musl has expressed interest in a future mDNS solution.

#### Note

While many variants of `getconf` and `getent` do exist in various BSD distributions,
and indeed, resourced for a handful of Linux distributions, the age, history and
quality can not all be assessed.

We will seek clean-room implementations with verifiable git history, proper tagging,
documentation and static analysis.
