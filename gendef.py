import copy
from optparse import OptionParser
import os, os.path
import sys, sets

import types
from pacg.util import initialCaps
from pacg.typesystem import *

# globals
DEFS = [] # A list of defs (dicts) that contains all output. The
          # values of these dicts are either strings or lists.

# `g_replacements' maps special replacement tokens in the file
# template (strings of the form %%TOKEN%%) to the values they should
# take.  Below are the replacement tokens whose values are invariant
# across all templates.
g_replacements = {}
g_replacements['STATS_CLASS'] = "Stats"
g_replacements['STATS_GLOBAL'] = "g_stats"
g_replacements['PARAM_CLASS'] = "Params"
g_replacements['PARAM_GLOBAL'] = "g_params"
g_replacements['USING_SIMICS_DEF'] = "USING_SIMICS"
g_replacements['SIMULATOR_REQUEST_CHANNEL_GLOBAL'] = "PYRITE_SIMULATOR_CHANNEL"
g_replacements['SIMULATOR_REQUEST_CHANNEL_TYPE'] = "simulator_channel"

SIMULATOR_REQUEST_CHANNEL_GLOBAL = "PYRITE_SIMULATOR_CHANNEL"
SIMULATOR_REQUEST_CHANNEL_ADDRS_GLOBAL = "simulatorChannelAddrs" # TODO?
SIMULATOR_REQUEST_NUM_CHANNELS = 3
NUM_SIMULATOR_REQUESTS = 0

USING_CCURED = False

USING_SIMICS_DEF = "USING_SIMICS"
DO_SANITY_CHECKS_DEF = "DO_SANITY_CHECKS"
DEBUG_CATEGORY_SHORTCUTS = sets.Set()

statTemplates = [ # STATISTICS
    
    { 'filename': ( lambda fileRoot: fileRoot + ".cpp" ),  # stat getter/setter/incrementer function defns
      'subdirectory': "src",
      'template':
"""%%AUTO_GENERATED_WARNING%%

#include "%%FILE_ROOT%%.h"
#include "debug_cat.h"
#include "params.h" // TODO: hard-coding this name here is a bit fragile
#include <ctime>

// this is initialized by the no-arg %%STATS_CLASS%% constructor
%%STATS_CLASS%% %%STATS_GLOBAL%%;

%%STATS_CLASS%%::%%STATS_CLASS%%() { cout << "Constructing g_stats" << endl; }
  
void %%STATS_CLASS%%::init(){ m_num_processors = g_params.getNumProcessors(); if (!g_params.getLoadingPyriteFromConfig()){ initStats(); }}

#if 0
// TODO: re-implement histograms
%%histogram_global%%
#endif

// all stat functions live in the class definition in "%%FILE_ROOT%%.h"

#if 0
// TODO: re-implement histograms
  %%create_histogram%%
#endif
  
void %%STATS_CLASS%%::dumpStats(ostream& os) {
    os << "{" << endl; // start python dict
  
    os << " 'stats': {" << endl;
    %%stat_dump%%
    os << "'theVeryLastStat':None }," << endl;

#if 0
  // TODO: re-implement histograms someday
  os << " 'histograms': {" << endl;
  %%histogram_dump%%
  os << "'theVeryLastHistogram':None }," << endl;
#endif
    time_t curr = time(0);
    g_params.setExperimentFinishDate(curr);
  
    os << " 'params': {" << endl;
  
    // NB: since we can't process >1 .def file at a time, we have to
    // resort to this hack. See the params_dump.inc template for more info
#include "params_dump.inc"
    os << "'theVeryLastParam':None }," << endl;
  
    os << "'theVeryLastThing':None }" << endl << flush; // end python dict
} // end %%STATS_CLASS%%::dumpStats()

attr_value_t %%STATS_CLASS%%::simwrap_dumpStats(void* arg, conf_object_t* obj, 
                                                attr_value_t* idx) {
    dumpStats( cout );
    return SIM_make_attr_nil();
}
"""},
    
    { 'filename': ( lambda fileRoot: fileRoot + ".h" ), # stat header file
      'subdirectory': "include",
      'template':
"""#ifndef %%HEADER_PROTECTION%%
#define %%HEADER_PROTECTION%%

#include <iostream>
#include <stdlib.h>
#include <simics/api.h>
#include <simics/arch/x86.h>
#include <simics/alloc.h>
//#include <simics/utils.h>
#include <assert.h>
#include "Global.h"
#include "Map.h"
#include "Vector.h"

using namespace std;

%%AUTO_GENERATED_WARNING%%

class %%STATS_CLASS%% {
  private:
%%stat_member%%
  int m_num_processors;

  public:
  %%STATS_CLASS%%();
  ~%%STATS_CLASS%%() { }
  void init();
  void dumpStats(ostream& os);
  attr_value_t simwrap_dumpStats(void* arg, conf_object_t* obj, attr_value_t* idx);

%%stat_method%%

  void initStats() {
  %%stat_init%%
#if 0
// TODO: re-implement histograms
%%histogram_init%%
#endif
  }

  void clearStats() {
  %%stat_clear%%
#if 0
// TODO: re-implement histograms, and make code for clearing them
// Insert code for clearing histograms here
#endif
  }


}; // end class Stats

extern %%STATS_CLASS%% %%STATS_GLOBAL%%;

typedef struct bucket {
  integer_t base;
  uinteger_t count;
  integer_t minSample;
  integer_t maxSample;
} bucket_t;

typedef struct histogram {
  bucket_t* bucketList;
  char* name;
  unsigned int allowOverflow : 1;
  int numBuckets;
  integer_t start;
  int bucketRange;
  uinteger_t totalSamples;
} histogram_t;

#if 0
// TODO: re-implement histograms
%%histogram_decl%%

histogram_t* newHistogram(int buckets, integer_t start, int step, bool allowOverflow, const char* name);
bool addHistogramSample(histogram_t* h, integer_t value);
void printHistogram(FILE* f, histogram_t* h);
void clearHistogram(histogram_t* h);
#endif

#endif
"""},

    { 'filename': ( lambda fileRoot: fileRoot + "_wrappers.inc" ), # simics getter/setter wrappers for stats
      'subdirectory': "include",
      'template':
"""%%AUTO_GENERATED_WARNING%%

%%stat_wrapper%%"""},

    { 'filename': ( lambda fileRoot: fileRoot + "_attributes.inc" ), # simics stat attributes
      'subdirectory': "include",
      'template':
"""%%AUTO_GENERATED_WARNING%%

%%stat_attribute%%
"""}
    
    ] # end stats templates

simulatorRequestTemplates = [ # SIMULATOR REQUESTS
    
{ 'filename': ( lambda fileRoot: fileRoot + ".c" ), # source code for userland sim request generators
  'subdirectory': "src",
  'template':
"""%%AUTO_GENERATED_WARNING%%

#include "%%FILE_ROOT%%_types.h"

// the sim request functions are all declared 'extern inline' and live in "%%FILE_ROOT%%.h"

%%SIMULATOR_REQUEST_CHANNEL_TYPE%%_t %%SIMULATOR_REQUEST_CHANNEL_GLOBAL%%;
"""},

    { 'filename': ( lambda fileRoot: fileRoot + ".h" ), # sim request userland header file
      'subdirectory': "include",
      'template': # TODO: paramaterize the currently embedded number
                  # of channel struct members (currently, 3)
"""#ifndef %%HEADER_PROTECTION%%
#define %%HEADER_PROTECTION%%

%%AUTO_GENERATED_WARNING%%

#include "x86-magic-request.h"
#include "%%FILE_ROOT%%_types.h"

extern %%SIMULATOR_REQUEST_CHANNEL_TYPE%%_t %%SIMULATOR_REQUEST_CHANNEL_GLOBAL%%;

%%simreq_function%%

// Keep CIL from blowing away our inline function defns as dead code,
// because we may add a use after the fact via our safec transformations

#if 0
%%simreq_function_cilnoremove%%
#pragma cilnoremove("PYRITE_experimentSetup__extinline")
#endif

extern inline void PYRITE_experimentSetup(void) {
#ifdef %%USING_SIMICS_DEF%%

%%init_channel%%
  PYRITE_generateRegisterProcessSimRequest();
%%register_channel_with_simulator%%

#endif // %%USING_SIMICS_DEF%%
}

#endif
"""}, 

{ 'filename': ( lambda fileRoot: fileRoot + "_types.h" ), # enum for the simulator request id's
  'subdirectory': "include",
  'template':
"""#ifndef %%HEADER_PROTECTION%%
#define %%HEADER_PROTECTION%%

%%AUTO_GENERATED_WARNING%%

typedef enum {
  INVALID, /* 0 */
%%simreq_enum_member%%
  SIMULATOR_REQUEST_ENUM_LENGTH
} %%FILE_ROOT%%_t;

typedef volatile struct %%SIMULATOR_REQUEST_CHANNEL_TYPE%% {
   volatile void* data0;
   volatile void* data1;
   volatile void* data2;
} %%SIMULATOR_REQUEST_CHANNEL_TYPE%%_t;

/** the logical addresses of the variables used as communication channels
    between simulator and simulated program */
typedef struct {
   unsigned long long virtAddrData0;
   unsigned long long virtAddrData1;
   unsigned long long virtAddrData2;
} %%SIMULATOR_REQUEST_CHANNEL_TYPE%%_addresses_t;

extern %%SIMULATOR_REQUEST_CHANNEL_TYPE%%_addresses_t* simulatorChannelAddrs;

#endif
"""},

    { 'filename': ( lambda fileRoot: fileRoot + "_handlers.h" ), # sim request handlers
      'subdirectory': "include",
      'template':
"""#ifndef %%HEADER_PROTECTION%%
#define %%HEADER_PROTECTION%%

%%AUTO_GENERATED_WARNING%%

/** converts a sim request integer to a string */
string simRequest2String(%%FILE_ROOT%%_t req);

%%simreq_handler_prototype%%

#endif
"""},

    { 'filename': ( lambda fileRoot: fileRoot + "_lut.cpp" ), # sim request string lookup table
      'subdirectory': "src",
      'template':
"""
#include "debug_cat.h"
#include "%%FILE_ROOT%%_types.h"

%%AUTO_GENERATED_WARNING%%

static const char* simRequestLookupTable[] = {
  "INVALID_BEG", /* 0 */
%%simreq_lut_entry%%
  "INVALID_END"
};

const string simRequest2String(%%FILE_ROOT%%_t req) {
   /* since we have a dummy 0th entry, the valid range is (0,num_requests] */
   ASSERT_MSG( req > 0 && req <= %%NUM_SIMULATOR_REQUESTS%%, "Sim request lookup out of bounds" );
   return string( simRequestLookupTable[req] );
}

"""},

    { 'filename': ( lambda fileRoot: fileRoot + "_switch.inc" ), # sim request switch statement
      'subdirectory': "include",
      'template':
"""%%AUTO_GENERATED_WARNING%%
%%simreq_branch%%
"""},

    { 'filename': ( lambda fileRoot: fileRoot + "_channel_wrappers.inc" ), # Simics-ified getter/setters for the channel
      'subdirectory': "include",
      'template':
"""%%AUTO_GENERATED_WARNING%%

%%channel_accessors%%
"""},

    { 'filename': ( lambda fileRoot: fileRoot + "_channel_attributes.inc" ), # register channel as a simics attribute
      'subdirectory': "include",
      'template':
"""%%AUTO_GENERATED_WARNING%%

%%register_channel_with_simics%%
"""}
    ] # end simulator request templates

paramTemplates = [ # PARAMETERS
    
    { 'filename': ( lambda fileRoot: fileRoot + ".cpp" ),  # param getter/setter function defns
      'subdirectory': "src",
      'template':
"""%%AUTO_GENERATED_WARNING%%

#include "%%FILE_ROOT%%.h"

// this is initialized by the no-arg %%PARAM_CLASS%% constructor
%%PARAM_CLASS%% %%PARAM_GLOBAL%%;

"""},
    
    { 'filename': ( lambda fileRoot: fileRoot + ".h" ), # param header file
      'subdirectory': "include",
      'template':
"""#ifndef %%HEADER_PROTECTION%%
#define %%HEADER_PROTECTION%%

%%AUTO_GENERATED_WARNING%%

#include "stats.h"
#include "debug_cat.h"

%%param_preamble%%

class %%PARAM_CLASS%% {
private:
%%param_member%%

public:
  %%PARAM_CLASS%%() {
  cout << "Constructing g_params" << endl;
%%param_init%%
}
  ~%%PARAM_CLASS%%() { }

%%param_method%%
}; // end class %%PARAM_CLASS%%

extern %%PARAM_CLASS%% %%PARAM_GLOBAL%%;

#endif
"""},

    { 'filename': ( lambda fileRoot: fileRoot + "_wrappers.inc" ), # simics getter/setter wrappers for params
      'subdirectory': "include",
      'template':
"""%%AUTO_GENERATED_WARNING%%

%%param_wrapper%%"""},

    { 'filename': ( lambda fileRoot: fileRoot + "_attributes.inc" ), # simics param attributes
      'subdirectory': "include",
      'template':
"""%%AUTO_GENERATED_WARNING%%

%%param_attribute%%
"""},

    # Dump params via dumpStats(). We can't just expand a %% token
    # inside dumpStats(), because we only process a single .defs file
    # at a time, so we never have the defns for stats and params at
    # the same time.
    { 'filename': ( lambda fileRoot: fileRoot + "_dump.inc" ), 
      'subdirectory': "include",
      'template':
"""%%AUTO_GENERATED_WARNING%%

%%param_dump%%
"""},

    { 'filename': ( lambda fileRoot: fileRoot + ".py" ),
      'subdirectory': "src",
      'template':
"""%%AUTO_GENERATED_WARNING%%

%%param_python_code%%
"""}

    ] # end param templates

debugCategoryTemplates = [ # DEBUG CATEGORIES

    { 'filename': ( lambda fileRoot: fileRoot + "_enum.inc" ), # debug enums
      'subdirectory': "include",
      'template':
"""%%AUTO_GENERATED_WARNING%%

%%debug_enum_member%%
"""},

    { 'filename': ( lambda fileRoot: fileRoot + "_shortcuts.inc" ), # debug shortcuts
      'subdirectory': "include",
      'template':
"""%%AUTO_GENERATED_WARNING%%

%%debug_shortcut%%
"""},

    { 'filename': ( lambda fileRoot: fileRoot + "_help.inc" ), # help text for Simics command
      'subdirectory': "include",
      'template':
"""
%%debug_help%%
"""},

    { 'filename': ( lambda fileRoot: fileRoot + "_colors.inc" ), # colorize each debug category differently
      'subdirectory': "include",
      'template':
"""
// weird alignment is so as not to break the macro defn with an unescaped newline
#define PRINT_CATEGORY_COLOR(CATEGORY,OSTREAM)\\
%%debug_color_ifelse%%OSTREAM << ""; // handling the dangling 'else' from the _switch above
"""}
    
    ] # end debug category templates

# "enum" for the various def types
class DefType:
    Stat, SimRequest, Param, DebugCategory = range( 1, 5 )

## STRING MANIPULATION FUNCTIONS ##

def stringify( arg ):
    """Return a string version of the supplied argument (which must
either be a string or a list). If the argument is a string, it is
simply returned. If the argument is a list, each element of the list
is joined with a newline."""
    if isinstance( arg, str ):
        return arg
    else:
        mustBeType( arg, [OneOrMore(str)] )
        return "\n".join( arg )
    
def headerify( s ):
    """Transform a filename into a suitably unique string for use in
header multiple-inclusion protection #define's."""
    mustBeString( s )
    s = s.replace(".", "_")
    return "__" + s.upper()

def simicsAttributeWrapper(name, simicsType, obj, generateSetter, listLength="", listInternalType=""):
    """!Returns C code for getter (and optionally, setter) wrappers
for a simics attribute, and code to register those wrappers as the
accessor methods of a Simics class attribute.

@param name a string giving the name of the attribute. The generated
code will call OBJ.getNAME() and OBJ.setNAME(), so be sure that these
names are actual functions.
@param simicsType a string naming the simics type of the attribute
@param obj a string naming a C++ object; see the `name' param
@param generateSetter a bool saying whether to generate code for a setter or not
@returns a tuple of strings: (wrapperCode, registrationCode)"""
    mustBeString( name )
    mustBeString( simicsType )
    mustBeString( obj )
    mustBeBool( generateSetter )

    getWrapper = "simwrap_get" + obj + name
    setWrapper = "simwrap_set" + obj + name
    
    if simicsType == "list":
      assert listLength != ""
      assert listInternalType == "integer" or listInternalType == "dict"

    getter = "static attr_value_t %s(void* arg, conf_object_t* obj, attr_value_t* idx) {\n" % getWrapper
    if simicsType == "string": # need to convert C++ string -> const char*
        getter += '''  
  assert(idx->kind == Sim_Val_Nil);
  return SIM_make_attr_%s( %s.get%s().c_str() );\n''' % (simicsType.lower(), obj, name)
    elif simicsType == "list":
      if listInternalType == "integer":
        getter += '''
  if (idx->kind == Sim_Val_Integer){
    return SIM_make_attr_integer(%s.get%s(idx->u.integer));
  } else{
    assert(idx->kind == Sim_Val_Nil);
    attr_value_t val = SIM_alloc_attr_list(%s);
    int i;
    for (i = 0; i < %s; i++){
      val.u.list.vector[i] = SIM_make_attr_integer(%s.get%s(i));
    }
    return val;
  }\n''' % (obj, name, listLength, listLength, obj, name)

      elif listInternalType == "dict":
        getter += '''  
  attr_value_t val;
  if (idx->kind == Sim_Val_Integer){
    int index = idx->u.integer;
    Vector<int> keys = %s.get%sKeys(index);
    val = SIM_alloc_attr_dict(keys.size());
    for (int j = 0; j < keys.size(); j++){
      attr_dict_pair_t pair;
      attr_value_t key = SIM_make_attr_integer(keys[j]);
      attr_value_t value = SIM_make_attr_integer(%s.get%s(index, keys[j]));
      pair.key = key;
      pair.value = value;
      val.u.dict.vector[j] = pair;
    }
  } else{
    val = SIM_alloc_attr_list(%s);
    int i;
    for (i = 0; i < %s; i++){
      Vector<int> keys = %s.get%sKeys(i);
      attr_value_t val_i = SIM_alloc_attr_dict(keys.size());
      for (int j = 0; j < keys.size(); j++){
        attr_dict_pair_t pair;
        attr_value_t key = SIM_make_attr_integer(keys[j]);
        attr_value_t value = SIM_make_attr_integer(%s.get%s(i, keys[j]));
        pair.key = key;
        pair.value = value;
        val_i.u.dict.vector[j] = pair;
      }
      val.u.list.vector[i] = val_i;
    }
  }
  return val;\n''' % (obj, name, obj, name, listLength, listLength, obj, name, obj, name)
    elif simicsType == "dict":
      getter += '''
  attr_value_t val;
  if (idx->kind != Sim_Val_Nil){
    assert(idx->kind == Sim_Val_Integer);
    int key = idx->u.integer;
    val = SIM_make_attr_integer(%s.get%s(key));
  } else{
    Vector<int> keys = %s.get%sKeys();
    val = SIM_alloc_attr_dict(keys.size());
    for (int i = 0; i < keys.size(); i++){
      attr_dict_pair_t pair;
      attr_value_t key = SIM_make_attr_integer(keys[i]);
      attr_value_t value = SIM_make_attr_integer(%s.get%s(keys[i]));
      pair.key = key;
      pair.value = value;
      val.u.dict.vector[i] = pair;
    }
  }
  return val;\n''' % (obj, name, obj, name, obj, name)
    else:
        getter += '''  
    assert(idx->kind == Sim_Val_Nil);
    return SIM_make_attr_%s( %s.get%s() );\n''' % (simicsType.lower(), obj, name)
    getter += "}\n"
    setter = ""
    if not(generateSetter):
      assert 0, "must include a setter for Simics to be able to load configurations including this attribute"
    if generateSetter:
        setter += "static set_error_t %s(void* arg, conf_object_t* obj, attr_value_t* val, attr_value_t* idx) {\n" % setWrapper
        if (simicsType != "dict"):
          setter += "  assert( val->kind == Sim_Val_%s );\n" % simicsType.capitalize()
        if simicsType == "list":
          setter += "  assert(idx->kind == Sim_Val_Nil);"
          if listInternalType == "integer":
            setter += '''  
  int size = val->u.list.size;
  %s.init%s(size);
  int i;
  for (i = 0; i < size; i++){
    int curr = val->u.list.vector[i].u.integer;
    %s.set%s(i, curr);
  }\n''' % (obj, name, obj, name)
          elif listInternalType == "dict":
            setter += '''  
  int size = val->u.list.size;
  %s.init%s(size);
  int i;
  for (i = 0; i < size; i++){
    attr_value_t val_i = val->u.list.vector[i];
    int dict_size = val_i.u.dict.size;
    int j;
    for (j = 0; j < dict_size; j++){
      attr_dict_pair_t curr = val_i.u.dict.vector[j];
      attr_value_t key = curr.key;
      assert(key.kind == Sim_Val_Integer);
      attr_value_t value = curr.value;
      assert(value.kind == Sim_Val_Integer);
      int key_value = key.u.integer;
      int value_value = value.u.integer;
      %s.set%s(i, key_value, value_value);
    }
  }\n''' % (obj, name, obj, name)
        elif simicsType == "dict":
          setter += '''  
  if (idx->kind == Sim_Val_Integer){
    assert(val->kind == Sim_Val_Integer);
    %s.set%s(idx->u.integer, val->u.integer);
  } else{
    assert(idx->kind == Sim_Val_Nil);
    assert(val->kind == Sim_Val_Dict);
    %s.init%s();
    int size = val->u.dict.size;
    int i;
    for (i = 0; i < size; i++){
      attr_dict_pair_t curr = val->u.dict.vector[i];
      attr_value_t key = curr.key;
      assert(key.kind == Sim_Val_Integer);
      attr_value_t value = curr.value;
      assert(value.kind == Sim_Val_Integer);
      int key_value = key.u.integer;
      int value_value = value.u.integer;
      %s.set%s(key_value, value_value);
    }
  }\n''' % (obj, name, obj, name, obj, name)
        else:
          setter += '''  
  assert(idx->kind == Sim_Val_Nil);
  %s.set%s( val->u.%s );\n''' % (obj, name, simicsType.lower())
        setter += "  return Sim_Set_Ok;\n"
        setter += "}\n"

    # NB: the `myClass' arg is just a local in scsim.c:init_local(),
    # and as such its presence here is very fragile!
    register = 'assert( 0 == SIM_register_typed_attribute(myClass, "%s", ' % name
    register += "%s, NULL, " % getWrapper
    if generateSetter:
        register += "%s, NULL, " % setWrapper
    else:
        register += "NULL, NULL, "
    register += "(attr_attr_t) (Sim_Attr_Optional"
    # add indexing for lists and dicts
    if (simicsType == "dict") or (simicsType == "list"):
      register += " | Sim_Attr_Integer_Indexed"
    register += "), "
    # non-indexed type
    if simicsType == "list":
      if listInternalType == "integer":
        register += '"[i*]", '
      elif listInternalType == "dict":
        register += '"[D*]", '
    elif simicsType == "dict":
      register += '"D", '
    else:
      register += '"%s", ' % simicsType.lower()[0]
    # indexed type
    if simicsType == "dict":
      register += '"i", '
    else:
      register += 'NULL, '
    register += '"auto-generated attribute: %s") );' % name

    return (getter + "\n" + setter, register)

def generateHistogram( hist ):
    """Adds a dictionary full of entries for the specified histogram
to the global DEFS list"""
    t = { 'kind':str,
          'name':str,
          'buckets':int,
          'start':int,
          'step':int,
          Optional('allowOverflow'):bool }
    mustBeType( hist, Strict(t) )
    
    myDef = {}

    globalVar = hist['name']
    myDef['histogram_global'] = ["histogram_t* %s;" % globalVar]

    myDef['histogram_decl'] = ["extern histogram_t* %s;" % globalVar]

    # create histogram, called as part of initStats()
    oflow = "TRUE"
    if 'allowOverflow' in hist and not hist['allowOverflow']:
        oflow = "FALSE"
    initCode = ('%s = newHistogram( %d/*buckets*/, %d/*start*/, %d/*step*/, %s/*allow oflow*/, "%s"/*name*/);'
                % (globalVar, hist['buckets'], hist['start'], hist['step'], oflow, globalVar) )
    myDef['create_histogram'] = [initCode]

    # clear histogram, called as part of clearStats()
    myDef['histogram_init'] = ["clearHistogram( %s );" % globalVar]

    # dump histogram as part of call to dumpStats()
    printCode = "printHistogram( f, %s );" % globalVar
    myDef['histogram_dump'] = [printCode]
    
    return myDef

def generateHashtableStat( stat ):
    """Returns a dictionary full of entries for the specified hashtable statistic"""

    t = { 'kind':str,
          'name':str,
          'initialValue':int,
          Optional('per-processor'):bool,
          Optional('printHexKey'):bool }
    mustBeType( stat, Strict(t) )
    
    myDef = {}
    memberName = stat['name']
    functionName = initialCaps( memberName )
    statInitialValue = stat['initialValue']
    assert statInitialValue >= 0, "STAT_HASHTABLE " + stat['name'] + " must have an unsigned 'initialValue'"

    perProcessorStat = ('per-processor' not in stat or stat['per-processor'])
    if 'printHexKey' in stat:
      printHexKey = stat['printHexKey']
    else:
      printHexKey = False

    preKey = ""
    postKey = ""
    if printHexKey:
      preKey = "std::hex << \"0x\" << "
      postKey = "std::dec <<"
    
    # stat function definition
    if perProcessorStat:
      statGetter = "integer_t get%s(int processor_number, int key) { return %s[processor_number]->lookup(key); }" % (functionName, memberName)
    else:
      statGetter = "integer_t get%s(int key) { return %s->lookup(key); }" % (functionName, memberName)
    myDef["stat_method"] = [statGetter]
    # init
    if perProcessorStat:
      statInit = '''
void init%s(int size){ 
  %s = (Map<int, int>**) native_malloc(sizeof(Map<int, int>*) * size);
  assert(%s != 0);
  for(int i = 0;\ni < size; i++){
    %s[i] = new Map<int, int>();
  }
}\n''' % (functionName, memberName, memberName, memberName)
    else:
      statInit = "void init%s(void) { %s = new Map<int, int>; }\n" % (functionName, memberName)
    myDef["stat_method"].append(statInit)
    if perProcessorStat:
      myDef["stat_init"] = "init%s(m_num_processors);" % functionName
    else:
      myDef["stat_init"] = "init%s();" % functionName
    # clear
    if perProcessorStat:
      statClear = '''
void clear%s(void){ 
  for(int i = 0; i < m_num_processors; i++){
    %s[i]->clear();
  }
}\n''' % (functionName, memberName)
    else:
      statClear = "void clear%s(void) { %s->clear(); }\n" % (functionName, memberName)
    myDef["stat_method"].append(statClear)
    myDef["stat_clear"] = "clear%s();" % functionName
    # setter
    if perProcessorStat:
        statSetter = '''
void set%s(int processor_number, int key, integer_t x){ 
  %s[processor_number]->add(key, x); 
}\n''' % (functionName, memberName)
    else:
        statSetter = "void set%s(int key, integer_t x){ %s->add(key, x); }\n" % (functionName, memberName)
    myDef["stat_method"].append( statSetter )
    # increment
    if perProcessorStat:
      statIncrementer = "void increment%s(int processor_number, int key){ incrementN%s(processor_number, key, 1); }\n" % (functionName, functionName)
    else:
      statIncrementer = "void increment%s(int key){ incrementN%s(key, 1); }\n" % (functionName, functionName)
    myDef["stat_method"].append( statIncrementer )
    # increment by n
    if perProcessorStat:
      statNIncrementer = '''
void incrementN%s(int processor_number, int key, int n){ 
  int value = n;
  if (!%s[processor_number]->exist(key)){ 
    value += %d;
  } else{
    value += %s[processor_number]->lookup(key);
  }
  %s[processor_number]->add(key, value);
}\n''' % (functionName, memberName, statInitialValue, memberName, memberName)
    else:
      statNIncrementer = '''
void incrementN%s(int key, int n){
  int value = n;
  if (!%s->exist(key)){
    value += %d;
  } else{
    value += %s->lookup(key);
  }
  %s->add(key, value);
}\n''' % (functionName, memberName, statInitialValue, memberName, memberName)
    myDef["stat_method"].append( statNIncrementer )
    # decrementer
    if perProcessorStat:
      statDecrementer = "void decrement%s(int processor_number, int key){ incrementN%s(processor_number, key, -1); }\n" % (functionName, functionName)
    else:
      statDecrementer =  "void decrement%s(int key){ incrementN%s(key, -1); }\n" % (functionName, functionName)
    myDef["stat_method"].append( statDecrementer )

    if perProcessorStat:
      statGetKeys = "Vector<int> get%sKeys(int processor_number){ return %s[processor_number]->keys();}\n" % (functionName, memberName)
    else:
      statGetKeys = "Vector<int> get%sKeys(void){ return %s->keys();}\n" % (functionName, memberName)
    myDef["stat_method"].append(statGetKeys)
    # stat dump function definition - into a python dict entry
    if perProcessorStat:
      myDef["stat_dump"] = '''    
    os << "'%s' : [";
    for (int i = 0; i < m_num_processors; i++){
      os << "{";
      Vector<int> keys = %s[i]->keys();
      for (int j = 0; j < keys.size(); j++){
        os << %s keys[j] << %s " : " << %s[i]->lookup(keys[j]) << "," << endl;
      }  
      os << "}," << endl;
    }
    os << "]," << endl;\n''' % (memberName, memberName, preKey, postKey, memberName)
    else:
      myDef["stat_dump"] = '''
    os << "'%s' : {";
    Vector<int> keys = %s->keys();
    for (int i = 0; i < keys.size(); i++){
      os << %s keys[i] << %s " : " << %s->lookup(keys[i]) << "," << endl;
    }  
    os << "}," << endl;''' % (memberName, memberName, preKey, postKey, memberName)

    # private instance variable of the Stats class
    if perProcessorStat:
      myDef["stat_member"] = "  Map<int, int>** %s;" % memberName
    else:
      myDef["stat_member"] = "  Map<int, int>* %s;" % memberName
    
    if perProcessorStat:
      wrapper, registration = simicsAttributeWrapper( functionName, simicsType="list", obj=g_replacements['STATS_GLOBAL'], generateSetter=True, listLength="g_params.getNumProcessors()", listInternalType="dict" )
    else:
      wrapper, registration = simicsAttributeWrapper( functionName, simicsType="dict", obj=g_replacements['STATS_GLOBAL'], generateSetter=True )
 
    myDef["stat_wrapper"] = wrapper
    myDef["stat_attribute"] = registration
    return myDef

def generateIntStat( stat ):
    """Returns a dictionary full of entries for the specified integer statistic"""

    t = { 'kind':str,
          'name':str,
          'initialValue':int,
          Optional('per-processor'):bool }
    mustBeType( stat, Strict(t) )
    
    myDef = {}

    memberName = stat['name']
    functionName = initialCaps( memberName )
    statInitialValue = stat['initialValue']
    assert statInitialValue >= 0, "STAT_INT " + stat['name'] + " must have an unsigned 'initialValue'"

    perProcessorStat = ('per-processor' not in stat or stat['per-processor'])
    
    # stat function definition
    if perProcessorStat:
      statGetter = "integer_t get%s(int processor_number) { return %s[processor_number]; }\n" % (functionName, memberName)
    else:
      statGetter = "integer_t get%s(void) { return %s; }\n" % (functionName, memberName)
    myDef["stat_method"] = [statGetter]
    if perProcessorStat:
      statInit = '''
void init%s(int size) { 
  %s = (integer_t*) native_malloc(sizeof(integer_t) * size);
  assert(%s != 0);
  for(int i = 0; i < size; i++){
    set%s( i, %d );
  }
}\n''' % (functionName, memberName, memberName, functionName, statInitialValue)
    else:
      statInit = "void init%s(void) { set%s( %d ); }\n" % (functionName, functionName, statInitialValue)
    myDef["stat_method"].append(statInit)
    if perProcessorStat:
      myDef["stat_init"] = "init%s(m_num_processors);" % functionName
    else:
      myDef["stat_init"] = "init%s();" % functionName
    if perProcessorStat:
      statClear = '''
void clear%s(void) { 
  for(int i = 0; i < m_num_processors; i++){
    set%s( i, %d );
  }
}\n''' % (functionName, functionName, statInitialValue)
    else:
      statClear = "void clear%s(void) { set%s( %d ); }\n" % (functionName, functionName, statInitialValue)
    myDef["stat_method"].append(statClear)
    myDef["stat_clear"] = "clear%s();" % functionName
    # setter
    if perProcessorStat: 
      statSetter = '''
void set%s(int processor_number, integer_t x) { 
  %s[processor_number] = x; 
}\n''' % (functionName, memberName)
    else:
      statSetter = "void set%s(integer_t x) { %s = x; }\n" % (functionName, memberName)
    myDef["stat_method"].append( statSetter )
    # incrementer
    if perProcessorStat:
      statIncrementer = "void increment%s(int processor_number) { incrementN%s(processor_number, 1); }\n" % (functionName, functionName)
    else:
      statIncrementer = "void increment%s(void) { incrementN%s(1); }\n" % (functionName, functionName)
    myDef["stat_method"].append( statIncrementer )
    # incrementbyN
    if perProcessorStat:
      statNIncrementer = "void incrementN%s(int processor_number, integer_t n) { (%s[processor_number]) += n; }\n" % (functionName, memberName)
    else:
      statNIncrementer = "void incrementN%s(integer_t n) { (%s) += n; }\n" % (functionName, memberName)
    myDef["stat_method"].append( statNIncrementer )
    # decrementer
    if perProcessorStat:
      statDecrementer = "void decrement%s(int processor_number) { incrementN%s(processor_number, -1); }\n" % (functionName, functionName)
    else:
      statDecrementer = "void decrement%s(void) { incrementN%s(-1); }\n" % (functionName, functionName)
    myDef["stat_method"].append( statDecrementer )

    # stat dump function definition - into a python dict entry
    if perProcessorStat:
      myDef["stat_dump"] = '''
    os << "'%s': [";
    for (int i = 0; i < m_num_processors; i++){
      os << get%s(i) << ",";
    }
    os << "]," << endl;''' % (memberName, functionName)
    else:
      myDef["stat_dump"] = 'os << "\'%s\': " << get%s() << "," << endl;\n' % (memberName, functionName)

    # private instance variable of the Stats class
    if perProcessorStat:
      myDef["stat_member"] = "  integer_t* %s;" % memberName
    else:
      myDef["stat_member"] = "  integer_t %s;" % memberName
    
    if perProcessorStat:
      wrapper, registration = simicsAttributeWrapper( functionName, simicsType="list", obj=g_replacements['STATS_GLOBAL'], generateSetter=True, listLength="g_params.getNumProcessors()", listInternalType="integer" )
    else:
      wrapper, registration = simicsAttributeWrapper( functionName, simicsType="integer", obj=g_replacements['STATS_GLOBAL'], generateSetter=True )
 
    myDef["stat_wrapper"] = wrapper
    myDef["stat_attribute"] = registration
    
    return myDef

def generateSimRequest( request ):
    """Adds a dictionary full of entries for the specified simulator
request to the global DEFS list"""

    t = { 'kind':str,
          'name':str,
          'returnType':str,
          'arguments':[ ZeroOrMore( (str,str) ) ],
          Optional('passFirstArgViaRegister'):bool,
          Optional('returnWhichArg'):bool,
          Optional('needsProcessorNumber'):bool,
          Optional('sanityCheck'):bool }
    mustBeType( request, Strict(t) )

    # check name formatting
    requestName = request['name']
    if not requestName.isupper():
        print "WARNING: '%s' system request does not obey all-caps naming convention" % requestName
    if requestName[:7] != "PYRITE_":
        print "ERROR: '%s' system request does not obey PYRITE_ prefix naming convention" % requestName
        sys.exit( 1 )

    # check return/argument types
    allowedUserTypes = [ 'void', 'int', 'int*', 'void*' ]
    # to avoid cast warnings, we translate from userland types to
    # those handled by the simulator (typically, 32 -> 64 bits)
    allowedSimTypes = { 'void': 'void',
                        'int': 'uinteger_t',
                        'int*': 'unsigned long long',
                        'void*': 'unsigned long long' }
    returnType = request['returnType']
    arguments = request['arguments']
    passFirstArgViaRegister = 'passFirstArgViaRegister' in request and request['passFirstArgViaRegister']
    needsProcessorNumber = 'needsProcessorNumber' in request and request['needsProcessorNumber']
    if returnType not in allowedUserTypes:
        print ("ERROR: '%s' system request has illegal return type '%s', should be one of %s" %
               (requestName, returnType, allowedUserTypes))
        sys.exit( 1 )
    
    assert len( arguments ) <= SIMULATOR_REQUEST_NUM_CHANNELS, "simulator requests with >3 args are not supported"
    for arg in arguments:
        argType = arg[0]
        argName = arg[1]
        
        if argType not in allowedUserTypes:
            print ("ERROR: '%s' system request has illegal argument type '%s', should be one of %s" %
                   (requestName, arg, allowedUserTypes))
            sys.exit( 1 )
        if argType == 'void':
            print "ERROR: '%s' system request has 'void' argument type; leave the list empty for a no-arg function" % requestName
            sys.exit( 1 )

    global DEFS
    myDef = {}

    # simulator request enum, and enum->string lookup table entry
    myDef['simreq_enum_member'] = ( "  "+requestName+", /* %%count%% */" )
    myDef['simreq_lut_entry']   = ( '  "'+requestName+'", /* %%count%% */' )

    # function to be called from the simulated program
    tmp = requestName[7:] # strip PYRITE_ prefix
    parts = tmp.split( "_" )
    parts = [ p.capitalize() for p in parts ]
    userFunctionName = "PYRITE_generate%sSimRequest" % "".join( parts )

    myDef['simreq_function_cilnoremove'] = '#pragma cilnoremove("%s__extinline")' % userFunctionName

    code = ""
    userSignature = "extern inline " + returnType + " " + userFunctionName + "("
    if len( arguments ) == 0 and returnType == "void":
        userSignature += "void"
    else:
        for argType, argName in arguments:
            userSignature += ( argType + " " + argName + ", " )
        if returnType != "void":
          userSignature += ( returnType + " " + "default_value, " )
        userSignature = userSignature.rstrip( ", " ) # strip one-too-many-th comma
    userSignature += ")"
    # DEPRECATED: now that sim req functions are 'extern inline',
    # prototypes are never actually needed in the autogen'd files.
    myDef['simreq_function_prototype'] = ( userSignature + ";" )
    
    code += ( userSignature + " {\n" )
    code += "#ifdef %s\n" % USING_SIMICS_DEF
    
    if 'sanityCheck' in request and request['sanityCheck']: # sanity check
        code += "#ifdef %s\n" % DO_SANITY_CHECKS_DEF

    if len( arguments ) > 0:
        if USING_CCURED:
            code += """#ifdef USING_CCURED  
{ __NOCUREBLOCK
#endif
"""
        counter = 0
        for argType, argName in arguments:
            code += ( "   %s.data%d = (void*) %s;\n" % (SIMULATOR_REQUEST_CHANNEL_GLOBAL, counter, argName) )
            counter += 1
        if USING_CCURED:
            code += """#ifdef USING_CCURED
}
#endif
"""
        
    if returnType != "void":
        code += "   return (%s) " % returnType
    if passFirstArgViaRegister:
        # TODO: this needs to change...or maybe not?
        code += ( "   SIMULATOR_REQUEST2( (void*) %s, (void*) %s );\n" % (requestName, arguments[0][1]) )
    else:
        code += ( "   SIMULATOR_REQUEST( (void*) %s );\n" % requestName )

    if 'sanityCheck' in request and request['sanityCheck']: # end sanity check
        code += "#endif // %s\n" % DO_SANITY_CHECKS_DEF

    code += "#else // %s\n" % USING_SIMICS_DEF
    if returnType != "void":
        code += "   return (%s) default_value;\n" % returnType 
    code += "#endif // %s\n" % USING_SIMICS_DEF
    code += "}"
    
    myDef['simreq_function'] = code

    # simulator request handler
    handlerName = "handle%sSimRequest" % "".join( parts )
    #handlerSignature = "inline " + returnType + " " + handlerName + "("
    handlerSignature = " " + returnType + " " + handlerName + "(" # don't inline anymore - sim reqs are so infrequent anyway
    if len( arguments ) == 0 and not needsProcessorNumber:
        handlerSignature += "void"
    else:
        if needsProcessorNumber:
            handlerSignature += "uinteger_t processorNumber, "
        for argType, argName in arguments:
            handlerSignature += (  allowedSimTypes[argType] + " " + argName + ", " )
        handlerSignature = handlerSignature.rstrip( ", " ) # strip one-too-many-th comma
    handlerSignature += ")"
    myDef['simreq_handler_prototype'] = handlerSignature + ";\n"

    # simulator switch statement - NOTE: this has a boatload of implicit
    # dependencies, including:
    #    physical_address_t physAddr
    #    unsigned long long virtAddrData[0-2] // virt addrs registered with the simulator
    #    uinteger_t data[0-2]
    #    conf_object_t* cpu // cpu that generated request
    #    processorNumber // number of cpu that generated request
    #    unaligned // bool
    #    no_translation // bool
    switch = "case %s: \n" % requestName
    # read arguments from the simulated program's memory space
    counter = 0
    passedArguments = arguments
    if passFirstArgViaRegister:
        # NB: if we're passing args via registers, the argument always resides in %ecx
        switch += '  data0 = processor->readRegister(REG_rcx, "ecx");\n'
        switch += "  CHECK_SIM_EXCEPTION();\n"
        # we've already handled the first arg
        counter += 1
        passedArguments = passedArguments[1:]
    # read the rest of the arguments
    for argType, argName in passedArguments:
        # TODO: we always read 4 bytes, but maybe we can make this more general??
        switch += "  assert(processor->addressHasTranslation(virtAddrData%d, Sim_DI_Data));\n" % (counter)
        switch += "  assert(processor->addressHasTranslation((virtAddrData%d + 3), Sim_DI_Data));\n" % (counter)
        switch += "  assert(processor->addressIsAligned(virtAddrData%d, 4));\n" % (counter)
        switch += "  data%d = processor->readFromMemory(virtAddrData%d, 2, Sim_DI_Data);\n" % (counter, counter)
        counter += 1
    # call simulator request handler
    switch += "  %s( " % handlerName
    counter = 0
    if needsProcessorNumber:
      switch += "processorNumber, "
    for argType, argName in arguments:
        switch += "(%s) data%d, " % (allowedSimTypes[argType], counter)
        counter += 1
    switch = switch.rstrip( ", " ) # strip one-too-many-th comma
    switch += " );\n"
    switch += "  break;\n"
    myDef['simreq_branch'] = switch

    DEFS.append( myDef )

def generateSimRequestChannelRegistration():
    """Returns a dictionary with replacement ruls to initialize and register
(with the simulator and with Simics) the sim request channels. We have to
register with Simics so that values persist across checkpoints."""
    replacements = {}
    replacements['init_channel'] = ""
    replacements['register_channel_with_simulator'] = ""
    replacements['register_channel_with_simics'] = ""
    replacements['channel_accessors'] = ""
    for i in range( SIMULATOR_REQUEST_NUM_CHANNELS ):
        # simulated-program-side stuff
        replacements['init_channel'] += "  %s.data%d = 0;\n" % (SIMULATOR_REQUEST_CHANNEL_GLOBAL, i)
        
        replacements['register_channel_with_simulator'] += "  PYRITE_generateRegisterChannelData%dSimRequest( (void *) &(%s.data%d) );\n" % (i, SIMULATOR_REQUEST_CHANNEL_GLOBAL, i)

        # simulator-side stuff
        # getter and setter
        replacements['channel_accessors'] += "static attr_value_t getSimReqChannelData%d(void* arg, conf_object_t* obj, attr_value_t* idx) {\n" % i
        replacements['channel_accessors'] += "  return SIM_make_attr_integer( %s->virtAddrData%d );\n" % (SIMULATOR_REQUEST_CHANNEL_ADDRS_GLOBAL, i)
        replacements['channel_accessors'] += "}\n\n"
        replacements['channel_accessors'] += "static set_error_t setSimReqChannelData%d(void* arg, conf_object_t* obj, attr_value_t *val, attr_value_t *idx) {\n" % i
        replacements['channel_accessors'] += "  assert( Sim_Val_Integer == val->kind );"
        replacements['channel_accessors'] += "  %s->virtAddrData%d = val->u.integer;\n" % (SIMULATOR_REQUEST_CHANNEL_ADDRS_GLOBAL, i)
        replacements['channel_accessors'] += "  return Sim_Set_Ok;"
        replacements['channel_accessors'] += "}\n\n"
        
        replacements['register_channel_with_simics'] += 'assert( 0 == SIM_register_typed_attribute(myClass, "simReqChannelData%d", ' % i
        replacements['register_channel_with_simics'] += "getSimReqChannelData%d, NULL, " % i # getter
        replacements['register_channel_with_simics'] += "setSimReqChannelData%d, NULL, " % i # setter
        replacements['register_channel_with_simics'] += "Sim_Attr_Optional, "
        replacements['register_channel_with_simics'] += '"i", ' # integer type
        replacements['register_channel_with_simics'] += 'NULL, "auto-generated parameter attribute: simReqChannelData%d") );\n' % i
        
    return replacements


def generateBoolParam( param ):
    """Adds a dictionary full of entries for the specified parameter
of boolean type to the global DEFS list"""
    t = { 'kind':str,
          'name':str,
          'initialValue':bool,
          Optional('readOnly'):bool }
    mustBeType( param, Strict(t) )

    myDef = {}

    paramName = param['name']
    functionName = initialCaps( paramName )
    paramCType = "bool" # the C type that corresponds to this param's type
    paramInitialValue = str( param['initialValue'] ).upper()

    generateSetter = ( 'readOnly' not in param or not param['readOnly'] )

    getter = "%s get%s(void) { return %s; }" % (paramCType, functionName, paramName)
    myDef['param_method'] = [ getter ]

    # param setter (optional)
    if generateSetter:
        setter = "void set%s(%s param) { %s = param; }\n" % (functionName, paramCType, paramName)
        myDef['param_method'].append( setter )

    myDef['param_dump'] = 'os << "\''+paramName+'\': " << (g_params.get'+functionName+'() ? "True" : "False") << "," << endl;'

    # class member
    myDef['param_member'] = "  %s %s;" % (paramCType, paramName)

    # init code
    if generateSetter:
        myDef['param_init'] = 'set%s( %s );' % (functionName, paramInitialValue)
    else:
        myDef['param_init'] = '%s = %s;' % (paramName, paramInitialValue)

    wrapper, registration = simicsAttributeWrapper( functionName,
                                                    simicsType="boolean",
                                                    obj=g_replacements['PARAM_GLOBAL'],
                                                    generateSetter=generateSetter )
    myDef["param_wrapper"] = wrapper
    myDef["param_attribute"] = registration

    return myDef

def generateIntParam( param ):
    """Adds a dictionary full of entries for the specified parameter
of integer type to the global DEFS list"""
    t = { 'kind':str,
          'name':str,
          'initialValue':int,
          Optional('readOnly'):bool }
    mustBeType( param, Strict(t) )

    myDef = {}

    paramName = param['name']
    functionName = initialCaps( paramName )
    paramInitialValue = param['initialValue']
    paramCType = "integer_t" # the C type that corresponds to this param's type

    generateSetter = ( 'readOnly' not in param or not param['readOnly'] )

    getter = "%s get%s(void) { return %s; }" % (paramCType, functionName, paramName)
    myDef['param_method'] = [ getter ]

    # param setter (optional)
    if generateSetter:
        setter = "void set%s(%s param) { %s = param; }\n" % (functionName, paramCType, paramName)
        myDef['param_method'].append( setter )

    myDef['param_dump'] = 'os << "\''+paramName+'\': " << g_params.get'+functionName+'() << "," << endl;'

    # class member
    myDef['param_member'] = "  %s %s;" % (paramCType, paramName)

    # init code
    if generateSetter:
        myDef['param_init'] = 'set%s( %s );' % (functionName, paramInitialValue)
    else:
        myDef['param_init'] = '%s = %s;' % (paramName, paramInitialValue)

    wrapper, registration = simicsAttributeWrapper( functionName,
                                                    simicsType="integer",
                                                    obj=g_replacements['PARAM_GLOBAL'],
                                                    generateSetter=generateSetter )
    myDef["param_wrapper"] = wrapper
    myDef["param_attribute"] = registration
    
    return myDef

def generateEnumParam( param ):
    """Adds a dictionary full of entries for the specified parameter
of enumeration type to the global DEFS list"""
    t = { 'kind':str,
          'name':str,
          'initialValue':str,
          'possibleValues':[ OneOrMore(str) ],
          Optional('readOnly'):bool }
    mustBeType( param, Strict(t) )

    myDef = {}

    paramName = param['name']
    functionName = initialCaps( paramName )
    paramInitialValue = param['initialValue']
    paramCType = "enum " + paramName + "_t" # the C type that corresponds to this param's type

    generateSetter = ( 'readOnly' not in param or not param['readOnly'] )

    getter = "%s get%s(void) { return %s; }" % (paramCType, functionName, paramName)
    myDef['param_method'] = [ getter ]

    # param setter (optional)
    if generateSetter:
        # NB: setter takes a simple int (to play nice with simics
        # attributes), but we still do range checking.
        setter =  "void set%s(int param) { \n" % functionName
        setter += ' ASSERT_MSG( (param >= 0 && param < %d), "Invalid value for enum param %s" );\n' % (len(param['possibleValues']), paramName)
        setter += " %s = (%s) param;\n" % (paramName, paramCType)
        setter += "}"
        myDef['param_method'].append( setter )

    dumpCode = '\n os << "\''+paramName+'\': \'";\n'
    dumpCode += "switch ( g_params.get%s() ) {\n" % functionName 
    for enum in param['possibleValues']:
        dumpCode += '  case '+enum+': os << "'+enum+'"; break;\n'
    dumpCode += '  default: ERROR_MSG( "Invalid value for %s" );\n' % paramName
    dumpCode += "}\n"
    dumpCode += 'os << "\'," << endl;\n\n'
    myDef['param_dump'] = dumpCode

    enumString = ", ".join( param['possibleValues'] )
    myDef['param_preamble'] = "%s { %s };" % (paramCType,enumString)
    myDef['param_python_code'] = []
    for pv in param['possibleValues']:
        myDef['param_python_code'].append( "%s = %d" % (pv, param['possibleValues'].index(pv)) )

    # class member
    myDef['param_member'] = "  %s %s;" % (paramCType, paramName)

    # init code
    if generateSetter:
        myDef['param_init'] = 'set%s( %s );' % (functionName, paramInitialValue)
    else:
        myDef['param_init'] = '%s = %s;' % (paramName, paramInitialValue)

    wrapper, registration = simicsAttributeWrapper( functionName,
                                                    simicsType="integer",
                                                    obj=g_replacements['PARAM_GLOBAL'],
                                                    generateSetter=generateSetter )
    myDef["param_wrapper"] = wrapper
    myDef["param_attribute"] = registration
    
    return myDef

def generateStringParam( param ):
    """Adds a dictionary full of entries for the specified parameter
of string type to the global DEFS list"""
    t = { 'kind':str,
          'name':str,
          'initialValue':str,
          Optional('readOnly'):bool }
    mustBeType( param, Strict(t) )

    myDef = {}

    paramName = param['name']
    debugCategory = False
    if paramName == 'debugCategory':
        debugCategory = True
    functionName = initialCaps( paramName )
    # we need a C string constant for the initial value
    paramInitialValue = '"%s"' % param['initialValue']
    paramCType = "string" # the C type that corresponds to this param's type
# TODO: what type should debugCategory be? const string&??
##     if paramName == 'debugCategory':
##         paramCType = "const string"

    generateSetter = ( 'readOnly' not in param or not param['readOnly'] )

    getter = "%s get%s() { return %s; }" % (paramCType, functionName, paramName)
    if debugCategory:
        getter = "%s get%s() {\n" % (paramCType, functionName)
        getter += """
  int temp = DEBUG_CATEGORY;
  if ( temp == DEBUG_NONE ) {
    DEBUG_CATEGORY_STRING = '#';
    return DEBUG_CATEGORY_STRING;
  } else if ( temp == DEBUG_ALL ) {
    DEBUG_CATEGORY_STRING = '*';
    return DEBUG_CATEGORY_STRING;
  }
  for ( int i = 0; i < (sizeof(int) * 8); i++ ) { // iterate over the bits in DEBUG_CATEGORY
    if ( temp & 0x1 ) {
      DEBUG_CATEGORY_STRING += DEBUG_CATEGORY_SHORTCUTS[i];
    }
    temp >>= 1;
  }
  return DEBUG_CATEGORY_STRING;
}
"""
    myDef['param_method'] = [ getter ]

    # param setter (optional)
    if generateSetter:
        setter =  "void set%s(%s param) { \n" % (functionName, paramCType)
        if not debugCategory:
            # TODO: need to copy the string to avoid leaks and dangling pointers...does "=" just work?
            setter += "  %s = param;\n" % (paramName)
        else:
            setter += """
  NO_DEBUG();
  for ( int i = 0; i < param.size(); i++ ) {
    char c = param[i];
    if ( c == '#' ) {
      NO_DEBUG();
      return;
    } else if ( c == '*' ) {
      ALL_DEBUG();
      return;
    }
    int j = 0;
    for ( char d = DEBUG_CATEGORY_SHORTCUTS[j]; d != ' '; d = DEBUG_CATEGORY_SHORTCUTS[++j] ) {
      if ( c == d ) {
        DEBUG_CAT( (DebugCategory)(1 << j) );
      }
    }
  }
"""        

        setter += "}"
        myDef['param_method'].append( setter )

    myDef['param_dump'] = 'os << "\''+paramName+'\': \'" << g_params.get'+functionName+'() << "\'," << endl;'

    # class member
    myDef['param_member'] = "  %s %s;" % (paramCType, paramName)

    # init code
    if generateSetter:
        myDef['param_init'] = 'set%s( %s );' % (functionName, paramInitialValue)
    else:
        # TODO: does this make a copy appropriately? Does this leak, or lead to dangling pointers?
        myDef['param_init'] = '%s = %s;' % (paramName, paramInitialValue)

    wrapper, registration = simicsAttributeWrapper( functionName,
                                                    simicsType="string",
                                                    obj=g_replacements['PARAM_GLOBAL'],
                                                    generateSetter=generateSetter )
    myDef["param_wrapper"] = wrapper
    myDef["param_attribute"] = registration
    
    return myDef

def generateDebugCategory( cat ):
    """Adds a dictionary full of entries for the specified debug
category to the global DEFS list"""

    t = { 'kind':str,
          'name':str,
          'shortcut':str,
          'color':str }
    mustBeType( cat, Strict(t) )

    myDef = {}

    catName = cat['name']
    shortcut = cat['shortcut']
    color = cat['color']

    if not catName.isupper():
        print "WARNING: debug category name '%s' doesn't conform to upper-case convention" % catName
    if not catName.startswith( "DEBUG_" ):
        print "WARNING: debug category name '%s' doesn't conform to DEBUG_ prefix convention" % catName
    if len( shortcut ) != 1:
        print "ERROR: debug category '%s' must have a 1-character shortcut, not '%s'" % (catName, shortcut)
        sys.exit( 1 )

    myDef['debug_enum_member'] = catName + " = (1 << %%count%%),"
    myDef['debug_color_ifelse'] = 'if ( CATEGORY & %s ) { OSTREAM << %s; } else \\' % (catName, color.upper())
    myDef['debug_shortcut'] = "'" + shortcut + "',"
    assert shortcut not in DEBUG_CATEGORY_SHORTCUTS, "Duplicate debug category shortcut: " + shortcut
    DEBUG_CATEGORY_SHORTCUTS.add( shortcut )

    myDef['debug_help'] = "%s = %s" % (shortcut, catName)

    return myDef

def main():
    """Runs the circus."""
    parser = OptionParser( usage="%prog [options] defs_file" )
    parser.disable_interspersed_args()

    parser.add_option("-o", "--output-dir-root", dest="outputDirectoryRoot",
                      default="SCRIPT_LOCATION/autogen",
                      help="root of auto-generated file tree. default: %default")

    parser.add_option("--deps", dest="generateDeps", default=False, action="store_true",
                      help="print whitespace-delimited list of files that will be generated on stdout")

    (opts, args) = parser.parse_args()
    if len( args ) > 1:
        print "ERROR: can only process 1 .def file at a time. Sorry."
        sys.exit( 1 )
        
    
    if opts.outputDirectoryRoot == "SCRIPT_LOCATION/autogen":
        # normalize paths to from wherever it is we're being invoked
        rootDir = os.path.abspath( os.path.dirname(sys.argv[0]) )
        if rootDir == "":
            rootDir = os.getcwd()
        rootDir = os.path.join( rootDir, "autogen" )
    else: # user-specified root
        rootDir = opts.outputDirectoryRoot

    global DEFS, NUM_SIMULATOR_REQUESTS, g_replacements
    defsFile = args[0]
    fileRoot = os.path.splitext( os.path.basename(defsFile) )[0]

    # check for filesystem errors
    if not os.path.isfile( defsFile ):
        print "ERROR: could not find defs file:", defsFile
        sys.exit( 1 )
    if not defsFile.endswith( ".def" ):
        print "WARNING: malformed file extension:", defsFile, "...processing anyway"

    # process .defs file
    
    lines = file( defsFile ).readlines()
    # filter out empty lines, which can confuse the python parser if
    # they're at the beginning of the file
    lines = [ x for x in lines if x.strip() != "" ]
    lines = "\n".join( lines )

    try:
        listOfDicts = eval( lines )
    except SyntaxError, se:
        print "Syntax error in parsing the defs file " + defsFile
        print "It should be a python list of dictionaries."
        print ""
        raise se # so we get the detailed error message
    
    malformedFileMessage = "ERROR: malformed .def file " + defsFile + ": "

    mustBeType( listOfDicts, [ OneOrMore(types.DictType) ] )
    
    # the dict with the 'global' key has metadata for this .def file
    defType = None
    autoGenCodeTemplates = None
    globalIndex = 0
    for d in listOfDicts:
        if 'global' in d:
            dtype = d['defType']
            if dtype == "Stat":
                defType = DefType.Stat
                autoGenCodeTemplates = statTemplates
            elif dtype == "SimulatorRequest":
                defType = DefType.SimRequest
                autoGenCodeTemplates = simulatorRequestTemplates
            elif dtype == "Param":
                defType = DefType.Param
                autoGenCodeTemplates = paramTemplates
            elif dtype == "DebugCategory":
                defType = DefType.DebugCategory
                autoGenCodeTemplates = debugCategoryTemplates
            else:
                assert False, "Invalid defType: " + dtype
            break
        globalIndex += 1
    del listOfDicts[ globalIndex ] # we don't need the global dict anymore

    # typecheck the templates
    for template in autoGenCodeTemplates:
        template_t = { 'filename':types.LambdaType,
                       'subdirectory':str,
                       'template':str }
        mustBeType( template, template_t )

    # print deps and quit
    if opts.generateDeps:
        for template in autoGenCodeTemplates:
            # create file
            filename = template['filename']( fileRoot )
            filePath = os.path.join( rootDir, template['subdirectory'], filename )
            print filePath
        return

    # actually generate code
    for d in listOfDicts:
        if defType == DefType.Stat:
            stat = None
            if d['kind'] == "STAT_INT":
                stat = generateIntStat( d )
                t = { 'stat_method':[OneOrMore(str)], 'stat_member':str,
                      'stat_dump':str, 'stat_init':str, 'stat_clear':str,
                      'stat_wrapper':str, 'stat_attribute':str }
                mustBeType( stat, Strict(t) )
            elif d['kind'] == "STAT_HISTOGRAM":
                stat = generateHistogram( d )
            elif d['kind'] == "STAT_HASHTABLE":
                stat = generateHashtableStat( d )
            DEFS.append( stat )
                
        elif defType == DefType.SimRequest:
            if d['kind'] == "SIM_REQUEST":
                NUM_SIMULATOR_REQUESTS += 1
                generateSimRequest( d )
                # TODO: typecheck sim request dictionaries

        elif defType == DefType.Param:
            param = None
            if d['kind'] == "PARAM_BOOL":
                param = generateBoolParam( d )
            elif d['kind'] == "PARAM_INT":
                param = generateIntParam( d )
            elif d['kind'] == "PARAM_ENUM":
                param = generateEnumParam( d )
            elif d['kind'] == "PARAM_STRING":
                param = generateStringParam( d )
            t = { 'param_method':[OneOrMore(str)], 'param_member':str,
                  'param_dump':str, 'param_init':str,
                  'param_wrapper':str, 'param_attribute':str,
                  Optional('param_python_code'):str,
                  Optional('param_preamble'):str }
            mustBeType( param, Strict(t) )
            DEFS.append( param )

        elif defType == DefType.DebugCategory:
            if d['kind'] == "DEBUG_CATEGORY":
                dbg = generateDebugCategory( d )
                t = { 'debug_enum_member':str,
                      'debug_color_ifelse':str,
                      'debug_shortcut':str,
                      'debug_help':str }
                mustBeType( dbg, Strict(t) )
                DEFS.append( dbg )
        
        else:
            # we reach here if we had an invalid defType or 'kind'
            print malformedFileMessage, "unknown def kind:", d['kind'],
            print "or def type:", defType
            sys.exit( 1 )
        
    autoGeneratedWarning = "\n// WARNING: this file was auto-generated from\n"
    autoGeneratedWarning +=  "// '%s'.\n" % defsFile
    autoGeneratedWarning +=  "// Do not edit this file directly!\n\n"

    for t in autoGenCodeTemplates:

        # create file
        filename = t['filename']( fileRoot )
        filePath = os.path.join( rootDir, t['subdirectory'], filename )
        autoGenFile = file( filePath, "w+" )

        # `replacements' maps special replacement tokens in the file
        # template (strings of the form %%TOKEN%%) to the values they
        # should take
        replacements = copy.deepcopy( g_replacements )
        
        # generic tokens
        replacements['AUTO_GENERATED_WARNING'] = autoGeneratedWarning
        replacements['FILE_ROOT'] = fileRoot
        replacements['HEADER_PROTECTION'] = headerify( filename )
        replacements['NUM_SIMULATOR_REQUESTS'] = str( NUM_SIMULATOR_REQUESTS )

        # add replacements for initializing and registering (with Simics and
        # with the simulator) the sim request channels
        for k, v in generateSimRequestChannelRegistration().iteritems():
            replacements[k] = v

        # compute replacement values for the entries in DEFS
        count = 1
        for d in DEFS:
            for key, val in d.iteritems():
                valString = stringify( val )
                valString = valString.replace( "%%count%%", str(count) )
                if key in replacements:
                    replacements[key] += ( valString + "\n" )
                else:
                    replacements[key] = ( valString + "\n" )
            count += 1

        fileContents = t['template']
        # NOTE: this *could* be optimized into a single pass, but it's
        # pretty durn fast as it is...
        for rcKey, rcVal in replacements.iteritems():
            fileContents = fileContents.replace( "%%" + rcKey + "%%", rcVal )

        # check that no tokens remain unexpanded
        if -1 != fileContents.find( "%%" ):
            print "Some tokens were not expanded! Template typo...?"
            print "Partially-expanded template contents:"
            print fileContents
            sys.exit( 1 )

        # NB: buffering the file contents into a single write() makes
        # a quite noticeable performance difference
        autoGenFile.write( fileContents )
        autoGenFile.close()

if __name__ == "__main__":
    main() # go to it!
