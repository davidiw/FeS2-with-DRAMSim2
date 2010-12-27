#!/usr/bin/python

import os, sys
from FeS2_imports import *

# install simics
print "Installing simics..."
shell.shellCommand(INSTALL_SIMICS_COMMAND)
print "Done installing simics"
