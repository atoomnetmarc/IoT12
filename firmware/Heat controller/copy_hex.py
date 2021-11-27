Import("env", "projenv")
from shutil import copyfile

def copyhex(*args, **kwargs):
    copyfile(str(kwargs['target'][0]), 'hex/'+env['BOARD_MCU']+'.hex')

env.AddPostAction("$BUILD_DIR/${PROGNAME}.hex", copyhex)
