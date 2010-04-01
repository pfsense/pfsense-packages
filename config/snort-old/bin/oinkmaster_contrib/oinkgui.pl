#!/usr/bin/perl -w

# $Id: oinkgui.pl,v 1.52 2005/12/31 13:42:46 andreas_o Exp $ #

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


use 5.006001;

use strict;
use File::Spec;
use Tk;
use Tk::Balloon;
use Tk::BrowseEntry;
use Tk::FileSelect;
use Tk::NoteBook;
use Tk::ROText;

use constant CSIDL_DRIVES => 17;

sub update_rules();
sub clear_messages();
sub create_cmdline($);
sub fileDialog($ $ $ $);
sub load_config();
sub save_config();
sub save_messages();
sub update_file_label_color($ $ $);
sub create_fileSelectFrame($ $ $ $ $ $);
sub create_checkbutton($ $ $);
sub create_radiobutton($ $ $);
sub create_actionbutton($ $ $);
sub execute_oinkmaster(@);
sub logmsg($ $);


my $version = 'Oinkmaster GUI v1.1';

my @oinkmaster_conf = qw(
    /etc/oinkmaster.conf
    /usr/local/etc/oinkmaster.conf
);

# List of URLs that will show up in the URL BrowseEntry.
my @urls = qw(
    http://www.bleedingsnort.com/bleeding.rules.tar.gz
    http://www.snort.org/pub-bin/downloads.cgi/Download/comm_rules/Community-Rules.tar.gz
    http://www.snort.org/pub-bin/oinkmaster.cgi/<oinkcode>/snortrules-snapshot-CURRENT.tar.gz
    http://www.snort.org/pub-bin/oinkmaster.cgi/<oinkcode>/snortrules-snapshot-2.3.tar.gz
);

my %color = (
    background        => 'Bisque3',
    button            => 'Bisque2',
    label             => 'Bisque1',
    notebook_bg       => 'Bisque2',
    notebook_inact    => 'Bisque3',
    file_label_ok     => '#00e000',
    file_label_not_ok => 'red',
    out_frame_fg      => 'white',
    out_frame_bg      => 'black',
    entry_bg          => 'white',
    button_active     => 'white',
    button_bg         => 'Bisque4',
);

my %config = (
    animate          => 1,
    careful          => 0,
    enable_all       => 0,
    check_removed    => 0,
    output_mode      => 'normal',
    diff_mode        => 'detailed',
    perl             => $^X,
    oinkmaster       => "",
    oinkmaster_conf  => "",
    outdir           => "",
    url              => "",
    varfile          => "",
    backupdir        => "",
    editor           => "",
);

my %help = (

  # File locations.
    oinkscript    => 'Location of the executable Oinkmaster script (oinkmaster.pl).',
    oinkconf      => 'The Oinkmaster configuration file to use.',
    outdir        => 'Where to put the new rules. This should be the directory where you '.
                     'store your current rules.',

    url           => 'Alternate location of rules archive to download/copy. '.
                     'Leave empty to use the location set in oinkmaster.conf.',
    varfile       => 'Variables that exist in downloaded snort.conf but not in '.
                     'this file will be added to it. Leave empty to skip.',
    backupdir     => 'Directory to put tarball of old rules before overwriting them. '.
                     'Leave empty to skip backup.',
    editor        => 'Full path to editor to execute when pressing the "edit" button '.
                     '(wordpad is recommended on Windows). ',

  # Checkbuttons.
    careful       => 'In careful mode, Oinkmaster will just check for changes, '.
                     'not update anything.',
    enable        => 'Some rules may be commented out by default (for a reason!). '.
                     'This option will make Oinkmaster enable those.',
    removed       => 'Check for rules files that exist in the output directory but not '.
                     'in the downloaded rules archive.',

  # Action buttons.
    clear        => 'Clear current output messages.',
    save         => 'Save current output messages to file.',
    exit         => 'Exit the GUI.',
    update       => 'Execute Oinkmaster to update the rules.',
    test         => 'Test current Oinkmaster configuration. ' .
                    'If there are no fatal errors, you are ready to update the rules.',
    version      => 'Request version information from Oinkmaster.',
);


my $gui_config_file = "";
my $use_fileop      = 0;


#### MAIN ####

select STDERR;
$| = 1;
select STDOUT;
$| = 1;

# Find out if can use Win32::FileOp.
if ($^O eq 'MSWin32') {
    BEGIN { $^W = 0 }
    $use_fileop = 1 if (eval "require Win32::FileOp");
}

# Find out which oinkmaster.pl file to default to.
foreach my $dir (File::Spec->path()) {
    my $file = "$dir/oinkmaster";
    if (-f "$file" && (-x "$file" || $^O eq 'MSWin32')) {
        $config{oinkmaster} = $file;
        last;
    } elsif (-f "$file.pl" && (-x "$file" || $^O eq 'MSWin32')) {
        $config{oinkmaster} = "$file.pl";
        last;
    }
}

# Find out which oinkmaster config file to default to.
foreach my $file (@oinkmaster_conf) {
    if (-e "$file") {
        $config{oinkmaster_conf} = $file;
        last;
    }
}

# Find out where the GUI config file is (it's not required).
if ($ENV{HOME}) {
    $gui_config_file = "$ENV{HOME}/.oinkguirc"
} elsif ($ENV{HOMEDRIVE} && $ENV{HOMEPATH}) {
    $gui_config_file = "$ENV{HOMEDRIVE}$ENV{HOMEPATH}\\.oinkguirc";
}


# Create main window.
my $main = MainWindow->new(
  -background => "$color{background}",
  -title      => "$version",
);


# Create scrolled frame with output messages.
my $out_frame = $main->Scrolled('ROText',
  -setgrid    => 'true',
  -scrollbars => 'e',
  -background => $color{out_frame_bg},
  -foreground => $color{out_frame_fg},
);


my $help_label = $main->Label(
    -relief     => 'groove',
    -background => "$color{label}",
);

my $balloon = $main->Balloon(
    -statusbar => $help_label,
);


# Create notebook.
my $notebook = $main->NoteBook(
  -ipadx              => 6,
  -ipady              => 6,
  -background         => $color{notebook_bg},
  -inactivebackground => $color{notebook_inact},
  -backpagecolor      => $color{background},
);


# Create tab with required files/dirs.
my $req_tab = $notebook->add("required",
  -label     => "Required files and directories",
  -underline => 0,
);

$req_tab->configure(-bg => "$color{notebook_inact}");


# Create frame with oinkmaster.pl location.
my $filetypes = [
  ['Oinkmaster script', 'oinkmaster.pl'],
  ['All files',         '*'            ]
];

my $oinkscript_frame =
  create_fileSelectFrame($req_tab, "oinkmaster.pl", 'EXECFILE',
                         \$config{oinkmaster}, 'NOEDIT', $filetypes);

$balloon->attach($oinkscript_frame, -statusmsg => $help{oinkscript});


# Create frame with oinkmaster.conf location.
$filetypes = [
  ['configuration files', '.conf'],
  ['All files',           '*'    ]
];

my $oinkconf_frame =
  create_fileSelectFrame($req_tab, "oinkmaster.conf", 'ROFILE',
                         \$config{oinkmaster_conf}, 'EDIT', $filetypes);

$balloon->attach($oinkconf_frame, -statusmsg => $help{oinkconf});


# Create frame with output directory.
my $outdir_frame =
  create_fileSelectFrame($req_tab, "output directory", 'WRDIR',
                         \$config{outdir}, 'NOEDIT', undef);

$balloon->attach($outdir_frame, -statusmsg => $help{outdir});



# Create tab with optional files/dirs.
my $opt_tab = $notebook->add("optional",
  -label     => "Optional files and directories",
  -underline => 0,
);

$opt_tab->configure(-bg => "$color{notebook_inact}");

# Create frame with alternate URL location.
$filetypes = [
  ['compressed tar files', '.tar.gz']
];

my $url_frame =
  create_fileSelectFrame($opt_tab, "Alternate URL", 'URL',
                         \$config{url}, 'NOEDIT', $filetypes);

$balloon->attach($url_frame, -statusmsg => $help{url});


# Create frame with variable file.
$filetypes = [
  ['Snort configuration files', ['.conf', '.config']],
  ['All files',    '*'                           ]
];

my $varfile_frame =
  create_fileSelectFrame($opt_tab, "Variable file", 'WRFILE',
                         \$config{varfile}, 'EDIT', $filetypes);

$balloon->attach($varfile_frame, -statusmsg => $help{varfile});


# Create frame with backup dir location.
my $backupdir_frame =
  create_fileSelectFrame($opt_tab, "Backup directory", 'WRDIR',
                         \$config{backupdir}, 'NOEDIT', undef);

$balloon->attach($backupdir_frame, -statusmsg => $help{backupdir});


# Create frame with editor location.
$filetypes = [
  ['executable files', ['.exe']],
  ['All files',    '*'                           ]
];

my $editor_frame =
  create_fileSelectFrame($opt_tab, "Editor", 'EXECFILE',
                         \$config{editor}, 'NOEDIT', $filetypes);

$balloon->attach($editor_frame, -statusmsg => $help{editor});



$notebook->pack(
  -expand => 'no',
  -fill   => 'x',
  -padx   => '5',
  -pady   => '5',
  -side   => 'top'
);


# Create the frame to the left.
my $left_frame = $main->Frame(
  -background => "$color{label}",
  -border     => '2',
)->pack(
  -side       => 'left',
  -fill       => 'y',
);


# Create "GUI settings" label.
$left_frame->Label(
  -text       => "GUI settings:",
  -background => "$color{label}",
)->pack(
  -side       => 'top',
  -fill       => 'x',
);


create_actionbutton($left_frame, "Load saved settings",   \&load_config);
create_actionbutton($left_frame, "Save current settings", \&save_config);


# Create "options" label at the top of the left frame.
$left_frame->Label(
  -text        => "Options:",
  -background  => "$color{label}",
)->pack(-side  => 'top',
        -fill  => 'x',
);


# Create checkbuttons in the left frame.
$balloon->attach(
  create_checkbutton($left_frame, "Careful mode", \$config{careful}),
  -statusmsg => $help{careful}
);

$balloon->attach(
  create_checkbutton($left_frame, "Enable all", \$config{enable_all}),
  -statusmsg => $help{enable}
);

$balloon->attach(
  create_checkbutton($left_frame, "Check for removed files", \$config{check_removed}),
  -statusmsg => $help{removed}
);


# Create "mode" label.
$left_frame->Label(
  -text        => "Output mode:",
  -background  => "$color{label}",
)->pack(
  -side        => 'top',
  -fill        => 'x',
);

# Create mode radiobuttons in the left frame.
create_radiobutton($left_frame, "super-quiet", \$config{output_mode});
create_radiobutton($left_frame, "quiet",       \$config{output_mode});
create_radiobutton($left_frame, "normal",      \$config{output_mode});
create_radiobutton($left_frame, "verbose",     \$config{output_mode});

# Create "Diff mode" label.
$left_frame->Label(
  -text        => "Diff  mode:",
  -background  => "$color{label}",
)->pack(
  -side  => 'top',
  -fill  => 'x',
);

create_radiobutton($left_frame, "detailed",      \$config{diff_mode});
create_radiobutton($left_frame, "summarized",    \$config{diff_mode});
create_radiobutton($left_frame, "remove common", \$config{diff_mode});


# Create "activity messages" label.
$main->Label(
  -text       => "Output messages:",
  -width      => '130',
  -background => "$color{label}",
)->pack(
  -side       => 'top',
  -fill       => 'x',
);



# Pack output frame.
$out_frame->pack(
  -expand     => 'yes',
  -fill       => 'both',
);


# Pack help label below output window.
$help_label->pack(
    -fill     => 'x',
);


# Create "actions" label.
$left_frame->Label(
  -text       => "Actions:",
  -background => "$color{label}",
)->pack(
  -side       => 'top',
  -fill       => 'x',
);


# Create action buttons.

$balloon->attach(
  create_actionbutton($left_frame, "Update rules!", \&update_rules),
  -statusmsg => $help{update}
);

$balloon->attach(
  create_actionbutton($left_frame, "Clear output messages", \&clear_messages),
  -statusmsg => $help{clear}
);

$balloon->attach(
  create_actionbutton($left_frame, "Save output messages", \&save_messages),
  -statusmsg => $help{save}
);

$balloon->attach(
  create_actionbutton($left_frame, "Exit", \&exit),
  -statusmsg => $help{exit}
);



# Make the mousewheel scroll the output window. Taken from Mastering Perl/Tk.
if ($^O eq 'MSWin32') {
    $out_frame->bind('<MouseWheel>' =>
        [ sub { $_[0]->yview('scroll', -($_[1] / 120) * 3, 'units')},
            Ev('D') ]
    );
} else {
    $out_frame->bind('<4>' => sub {
        $_[0]->yview('scroll', -3, 'units') unless $Tk::strictMotif;
    });

    $out_frame->bind('<5>' => sub {
        $_[0]->yview('scroll', +3, 'units') unless $Tk::strictMotif;
    });
}



# Now the fun begins.
if ($config{animate}) {
    foreach (split(//, "Welcome to $version")) {
        logmsg("$_", 'MISC');
        $out_frame->after(5);
    }
} else {
    logmsg("Welcome to $version", 'MISC');
}

logmsg("\n\n", 'MISC');

# Load gui settings into %config.
load_config();


# Warn if any required file/directory is not set.
logmsg("No oinkmaster.pl set, please select one above!\n\n", 'ERROR')
  if ($config{oinkmaster} !~ /\S/);

logmsg("No oinkmaster configuration file set, please select one above!\n\n", 'ERROR')
  if ($config{oinkmaster_conf} !~ /\S/);

logmsg("Output directory is not set, please select one above!\n\n", 'ERROR')
  if ($config{outdir} !~ /\S/);


MainLoop;



#### END ####



sub fileDialog($ $ $ $)
{
    my $var_ref   = shift;
    my $title     = shift;
    my $type      = shift;
    my $filetypes = shift;
    my $dirname;

    if ($type eq 'WRDIR') {
        if ($use_fileop) {
            $dirname = Win32::FileOp::BrowseForFolder("title", CSIDL_DRIVES);
        } else {
            my $fs = $main->FileSelect();
            $fs->configure(-verify => ['-d', '-w'], -title => $title);
            $dirname = $fs->Show;
        }
        $$var_ref = $dirname if ($dirname);
    } elsif ($type eq 'EXECFILE' || $type eq 'ROFILE' || $type eq 'WRFILE' || $type eq 'URL') {
        my $filename = $main->getOpenFile(-title => $title, -filetypes => $filetypes);
        $$var_ref = $filename if ($filename);
    } elsif ($type eq 'SAVEFILE') {
        my $filename = $main->getSaveFile(-title => $title, -filetypes => $filetypes);
        $$var_ref = $filename if ($filename);
    } else {
        logmsg("Unknown type ($type)\n", 'ERROR');
    }
}



sub update_file_label_color($ $ $)
{
    my $label    = shift;
    my $filename = shift;
    my $type     = shift;

    $filename =~ s/^\s+//;
    $filename =~ s/\s+$//;

    unless ($filename) {
        $label->configure(-background => $color{file_label_not_ok});
        return (1);
    }

    if ($type eq "URL") {
        if ($filename =~ /^(?:http|ftp|scp):\/\/.+\.tar\.gz$/) {
            $label->configure(-background => $color{file_label_ok});
        } elsif ($filename =~ /^(?:file:\/\/)*(.+\.tar\.gz)$/) {
            my $file = $1;
            if (-f "$file" && -r "$file") {
                $label->configure(-background => $color{file_label_ok});
            } else {
                $label->configure(-background => $color{file_label_not_ok});
            }
        } else {
            $label->configure(-background => $color{file_label_not_ok});
        }
    } elsif ($type eq "ROFILE") {
        if (-f "$filename" && -r "$filename") {
            $label->configure(-background => $color{file_label_ok});
        } else {
            $label->configure(-background => $color{file_label_not_ok});
        }
    } elsif ($type eq "EXECFILE") {
        if (-f "$filename" && (-x "$filename" || $^O eq 'MSWin32')) {
            $label->configure(-background => $color{file_label_ok});
        } else {
            $label->configure(-background => $color{file_label_not_ok});
        }
    } elsif ($type eq "WRFILE") {
        if (-f "$filename" && -w "$filename") {
            $label->configure(-background => $color{file_label_ok});
        } else {
            $label->configure(-background => $color{file_label_not_ok});
        }
    } elsif ($type eq "WRDIR") {
        if (-d "$filename" && -w "$filename") {
            $label->configure(-background => $color{file_label_ok});
        } else {
            $label->configure(-background => $color{file_label_not_ok});
        }
    } else {
       print STDERR "incorrect type ($type)\n";
       exit;
    }

    return (1);
}



sub create_checkbutton($ $ $)
{
    my $frame   = shift;
    my $name    = shift;
    my $var_ref = shift;

    my $button = $frame->Checkbutton(
      -text                => $name,
      -background          => $color{button},
      -activebackground    => $color{button_active},
      -highlightbackground => $color{button_bg},
      -variable            => $var_ref,
      -relief              => 'raise',
      -anchor              => 'w',
    )->pack(
      -fill                => 'x',
      -side                => 'top',
      -pady                => '1',
    );

    return ($button);
}



sub create_actionbutton($ $ $)
{
    my $frame    = shift;
    my $name     = shift;
    my $func_ref = shift;

    my $button = $frame->Button(
      -text                => $name,
      -command             => sub {
                                &$func_ref;
                                $out_frame->focus;
                              },
      -background          => $color{button},
      -activebackground    => $color{button_active},
      -highlightbackground => $color{button_bg},
    )->pack(
      -fill                => 'x',
    );

    return ($button);
}



sub create_radiobutton($ $ $)
{
    my $frame    = shift;
    my $name     = shift;
    my $mode_ref = shift;

    my $button = $frame->Radiobutton(
      -text                => $name,
      -highlightbackground => $color{button_bg},
      -background          => $color{button},
      -activebackground    => $color{button_active},
      -variable            => $mode_ref,
      -relief              => 'raised',
      -anchor              => 'w',
      -value               => $name,
    )->pack(
      -side                => 'top',
      -pady                => '1',
      -fill                => 'x',
    );

    return ($button);
}



# Create <label><entry><browsebutton> in given frame.
sub create_fileSelectFrame($ $ $ $ $ $)
{
    my $win       = shift;
    my $name      = shift;
    my $type      = shift;  # FILE|DIR|URL
    my $var_ref   = shift;
    my $edtype    = shift;  # EDIT|NOEDIT
    my $filetypes = shift;

  # Create frame.
    my $frame = $win->Frame(
      -bg => $color{background},
    )->pack(
      -padx => '2',
      -pady => '2',
      -fill => 'x'
    );

  # Create label.
    my $label = $frame->Label(
      -text       => $name,
      -width      => '16',
      -relief     => 'raised',
      -background => "$color{file_label_not_ok}",
    )->pack(
      -side       => 'left'
    );

    my $entry;

    if ($type eq 'URL') {
        $entry = $frame->BrowseEntry(
          -textvariable    => $var_ref,
          -background      => $color{entry_bg},
          -width           => '80',
          -choices         => \@urls,
          -validate        => 'key',
          -validatecommand => sub { update_file_label_color($label, $_[0], $type) },
        )->pack(
          -side            => 'left',
          -expand          => 'yes',
          -fill            => 'x'
        );
    } else {
        $entry = $frame->Entry(
          -textvariable    => $var_ref,
          -background      => $color{entry_bg},
          -width           => '80',
          -validate        => 'key',
          -validatecommand => sub { update_file_label_color($label, $_[0], $type) },
        )->pack(
          -side            => 'left',
          -expand          => 'yes',
          -fill            => 'x'
       );
    }

  # Create edit-button if file is ediable.
    if ($edtype eq 'EDIT') {
        my $edit_but = $frame->Button(
          -text       => "Edit",
          -background => "$color{button}",
          -command    => sub {
                                 unless (-e "$$var_ref") {
                                     logmsg("Select an existing file first!\n\n", 'ERROR');
                                     return;
                                 }

                                 if ($config{editor}) {
                                     $main->Busy(-recurse => 1);
                                     logmsg("Launching " . $config{editor} .
                                            ", close it to continue the GUI.\n\n", 'MISC');
                                     sleep(2);
                                     system($config{editor}, $$var_ref); # MainLoop will be put on hold...
                                     $main->Unbusy;
                                 } else {
                                     logmsg("No editor set\n\n", 'ERROR');
                                 }
                             }
        )->pack(
          -side       => 'left',
        );
    }

  # Create browse-button.
    my $but = $frame->Button(
      -text       => "browse ...",
      -background => $color{button},
      -command    => sub {
                            fileDialog($var_ref, $name, $type, $filetypes);
                         }
    )->pack(
      -side       => 'left',
    );

    return ($frame);
}



sub logmsg($ $)
{
    my $text = shift;
    my $type = shift;

    return unless (defined($text));

    $out_frame->tag(qw(configure OUTPUT -foreground grey));
    $out_frame->tag(qw(configure ERROR  -foreground red));
    $out_frame->tag(qw(configure MISC   -foreground white));
    $out_frame->tag(qw(configure EXEC   -foreground bisque2));

    $out_frame->insert('end', "$text", "$type");
    $out_frame->see('end');
    $out_frame->update;
}




sub execute_oinkmaster(@)
{
    my @cmd = @_;
    my @obfuscated_cmd;

  # Obfuscate possible password in url.
    foreach my $line (@cmd) {
        if ($line =~ /^(\S+:\/\/.+?):.+?@(.+)/) {
            push(@obfuscated_cmd, "$1:*password*\@$2");
        } else {
            push(@obfuscated_cmd, $line);
        }
    }

    logmsg("@obfuscated_cmd:\n", 'EXEC');

    $main->Busy(-recurse => 1);

    if ($^O eq 'MSWin32') {
        open(OINK, "@cmd 2>&1|");
        while (<OINK>) {
            logmsg($_, 'OUTPUT');
        }
        close(OINK);
    } else {
        if (open(OINK,"-|")) {
            while (<OINK>) {
                logmsg($_, 'OUTPUT');
            }
        } else {
            open(STDERR, '>&STDOUT');
            exec(@cmd);
        }
        close(OINK);
    }

    $main->Unbusy;
    logmsg("done.\n\n", 'EXEC');
}



sub clear_messages()
{
    $out_frame->delete('1.0','end');
    $out_frame->update;
}



sub save_messages()
{
    my $text  = $out_frame->get('1.0', 'end');
    my $title = 'Save output messages';
    my $filename;

    my $filetypes = [
      ['Log files', ['.log', '.txt']],
      ['All files',    '*'                           ]
    ];


    if (length($text) > 1) {
        fileDialog(\$filename, $title, 'SAVEFILE', $filetypes);
        if (defined($filename)) {

            unless (open(LOG, ">", "$filename")) {
                logmsg("Could not open $filename for writing: $!\n\n", 'ERROR');
                return;
            }

            print LOG $text;
            close(LOG);
            logmsg("Successfully saved output messages to $filename\n\n", 'MISC');
        }

    } else {
        logmsg("Nothing to save.\n\n", 'ERROR');
    }
}



sub update_rules()
{
    my @cmd;

    create_cmdline(\@cmd) || return;
    clear_messages();
    execute_oinkmaster(@cmd);
}



sub create_cmdline($)
{
    my $cmd_ref = shift;

    my $oinkmaster      = $config{oinkmaster};
    my $oinkmaster_conf = $config{oinkmaster_conf};
    my $outdir          = $config{outdir};
    my $varfile         = $config{varfile};
    my $url             = $config{url};
    my $backupdir       = $config{backupdir};

  # Assume file:// if url prefix is missing.
    if ($url) {
        $url = "file://$url" unless ($url =~ /(?:http|ftp|file|scp):\/\//);
        if ($url =~ /.+<oinkcode>.+/) {
            logmsg("You must replace <oinkcode> with your real oinkcode, see the FAQ!\n\n", 'ERROR');
            return (0);
        }
    }

    $oinkmaster = File::Spec->rel2abs($oinkmaster)
      if ($oinkmaster);

    $outdir    = File::Spec->canonpath("$outdir");
    $backupdir = File::Spec->canonpath("$backupdir");

  # Clean leading/trailing whitespaces.
    foreach my $var_ref (\$oinkmaster, \$oinkmaster_conf, \$outdir,
                         \$varfile, \$url, \$backupdir) {
        $$var_ref =~ s/^\s+//;
        $$var_ref =~ s/\s+$//;
    }

    unless ($config{oinkmaster} && -f "$config{oinkmaster}" &&
     (-x "$config{oinkmaster}" || $^O eq 'MSWin32')) {
        logmsg("Location of oinkmaster.pl is not set correctly!\n\n", 'ERROR');
        return;
    }

    unless ($oinkmaster_conf && -f "$oinkmaster_conf") {
        logmsg("Location of configuration file is not set correctlyy!\n\n", 'ERROR');
        return (0);
    }

    unless ($outdir && -d "$outdir") {
        logmsg("Output directory is not set correctly!\n\n", 'ERROR');
        return (0);
    }

  # Add leading/trailing "" if win32.
    foreach my $var_ref (\$oinkmaster, \$oinkmaster_conf, \$outdir,
                         \$varfile, \$url, \$backupdir) {
        if ($^O eq 'MSWin32' && $$var_ref) {
            $$var_ref = "\"$$var_ref\"";
        }
    }

    push(@$cmd_ref,
      "$config{perl}", "$oinkmaster",
      "-C", "$oinkmaster_conf",
      "-o", "$outdir");

    push(@$cmd_ref, "-c")               if ($config{careful});
    push(@$cmd_ref, "-e")               if ($config{enable_all});
    push(@$cmd_ref, "-r")               if ($config{check_removed});
    push(@$cmd_ref, "-q")               if ($config{output_mode} eq "quiet");
    push(@$cmd_ref, "-Q")               if ($config{output_mode} eq "super-quiet");
    push(@$cmd_ref, "-v")               if ($config{output_mode} eq "verbose");
    push(@$cmd_ref, "-m")               if ($config{diff_mode}   eq "remove common");
    push(@$cmd_ref, "-s")               if ($config{diff_mode}   eq "summarized");
    push(@$cmd_ref, "-U", "$varfile")   if ($varfile);
    push(@$cmd_ref, "-b", "$backupdir") if ($backupdir);

    push(@$cmd_ref, "-u", "$url")
      if ($url);

    return (1);
}



# Load $config file into %config hash.
sub load_config()
{
    unless (defined($gui_config_file) && $gui_config_file) {
        logmsg("Unable to determine config file location, is your \$HOME set?\n\n", 'ERROR');
        return;
    }

    unless (-e "$gui_config_file") {
        logmsg("$gui_config_file does not exist, keeping current/default settings\n\n", 'MISC');
        return;
    }

    unless (open(RC, "<", "$gui_config_file")) {
        logmsg("Could not open $gui_config_file for reading: $!\n\n", 'ERROR');
        return;
    }

    while (<RC>) {
        next unless (/^(\S+)=(.*)/);
        $config{$1} = $2;
    }

    close(RC);
    logmsg("Successfully loaded GUI settings from $gui_config_file\n\n", 'MISC');
}



# Save %config into file $config.
sub save_config()
{
    unless (defined($gui_config_file) && $gui_config_file) {
        logmsg("Unable to determine config file location, is your \$HOME set?\n\n", 'ERROR');
        return;
    }

    unless (open(RC, ">", "$gui_config_file")) {
        logmsg("Could not open $gui_config_file for writing: $!\n\n", 'ERROR');
        return;
    }

    print RC "# Automatically created by Oinkgui. ".
             "Do not edit directly unless you have to.\n";

    foreach my $option (sort(keys(%config))) {
        print RC "$option=$config{$option}\n";
    }

    close(RC);
    logmsg("Successfully saved current GUI settings to $gui_config_file\n\n", 'MISC');
}
