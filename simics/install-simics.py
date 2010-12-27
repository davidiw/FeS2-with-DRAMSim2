#!/usr/bin/python
# Script that downloads and installs the current simics version

import os, sys
from FeS2_imports import *

################# MAIN #################################

# setup workspace
print "Setting up FeS2 workspace..."
shell.shellCommand(SIMICS_WORKSPACE_SETUP_COMMAND)

# add simlink to our module
if not shell.doesFileExist(SIMICS_MODULE_SYMLINK):
  print "Adding symlink to FeS2 module..."
  shell.shellCommand(SIMICS_CREATE_MODULE_PATH_COMMAND)

# set up symlink
print "Adding symlink to simics directory..."
if shell.doesFileExist(file=SIMICS_SYMLINK):
  shell.shellCommand(SIMICS_REMOVE_SIMICS_SYMLINK_COMMAND)
shell.shellCommand(SIMICS_CREATE_SIMICS_SYMLINK_COMMAND)

print "My work here is done"
