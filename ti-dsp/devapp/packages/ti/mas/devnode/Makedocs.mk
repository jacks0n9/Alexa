# ==============================================================================
# File            : Makedocs.mk
# Description     : 
#
#   GNU makefile to generate DEVNODE documentation from sources using Doxygen
#
# ==============================================================================

# The target for generating documents using doxygen is gendocs

release : genapidocs

genapidocs : idevnode.h
	-@echo generating DEVNODE API documentation ...
	doxygen ./docs/doxygen/APIDoxyfile

	# End of Makedocs.mk
