#!/usr/bin/perl

$grey     = "\033[01;30m";
$red      = "\033[01;31m";
$alert    = "\033[41m\033[01;37m";
$darkred  = "\033[00;31m";
$green    = "\033[01;32m";
$yellow   = "\033[01;33m";
$brown    = "\033[00;33m";
$blue     = "\033[01;34m";
$magenta  = "\033[01;35m";
$purple   = "\033[00;35m";
$cyan     = "\033[01;36m";
$darkcyan = "\033[00;36m";
$white    = "\033[01;37m";

$default = "\033[00m";

while (<STDIN>)
{
	# private tell
	if (/^[^\s]+ tells you/ || /^[^\s]+ says/)
	{
		$color = $yellow;
	}
	# channel tell
	elsif (/^[^\s]+?\(\d+\):/)
	{
		$color = $green;
	}
	# whisp/kib
	elsif (/^[^\s]+ whispers/ || /^[^\s]+ kibitzes/)
	{
		$color = $magenta;
	}
	# shout
	elsif (/^-->/ || /^[^\s]+ shouts/)
	{
		$color = $cyan;
	}
	# challenge
	elsif (/^Challenge/ || /^Your [^\s]+ rating will change/ ||
	       /^Your new RD/ || /^You can \"accept\" or \"decline\"/ ||
	       /^Creating:/ || /^Issuing:/)
	{
		$color = $red;
	}
	# result/creating/etc
	elsif (/^\{/)
	{
		$color = $red;
	}
	# qtell
	elsif (/^:/)
	{
		$color = $brown;
	}
	# notification
	elsif (/^Notification/)
	{
		$color = $magenta;
	}
	# nick highlight - lowest priority
	elsif (/bistromath/ || /warriorness/)
	{
		$color = $yellow;
	}
	# default, unless line continuation leave it as it is
	elsif (!/^\\/)
	{
		$color = $white;
	}

	print "$color$_$default";
}
