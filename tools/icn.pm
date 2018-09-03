#
# Seven Kingdoms: Ancient Adversaries
#
# Copyright 1997,1998 Enlight Software Ltd.
# Copyright 2018 Jesse Allen
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#

package icn;

use warnings;
use strict;
use Carp;

my $TRANSPARENT_CODE = 255;

sub height {
	return $_[0]->{height};
}

# read new icn from read file handle, return undef on error
sub read_file {
	my ($class, $fh) = @_;
	my $icn = {};
	my $buf;
	read($fh, $buf, 4);
	($icn->{width}, $icn->{height}) = unpack('ss', $buf);
	my $bmp_size = $icn->{height}*$icn->{width};
	read($fh, $buf, $bmp_size);
	$icn->{pixels} = [unpack('C*', $buf)];
	return bless($icn, $class);
}

# print an ascii representation to file handle
sub print_ascii {
	my ($icn, $fh) = @_;
	my $bmp_size = $icn->{width} * $icn->{height};
	print "="x($icn->{width}*3) . "\n";
	for (my $i = 0; $i < $bmp_size; $i++) {
		if ($icn->{pixels}[$i] == $TRANSPARENT_CODE) {
			print ".. ";
		} else {
			printf("%02x ", $icn->{pixels}[$i]);
#printf("H:%02x\n", $icn->{pixels}[$i]);
		}
		if (($i+1) % $icn->{width} == 0) {
			print "\n";
		}
	}
	print "="x($icn->{width}*3) . "\n";
}

sub width {
	return $_[0]->{width};
}

# write icn to write file handle
sub write_file {
	my ($icn, $fh) = @_;
	my $buf;
	$buf = pack('ss', $icn->{width}, $icn->{height});
	print $fh $buf;
	$buf = pack('C*', @{$icn->{pixels}});
	print $fh $buf;
}

1;
