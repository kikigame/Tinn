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
	@rtn = sort keys %rtn;
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
print "CXX ?= c++\n\n"; 
print "WINCXX ?= /usr/bin/x86_64-w64-mingw32-c++ -I../mingw-64/ncurses/include -unicode -L../mingw-64/ncurses/lib/ -Wl,--enable-auto-import\n\n";
print "CXXLINK ?= -lncursesw\n\n";
print "WINCXXLINK = -lncursesw -lpsapi -static\n\n";


print join(" ", "tinn", ":", "Makefile", "ofiles", "\n");
print join(" ", "ofiles", ":", @ofiles, "\n\n");
print join(" ", "\t\$(CXX) ", @ofiles, " $CXXFLAGS \$(CXXLINK) -o tinn\n\n");

print "# Windown port \n";
print join(" ", "tinn.exe", ":", "Makefile", "clean", "\n");
print "\tCXX=\"\$(WINCXX)\" make -k ofiles && \\\n";
print join(" ", "\t\$(WINCXX) ", @ofiles, " $CXXFLAGS \$(CXXLINK) -o tinn.exe\n\n");

print "Makefile: build.pl\n";
print "\t./build.pl > Makefile\n\n";

print "lint:\n";
print "\tcppcheck --enable=performance --enable=warning --enable=portability src\n\n";

print "clean:\n";
print join(" ", "\trm -f  ", @ofiles, "\n\n");

for my $cfile (@cfiles) {
    my $ofile = shift @ofiles;
    my @deps = getDeps($cfile);

    print join(" ", "$ofile",":", $cfile, @deps, "\n");
#    print "\t\$(CXX) -O2 $cfile -c $CXXFLAGS -o $ofile\n\n"
    print "\t\$(CXX) $cfile -c $CXXFLAGS -o $ofile -finput-charset=utf8 -fexec-charset=utf8\n\n"
#    print "\t\$(CXX) $cfile -c $CXXFLAGS -o $ofile -DDEBUG=1\n\n"
}

