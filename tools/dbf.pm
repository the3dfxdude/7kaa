#
# Seven Kingdoms: Ancient Adversaries
#
# Copyright 1997,1998 Enlight Software Ltd.
# Copyright 2017 Jesse Allen
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

package dbf;

use warnings;
use strict;

sub trim {
	$_[0] =~ s/^\s+//;
	$_[0] =~ s/\s+$//;
	return $_[0];
}

sub read_file {
	my $class;
	my $dbf_file;
	my $fh;
	my $buf;
	my %header;
	my $end;
	my @fields;
	my @records;
	my $rec_packing;
	my $success;

	($class, $dbf_file) = @_;

	$success = 0;
	if (!open($fh, '<', $dbf_file)) {
		#print "Error: Cannot open $dbf_file\n";
		goto OUT;
	}
	if (read($fh, $buf, 32) != 32) {
		#print "Error: Corrupt file\n";
		goto OUT;
	}
	($header{dbf_id},
		$header{year}, 
		$header{month}, 
		$header{day}, 
		$header{records},
		$header{hsize},
		$header{rsize},
	) = unpack('ccccLSS', $buf);
	$rec_packing = 'C'; # record status indicator, included in record's array
	while (!eof($fh)) {
		my ($name, $type, $reserved0, $len);
		if (read($fh, $buf, 32) != 32) {
			#print "Error: Corrupt file\n";
			goto OUT;
		}
		($end) = unpack('C', $buf);
		if ($end == hex('0D')) {
			seek($fh, $header{hsize}, 0);
			last;
		}
		($name, $type, $reserved0, $len) = unpack('Z11aLC', $buf);
		if ($type eq 'C' || $type eq 'N') {
			# even though field type 'C' could be extracted with
			# pack indicator of 'A', this would mess up the handling
			# of the PTR type fields
			$rec_packing .= "a$len";
		} else {
			print "Error: Unhandled record type '$type'\n";
			goto OUT;
		}
		push(@fields, [$name, $type, $reserved0, $len]);
	}
	while (!eof($fh)) {
		my @record;
		my $bytes = read($fh, $buf, $header{rsize});
		($end) = unpack('C', $buf);
		if ($end == hex('1A')) {
			last;
		}
		if ($bytes != $header{rsize}) {
			#print "Error: Corrupt file\n";
			goto OUT;
		}
		@record = unpack($rec_packing, $buf);
		push(@records, \@record);
	}
	$success = 1;
OUT:
	close($fh);
	if ($success) {
		my $dbf = {
			header => \%header,
			fields => \@fields,
			records => \@records,
		};
		bless($dbf, $class);
		return $dbf;
	}
	return undef;
}

sub write_file {
	my $dbf;
	my $dbf_file;
	my $fh;
	my $buf;
	my $packing;
	($dbf, $dbf_file) = @_;
	if (!open($fh, '>', $dbf_file)) {
		print "Error: Cannot write $dbf_file\n";
		close($fh);
		return;
	}
	print $fh pack('ccccLSSx20',
		$dbf->{header}{dbf_id},
		$dbf->{header}{year}, 
		$dbf->{header}{month}, 
		$dbf->{header}{day}, 
		$dbf->{header}{records},
		$dbf->{header}{hsize},
		$dbf->{header}{rsize},
	);
	$packing = 'C'; # record status indicator
	for (my $i = 0; $i < @{$dbf->{fields}}; $i++) {
		my $field;
		$field = $dbf->{fields}[$i];
		print $fh pack('Z11aLCx15', @$field);
		if ($field->[1] eq 'C' || $field->[1] eq 'N') {
			$packing .= "a$field->[3]";
		}
	}
	print $fh pack('C', hex('0D'));
	for (my $i = 0; $i < @{$dbf->{records}}; $i++) {
		my $record;
		$record = $dbf->{records}[$i];
		print $fh pack($packing, @$record);
	}
	print $fh pack('C', hex('1A'));
	close($fh);
	return;
}

# return the index of $field_name
sub get_field {
	my $dbf;
	my $field_name;
	($dbf, $field_name) = @_;

	for (my $i = 0; $i < @{$dbf->{fields}}; $i++) {
		my $field;
		my $name;
		$field = $dbf->{fields}[$i];
		$name = $field->[0];
print "$name eq $field_name\n";
		if ($name eq $field_name) {
			return $i;
		}
	}
	return -1;
}

# returns the field record length
sub get_field_len {
	my $dbf;
	my $field;
	($dbf, $field) = @_;
	if (!defined($dbf->{fields}[$field])) {
		return 0;
	}
	return $dbf->{fields}[$field][3];
}

# returns the number of fields in a record
sub get_field_names {
	my $dbf;
	my @names;
	($dbf) = @_;
	@names = map {$_->[0]} @{$dbf->{fields}};
	return \@names;
}

# return array representing the values in the record
sub get_record {
	my $dbf;
	my $index;
	my $record;
	my @values;
	($dbf, $index) = @_;
	if (!defined($dbf->{records}[$index])) {
		return undef;
	}
	# check for deleted record, just in case.
	if ($dbf->{records}[$index][0] == hex('2A')) {
		return undef;
	}
	$record = $dbf->{records}[$index];
	@values = @{$record}[1..$#$record];
	return \@values;
}

# return the number of records
sub get_records {
	my $dbf;
	($dbf) = @_;
	return $dbf->{header}{records};
}

# return the value of a record field
# $record and $field are indexes
# values are generally treated as strings, without whitespace trimming
sub get_value {
	my $dbf;
	my $record;
	my $field;
	my $record_field_idx;
	my $value;
	($dbf, $record, $field) = @_;
	$record_field_idx = $field+1; # adjusted due to record status indicator
	if (!defined($dbf->{records}[$record])) {
		return undef;
	}
	# check for deleted record, just in case.
	if ($dbf->{records}[$record][0] == hex('2A')) {
		return undef;
	}
	if (!defined($dbf->{records}[$record][$record_field_idx])) {
		return undef;
	}
	$value = $dbf->{records}[$record][$record_field_idx];
	return $value;
}

# sets the value of a record field
# the user must properly format $value to the expected convention
sub set_value {
	my $dbf;
	my $record;
	my $field;
	my $value;
	my $record_field_idx;
	($dbf, $record, $field, $value) = @_;
	$record_field_idx = $field+1; # adjusted due to record status indicator
	if (!defined($dbf->{records}[$record])) {
		return 0;
	}
	if (!defined($dbf->{records}[$record][$record_field_idx])) {
		return 0;
	}
	$dbf->{records}[$record][$record_field_idx] = $value;
	return 1;
}

1;
