#!/usr/bin/perl -w

#usage: rename perl_expression [files]
my $usage = qq{rename [-v] s/pat/repl/ [filenames...]\t (c)2001 hellweg\@snark.de
rename files read from the commandline or stdin

License to use, modify and redistribute granted to each and every lifeform on
this planet (as long as credit to hellweg\@snark.de remains). No guarantee that
'rename' does or does not perform the way you want...

} ;
$verbose = 0 ;
$quiet = 0 ;

$op=shift || 0 ;
if($op eq "-v") {
    $verbose++ ; $quiet = 0 ;
    $op=shift || 0 ;
}
if($op eq "-q") {
    $quiet++ ; $verbose = 0 ;
    $op=shift || 0 ;
}
if($op =~ /^-h/) {
    print $usage; exit(0) ;
}

if(! $op) {
    print $usage; exit(-1) ;
}

if (!@ARGV) {
    @ARGV = <STDIN>;
}

$count=0 ;
my($m, $d, $y, $T) ;
for (@ARGV) {
    chomp ;
    if(-e $_) {
	$was = $_;
	if($op =~ /\$[Tdym]/) {
	    my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)=localtime((stat($_))[9]);
	    $m = sprintf("%0.2i", $mon+1);
	    $d = sprintf("%0.2i", $mday);
	    $y = $year + 1900 ;
	    $T = "$y$m$d" ;
	}
	eval $op;
        die $@ if $@;
	if(-f $_) { print("! exists already: $was -> $_ \n") unless $quiet ; }
	else { 
	    if(rename($was, $_)) { 
		print("$was -> $_\n") if $verbose ; 
		$count++; 
	    } else {
		if(/\//) {
		    # maybe we need to create dirs?
		    my $createRes = createDirs($_) ;
		    if($createRes) {
			print("! fauled to create $createRes for $_\n") 
			    unless $quiet ;
		    }
		    else { # try again
			if(rename($was, $_)) { 
			    print("$was -> $_\n") if $verbose ; 
			    $count++; 
			} else {
			    print("! failed to rename $was -> $_ \n") 
				unless $quiet ;
			}
		    }
		}
		else {
		    print("! failed to rename $was -> $_ \n") unless $quiet ;
		}
	    }
	}
    }
    else { print("! not found: $_ \n") ; }
}
print("renamed $count files\n") if $verbose ;


sub createDirs { # return the dir we failed to create or 0
    my $file = shift ;
    my @dirs = split /\//, $file ;
    pop @dirs ; # don't try to mkdir the file itself
    my $current = "" ;
    $current = "/" if ($file =~ /^\//) ;
    foreach (@dirs) {
	$current .= $_ ;
	if(! -d $current) {
	    mkdir $current, 0700 || return $current ;
	    print "mkdir $current\n" if ($verbose) ;
	}
	$current .= "/" ;
    }
    return 0 ; # success
}
