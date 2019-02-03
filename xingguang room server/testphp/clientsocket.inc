<?php
function TCPRequest($host,$port,&$comm,$commlen,$conntimeout = 1,$rwtimeout = 1)
{
        //connect server
        $fp = fsockopen($host,$port,$errno,$errstr,3);
        if ($fp === false)
        {
		echo "5555";
                return false;
        }

        //set send/recv timeout
        stream_set_timeout($fp,$rwtimeout);
        $len = $commlen;
        $written = 0;

        do{
                $result = fwrite($fp,$comm);
                if ($result === false)
                {
                        fclose($fp);
                        return false;
                }
                $written += $result;
                if($written < $len)
                        $comm = substr($comm,$written);
        }while($written < $len);

	
	/*
        //read total size
        $lenstr = fread($fp,6);
        $len = hexdec($lenstr);
	//echo "\n".$lenstr;

        if($len == 0)
                return false;

        $defReadBufSize = 8192;
        $hasread = 0;

        $val = '';
        do
	{
                $len_unrecv = $len - $hasread;
                if($len_unrecv < $defReadBufSize)
                {
                        $defReadBufSize = $len_unrecv;
                }

                $res = fread($fp, $defReadBufSize);
                if ($res === false)
                {
                        fclose($fp);
                        return false;
                }
                $hasread += strlen($res);
                $val .= $res;
        }
	while($hasread < $len);
        fclose($fp);

        //return
        return $lenstr . $val;*/
	$val = "";
	while(!feof($fp)){
		$ret = fread($fp,8192);
		if($ret === false)
		{
			fclose($fp);
			return false;
		}
		$val .= $ret;
	}
	fclose($fp);
	return $val;
}
?>
