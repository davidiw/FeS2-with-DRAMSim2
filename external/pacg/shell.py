'''Collection of library functions relating to the unix shell'''

import os, sys, shutil
import util
from typesystem import *

def readLineFromStdin():
  '''!Reads line from stdin, chopping off final newline.
@returns string'''
  reply = sys.stdin.readline()
  reply = util.chopNewline(reply)
  return reply

def promptUserForYesNoResponse(query):
  '''!Prints query and returns value of user's y/n reply.
@param query string
@returns bool'''
  mustBeString(query)
  print query
  sys.stdout.write("y/n: ")
  reply = readLineFromStdin()
  ret = True
  if reply != "y":
    ret = False
  return ret

def promptUserForContinue():
  '''!Prints "When ready to continue, hit Enter" to the screen and waits for 
user to hit Enter.
@returns void'''
  sys.stdout.write("When ready to continue, hit Enter")
  readLineFromStdin()

def shellCommand(command, error_message=None):
  '''!Evaluates given shell command and asserts that there was no error.
If there is an error, prints given error message (if none was given, prints a
generic error message).
@param command string
@param error_message string
@returns void'''
  mustBeString(command)
  if error_message == None:
    error_message = "Something went wrong with " + command
  mustBeString(error_message)
  ret = os.system(command)
  assert ret == 0, error_message

def downloadToFile(url, file):
  '''!Downloads the data at the given url and dumps into the given file.
@param url string
@param file string
@returns void'''
  mustBeString(url)
  mustBeString(file)
  download_cmd = "wget --no-check-certificate " + url + " -O " + file
  shellCommand(download_cmd)

def unpackTarball(tarball, destination):
  '''!Unpacks the given tarball into the given directory.
@param tarball string
@param destination string
@returns void'''
  mustBeString(tarball)
  mustBeString(destination)
  assertFileExists(destination, "directory")
  assertFileExists(tarball, "file")
  unpack_tarball_cmd = "tar -xvf " + tarball + " -C " + destination
  shellCommand(unpack_tarball_cmd)

def assertFileIsOfType(file, type):
  '''!Asserts that a file is of a certain type.
Allowed types are "file" and "directory".
@param file string
@param type string
@returns void'''
  mustBeString(file)
  mustBeString(type)
  assert os.path.exists(file), file + " doesn't exist" # for sanity
  assert type in ["file", "directory"]
  if type == "file":
    assert os.path.isfile(file), file + " exists but is not a regular file"
  elif type == "directory":
    assert os.path.isdir(file), file + " exists but is not a directory"

def assertFileExists(file, type=None):
  '''!Asserts that a given file exists, and optionally asserts that it is of a 
certain type.
@param file string
@param type string
@returns void
@sa assertFileIsOfType'''
  mustBeString(file)
  assert os.path.exists(file), file + " doesn't exist"
  if type == None:
    return
  mustBeString(type)
  assertFileIsOfType(file, type)

def doesFileExist(file, type=None):
  '''!Checks whether a given file exists, and optionally asserts that if it 
exists, it is of a certain type.
@param file string
@param type string
@returns bool
@sa assertFileIsOfType'''
  mustBeString(file)
  ret = os.path.exists(file)
  if (type == None) or (ret == False):
    return ret
  mustBeString(type)
  assertFileIsOfType(file, type)
  return ret

def pathJoinExists(*path):
  '''!Joins the given path components, asserts that the path exists, and returns
it.
@param path string string ...
@returns string'''
  mustBeType(path, (OneOrMore(str),))
  joined_path = os.path.join(*path)
  assertFileExists(joined_path)
  return joined_path

def promptUserForFileRemoval(file):
  '''!Asks if user wants to remove given file, and if so, removes it.
@param file string
@returns void'''
  mustBeString(file)
  assertFileExists(file, "file")
  query = "Remove file " + file + "?"
  reply_value = promptUserForYesNoResponse(query)
  if reply_value:
    print "Removing " + file
    os.remove(file)

def promptUserForDirectoryRemoval(dir):
  '''!Asks if user wants to remove given directory, and if so, removes it.
@param dir string
@returns void'''
  mustBeString(dir)
  assertFileExists(dir, "directory")
  query = "Remove directory " + dir + "?"
  reply_value = promptUserForYesNoResponse(query)
  if reply_value:
    print "Removing " + dir
    shutil.rmtree(dir)

def appendToFile(file, text):
  '''!Appends text to file, adding a trailing newline.
@param file string
@param text string
@returns void'''
  mustBeString(file)
  mustBeString(text)
  assertFileExists(file, "file")
  f = open(file, 'a')
  f.write(text + "\n")
  f.close()

def basename(file, suffix=None):
  '''!Strips directory prefix from file and optionally strips suffix as well.
@param file string
@param suffix string
@returns string'''
  mustBeString(file)
  ret = os.path.basename(file)
  if suffix == None:
    return ret
  mustBeString(suffix)
  assert ret.endswith(suffix), "File " + file + " doesn't end in " + suffix
  ret = ret[:-(len(suffix))]
  return ret
