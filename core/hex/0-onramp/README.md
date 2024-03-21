This is the first stage hex tool for the Onramp VM. It is written in hexadecimal Onramp bytecode.

This tool is not platform-specific but it must be built in a platform-specific way. Relative to the root of the Onramp source tree, the output must be placed in:

```
build/intermediate/hex-0-onramp/hex.oe
```

See the [Setup Guide](../../../docs/setup-guide.md) for more information.

Build scripts exist in [`platform/hex/onramp/`](../../../platform/hex/onramp/) to build and test this for specific platforms.
