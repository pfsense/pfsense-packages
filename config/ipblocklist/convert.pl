#!/usr/bin/perl -w
use strict;

if($#ARGV != 1) {
    print("Usage: $0 <input file> <output file>\n");
    exit(1);
}

my ($line,$title,$iprange,$cidr);
my $i = 30000;

open(INFILE,'<',$ARGV[0]) or die("Could not open input file $ARGV[0]");
open(OUTFILE,'>>',$ARGV[1]) or die("Could not open output file $ARGV[1]");

foreach $line (<INFILE>) {
    chomp($line);
    $line =~ s/:((\d{1,3}[-\.]*){8})//;
    $iprange = $1;
    print OUTFILE "#$line\n";
    foreach $cidr (split(/\n/,range($iprange))) {
        print OUTFILE "$cidr\n";     
        #print OUTFILE "ipfw -q add 1000 drop ip from any to $cidr\n";   (version 0.1.4)
        #$i++;
        #print OUTFILE "ipfw -q add 1001 drop ip from $cidr to any\n";   (version 0.1.4) 
        #$i++;
    }
}

close(INFILE);
close(OUTFILE);

sub ntoa {
    return join ".",unpack("CCCC",pack("N",shift));
}
sub aton {
    return unpack 'N', pack 'C4', split/\./, shift;
}
sub deaggregate {
    my $thirtytwobits = 4294967295;
    my $start = shift;
    my $end = shift;
    my $base = $start;
    my ($step,$output);
    while ($base <= $end) {
        $step = 0;
        while (($base | (1 << $step)) != $base) {
            if (($base | (((~0) & $thirtytwobits) >> (31-$step))) > $end) {
                last;
            }
            $step++;
        }
        if($step == 0) {
            $output .= ntoa($base);
        }else{
            $output .= ntoa($base)."/" .(32-$step);
        }
        $output .= "\n";
        $base += 1 << $step;
    }
    return $output;
}
sub range {
    my ($address,$address2) = split /-/, shift;
    $address = aton($address);
    $address2 = aton($address2);
    return deaggregate($address,$address2);
}