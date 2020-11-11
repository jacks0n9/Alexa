include Rules.mak

.PHONY:	all biospsp

#==============================================================================
# Default build target
#==============================================================================
default: all

#==============================================================================
# Build target build necessary components to enable all other build targets.
#==============================================================================
components: biospsp devnodeServer libCE 

#==============================================================================
# Clean up the targets built by 'make all'.
#==============================================================================
clean: devnodeServer_clean biospsp_clean libCE_clean

#==============================================================================
# Build everything rebuildable.
#==============================================================================
all: components

#==============================================================================
# Build devnodeServer
#==============================================================================
devnodeServer:
	make -C devapp/packages/ti/mas/examples/devnodeServer all

devnodeServer_clean:
	make -C devapp/packages/ti/mas/examples/devnodeServer clean

#==============================================================================
# Build PLATFORMS
#==============================================================================
biospsp:
	make -C biospsp/packages/ti/pspiom all

biospsp_clean:
	make -C biospsp/packages/ti/pspiom clean

#==============================================================================
# Build libCE
#==============================================================================
libCE: biospsp devnodeServer
	make -C devapp/packages/ti/mas/examples/devnodeApp libCE

libCE_clean:
	make -C devapp/packages/ti/mas/examples/devnodeApp clean

