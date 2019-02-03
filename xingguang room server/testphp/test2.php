<?php
	include "stream.inc";
	$wstream = new TextWriteStream();
	$wstream->WriteParam(99);
	$wstream->WriteParam("tuxedoking");
	$wstream->Flush();
	//echo $wstream->GetData();
	//echo $wstream->GetSize();

	$text = "00001e0000029900000atuxedoking";
	$rstream = new TextReadStream($text);
	$str = "";
	while($str = $rstream->ReadParam())
	{
		echo $str . "\n";
	}
?>
