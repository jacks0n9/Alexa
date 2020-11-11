include ../../../../Rules.mak


.PHONY:	all i2c prcm mcbsp platforms mcbsp_sample_master

#==============================================================================
# Default build target
#==============================================================================
default: all

#==============================================================================
# Build target build necessary components to enable all other build targets.
#==============================================================================
components: i2c prcm mcbsp platforms mcbsp_sample_master mcbsp_sample_slave

#==============================================================================
# Clean up the targets built by 'make all'.
#==============================================================================
clean:	i2c_clean prcm_clean mcbsp_clean platforms_clean mcbsp_sample_master_clean mcbsp_sample_slave_clean

#==============================================================================
# Build everything rebuildable.
#==============================================================================
all: components 

#==============================================================================
# Build PRCM 
#==============================================================================
prcm:
	cd $(BIOSPSP_BUILD_ROOT)/packages/ti/pspiom/prcm/Debug && $(MAKE) all	

prcm_clean:
	-cd $(BIOSPSP_BUILD_ROOT)/packages/ti/pspiom/prcm/Debug && $(MAKE) clean

#==============================================================================
# Build PLATFORMS 
#==============================================================================
platforms:
	cd $(BIOSPSP_BUILD_ROOT)/packages/ti/pspiom/platforms/dm3730/build/ccs4/Debug && $(MAKE) all

platforms_clean:
	cd $(BIOSPSP_BUILD_ROOT)/packages/ti/pspiom/platforms/dm3730/build/ccs4/Debug && $(MAKE) clean

#==============================================================================
# Build I2C
#==============================================================================
i2c:
	cd $(BIOSPSP_BUILD_ROOT)/packages/ti/pspiom/i2c/Debug && $(MAKE) all

i2c_clean:
	cd $(BIOSPSP_BUILD_ROOT)/packages/ti/pspiom/i2c/Debug && $(MAKE) clean

#==============================================================================
# Build MCBSP 
#==============================================================================
mcbsp:
	cd $(BIOSPSP_BUILD_ROOT)/packages/ti/pspiom/mcbsp/build/dm3730/ccs4/Debug && $(MAKE) all	

mcbsp_clean:
	cd $(BIOSPSP_BUILD_ROOT)/packages/ti/pspiom/mcbsp/build/dm3730/ccs4/Debug && $(MAKE) clean


#==============================================================================
# Build MCBSPSAMPLEMASTER 
#==============================================================================
mcbsp_sample_master:
	cd $(BIOSPSP_BUILD_ROOT)/packages/ti/pspiom/examples/dm3730/mcbsp/build/ccs4/master/Debug && $(MAKE) all	

mcbsp_sample_master_clean:
	cd $(BIOSPSP_BUILD_ROOT)/packages/ti/pspiom/examples/dm3730/mcbsp/build/ccs4/master/Debug && $(MAKE) clean


#==============================================================================
# Build MCBSPSAMPLESLAVE
#==============================================================================
mcbsp_sample_slave:
	cd $(BIOSPSP_BUILD_ROOT)/packages/ti/pspiom/examples/dm3730/mcbsp/build/ccs4/slave/Debug && $(MAKE) all	

mcbsp_sample_slave_clean:
	cd $(BIOSPSP_BUILD_ROOT)/packages/ti/pspiom/examples/dm3730/mcbsp/build/ccs4/slave/Debug && $(MAKE) clean

