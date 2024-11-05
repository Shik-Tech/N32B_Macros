Import("env")

# Custom Tools.
Config = env.GetProjectConfig()

uf2conv_cmd = Config.get("custom_tools", "uf2conv")
uf2_family = Config.get("custom_tools", "uf2_family")
uf2_baseaddr = Config.get("custom_tools", "uf2_baseaddr")

# Custom HEX from ELF
env.AddPostAction(
    "$BUILD_DIR/${PROGNAME}.elf",
    env.VerboseAction(" ".join([
        "$OBJCOPY", "-O", "ihex", "-R", ".eeprom",
        "$BUILD_DIR/${PROGNAME}.elf", "$BUILD_DIR/${PROGNAME}.hex"
    ]), "Building $BUILD_DIR/${PROGNAME}.hex")
)

# Generate .bin file.
env.AddPostAction(
    "$BUILD_DIR/${PROGNAME}.elf",
    env.VerboseAction(" ".join([
        "$OBJCOPY", "-O", "binary",
        "$BUILD_DIR/${PROGNAME}.elf", "$BUILD_DIR/${PROGNAME}.bin"
    ]), "Building $BUILD_DIR/${PROGNAME}.bin")
)

# Generate .uf2 file.
env.AddPostAction(
    "$BUILD_DIR/${PROGNAME}.bin",
    env.VerboseAction(" ".join([
        uf2conv_cmd, "-f", uf2_family, "-b", uf2_baseaddr,
        "--convert",
        "$BUILD_DIR/${PROGNAME}.bin", "-o", "$BUILD_DIR/${PROGNAME}.uf2"
    ]), "Building $BUILD_DIR/${PROGNAME}.uf2")
)
