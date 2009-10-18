#!/usr/bin/perl

use File::stat;
require "files.pl";


foreach (@asm_files) {
  my $flag = 1;

  # check modification time
  if (-e "$_.o") {
    my $objstat = stat("$_.o");
    my $cppstat = stat("$_.asm");
    $cppstat->mtime >= $objstat->mtime or $flag = 0;
  }

  if ($flag) {
    my $cmd = "jwasm -q -elf -zt1 $_.asm";
    print "$cmd\n";
    system $cmd and die "jwasm failed";
  }
}
