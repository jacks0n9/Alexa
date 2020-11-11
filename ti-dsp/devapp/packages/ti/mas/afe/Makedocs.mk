# ==============================================================================
# File            : Makedocs.mk
# Description     : 
#
#   GNU makefile to generate AFE documentation from sources using Doxygen
#
# ==============================================================================

# The target for generating documents using doxygen is gendocs

release : genapidocs

genapidocs : afe.h
	-@echo generating AFE API documentation ...
	doxygen ./docs/doxygen/APIDoxyfile

# End of Makedocs.mk
