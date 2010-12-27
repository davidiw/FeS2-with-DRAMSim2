#!/usr/bin/python
from pacg.typesystem import *
import unittest

class TestTypeChecker(unittest.TestCase):

    # base types
    def test0(self): typecheck(1, int)
    def test1(self): typecheck(100000000000, int)
    def test2(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *(1, str))
    def test3(self): typecheck(1000000000000000000000000000000000, int)
    def test4(self): typecheck("", str)
    def test5(self): typecheck("s", str)
    def test6(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *("s", int))

    # lists
    def test7(self): typecheck([], [])
    def test8(self): typecheck([1], [int])
    def test9(self): typecheck(["s"], [str])
    def test10(self): typecheck([1, "s"], [int, str])
    def test11(self): typecheck([[]], [[]])
    def test12(self): typecheck([[1]], [[int]])
    def test13(self): typecheck([["s"]], [[str]])
    def test14(self): typecheck([["a"], "s"], [[str], str])
    def test15(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *([], int))
    def test16(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *([], str))
    def test17(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *([1, "s"], [str, int]))
    def test18(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *([[]], []))
    def test19(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *([[[]]], [[]]))
    def test20(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *([], [[]]))
    def test21(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *([["a"]], [[int]]))
    def test22(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *([["a"], "s"], [[str], int]))

    # maps
    def test23(self): typecheck({}, {})
    def test24(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *({}, []))
    def test25(self): typecheck({"key" : 1}, {"key" : int})
    def test26(self): typecheck({"key" : "s"}, {"key" : str})
    def test27(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *({"key" : "s"}, {"key" : int}))
    def test28(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *({"key" : "s"}, {"k" : str}))
    def test29(self): typecheck({"key" : []}, {"key" : []})
    def test30(self): typecheck({"key" : [0]}, {"key" : [int]})
    def test31(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *({"key" : [0]}, {"k" : [int]}))
    def test32(self): typecheck({"key" : [0, "s"]}, {"key" : [int, str]})
    def test33(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *({"key" : [0, "s"]}, {"key" : [str, int]}))
    def test34(self): typecheck({ "key1" : 1,
                                  "key2" : "s" 
                                  },
                                { "key1" : int,
                                  "key2" : str
                                  })
    def test35(self): typecheck({ "key1" : [1],
                                  "key2" : ["s"]
                                  },
                                { "key1" : [int],
                                  "key2" : [str]
                                  })
    def test36(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *({ "key1" : [1],
                                                                                 "key2" : ["s"]
                                                                                 },
                                                                               { "key1" : [int],
                                                                                 "k" : [str]
                                                                                 }))
    
    # zero or more
    def test37(self): self.failUnlessRaises(ValueError, typecheck, 8, ZeroOrMore(int))
    def test38(self): typecheck([], [ZeroOrMore(int)])
    def test39(self): typecheck([], [ZeroOrMore(str)])
    def test40(self): typecheck([], [ZeroOrMore([])])
    def test41(self): typecheck([], [ZeroOrMore([[[[[]]]]])])
    def test42(self): typecheck([], [ZeroOrMore({})])
    def test43(self): typecheck([1], [ZeroOrMore(int)])
    def test44(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *([1], [ZeroOrMore(str)]))
    def test45(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *([1], [ZeroOrMore([])]))
    def test46(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *([1], [ZeroOrMore({})]))
    def test47(self): typecheck([1,1,1,2], [ZeroOrMore(int)])
    def test48(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *([1,1,1,2], [ZeroOrMore(str)]))
    def test49(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *([1,1,1,2], [ZeroOrMore([])]))
    def test50(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *([1,1,1,2], [ZeroOrMore({})]))
    def test51(self): typecheck([1,1,1,2], [ZeroOrMore(int),
                                            ZeroOrMore(int)])
    def test52(self): typecheck([1,1,1,2], [ZeroOrMore(int),
                                            ZeroOrMore(str)])
    def test53(self): typecheck([1,1,1,2], [ZeroOrMore(int),
                                            ZeroOrMore([])])
    def test54(self): typecheck([1,1,1,2], [ZeroOrMore(int),
                                            ZeroOrMore([])])
    def test55(self): typecheck([1,1,1,2], [ZeroOrMore(int),
                                            ZeroOrMore({})])
    def test56(self): typecheck([1,1,1,2], [ZeroOrMore(str),
                                            ZeroOrMore(int)])
    def test57(self): typecheck([1,1,1,2], [ZeroOrMore([]),
                                            ZeroOrMore(int)])
    def test58(self): typecheck([1,1,1,2], [ZeroOrMore({}),
                                            ZeroOrMore(int)])
    def test59(self): typecheck([1,1,1,2], [int, ZeroOrMore(int)])
    def test60(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *([1,1,1,2], [ZeroOrMore(int),
                                                                                           int]))
    def test61(self): typecheck(["s",1,1,2], [ZeroOrMore(str),
                                              int, int, int])
    def test62(self): typecheck(["s",1,1,2], [ZeroOrMore(str),
                                              ZeroOrMore(int)])
    def test63(self): typecheck(["s",{},1,2], [ZeroOrMore(str),
                                               {},
                                               ZeroOrMore(int)])
    def test64(self): typecheck(["s",{},1,2], [ZeroOrMore(str),
                                               ZeroOrMore({}),
                                               ZeroOrMore(int)])
    def test65(self): typecheck({ "key1" : 1,
                                  "key2" : ["s", "y"]
                                  },
                                { "key1" : int,
                                  "key2" : [ZeroOrMore(str)]
                                  })
    def test66(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *({ "key1" : 1,
                                                                                 "key2" : ["s", "y"]
                                                                                 },
                                                                               { "key1" : [ZeroOrMore(int)],
                                                                                 "key2" : [ZeroOrMore(str)]
                                                                                 }))
    def test67(self): typecheck([{"k1" : "v1", "k2" : 0},
                                 {"k1" : "v1", "k2" : 0}],
                                [ZeroOrMore({"k1" : str, "k2" : int})])
    
    # one or more
    def test68(self): self.failUnlessRaises(ValueError, typecheck, 8, OneOrMore(int))
    def test69(self): typecheck([8], [OneOrMore(int)])
    def test70(self): typecheck([""], [OneOrMore(str)])
    def test71(self): typecheck([[]], [OneOrMore([])])
    def test72(self): typecheck([{}], [OneOrMore({})])
    def test73(self): typecheck([1], [OneOrMore(int)])
    def test74(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *([1], [OneOrMore(str)]))
    def test75(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *([1], [OneOrMore([])]))
    def test76(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *([1], [OneOrMore({})]))
    def test77(self): typecheck([1,1,1,2], [OneOrMore(int)])
    def test78(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *([1,1,1,2], [OneOrMore(str)]))
    def test79(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *([1,1,1,2], [OneOrMore([])]))
    def test80(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *([1,1,1,2], [OneOrMore({})]))
    def test81(self): typecheck([1,1,1,2], [int, OneOrMore(int)])
    def test82(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *([1,1,1,2], [OneOrMore(int),
                                                                                           int]))
    def test83(self): typecheck(["s",1,1,2], [OneOrMore(str),
                                              int, int, int])
    def test84(self): typecheck(["s",1,1,2], [OneOrMore(str),
                                              OneOrMore(int)])
    def test85(self): typecheck(["s",{},1,2], [OneOrMore(str),
                                               {},
                                               OneOrMore(int)])
    def test86(self): typecheck(["s",{},1,2], [OneOrMore(str),
                                               OneOrMore({}),
                                               OneOrMore(int)])
    def test87(self): typecheck({ "key1" : 1,
                                  "key2" : ["s", "y"]
                                  },
                                { "key1" : int,
                                  "key2" : [OneOrMore(str)]
                                  })
    def test88(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *({ "key1" : 1,
                                                                                 "key2" : ["s", "y"]
                                                                                 },
                                                                               { "key1" : [OneOrMore(int)],
                                                                                 "key2" : [OneOrMore(str)]
                                                                                 }))
    def test89(self): typecheck([{"k1" : "v1", "k2" : 0},
                                 {"k1" : "v1", "k2" : 0}],
                                [OneOrMore({"k1" : str, "k2" : int})])
    
    
    # optional
    def test90(self): typecheck([1], [Optional(int)])
    def test91(self): typecheck([], [Optional(int)])
    def test92(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *([1], [Optional(str)]))
    def test93(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *([1], [Optional(int),int]))
    def test94(self): typecheck([1,2,3], [int,int,int,Optional(int)])
    def test95(self): typecheck([1,2,3],
                                [int,int,int,Optional(int),Optional(bool),Optional(str)])
    def test96(self): typecheck([1,2,3], [int,int,Optional(int)])
    def test97(self): typecheck([1,2], [Optional(int),Optional(int)])
    def test98(self): typecheck([1,[2]], [Optional(int),Optional([Optional(int)])])
    def test100(self): typecheck({},
                                 {Optional("key1") : bool})
    def test100(self): typecheck({},
                                 {Optional("key1") : int, Optional("key2") : int})
    def test101(self): typecheck({"key1" : "v1"},
                                 {Optional("key1") : str})
    def test102(self): typecheck({"key1" : ["v1"]},
                                 {Optional("key1") : [str]})
    def test103(self): typecheck({"key1" : "v1", "key2" : "v2"},
                                 {Optional("key1") : str, "key2" : str})
    def test104(self): typecheck({"key1" : "v1", "key2" : "v2"},
                                 {Optional("key1") : str, Optional("key2") : str})
    def test105(self): typecheck({"key1" : "v1", "key2" : ["v2"]},
                                 {Optional("key1") : str, "key2" : [Optional(str)]})
    def test106(self): typecheck({"key1" : "v1", "key2" : []},
                                 {Optional("key1") : str, "key2" : [Optional(str)]})

    # illegal combos
    def test107(self): self.failUnlessRaises(AssertionError, OneOrMore,  
                                             Optional(bool))
    def test108(self): self.failUnlessRaises(AssertionError, ZeroOrMore,
                                             Optional(bool))
    def test109(self): self.failUnlessRaises(AssertionError, Optional,
                                             OneOrMore(bool))
    def test110(self): self.failUnlessRaises(AssertionError, Optional,
                                             ZeroOrMore(bool))
    def test111(self): self.failUnlessRaises(AssertionError, OneOrMore,
                                             Optional(bool))
    def test112(self): self.failUnlessRaises(AssertionError, ZeroOrMore,
                                             Optional(bool))
    def test113(self): self.failUnlessRaises(AssertionError, Optional, 
                                             OneOrMore(bool))
    def test114(self): self.failUnlessRaises(AssertionError, Optional,
                                             ZeroOrMore(bool))

    # tuples
    def test115(self): typecheck((), ())

    # some
    def test116(self): typecheck(None, Some(int))
    def test117(self): typecheck(None, Some(bool))
    def test118(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *(str, Some(bool)))
    def test119(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *(int, Some(bool)))
    def test120(self): self.failUnlessRaises(TypeCheckFailedError, typecheck, *(None, bool))
    def test121(self): typecheck(1, Some(int))
    def test121(self): typecheck("", Some(str))
    def test122(self): typecheck([1,2,3], Some(list))
    def test123(self): typecheck([None], [Some(int)])
    def test124(self): typecheck([None, None, None],
                                 [Some(int), Some(bool), types.NoneType])
    def test125(self): typecheck({'key':1, 2:3},
                                 {'key':int, 2:Some(int)})
    def test126(self): typecheck({'key':True, 2:3},
                                 {'key':Some(bool), 2:Some(int)})
    def test127(self): typecheck({'key':True, 2:3},
                                 Some({'key':Some(bool), 2:Some(int)}))
    def test128(self): typecheck([None, 2, None, 5],
                                 [OneOrMore(Some(int))])
    def test129(self): typecheck([None, None, None, 1],
                                 [ZeroOrMore(Some(int))])
    def test130(self): typecheck(None, Some([Some(int),OneOrMore(str)]))
    def test131(self): self.failUnlessRaises(TypeCheckFailedError,
                                             typecheck, *(None, str))

    # strict dictionary key checks
    def test132(self): typecheck({'a': 1, 'b': 2}, Strict({'a':int, 'b':int}))
    def test133(self): typecheck({'a': 1, 'b': 2}, Strict({Optional('a'):int, 'b':int}))
    def test134(self): typecheck({'a': 1, 'b': 2}, Strict({'a':int, 'b':Some(int)}))
    def test135(self): typecheck({'a': 1, 'b': 2}, Strict({Optional('a'):int, Optional('b'):int}))
    def test136(self): typecheck({'a': [1], 'b': [2]}, Strict({'a':[ZeroOrMore(int)], 'b':[OneOrMore(int)]}))
    def test137(self): self.failUnlessRaises(TypeCheckFailedError, typecheck,
                                             {'a': 1, 'b': 2, 'c': 3},
                                             Strict({'a':int, 'b':int}))
    def test138(self): self.failUnlessRaises(TypeCheckFailedError, typecheck,
                                             {'a': 1, 'b': 2, 'c': 3},
                                             Strict({Optional('a'):int, Optional('b'):int}))
    def test139(self): self.failUnlessRaises(TypeCheckFailedError, typecheck,
                                             {'a': 1, 'b': 2, 'c': 3},
                                             Strict({'a':int, 'b':Some(int)}))
    def test140(self): self.failUnlessRaises(TypeCheckFailedError, typecheck,
                                             {'a': 1, 'b': [2], 'c': 3},
                                             Strict({'a':int, 'b':[OneOrMore(int)]}))
    def test141(self): self.failUnlessRaises(TypeCheckFailedError, typecheck,
                                             {'a': 1, 'b': 2, 'c': 3},
                                             Strict({Optional('c'):Some(int), 'b':Some(int)}))

if __name__ == '__main__':
    unittest.main()
