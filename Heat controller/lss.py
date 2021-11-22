Import("env", "projenv")
from os import makedirs

env.AddPostAction(
	"$BUILD_DIR/${PROGNAME}.elf",
	env.VerboseAction("arm-none-eabi-objdump -h -S \"$BUILD_DIR/${PROGNAME}.elf\" > \"hex/"+env['BOARD_MCU']+".lss\"",
	"Creating $BUILD_DIR/${PROGNAME}.lss")
)

