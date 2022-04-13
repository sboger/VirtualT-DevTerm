#!/usr/bin/perl

open RAM,"<& RAM.bin";
open ROM,"<& optROM.bin";
my $rom=1;
while(<>) {
	if(/^:([0-9A-F]{2})([0-9A-F]{4})([0-9A-F]{2})([0-9A-F]*)/) {
		my($bytes,$offset,$checkum,$data)=($1,$2,$3,$4);
		my $pos=unpack("n",pack("H4",$offset));
		if($pos>=2**15) {
			$pos-=2**15;
			$rom=0;
		}
		else {
			$rom=1;
		}
		my $bytes=ord(pack("H2",$bytes));
		my $pack="H".($bytes*2);
		my $line=pack($pack,$4);
		print "Pos: $pos  $pack  $bytes\n";
		if($rom) {
			sysseek(ROM,$pos,0);
			syswrite(ROM,$line);
		}
		else {
			sysseek(RAM,$pos,0);
			syswrite(RAM,$line);
		}
	}
}
