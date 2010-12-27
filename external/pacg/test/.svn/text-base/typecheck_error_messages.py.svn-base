#!/usr/bin/python
from pacg.typesystem import *

def printErrorMessage(expr, expr_type):
  try:
    typecheck(expr, expr_type)
  except TypeCheckFailedError, err:
    message = err.getMessage()
    print message

printErrorMessage(0, str)
printErrorMessage("s", int)
printErrorMessage("s", Some(int))
printErrorMessage(["s"], [int])
printErrorMessage({"key1" : 0}, {"key1" : str})
printErrorMessage(["a"], {"key1" : str})
printErrorMessage({"key1" : 0}, {"key1" : str, "key2" : int})
printErrorMessage("a", [])
printErrorMessage([], ())
printErrorMessage([], [OneOrMore(int)])
printErrorMessage(["s"], [OneOrMore(int)])
printErrorMessage([], [int])
printErrorMessage([int], [])
printErrorMessage([{"key1" : 0}], [{"key1" : str}])
printErrorMessage({"key1" : 0}, {})
