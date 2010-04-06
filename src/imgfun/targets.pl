my @obj_files;

if ($no_asm) {

  ## compiler flags ##
  @defines = qw( AMPLUS );
  if (defined($debug) && $debug) {
    push (@defines, "DEBUG");
  }
  if (defined($no_asm) && $no_asm) {
    push (@defines, "NO_ASM");
  }
  ## end compiler flags ##

  ## include paths ##
  @includes = qw( ../../include );

  if (defined($wine_prefix)) {
    push (@includes, "$wine_prefix/include/wine/windows",
                     "$wine_prefix/include/wine/msvcrt");
  }
  ## include paths ##


  ### Compiler targets ###

  my @targets;

  if ($no_asm) {
    push (@targets, qw(
      CRC.cpp
      IB.cpp
      IB2.cpp
      IB_32.cpp
      IB_A.cpp
      IB_AR.cpp
      IB_AT.cpp
      IB_ATD.cpp
      IB_ATDM.cpp
      IB_ATR.cpp
      IB_ATRD.cpp
      IB_ATRDM.cpp
      IB_DW.cpp
      IB_R.cpp
      IB_T.cpp
      IB_TD.cpp
      IB_TDM.cpp
      IB_TR.cpp
      IB_TRD.cpp
      IB_TRDM.cpp
      IC.cpp
      IC_R.cpp
      IJ_T.cpp
      IR.cpp
      IR_A.cpp
      IR_AM.cpp
      IR_BAR.cpp
      IR_M.cpp
      I_BAR.cpp
      I_BLACK.cpp
      I_EMASK.cpp
      I_EREMAP.cpp
      I_FREMAP.cpp
      I_LINE.cpp
      I_PIXEL.cpp
      I_READ.cpp
      I_SNOW.cpp
    ));
  }

  push (@obj_files, build_targets(\@targets, \@includes, \@defines));

} else {

  ## build asm ##
  unless ($no_asm) {
    push (@obj_files, include_targets('asm/targets.pl'));
  }
  ## end build asm ##

}

@obj_files;
