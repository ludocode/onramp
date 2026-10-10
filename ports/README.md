# Ports

This directory contains instructions, patches and build scripts to bootstrap third-party programs and libraries with Onramp.

Some projects may need significant patching to work within the limitations of Onramp's environment. Patches may be included here (and may have any license compatible with the project.)

Some projects also need new build scripts. This is usually because we want to build them before their dependencies (such as building a compiler before a make tool), or because they have generated build scripts or other components. We aim to avoid all generated code when bootstrapping. For some projects a pure source bootstrap is incomplete; such cases will be noted in the README.

If a project works out-of-the-box, the port may simply be a README explaining how to build it. In any case, if a port is here, read the README before trying to build it.
