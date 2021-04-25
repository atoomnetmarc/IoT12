Import("env", "projenv")

env.AddPostAction(
	"$BUILD_DIR/${PROGNAME}.elf",
	env.VerboseAction("arm-none-eabi-objdump -h -S \"$BUILD_DIR/${PROGNAME}.elf\" > \"$BUILD_DIR/${PROGNAME}.lss\"",
	"Creating $BUILD_DIR/${PROGNAME}.lss")
)
