# ==============================================================================
# File            : Makedocs.mk
# Description     : 
#
#   GNU makefile to generate ECO documentation from sources using Doxygen
#
# ==============================================================================

# The target for generating documents using doxygen is gendocs

release : genapidocs

genapidocs : ieco.h
	-@echo generating ECO API documentation ...
	doxygen ./docs/doxygen/APIDoxyfile

	# End of Makedocs.mk
