<?php

define('STREAM_HEAD_LEN', 6);
define('STREAM_FORMAT_STRING',"%06x");
define('CMD_SELECT',102);
define('CMD_UPDATE',100);

class TextReadStream
{
	var $m_strText = "";
	var $m_pos = 0;
	var $m_len = 0;

	function __construct(&$str) {
		sscanf(substr($str,0,STREAM_HEAD_LEN),STREAM_FORMAT_STRING,$this->m_len);
		$this->m_pos = STREAM_HEAD_LEN;
		$this->m_strText = $str;
		//$this->m_strText = substr($str,STREAM_HEAD_LEN,$this->m_len);
	}
	function ReadParam()
	{
		if($this->m_pos == $this->m_len)
		{
			//echo $this->m_pos . " " . $this->m_len;
			return false;
		}
		sscanf(substr($this->m_strText,$this->m_pos,STREAM_HEAD_LEN),STREAM_FORMAT_STRING,$len);
		$this->m_pos += STREAM_HEAD_LEN;
		$str = substr($this->m_strText,$this->m_pos,$len);

		$this->m_pos += $len;
		return $str;
	}
}

class TextWriteStream
{
	var $m_strText = "";
	var $m_pos = 0;
	function GetData()
	{
		return $this->m_strText;
	}
	function GetSize()
        {
                return $this->m_pos;
        }
	function Clear()
	{
		$this->m_strText = "";
		$this->m_pos = 0;
	}
	function WriteParam($str)
	{
		$len = strlen($str);
		$lenstr = sprintf(STREAM_FORMAT_STRING,$len);
		$this->m_strText .= $lenstr;
		$this->m_strText .= $str;
		$this->m_pos += (STREAM_HEAD_LEN + $len);
		//echo $this->m_strText . "\n";
	}
	function Flush()
	{
		$headlen = $this->m_pos + STREAM_HEAD_LEN;
		$this->m_strText = sprintf(STREAM_FORMAT_STRING,$headlen) . $this->m_strText;
	}
}

function CreateQueryString($sql)
{
	$cmd = 0;
	if(stripos($sql,"select ") === 0)
		$cmd = CMD_SELECT;
	else
		$cmd = CMD_UPDATE;
	$wstream = new TextWriteStream();
	$wstream->WriteParam($cmd);
	srand(make_seed());
	$seq = rand(1,99999999);
	$wstream->WriteParam($seq);
	$wstream->WriteParam($sql);
	$wstream->Flush();
}

function make_seed()
{
    list($usec, $sec) = explode(' ', microtime());
    return (float) $sec + ((float) $usec * 100000);
}

?>
