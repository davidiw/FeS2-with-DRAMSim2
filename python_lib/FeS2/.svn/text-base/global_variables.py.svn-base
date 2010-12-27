import os, sys 
'''Specfies FeS2 global variables'''

FES2_HOME_DIR=None

try:
  FES2_HOME_DIR = os.environ["FES2_HOME"]
except KeyError:
  print "No environment variable FES2_HOME!"
  print "You didn't follow the instructions in INSTALL" 
  print "  (or forgot to source your shell rc file afterwards)"
  print "Please fix the problem and try again"
  sys.exit(1)

SIMICS_INSTALL_DIR=None
try:
  SIMICS_INSTALL_DIR = os.environ["SIMICS_INSTALL_DIR"]
except KeyError:
  print "No environment variable SIMICS_INSTALL_DIR!"
  print "You didn't follow the instructions in INSTALL" 
  print "  (or forgot to source your shell rc file afterwards)"
  print "Please fix the problem and try again"
  sys.exit(1)

FES2_COMMON_DIR = FES2_HOME_DIR + "/common/"
FES2_DECODER_DIR = FES2_HOME_DIR + "/decoder/"
FES2_DOC_DIR = FES2_HOME_DIR + "/doc/"
FES2_OBJ_DIR = FES2_HOME_DIR + "/obj/"
#FES2_PYRITE_DIR = FES2_HOME_DIR + "/pyrite/" # redundant with below
FES2_PYTHON_LIB_DIR = FES2_HOME_DIR + "/python_lib/"
FES2_SIMICS_DIR = FES2_HOME_DIR + "/simics/"
FES2_TEST_DIR = FES2_HOME_DIR + "/test/"
FES2_TEST_CONFIG_DIR = FES2_TEST_DIR + "/configs/"
FES2_TEST_SCRIPT_DIR = FES2_TEST_DIR + "/scripts/"
FES2_TEST_SOURCE_DIR = FES2_TEST_DIR + "/src/"
FES2_TEST_BIN_DIR = FES2_TEST_DIR + "/bin/"
FES2_TOOLS_DIR = FES2_HOME_DIR + "/tools/"
FES2_EXPERIMENTS_DIR = FES2_HOME_DIR + "/experiments/"
FES2_WORKLOAD_DIR = FES2_HOME_DIR + "/workloads/"
FES2_COMPILE_SCRIPT = FES2_WORKLOAD_DIR + "/simics-compile"

# simics-related
INSTALL_SIMICS_COMMAND = FES2_HOME_DIR + "/simics/install-simics.py"
SIMICS_BASE_DIR = FES2_HOME_DIR + "/simics/"
SIMICS_SYMLINK = SIMICS_BASE_DIR + "/simics"
SIMICS_WORKSPACE_DIR = SIMICS_BASE_DIR + "/FeS2-workspace/"
SIMICS_IMAGES_DIR = SIMICS_WORKSPACE_DIR
SIMICS_STARTUP_COMMANDS_FILE = SIMICS_INSTALL_DIR + "/config/startup-commands"
SIMICS_STARTUP_COMMAND = "add-directory " + SIMICS_IMAGES_DIR
SIMICS_WORKSPACE_SETUP_SCRIPT = SIMICS_INSTALL_DIR + "/bin/workspace-setup"
SIMICS_WORKSPACE_SETUP_COMMAND = SIMICS_WORKSPACE_SETUP_SCRIPT + " " + SIMICS_WORKSPACE_DIR
SIMICS_MODULES_DIR = SIMICS_WORKSPACE_DIR + "/modules/"
SIMICS_SCRIPTS_DIR = SIMICS_BASE_DIR + "/scripts"
SIMICS_MODULE_NAME = "pyrite"
# TODO: belong above?
SIMICS_MODULE_SOURCE_DIR = FES2_HOME_DIR + "/pyrite/"

SIMICS_MODULE_SYMLINK = SIMICS_MODULES_DIR + "/" + SIMICS_MODULE_NAME
SIMICS_CREATE_MODULE_PATH_COMMAND = "ln -s " + SIMICS_MODULE_SOURCE_DIR + " " + SIMICS_MODULE_SYMLINK
SIMICS_REMOVE_SIMICS_SYMLINK_COMMAND = "rm " + SIMICS_SYMLINK
SIMICS_CREATE_SIMICS_SYMLINK_COMMAND = "ln -s " + SIMICS_INSTALL_DIR + " " + SIMICS_SYMLINK
SIMICS_STARTUP_SCRIPTS_DIR = SIMICS_WORKSPACE_DIR + "/targets/x86-440bx"
SIMICS_STARTUP_SCRIPT_SUFFIX = "-common.simics"
SIMICS_CONFIGS_DIR = SIMICS_BASE_DIR + "/configs"
SIMICS_BOOT_SCRIPT_SUFFIX = "-boot.simics"
SIMICS_BOOT_CONFIGS_DIR = FES2_EXPERIMENTS_DIR + "/configs/" 

# parameterized "global variables"
def SIMICS_STARTUP_SCRIPT_NAME(machine_name):
  return machine_name + SIMICS_STARTUP_SCRIPT_SUFFIX

def SIMICS_STARTUP_SCRIPT_PATH(machine_name):
  return SIMICS_STARTUP_SCRIPTS_DIR + "/" + SIMICS_STARTUP_SCRIPT_NAME(machine_name)

def SIMICS_BOOT_SCRIPT_NAME(machine_name):
  return machine_name + SIMICS_BOOT_SCRIPT_SUFFIX

def SIMICS_BOOT_SCRIPT_PATH(machine_name):
  return FES2_EXPERIMENTS_DIR + "/" + SIMICS_BOOT_SCRIPT_NAME(machine_name)

def SIMICS_BOOT_CONFIG_NAME(machine_name, frequency, number_processors):
  return machine_name + str(frequency) + "MHz-" + str(number_processors) + "p-after-boot.config"

def SIMICS_BOOT_CONFIG_PATH(machine_name, frequency, number_processors):
  return SIMICS_BOOT_CONFIGS_DIR + "/" + SIMICS_BOOT_CONFIG_NAME(machine_name, frequency, number_processors)
  
