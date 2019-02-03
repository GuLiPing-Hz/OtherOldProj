<?php
	include "stream.inc";
	srand(make_seed());
	for($i = 0;$i < 50;$i++)
	{
        	$seq = rand(1,99999999);
		echo $seq . "\n";
	}
?>
