'''Collection of utility functions'''
import shell, os, glob
from typesystem import *

def chopNewline(string):
  '''!Chops the trailing newline off input string.
@param string string
@returns string'''
  mustBeString(string)
  assert (string[-1] == "\n"), string + " doesn't end in a newline"
  ret = string[:-1]
  return ret

def stringIsWhitespace(string):
  '''!Returns True iff input string consists only of whitespace characters.
@param string string
@returns bool'''
  mustBeString(string)
  s1 = string.replace(" ", "")
  s2 = s1.replace("\t", "")
  ret = (s2 == "")
  return ret

def quote(string):
  '''!Returns input string enclosed in double-quotes.
@param string string
@returns string'''
  mustBeString(string)
  return '"' + string + '"'

def initialCaps(s):
    """!Returns a copy of `s' with its first character made uppercase;
all other characters are left alone.
@param s string
@returns string"""
    return s[0].upper() + s[1:]

def getAllPythonModulesFromDirectory(directory):
  '''!Returns a list of all .py files in the given directory, stripped of directory
prefix and .py suffix (suitable for use in __init__.py files)
@param directory string
@returns [string, ...]'''
  mustBeString(directory)
  shell.assertFileExists(directory, "directory")
  modules_list = glob.glob(directory + "/*.py")
  init_file = os.path.join(directory, "__init__.py")
  modules_list = filter((lambda s: s != init_file), modules_list)
  module_names = [ shell.basename(module, ".py") for module in modules_list ]
  return module_names

