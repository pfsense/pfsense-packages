#!/usr/bin/perl -w

# $Id: addsid.pl,v 1.30 2005/12/31 13:42:46 andreas_o Exp $ #

# Copyright (c) 2004-2006 Andreas Östling <andreaso@it.su.se>
# All rights reserved.
#
#  Redistribution and use in source and binary forms, with or
#  without modification, are permitted provided that the following
#  conditions are met:
#
#  1. Redistributions of source code must retain the above
#     copyright notice, this list of conditions and the following
#     disclaimer.
#
#  2. Redistributions in binary form must reproduce the above
#     copyright notice, this list of conditions and the following
#     disclaimer in the documentation and/or other materials
#     provided with the distribution.
#
#  3. Neither the name of the author nor the names of its
#     contributors may be used to endorse or promote products
#     derived from this software without specific prior written
#     permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
# CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


use strict;


sub get_next_entry($ $ $ $ $ $);
sub parse_singleline_rule($ $ $);
sub get_next_available_sid(@);


# Set this to the default classtype you want to add, if missing.
# Set to 0 or "" if you don't want to add a classtype.
my $CLASSTYPE = "misc-attack";

# If ADD_REV is set to 1, "rev: 1;" will be added to rule if it has no rev.
# Set to 0 if you don't want to add it.
my $ADD_REV = 1;

# Minimum SID to add. Normally, the next available SID will be used,
# unless it's below this value. Only SIDs >= 1000000 are reserved for
# personal use.
my $MIN_SID = 1000001;

# Regexp to match the start of a multi-line rule.
# %ACTIONS% will be replaced with content of $config{actions} later.
my $MULTILINE_RULE_REGEXP  = '^\s*#*\s*(?:%ACTIONS%)'.
                             '\s.*\\\\\s*\n$'; # ';

# Regexp to match a single-line rule.
my $SINGLELINE_RULE_REGEXP = '^\s*#*\s*(?:%ACTIONS%)'.
                             '\s.+;\s*\)\s*$'; # ';


my $USAGE = << "RTFM";

Parse *.rules in one or more directories and add "sid:<sid>;" to 
active rules that don't have any "sid" entry, starting with the next 
available SID after parsing all rules files (but $MIN_SID at minumum).
Also, "rev:1;" is added to rules without a "rev" entry, and 
"classtype:misc-attack;" is added to rules without a "classtype" entry
(edit options at the top of $0 if you want to change this).

Usage: $0 <rulesdir> [rulesdir2, ...]

RTFM


# Start in verbose mode.
my $verbose = 1;

my (%all_sids, %active_sids, %config);

my @rulesdirs = @ARGV;

die($USAGE) unless ($#rulesdirs > -1);

$config{rule_actions} = "alert|drop|log|pass|reject|sdrop|activate|dynamic";

$SINGLELINE_RULE_REGEXP =~ s/%ACTIONS%/$config{rule_actions}/;
$MULTILINE_RULE_REGEXP  =~ s/%ACTIONS%/$config{rule_actions}/;


# Find out the next available SID.
my $next_sid = get_next_available_sid(@rulesdirs);

# Avoid seeing possible warnings about broken rules twice.
$verbose = 0;

# Add sid/rev/classtype to active rules that don't have any.
foreach my $dir (@rulesdirs) {
    opendir(RULESDIR, "$dir") or die("could not open \"$dir\": $!\n");

    while (my $file = readdir(RULESDIR)) {
        next unless ($file =~ /\.rules$/);

        open(OLDFILE, "$dir/$file")
          or die("could not open \"$dir/$file\": $!\n");
        my @file = <OLDFILE>;
        close(OLDFILE);

        open(NEWFILE, ">", "$dir/$file")
          or die("could not open \"$dir/$file\" for writing: $!\n");

        my ($single, $multi, $nonrule, $msg, $sid);
        while (get_next_entry(\@file, \$single, \$multi, \$nonrule, \$msg, \$sid)) {

            if (defined($nonrule)) {
   	        print NEWFILE "$nonrule";
	        next;
            }

            $multi = $single unless (defined($multi));

          # Don't care about inactive rules.
            if ($single =~ /^\s*#/) {
	        print NEWFILE "$multi";
	        next;
            }

            my $added;

          # Add SID.
            if ($single !~ /sid\s*:\s*\d+\s*;/) {
                $added .= "SID $next_sid,";
                $multi =~ s/\)\s*\n/sid:$next_sid;)\n/;
                $next_sid++;
            }

          # Add revision.
            if ($ADD_REV && $single !~ /rev\s*:\s*\d+\s*;/) {
                $added .= "rev,";
                $multi =~ s/\)\s*\n/rev:1;)\n/;
            }

          # Add classtype.
            if ($CLASSTYPE && $single !~ /classtype\s*:\s*.+\s*;/) {
                $added .= "classtype $CLASSTYPE,";
                $multi =~ s/\)\s*\n/classtype:$CLASSTYPE;)\n/;
            }

            if (defined($added)) {
                $added =~ s/,$//;
                print "Adding $added to rule \"$msg\"\n"
                  if (defined($added));
            }

            print NEWFILE "$multi";
        }

        close(NEWFILE);
    }

    closedir(RULESDIR);
}



# Read in *.rules in given directory and return highest SID.
sub get_next_available_sid(@)
{
    my @dirs = @_;

    foreach my $dir (@dirs) {
        opendir(RULESDIR, "$dir") or die("could not open \"$dir\": $!\n");

      # Only care about *.rules.
        while (my $file = readdir(RULESDIR)) {
            next unless ($file =~ /\.rules$/);

            open(OLDFILE, "<$dir/$file") or die("could not open \"$dir/$file\": $!\n");
            my @file = <OLDFILE>;
            close(OLDFILE);

            my ($single, $multi, $nonrule, $msg, $sid);

            while (get_next_entry(\@file, \$single, \$multi, \$nonrule, \$msg, \$sid)) {
                if (defined($single) && defined($sid)) {
   	            $all_sids{$sid}++;

                  # If this is an active rule add to %active_sids and
                  # warn if it already exists.
                    if ($single =~ /^\s*alert/) {
                        print STDERR "WARNING: duplicate SID: $sid\n"
    	                  if (exists($active_sids{$sid}));
                        $active_sids{$sid}++
                    }
                }
            }
        }
    }

  # Sort sids and use highest one + 1, unless it's below MIN_SID.
    @_ = sort {$a <=> $b} keys(%all_sids);
    my $sid = pop(@_);

    if (!defined($sid)) {
        $sid = $MIN_SID
    } else {
        $sid++;
    }

  # If it's below MIN_SID, use MIN_SID instead.
    $sid = $MIN_SID if ($sid < $MIN_SID);

    return ($sid)
}



sub get_next_entry($ $ $ $ $ $)
{
    my $arr_ref     = shift;
    my $single_ref  = shift;
    my $multi_ref   = shift;
    my $nonrule_ref = shift;
    my $msg_ref     = shift;
    my $sid_ref     = shift;

    undef($$single_ref);
    undef($$multi_ref);
    undef($$nonrule_ref);
    undef($$msg_ref);
    undef($$sid_ref);

    my $line = shift(@$arr_ref) || return(0);
    my $disabled = 0;
    my $broken   = 0;

  # Possible beginning of multi-line rule?
    if ($line =~ /$MULTILINE_RULE_REGEXP/oi) {
        $$single_ref = $line;
        $$multi_ref  = $line;

        $disabled = 1 if ($line =~ /^\s*#/);

      # Keep on reading as long as line ends with "\".
        while (!$broken && $line =~ /\\\s*\n$/) {

          # Remove trailing "\" and newline for single-line version.
            $$single_ref =~ s/\\\s*\n//;

          # If there are no more lines, this can not be a valid multi-line rule.
            if (!($line = shift(@$arr_ref))) {

                warn("\nWARNING: got EOF while parsing multi-line rule: $$multi_ref\n")
                  if ($config{verbose});

                @_ = split(/\n/, $$multi_ref);

                undef($$multi_ref);
                undef($$single_ref);

              # First line of broken multi-line rule will be returned as a non-rule line.
                $$nonrule_ref = shift(@_) . "\n";
                $$nonrule_ref =~ s/\s*\n$/\n/;    # remove trailing whitespaces

              # The rest is put back to the array again.
                foreach $_ (reverse((@_))) {
                    unshift(@$arr_ref, "$_\n");
                }

                return (1);   # return non-rule
            }

          # Multi-line continuation.
            $$multi_ref .= $line;

          # If there are non-comment lines in the middle of a disabled rule,
          # mark the rule as broken to return as non-rule lines.
            if ($line !~ /^\s*#/ && $disabled) {
                $broken = 1;
            } elsif ($line =~ /^\s*#/ && !$disabled) {
                # comment line (with trailing slash) in the middle of an active rule - ignore it
            } else {
                $line =~ s/^\s*#*\s*//;  # remove leading # in single-line version
                $$single_ref .= $line;
            }

        } # while line ends with "\"

      # Single-line version should now be a valid rule.
      # If not, it wasn't a valid multi-line rule after all.
        if (!$broken && parse_singleline_rule($$single_ref, $msg_ref, $sid_ref)) {

            $$single_ref =~ s/^\s*//;     # remove leading whitespaces
            $$single_ref =~ s/^#+\s*/#/;  # remove whitespaces next to leading #
            $$single_ref =~ s/\s*\n$/\n/; # remove trailing whitespaces

            $$multi_ref  =~ s/^\s*//;
            $$multi_ref  =~ s/\s*\n$/\n/;
            $$multi_ref  =~ s/^#+\s*/#/;

            return (1);   # return multi
        } else {
            warn("\nWARNING: invalid multi-line rule: $$single_ref\n")
              if ($config{verbose} && $$multi_ref !~ /^\s*#/);

            @_ = split(/\n/, $$multi_ref);

            undef($$multi_ref);
            undef($$single_ref);

          # First line of broken multi-line rule will be returned as a non-rule line.
            $$nonrule_ref = shift(@_) . "\n";
            $$nonrule_ref =~ s/\s*\n$/\n/;   # remove trailing whitespaces

          # The rest is put back to the array again.
            foreach $_ (reverse((@_))) {
                unshift(@$arr_ref, "$_\n");
            }

            return (1);   # return non-rule
        }
     } elsif (parse_singleline_rule($line, $msg_ref, $sid_ref)) {
        $$single_ref = $line;
        $$single_ref =~ s/^\s*//;
        $$single_ref =~ s/^#+\s*/#/;
        $$single_ref =~ s/\s*\n$/\n/;

        return (1);   # return single
    } else {                          # non-rule line

      # Do extra check and warn if it *might* be a rule anyway,
      # but that we just couldn't parse for some reason.
        warn("\nWARNING: line may be a rule but it could not be parsed ".
             "(missing sid or msg?): $line\n")
          if ($config{verbose} && $line =~ /^\s*alert .+msg\s*:\s*".+"\s*;/);

        $$nonrule_ref = $line;
        $$nonrule_ref =~ s/\s*\n$/\n/;

        return (1);   # return non-rule
    }
}



# From oinkmaster.pl except that this version
# has been modified so that the sid is *optional*.
sub parse_singleline_rule($ $ $)
{
    my $line    = shift;
    my $msg_ref = shift;
    my $sid_ref = shift;

    if ($line =~ /$SINGLELINE_RULE_REGEXP/oi) {

        if ($line =~ /\bmsg\s*:\s*"(.+?)"\s*;/i) {
            $$msg_ref = $1;
        } else {
            return (0);
        }

        if ($line =~ /\bsid\s*:\s*(\d+)\s*;/i) {
            $$sid_ref = $1;
#        } else {
#            return (0);
        }

        return (1);
    }

    return (0);
}
