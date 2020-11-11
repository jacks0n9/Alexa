# ==============================================================================
# File            : Makedocs.mk
# Description     : 
#
#   GNU makefile to generate APU documentation from sources using Doxygen
#
# ==============================================================================

# The target for generating documents using doxygen is gendocs

release : genapidocs

genapidocs : genCtl.h
	-@echo generating APU API documentation ...
	doxygen ./docs/doxygen/APIDoxyfile

# End of Makedocs.mk
