# Onramp Hex Tool

`hex` is a hexadecimal converter. It converts plain text hexadecimal into binary, stripping comments and (optionally) performing address assertions.

This document describes the implementation of the tool. For a description of the language, see [Onramp Hexadecimal](../../docs/hexadecimal.md).



## Overview

In its simplest form, the hex tool converts hex bytes and ignores everything else. The below is a correct and sufficient implementation of an Onramp hex tool:

```
sed 's/[@;#].*//' | xxd -r -p
```

This is inspired by the "HEX1" tool from [bcompiler](https://web.archive.org/web/20160502230021fw_/http://homepage.ntlworld.com/edmund.grimley-evans/bcompiler.html) and the "hex0" tool from [stage0](https://bootstrapping.miraheze.org/wiki/Stage0). (See [Inspiration and Resources](inspirattion.md) for more.) Unlike those, additional hex features are not considered later stages of this tool but of [`ld`](../core/ld/), the Onramp linker. This hex tool instead adds optional address assertions to help write larger architecture-specific programs without labels.



## Implementations

The reference implementation is in [`c89/`](c89/).

The POSIX shell implementation in [`sh/`](sh/) is fully-featured and should work in any conforming shell. This is the default hex tool used on POSIX systems. Alternate shell implementations exist in [`sh-alt/`](sh-alt/).

The bootstrap implementation depends on your platform or on the scripting languages you have available.
