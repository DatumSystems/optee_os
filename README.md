# OP-TEE Trusted OS
This git contains source code for the secure side implementation of OP-TEE
project.

All official OP-TEE documentation has moved to http://optee.readthedocs.io.

// OP-TEE core maintainers

# Datum Notes
You need to install the Python elftools:
```
> sudo apt install python3-pyelftools
```

To build the TEE OS stand-alone (from STMicro Wiki):
> export PATH=<path-to-toolchain>:$PATH
> export CROSS_COMPILE=<toolchain-prefix>-
> cd <optee-os>
> make PLATFORM=stm32mp1 \
           CFG_EMBED_DTB_SOURCE_FILE=stm32mp157c-ev1.dts \
           CFG_TEE_CORE_LOG_LEVEL=2 O=build all

For Mark's system:
```
cd optee_os
export PATH=/var/gcc-arm-9.2-2019.12-x86_64-arm-none-linux-gnueabihf/bin:$PATH
export CROSS_COMPILE=arm-none-linux-gnueabihf-
make PLATFORM=stm32mp1 CFG_EMBED_DTB_SOURCE_FILE=stm32mp153c-brick-mx.dts CFG_TEE_LOG_LEVEL=2 CFG_STM32MP15=y O=build all
```

Another useful make target is the memory size required:

You can check the memory usage by using the make mem_usage target in optee_os, for example:
```
$ make ... mem_usage
# Which will output a file with the figures here:
# out/arm/core/tee.mem_usage
```
You will of course get different sizes depending on what compile time flags you have enabled when running make mem_usage.


