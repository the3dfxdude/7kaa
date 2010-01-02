#!/usr/bin/perl

use File::stat;
require "files.pl";
require "opts.pl";

chdir "asm" or die "Missing asm dir";
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
chdir "..";

foreach (@c_files) {
  my $flag = 1;

  # check modification time
  if (-e "$_.o") {
    my $objstat = stat("$_.o");
    my $cppstat = stat("$_.cpp");
    $cppstat->mtime >= $objstat->mtime or $flag = 0;
  }

  if ($flag) {
    my $optlevel = $_ eq "OWORLD" ? "-O1" : "-O2";
    my $cmd = "g++ $optlevel -g -c $defines $includes $_.cpp";
    print "$cmd\n";
    system $cmd and die "g++ failed";
  }
}

@obj_files = map { "$_.o" } @c_files;
push ( @obj_files, map { "asm/$_.o" } @asm_files );

my $exe = "AM.exe";
my $flag = 0;
foreach (@obj_files) {
  (-e $_) or die "Missing file '$_' for linking";

  if (-e $exe) {
    my $exestat = stat($exe);
    my $objstat = stat($_);
    $objstat->mtime >= $exestat->mtime and $flag = 1;
  } else {
    $flag = 1;
  }
}

if ($flag) {
  my @libs = qw(
    gdi32 ddraw msvcrt ole32 dinput dplay dplayx dsound winmm
  );

  @libs = map { "-l$_" } @libs;

  my $cmd = "wineg++ -g @obj_files @libs -o AM.exe";
  print $cmd,"\n";
  system $cmd and die "wineg++ failed\n";

  #system "mv AM.exe* ~/local/sevenkingdoms" and die "oops";
}

print "\nBuild Complete\n";
