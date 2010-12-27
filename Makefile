SIMICS_ROOT := simics/FeS2-workspace
HOSTTYPE = $(shell simics/simics/scripts/host-type.sh)
HOSTLIBDIR = ${HOSTTYPE}/lib

HOST32 = 0
ifeq (x86-linux, ${HOSTTYPE})
  HOST32 = 1
endif

ifdef ENABLE_DEBUG
  SCONS := scons --implicit-cache debug=1 host32=${HOST32} -j2
else
  SCONS := scons --implicit-cache host32=${HOST32} -j2
endif

#################
# begin targets #
#################

.PHONY: all over world pyrite-module clean

all: world pyrite-module

over: clean all

world:
	@echo "Building the world..."
	$(SCONS) $(DEBUG)
	@echo "...Done building the world"

pyrite-module:
	@echo "Building pyrite Simics module..."
	cd $(SIMICS_ROOT); $(MAKE) pyrite VERBOSE=yes 
	# for SGE
	chmod 755 $(SIMICS_ROOT)/$(HOSTLIBDIR)/pyrite.so
# for SGE
#chmod 755 $(SIMICS_ROOT)/x86-linux/lib/IS-sim.so
	@echo "...Done building pyrite Simics module"

clean: 
	$(SCONS) --clean --quiet
	cd $(SIMICS_ROOT); make clean-pyrite
	rm -rf obj/
