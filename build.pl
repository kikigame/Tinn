#!/usr/bin/perl

# this is just a handy helper to make the Makefile

# This was my original Makefile:
#
#compile: 
#	c++ -g src/*.cpp -o tinn -Wall -std=c++11 -lcurses

my %deps;


# Return all dependencies for a file, ie lines starting #include and not system includes (<>)
sub getDeps {
    my $file = shift @_;
    chomp $file;
    return @{$deps{$file}} if exists $deps{$file};
    my @rtn;
    for my $inc (`grep '^#include' $file`) {
        next if $inc =~ /[\<\>]/;
	$inc =~ /.*"(.*)"/; $inc = "src/$1";
	push @rtn, $inc;
    }
    for (1..10) {
	my @r;
	for my $f (@rtn) {
	    push @r, getDeps($f);
	}
	%rtn = map { $_ => 1 } @rtn, @r;
	@rtn = keys %rtn;
   }
    $deps{$file} = \@rtn;
    @rtn;
}




my @ofiles, @cfiles;

for my $cfile (`ls src/*.cpp`) {
    chomp $cfile;
    push @cfiles,$cfile;
    my $ofile = $cfile; $ofile =~ s/.cpp$/.o/g;
    push @ofiles,$ofile;
}

# Output

my $CXXFLAGS = "-Wall -g -std=c++11";

print "# This is an auto-generated file. Please make changes in build.pl\n\n";
print join(" ", "tinn", ":", "Makefile", @ofiles, "\n");
print join(" ", "\tc++ -g  ", @ofiles, " $CXXFLAGS -lncursesw -o tinn\n\n");

print "Makefile: build.pl\n";
print "\t./build.pl > Makefile\n\n";

print "clean:\n";
print join(" ", "\trm -f  ", @ofiles, "\n\n");

for my $cfile (@cfiles) {
    my $ofile = shift @ofiles;
    my @deps = getDeps($cfile);

    print join(" ", "$ofile",":", $cfile, @deps, "\n");
#    print "\tc++ -O2 $cfile -c $CXXFLAGS -o $ofile\n\n"
    print "\tc++ -g $cfile -c $CXXFLAGS -o $ofile -finput-charset=utf8 -fexec-charset=utf8\n\n"
#    print "\tc++ -g $cfile -c $CXXFLAGS -o $ofile -DDEBUG=1\n\n"
}

