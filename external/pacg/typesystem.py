"""Type system that adds expressivity to Python's base types.

A type-expression t is specified as follows::
  b := Python base type (e.g., int, str, etc)
  t := b | [t, t, ...] | {str : t, ...} | Some(t) | Optional(t) | ZeroOrMore(t) | OneOrMore(t) | Strict(t)

[t1, t2, ..., tn] specifies a list that contains n elements, the first of type 
t1, the second of type t2, and so on.

{"a1" : t1, "a2" : t2, ..., "an" : tn} specifies a dictionary that has
<b>at least</b> the keys "a1" through "an", with the key "a1" mapping
to an element of type t1, the key "a2" mapping to an element of type
t2, and so on.

Strict({...}) specifies a dictionary that <b>must</b> match the shape
of the provided type, instead of just having at least the shape of the
provided type.

Some(t) specifies an expression that can either be of type t or can be None.

Optional(t) is allowed in list entries and dictionary keys, and specifies an
element that is optional.

ZeroOrMore(t) is allowed in list entries, and specifies zero or more instances
of type t.

OneOrMore(t) is allowed in list entries, and specifies one or more instances
of type t.

EXAMPLES:
typecheck(0, int) -> True
typecheck(["a"], [str]) -> True
typecheck(["a"], [str, str]) -> False
typecheck(["a"], [ZeroOrMore(str)] -> True
typecheck({"a" : "b"}, {"a" : str}) -> True
typecheck({"a" : "b"}, {"b" : str}) -> False
typecheck(None, Some(int)) -> True
typecheck(None, int) -> False
typecheck([], [Optional(int)]) -> True

KNOWN BUGS:
The typechecker is overly greedy with respect to ZeroOrMore and OneOrMore,
causing an expression such as typecheck(["a", "a"], [ZeroOrMore(str), str]) to
return False."""

import types, re

class ZeroOrMore:
  """Represents a type-expression that is repeated zero or more times."""

  def __init__(self, inner_type):
    assert not isContainerType(inner_type), "cannot nest container types"
    self.inner_type = inner_type

  def unbox(self):
    """@returns inner_type"""
    return self.inner_type

  def __repr__(self):
    return "ZeroOrMore(" + str(self.inner_type) + ")"

class OneOrMore:
  """Represents a type-expression that is repeated one or more times."""

  def __init__(self, inner_type):
    assert not isContainerType(inner_type), "cannot nest container types"
    self.inner_type = inner_type

  def unbox(self):
    """@returns inner_type"""
    return self.inner_type
  
  def __repr__(self):
    return "OneOrMore(" + str(self.inner_type) + ")"

class Optional:
  """Represents a type-expression that may or may not
appear. Used for optional keys in a dictionary."""

  def __init__(self, inner_type):
    assert not isContainerType(inner_type), "cannot nest container types"
    self.inner_type = inner_type

  def unbox(self):
    """@returns inner_type"""
    return self.inner_type

  def __repr__(self):
    return "Optional(" + str(self.inner_type) + ")"

class Some:
  """Represents a type-expression that may be either the given type or
None."""

  def __init__(self, inner_type):
    self.inner_type = inner_type
    
  def unbox(self):
    """@returns inner_type"""
    return self.inner_type

  def __repr__(self):
    return "Some(" + str(self.inner_type) + ")"

class Strict:
  """Represents a type-expression that gives <b>all</b> the keys for a
dictionary, instead of just a subset. Used for dictionaries."""

  def __init__(self, inner_type):
    assert isinstance( inner_type, dict ), "Strict() only accepts dictionaries"
    self.inner_type = inner_type

  def unbox(self):
    """@returns inner_type"""
    return self.inner_type

  def __repr__(self):
    return "Strict(" + str(self.inner_type) + ")"

class TypeCheckFailedError(Exception):
  """Exception raised when type checking fails."""
  
  def __init__(self, message):
    # replace substrings like "<type 'int'>" by "int"
    self.message = re.sub("<type '([^']*)'>", "\\1", message)
    
  def __str__(self):
    transformed_message = re.sub("<type '([^']*)'>", "\\1", self.message)
    return repr(transformed_message)
  
  def getMessage(self):
    return self.message

def isContainerType(expr_type):
  '''!Returns True iff expr_type is an instance of Optional, ZeroOrMore, 
OneOrMore
@param expr_type type-expression
@returns bool'''
  return isinstance(expr_type, (Optional,ZeroOrMore,OneOrMore))

def mustBeType(expr, expr_type):
  '''!A nicer-named, but functionally equivalent, version of typecheck()
@param expr python-expression
@param expr_type type-expression
@returns void'''
  typecheck(expr, expr_type)

def mustBeInt(expr):
  '''!Asserts that expr is of type int.
@param expr python-expression
@returns void'''
  mustBeType(expr, int)

def mustBeString(expr):
  '''!Asserts that expr is of type string.
@param expr python-expression
@returns void'''
  mustBeType(expr, str)

def mustBeBool(expr):
  '''!Asserts that expr is of type bool.
@param expr python-expression
@returns void'''
  mustBeType(expr, bool)

def mustBeList(expr):
  '''!Asserts that expr is of type list.
@param expr python-expression
@returns void'''
  mustBeType(expr, list)

def mustBeDictionary(expr):
  '''!Assert that expr is of type dict.
@param expr python-expression
@returns void'''
  mustBeType(expr, dict)

def typecheck(expr, expr_type):
  '''!Raises a TypeCheckFailedError exception if expr is not of type expr_type.
@param expr python-expression
@param expr_type type-expression
@returns void'''
  if isinstance(expr_type, (type,Some)):
    typecheckBaseType(expr, expr_type)
  
  elif isinstance(expr_type, (list,tuple)): 
    typecheckListOrTuple(expr, expr_type)

  elif isinstance(expr_type, (dict,Strict)): 
    typecheckDictionary(expr, expr_type)
  
  else: # unhandled type
    raise ValueError("I can't handle the type:" + str(expr_type))

def constructTypeCheckErrorMessage(expr, expr_type, suffix=None):
  msg = "Typing " + str(expr) + " as " + str(expr_type) + " failed"
  if suffix != None:
    msg += suffix
  return msg

def typecheckWrapper(expr, expr_type, failure_message):
  try:
    typecheck(expr, expr_type)
  except TypeCheckFailedError,err:
    inner_error = err.getMessage()
    failure_message += " ->"
    raise TypeCheckFailedError(failure_message + "\n" + inner_error)

def typecheckBaseType(expr, expr_type):
  failure_msg = constructTypeCheckErrorMessage(expr, expr_type)
  if isinstance(expr_type, Some):
    if expr is None:
      return # success
    typecheckWrapper(expr, expr_type.unbox(), failure_msg)
  else:
    if isinstance(expr, expr_type): 
      return # success
    if (expr_type == int) and isinstance(expr, long): 
      return # success, treat longs as ints
    raise TypeCheckFailedError(failure_msg)
  
def typecheckListOrTuple(expr, expr_type):
  if isinstance(expr_type, list):
    if not isinstance(expr, list):
      failure_msg = constructTypeCheckErrorMessage(expr, expr_type, ": not a list")
      raise TypeCheckFailedError(failure_msg)
  else:
    if not isinstance(expr, tuple):
      failure_msg = constructTypeCheckErrorMessage(expr, expr_type, ": not a tuple")
      raise TypeCheckFailedError(failure_msg)

  type_index = 0
  expr_index = 0
  while type_index < len(expr_type):
    element_type = expr_type[type_index]
    
    if isinstance(element_type, (ZeroOrMore, OneOrMore)):
      inner_type = element_type.unbox()
      found_match = False
      failure_msg = constructTypeCheckErrorMessage(expr, expr_type, " at index " + str(expr_index))
      while (expr_index < len(expr)): # consume as many elements as we can
        failure_msg = constructTypeCheckErrorMessage(expr, expr_type, " at index " + str(expr_index))
        element_expr = expr[expr_index]
        try:
          # typecheck failure is a sentinel with One/ZeroOrMore, since
          # we will consume one extra token from the expr list, and
          # the typecheck failure tells us that we should move on to
          # the next token in the type list
          typecheckWrapper(element_expr, inner_type, failure_msg)
          expr_index += 1
          found_match = True
        except TypeCheckFailedError,tcfe:
          failure_msg = tcfe.getMessage()
          break
      if isinstance(element_type, OneOrMore): # we should have matched (at least) once
        if not found_match:
          raise TypeCheckFailedError(failure_msg)

    elif isinstance(element_type, Optional):
      inner_type = element_type.unbox()
      if expr_index < len(expr): # type list can be longer than expr list
        element_expr = expr[expr_index]
        try:
          typecheck(element_expr, inner_type)
          expr_index += 1
        except TypeCheckFailedError,tcfe:
          pass # type is Optional, so typecheck failure is OK

    else: # typechecking element against a base type
      if expr_index >= len(expr):
        failure_msg = constructTypeCheckErrorMessage(expr, expr_type, " at index " + str(expr_index) + ": too few elements")
        raise TypeCheckFailedError(failure_msg)
      element_expr = expr[expr_index]
      failure_msg = constructTypeCheckErrorMessage(expr, expr_type, " at index "
                                                   + str(expr_index))
      typecheckWrapper(element_expr, element_type, failure_msg)
      expr_index += 1
    type_index += 1
    
  if (expr_index != len(expr)): # did we process whole expr?
    failure_msg = constructTypeCheckErrorMessage(expr, expr_type, " at index " + str(expr_index) + ": too many elements")
    raise TypeCheckFailedError(failure_msg)

def unboxOptional(i):
  if isinstance( i, Optional ):
    return i.unbox()
  return i

def typecheckDictionary(expr, expr_type):
  if not isinstance(expr, dict):
    failure_msg = constructTypeCheckErrorMessage(expr, expr_type, ": not a dictionary")
    raise TypeCheckFailedError(failure_msg)

  isStrict = False
  if isinstance(expr_type, Strict):
    isStrict = True
    expr_type = expr_type.unbox()

  for key,val_type in expr_type.iteritems():
    if not key in expr:
      if isinstance(key, Optional):
        continue
      failure_msg = constructTypeCheckErrorMessage(expr, expr_type, ": missing required key " + str(key))
      raise TypeCheckFailedError(failure_msg)
    expr_val = expr[key]
    failure_msg = constructTypeCheckErrorMessage(expr, expr_type, " at key " +
                                                 str(key))
    typecheckWrapper(expr_val, val_type, failure_msg)
    
  if isStrict:
    allowed_keys = [ unboxOptional(k) for k in expr_type.keys() ]
    for key in expr.keys():
      if not key in allowed_keys:
        failure_msg = constructTypeCheckErrorMessage(expr, expr_type, ": contains unallowed key " + str(key))
        raise TypeCheckFailedError(failure_msg)



      
