#!/usr/bin/perl

use Socket;
use Net::DNS;

my $arg;
my %condmap;

%condmap = (
   "if_common" =>  1,
   "if_any_common" => 1,
   "if_none_common" => 9,
   "if_new" => 2,
   "if_any_extra" => 2,
   "if_any_another" => 2,
   "if_none_extra" => 10,
   "if_none_another" => 10,
   "if_exist" => 3,
   "if_any" => 3,
   "if_none" => 11,
   "if_any_absent" => 4,
   "if_none_absent"=> 12,
   "if_another" => 6,
   "if_differs" => 6,
   "if_not_differs" => 14
);

for $arg ( @ARGV ) {
  if( $arg eq '-' ) {
    &process_file( *STDIN{IO} );
  }
  else {
    open( INP, $arg ) or die "cannot open $arg";
    &process_file( *INP{IO} );
    close INP;
  }
}
exit(0);

sub process_file {
  my $fh = shift;
  my( $l, $cmd, $etalon );
  for(;;) {
    $l = <$fh>;
    last unless defined $l;
    chomp $l; $l =~ s/\s+$//; $l =~ s/^\s+//;
    next if $l eq '' or $l =~ /^#/;
    #- print "_: $l\n";
    die "fatal: main: bad syntax: $l" unless $l =~ /^(\S+)\s+/;
    $cmd = $1; $etalon = $';
    if( $cmd eq 'gethostbyname' ||
        $cmd =~ /^gethostbyaddr\b/ )
    {
      &do_netdb( $cmd, $etalon );
    }
    elsif( $cmd eq 'dns' ) {
      &do_dns( $etalon );
    }
  }
}

## syntax: gethostby{name|addr} $domain $condition $data

sub do_netdb {
  my $cmd = shift;
  my $params = shift;
  my( $domain, $condition, $etalon, $h_errno, $haddr );
  my( $hname, $haliases, $haddrtype, $haddrlength );
  my @haddrs;
  my %reality;
  die "fatal: do_netdb: bad syntax: $params"
      unless $params =~ /^(\S+)\s+(\S+)\s*/;
  $domain = $1;
  $condition = $2;
  $etalon = $';
  %reality = ();
  if( $cmd eq 'gethostbyname' ) {
    ($hname,$haliases,$haddrtype,$haddrlength,@haddrs) =
        gethostbyname( $domain );
    $h_errno = $?;
    #- print "_: do_netdb: domain=$domain: h_errno=$h_errno\n";
    if( @haddrs ) {
      die unless $haddrtype = PF_INET;
      for $haddr ( @haddrs ) {
        $reality{inet_ntoa($haddr)} = 1;
      }
    }
  }
  elsif( $cmd eq 'gethostbyaddr' ) {
    print "_: do_netdb: gethostbyaddr\n";
    ($hname,$haliases,$haddrtype,$haddrlength,@haddrs) =
        gethostbyaddr(inet_aton($domain), PF_INET);
    $h_errno = $?;
    if( $hname ) { $reality{$hname} = 1; }
  }
  else {
    die "do_netdb: fatal: unknown query: $cmd";
  }
  #- printf "_: do_netdb: domain=%s: reality: %s\n", $domain,
  #-     join( ' ', sort keys %reality );
  &conclude( "$cmd $domain $condition", $condition, \%reality, $etalon );
}

sub conclude {
  my $banner = shift;
  my $cond = shift;
  my $realref = shift;
  my $etalonstr = shift;
  my %etalon = ();
  my( $k, $ci, $flag_rev );
  my %result;

  for $k ( split( /\s+/, $etalonstr ) ) { $etalon{$k} = 1 if $k; }

  $ci = $condmap{$cond};
  die unless $ci;
  $flag_rev = ( $ci >= 8 );
  $ci &= 7;
  ## Form result
  %result = ();
  if( $ci == 1 ) { ## if_any_common, if_none_common
    for $k ( keys %$realref ) { $result{$k} = 1 if $etalon{$k}; }
  }
  elsif( $ci == 2 ) { ## if_any_extra, if_none_extra
    for $k ( keys %$realref ) { $result{$k} = 1 unless $etalon{$k}; }
  }
  elsif( $ci == 3 ) { ## if_any, if_none
    %result = %$realref;
  }
  elsif( $ci == 4 ) { ## if_any_absent, if_none_absent
    for $k ( keys %etalon ) { $result{$k} = 1 unless $$realref{$k}; }
  }
  elsif( $ci == 6 ) { ## if_differs, if_not_differs
    for $k ( keys %$realref ) { $result{$k} = 1 unless $etalon{$k}; }
    for $k ( keys %etalon ) { $result{$k} = 1 unless $$realref{$k}; }
  }
  else { die; }
  if( $flag_rev ) {
    printf "%s: TRAP: no names\n", $banner
        unless scalar %result;
  }
  else {
    printf "%s: TRAP: %s\n", $banner, join( ' ', sort keys %result )
        if scalar %result;
  }
}

