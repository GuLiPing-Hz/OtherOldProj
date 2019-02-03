<?php
	$str = "haha";
	$str = pack("nnN",strlen($str)+6,0x01,0x0000);
	$str .= $str;

	$fp = fsockopen("127.0.0.1", 8200, $errno, $errstr, 30);
	if (!$fp) 
	{
		echo "$errstr ($errno)<br />\n";
	} 
	else 
	{
		echo "connect success";
		//fwrite($fp, $str);
		fclose($fp);
	}

?>
