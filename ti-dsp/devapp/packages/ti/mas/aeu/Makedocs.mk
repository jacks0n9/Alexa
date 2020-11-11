# ==============================================================================
# File            : Makedocs.mk
# Description     : 
#
#   GNU makefile to generate AEU documentation from sources using Doxygen
#
# ==============================================================================

# The target for generating documents using doxygen is gendocs

release : genapidocs

genapidocs : aeu.h
	-@echo generating AEU API documentation ...
	doxygen ./docs/doxygen/APIDoxyfile

# End of Makedocs.mk
