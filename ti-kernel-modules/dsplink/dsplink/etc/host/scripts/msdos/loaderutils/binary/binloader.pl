#!/usr/bin/perl
#   ============================================================================
#   @file   binloader.pl
#
#   @path   $(DSPLINK)/etc/host/scripts/msdos/loaderutils/binary/
#
#   @desc   Generates source and header files with information about the DSP
#           executable to be loaded by the binary loader. It also converts the
#           DSP executable from COFF into binary format, which can be loaded by
#           the binary loader.
#           The files generated are:
#           1. 'C' file containing an instance of the BINLOADER_ImageInfo
#              structure required by the binary loader.
#              This generated file can be built with the ARM-side example to
#              generate information used by the binary loader.
#           2. 'H' file containing declaration of the instance of the
#              BINLOADER_ImageInfo structure in the 'C' file. This file can be
#              included by the ARM-side application using the binary loader.
#           3. 'BIN' file generated through COFF to binary conversion of the
#              DSP executable.
#           4. 'DAT' file, which is an intermediate file generated by the script
#              and used for conversion of the DSP executable from COFF to
#              binary format. This file is available to the user for reference
#              about the memory map used for generation of the binary file.
#
#   @ver    1.65.01.05_eng
#   ============================================================================
#   Copyright (C) 2002-2009, Texas Instruments Incorporated -
#   http://www.ti.com/
#
#   Redistribution and use in source and binary forms, with or without
#   modification, are permitted provided that the following conditions
#   are met:
#   
#   *  Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#   
#   *  Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#      documentation and/or other materials provided with the distribution.
#   
#   *  Neither the name of Texas Instruments Incorporated nor the names of
#      its contributors may be used to endorse or promote products derived
#      from this software without specific prior written permission.
#   
#   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
#   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
#   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
#   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
#   OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
#   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
#   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
#   EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#   ============================================================================

#-------------------------------------------------------------------------------
# Older revisions of Perl do not support "use warnings".  Can be replaced
# by invoking Perl with -w.
#-------------------------------------------------------------------------------
use warnings;
use strict;

#-------------------------------------------------------------------------------
# Include the XML parser package
#-------------------------------------------------------------------------------
use XML::Simple;
use File::Copy;

#-------------------------------------------------------------------------------
# Paths used
#-------------------------------------------------------------------------------
my $dsplinkDir      = $ENV {'DSPLINK'} ;
my $cgtoolsDir      = "c:\\ti-tools\\c6000\\cgtools" ;

#-------------------------------------------------------------------------------
# Initialize variables for COFF file information
#-------------------------------------------------------------------------------
my $binorg = 0xFFFFFFFF;
my $binlen = 0x0;
my $binend = 0x0;

#-------------------------------------------------------------------------------
# Process the command line
#-------------------------------------------------------------------------------
if (@ARGV != 2) {
    print "  -------------------------------------------------------------------\n" ;
    print "  Usage: binloader.pl <GPP o/p path> <DSP i/p file>\n" ;
    print "       <GPP o/p path>: Path where GPP-side files should be generated.\n" ;
    print "       <DSP i/p file>: Name (with path) of input DSP COFF executable.\n\n" ;
    print "  Example: binloader.pl L:\\dsplink\\gpp\\src\\samples\\message\\PrOS L:\\dsplink\\dsp\\export\\BIN\\DspBios\\Davinci\\RELEASE\\message.out\n" ;
    print "  -------------------------------------------------------------------\n" ;
    exit(0);
}

my $gppOutFilePath  =  $ARGV [0] ;
my $filename        =  $ARGV [1] ;

print "  -------------------------------------------------------------------\n" ;
print "  CGTOOLS path used           : " . $cgtoolsDir . "\n" ;
print "  DSPLINK environment variable: " . $dsplinkDir . "\n" ;
print "  Generating files ...\n\n" ;

#-------------------------------------------------------------------------------
# Get the file names.
#-------------------------------------------------------------------------------
my (@path) = split ('\\\\', $filename) ;
my $fnameext = pop @path ;
my ($fname, $fextn) = split ('\.', $fnameext) ;
my $dspOutFilePath = join ('\\', @path) ;

my $dsplinkGppCFile   = $gppOutFilePath . "\\" . $fname . "_binloader.c" ;
my $dsplinkGppHFile   = $gppOutFilePath . "\\" . $fname . "_binloader.h" ;
my $dsplinkDspDatFile = $dspOutFilePath . "\\" . $fname . ".dat" ;
my $dsplinkDspBinFile = $dspOutFilePath . "\\" . $fname . ".bin" ;
my $dsplinkXmlFile    = $dspOutFilePath . "\\" . $fname . ".xml" ;

#-------------------------------------------------------------------------------
# Run the OFD utility to get the XML output and write to file.
#-------------------------------------------------------------------------------
my $xml      = `$cgtoolsDir\\bin\\ofd6x -x $filename`;
my $outputFileName = ">" . $dsplinkXmlFile ;
open(OUTPUT, $outputFileName) || die "Could not open $outputFileName.\n";
print OUTPUT $xml ;
close(OUTPUT) || die "Could not close $outputFileName: $!";

#-------------------------------------------------------------------------------
# Strip the information between <raw_data> and </raw_data> tags in the xml
# file. This is done to enable faster processing of the xml input.
#-------------------------------------------------------------------------------
open(INPUT, $dsplinkXmlFile) || die "Could not open $dsplinkXmlFile.\n";

# Create a temporary output file which will contain all contents except
# those within <raw_data> tags.
$outputFileName = ">temp";
open(OUTPUT, $outputFileName) || die "Could not open $outputFileName.\n";

my $raw_data_start = "<raw_data>";
my $raw_data_end   = "</raw_data>";
my $raw_data_flag  = 0;
my $line_count     = 0;

while (<INPUT>)
{
    $line_count ++;
    if ( $_ =~ /$raw_data_start/ == 1)
    {
        # Indicate start of raw_data section.
        $raw_data_flag = 1;
        $line_count    = 0;
    }
    if ( $_ =~ /$raw_data_end/ == 1)
    {
        # Indicate end of raw_data section.
        $raw_data_flag = 0;
        if ($line_count != 0)
        {
            # Print start tag only if the tag spans more than one line.
            print OUTPUT $raw_data_start . " ";
        }
        $line_count    = 0;
    }
    if ($raw_data_flag == 0)
    {
        print OUTPUT $_;
    }
}
close(INPUT) || die "Could not close $filename: $!";
close(OUTPUT) || die "Could not close $outputFileName: $!";

# Overwrite original file with temporary output file.
move("temp", $dsplinkXmlFile) || die "Could not move $outputFileName: $!";

#-------------------------------------------------------------------------------
# Parse the XML output
# XMLin API comes from XML::Simple
#-------------------------------------------------------------------------------
my $config   = XMLin($dsplinkXmlFile);

#-------------------------------------------------------------------------------
# Pointer to all 'section' entities
#-------------------------------------------------------------------------------
my $sections = $config->{'object_file'}->{'ti_coff'}->{'section'};
my $symbols  = $config->{'object_file'}->{'ti_coff'}->{'symbol_table'}->{'symbol'};
my($buff, $sectname);

#-------------------------------------------------------------------------------
# Declaration of the symbol for DSPLINK SHM base.
#-------------------------------------------------------------------------------
my $dsplinkShmBaseName  = "_DSPLINK_shmBaseAddress" ;
my $dsplinkShmBaseValue = 0 ;

#-------------------------------------------------------------------------------
# Open output files for writing
#-------------------------------------------------------------------------------
open (OUTC,   ">$dsplinkGppCFile");
open (OUTH,   ">$dsplinkGppHFile");
open (OUTDAT, ">$dsplinkDspDatFile");

#-------------------------------------------------------------------------------
# Get entry point.
#-------------------------------------------------------------------------------
my $entryPt = hex($config->{'object_file'}->{'ti_coff'}->{'optional_file_header'}->{'entry_point'});

#-------------------------------------------------------------------------------
# Get the symbol value for the DSPLINK SHM base.
#-------------------------------------------------------------------------------
$dsplinkShmBaseValue = get_symbol_value($dsplinkShmBaseName);

#-------------------------------------------------------------------------------
# Open input COFF file (.out) for reading
#-------------------------------------------------------------------------------
open (OBJFILE, "<$filename") || die "Could not open input file.\n";
binmode(OBJFILE);

#-------------------------------------------------------------------------------
# For each 'section' entity
#-------------------------------------------------------------------------------
foreach $sectname (keys %$sections)
{
    #---------------------------------------------------------------------------
    # Find section's name, length and address
    #---------------------------------------------------------------------------
    my $sect  = $sections->{$sectname};
    my $size  = hex($sect->{'raw_data_size'});
    my $paddr = hex($sect->{'physical_addr'});

    #---------------------------------------------------------------------------
    # Find pointer to section's raw data inside the COFF file
    #---------------------------------------------------------------------------
    my $ptr   = hex($sect->{'file_offsets'}->{'raw_data_ptr'});

    #---------------------------------------------------------------------------
    # if section is non-zero length and there is data in the file ...
    #---------------------------------------------------------------------------
    if ($size != 0 && $ptr != 0    &&
    #---------------------------------------------------------------------------
    # and it is an initialized section ...
    #---------------------------------------------------------------------------
       (not defined $sect->{'bss'})   &&
    #---------------------------------------------------------------------------
    # and it is not a copy section ...
    # (copy sections are information of some kind that is not to be
    # loaded to the target.  Best example is .cinit when linked under
    # -cr.  Dwarf debug sections are also copy sections.)
    #---------------------------------------------------------------------------
      (not defined $sect->{'copy'})  &&
    #---------------------------------------------------------------------------
    # and not a dummy section ...
    # (dummy is a legacy section type typically not used)
    #---------------------------------------------------------------------------
      (not defined $sect->{'dummy'}) &&
    #---------------------------------------------------------------------------
    # and not a noload section.
    # (noload sections come up when you are doing particularly complicated
    # partial linking tricks.  As the name implies, the section is not to
    # loaded.)
    #---------------------------------------------------------------------------
      (not defined $sect->{'noload'}))
    {
        #-----------------------------------------------------------------------
        # only initialized sections will pass through here.
        # If its base address is lower than current lowest, set $binorg to the
        # new low.
        # If the section's base + len is higher than current highest, set
        # $binend to the new high.
        #-----------------------------------------------------------------------
        if ($paddr < $binorg) {
            $binorg = $paddr ;
        }
        if (($paddr + $size) > $binend) {
            $binend = $paddr + $size ;
        }
    }
}


#-------------------------------------------------------------------------------
# Close object file
#-------------------------------------------------------------------------------
close OBJFILE;

#-------------------------------------------------------------------------------
# Calculate the length of the generated binary file
#-------------------------------------------------------------------------------
$binlen = $binend - $binorg ;

#-------------------------------------------------------------------------------
# Write 'C' file with information required for binary loader
#-------------------------------------------------------------------------------
print OUTC "/** ============================================================================\n";
print OUTC " *  \@name   " . uc($fname) . "_DspBinLoaderInfo\n";
print OUTC " *\n";
print OUTC " *  \@desc   Array containing information of the DSP executable within the\n";
print OUTC " *          generated source file as required by the Binary Loader.\n";
print OUTC " *          The variable below corresponds to type BINLOADER_ImageInfo.\n";
print OUTC " *  ============================================================================\n";
print OUTC " */\n";
print OUTC "const unsigned long " . uc($fname) . "_DspBinLoaderInfo [5] = {\n";
print  OUTC "    0x0,\n" ;
printf OUTC "    0x%x,\n", $binorg;
printf OUTC "    0x%x,\n", $entryPt;
printf OUTC "    0x%x,\n", $binlen;
printf OUTC "    0x%x\n",  $dsplinkShmBaseValue;
print OUTC "} ;\n";
close OUTC;

#-------------------------------------------------------------------------------
# Write 'H' file with information required for binary loader
#-------------------------------------------------------------------------------
print OUTH "/** ============================================================================\n";
print OUTH " *  \@name   " . uc($fname) . "_DspBinLoaderInfo\n";
print OUTH " *\n";
print OUTH " *  \@desc   Array containing information of the DSP executable within the\n";
print OUTH " *          generated source file as required by the Binary Loader.\n";
print OUTH " *          The variable below corresponds to type BINLOADER_ImageInfo.\n";
print OUTH " *  ============================================================================\n";
print OUTH " */\n";
print OUTH "extern const unsigned long " . uc($fname) . "_DspBinLoaderInfo [5] ;\n";
close OUTH;

#-------------------------------------------------------------------------------
# Write .dat file for COFF to BIN conversion
#-------------------------------------------------------------------------------
print OUTDAT $filename . " -o " . $dsplinkDspBinFile . " -i -memwidth 8 -romwidth 8 -image -b\n\n" ;
print OUTDAT "ROMS\n{\n";
printf OUTDAT "    RAM: origin=0x%x, length=0x%x, romwidth=8, memwidth=8\n", $binorg, $binlen;
print OUTDAT "}";
close OUTDAT;

#-------------------------------------------------------------------------------
# Convert COFF file to binary file
#-------------------------------------------------------------------------------
my $hex6x    = `$cgtoolsDir\\bin\\hex6x $dsplinkDspDatFile`;

#-------------------------------------------------------------------------------
# Print where the files are generated
#-------------------------------------------------------------------------------
print "  Generated files:\n" ;
print "  1. " . $dsplinkGppCFile   . "\n" ;
print "  2. " . $dsplinkGppHFile   . "\n" ;
print "  3. " . $dsplinkDspBinFile . "\n" ;
print "  4. " . $dsplinkDspDatFile . "\n" ;
print "  -------------------------------------------------------------------\n" ;
exit(0);

################################################################################
# GET_SYMBOL_VALUE - Get value of a symbol.
################################################################################
sub get_symbol_value
{
    my($getsymname) = @_;
    my $symname;
    my $getvalue = 0;

    #---------------------------------------------------------------------------
    # For each 'symbol' entity
    #---------------------------------------------------------------------------
    foreach $symname (keys %$symbols)
    {
        #-----------------------------------------------------------------------
        # Check if the symbol name matches the required name.
        #-----------------------------------------------------------------------
        if ($symname eq $getsymname)
        {
            #-------------------------------------------------------------------
            # Find section's name, length and address
            #-------------------------------------------------------------------
            my $sym   = $symbols->{$symname};
            $getvalue = hex($sym->{'value'});
        }
    }

    return $getvalue ;
}
