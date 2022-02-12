# Build Descent 1 for DOS

This repo contains makefiles and support files that allow you to build Descent 1 for DOS.
It uses a binextr utility to extracts the necessary external libraries from an existing
descentr.exe that you'll need to provide.
It also contains a few patches the source code to make it equal to the
Descent 1.5 exe.

The only differences in the resulting exe are different uninitialized padding bytes between strings and a different order for the relocation metadata.

You'll need Watcom 9.5b, MASM 6.11, the Descent 1.5 exe and a DOS(Box)/Win32/wine environment.
For wine, patch wlink.exe with [this patch](https://gist.github.com/arbruijn/acea8253bf454e2e2f1f928ae1b7bc90).

The Descent exe must be copied to the `original` directory.

Start compilation with `wmake` in the top directory.
