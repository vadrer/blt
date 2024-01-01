use strict;
use Getopt::Long;
use ExtUtils::MakeMaker;

WriteMakefile(
    VERSION => '2.4.2.3',
    NAME => "Tcl::Tk::Tkwidget::BLT",
    PREREQ_FATAL => 1,
    PREREQ_PM => {
       'Tcl::Tk' => 1.0,
    },
);
