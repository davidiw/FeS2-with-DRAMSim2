#!/usr/bin/python
import sys

if(len(sys.argv) < 2):
    print "USAGE: " + sys.argv[0] + " <infile>"
    print "\t<infile>: stats file to summarize"
    sys.exit(1)

infilename = sys.argv[1]
infile = eval(file(infilename).read())
stats = infile['stats']
params = infile['params']

print "-"*80
print "infile: " + infilename

print "total cycles:", stats['totalCycles']

print "total UOps:",
for i in range(params['numProcessors']):
    print stats['totalUOps'][i], "( UPC: %.3f" % (float(stats['totalUOps'][i])/stats['totalCycles']), "),",
print

print "total X86 Ops:",
for i in range(params['numProcessors']):
    print stats['totalX86Instructions'][i], '( IPC: %.3f' % (float(stats['totalX86Instructions'][i])/stats['totalCycles']), "),",
print

print 'error rate:',
for i in range(params['numProcessors']):
    print '%(percent).4f%%' % {'percent' : (1 - float(stats['correctlyExecutedX86Instructions'][i])/stats['totalX86Instructions'][i]) * 100}, ",",
print

print 'branch prediction:',
for i in range(params['numProcessors']):
    branchPredictionRate = float('infinity')
    if stats['totalBranchPredictions'][i] > 0:
        totalBranchPredictions = stats['totalBranchPredictions'][i]
        totalBranchMispredictions = stats['totalBranchMispredictions'][i]
        branchPredictionRate = (1 - float(totalBranchMispredictions)/totalBranchPredictions) * 100
    print '%(percent).2f%%' % {'percent' : branchPredictionRate},
    print "(", totalBranchPredictions - totalBranchMispredictions, "/", totalBranchPredictions, "),",
print

print 'L1 cache hit rate:',
for i in range(params['numProcessors']):
    totalL1Accesses = 0
    if stats['loads'][i] > 0 or stats['stores'][i] > 0:
        totalL1Accesses = stats['loads'][i] + stats['stores'][i]
        l1Hits = totalL1Accesses - stats['l1Misses'][i]
        l1HitRate = (float(l1Hits)/totalL1Accesses) * 100
    else:
        l1Hits = 0
        l1HitRate = float('infinity')
    print '%(percent).2f%%' % {'percent' : l1HitRate},
    print "(", l1Hits, "/", totalL1Accesses, "),",
print

print 'L2 cache hit rate:',
for i in range(params['numProcessors']):
    totalL2Accesses = 0
    if stats['l1Misses'][i] > 0:
        totalL2Accesses = stats['l1Misses'][i]
        l2Hits = totalL2Accesses - stats['l2Misses'][i]
        l2HitRate = (float(l2Hits)/totalL2Accesses) * 100
    else:
	l2Hits = 0
	l2HitRate = float('infinity')
    print '%(percent).2f%%' % {'percent' : l2HitRate},
    print "(", l2Hits, "/", totalL2Accesses, "),",
print

print "-"*80
