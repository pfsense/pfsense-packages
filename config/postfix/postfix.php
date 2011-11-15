<?php
/*
	postfix.php
	part of pfSense (http://www.pfsense.com/)
	Copyright (C) 2011 Marcello Coutinho <marcellocoutinho@gmail.com>
	based on varnish_view_config.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice,
	   this list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright
	   notice, this list of conditions and the following disclaimer in the
	   documentation and/or other materials provided with the distribution.

	THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
	INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
	AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
	AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
	OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
*/
require_once("/etc/inc/util.inc");
require_once("/etc/inc/functions.inc");
require_once("/etc/inc/pkg-utils.inc");
require_once("/etc/inc/globals.inc");
function grep_log($from="",$to="",$subject=""){
	global $postfix_dir,$postfix_db,$postfix_arg;
	create_db();
	
	$total_lines=0;
	$grep="postfix.\(cleanup\|smtp\|error\|qmgr\)";
	$curr_time = time();
	$m=date('M',strtotime($postfix_arg['time'],$curr_time));
	$j=substr("  ".date('j',strtotime($postfix_arg['time'],$curr_time)),-3);
	# file grep loop
	foreach ($postfix_arg['grep'] as $hour){
	  print "/usr/bin/grep '^".$m.$j." ".$hour.".*".$grep."' /var/log/maillog\n"; 
	  $lists=array();
	  exec("/usr/bin/grep " . escapeshellarg('^'.$m.$j." ".$hour.".*".$grep)." /var/log/maillog", $lists);
	  $stm_noqueue="BEGIN;\n";
	  $stm_queue="BEGIN;\n";
	  foreach ($lists as $line){
		$status=array();
		$total_lines++;
		#Nov  8 09:31:50 srvchunk01 postfix/smtpd[43585]: 19C281F59C8: client=pm03-974.auinmeio.com.br[177.70.232.225]
		if(preg_match("/(\w+\s+\d+\s+[0-9,:]+) (\w+) postfix.smtpd\W\d+\W+(\w+): client=(.*)/",$line,$email)){
			$values="'".$email[3]."','".$email[1]."','".$email[2]."','".$email[4]."'";
			if(${$email[3]}!=$email[3])
				$stm_queue.='insert into mail_queue(sid,date,server,client) values('.$values.');'."\n";
			${$email[3]}=$email[3];
		}
		#Nov 13 00:09:07 srvchunk01 postfix/smtp[51436]: 9145C1F67F7: to=<lidia.santos@ma.mail.test.com>, relay=srvmail1-ma.ma.mail.test.com[172.23.3.6]:25, delay=2.4, delays=2.2/0/0.13/0.11, dsn=5.7.1, status=bounced (host srvmail1-ma.ma.mail.test.com[172.23.3.6] said: 550 5.7.1 Unable to relay for lidia.santos@ma.mail.test.com (in reply to RCPT TO command))
		#Nov  3 21:45:32 srvchunk01 postfix/smtp[18041]: 4CE321F4887: to=<vitrineabril@vitrineabril.alphainteractive.com.br>, relay=smtpe1.emv3.com[81.92.120.9]:25, delay=1.9, delays=0.06/0.01/0.68/1.2, dsn=2.0.0, status=sent (250 2.0.0 Ok: queued as 2C33E2382C8)
		else if(preg_match("/(\w+\s+\d+\s+[0-9,:]+) (\w+) postfix.smtp\W\d+\W+(\w+): to=\<(.*)\>, relay=(.*), delay=([0-9.]+).*dsn=([0-9.]+), status=(\w+) (.*)/",$line,$email)){
			$stm_queue.= "update mail_queue set too='".$email[4]."', relay='".$email[5]."', dsn='".$email[7]."', status='".$email[8]."', status_info='".preg_replace("/(\s+|\'|\")/"," ",$email[9])."', delay='".$email[6]."' where sid='".$email[3]."';\n";
			#print "update mail_queue set too='".$email[4]."', relay='".$email[5]."', dsn='".$email[9]."',status='".$email[8]."', status_info='".$email[9]."', delay='".$email[6]."' where sid='".$email[3]."';\n";
		}
		#Nov 13 01:48:44 srvch011 postfix/cleanup[16914]: D995B1F570B: message-id=<61.40.11745.10E3FBE4@ofertas6>
		else if(preg_match("/(\w+\s+\d+\s+[0-9,:]+) (\w+) postfix.cleanup\W\d+\W+(\w+): message-id=\<(.*)\>/",$line,$email)){
			$stm_queue.="update mail_queue set msgid='".$email[4]."' where sid='".$email[3]."';";
		}
		#Nov 14 02:40:05 srvchunk01 postfix/qmgr[46834]: BC5931F4F13: from=<ceag@mx.crmall.com.br>, size=32727, nrcpt=1 (queue active)
		else if(preg_match("/(\w+\s+\d+\s+[0-9,:]+) (\w+) postfix.qmgr\W\d+\W+(\w+): from=\<(.*)\>\W+size=(\d+)/",$line,$email)){
			$stm_queue.= "update mail_queue set fromm='".$email[4]."', size='".$email[5]."' where sid='".$email[3]."';\n";
		}
		#Nov 13 00:09:07 srvchunk01 postfix/bounce[56376]: 9145C1F67F7: sender non-delivery notification: D5BD31F6865
		else if(preg_match("/(\w+\s+\d+\s+[0-9,:]+) (\w+) postfix.bounce\W\d+\W+(\w+): sender non-delivery notification: (\w+)/",$line,$email)){
			$stm_queue.= "update mail_queue set bounce='".$email[4]."' where sid='".$email[3]."';\n";
		}
		#Nov  9 02:14:57 srvch011 postfix/cleanup[6856]: 617A51F5AC5: warning: header Subject: Mapeamento de Processos from lxalpha.12b.com.br[66.109.29.225]; from=<apache@lxalpha.12b.com.br> to=<ritiele.faria@mail.test.com> proto=ESMTP helo=<lxalpha.12b.com.br>
		#Nov  8 09:31:50 srvch011 postfix/cleanup[11471]: 19C281F59C8: reject: header From: "Giuliana Flores - Parceiro do Grupo Virtual" <publicidade@parceiro-grupovirtual.com.br> from pm03-974.auinmeio.com.br[177.70.232.225]; from=<publicidade@parceiro-grupovirtual.com.br> to=<jorge.lustosa@mail.test.com> proto=ESMTP helo=<pm03-974.auinmeio.com.br>: 5.7.1 [SN007]
		else if(preg_match("/(\w+\s+\d+\s+[0-9,:]+) (\w+) postfix.cleanup\W\d+\W+(\w+): (\w+): header (.*) from ([a-z,A-Z,0-9,.,-]+)\W([0-9,.]+)\W+from=\<(.*)\> to=\<(.*)\>.*helo=\W([a-z,A-Z,0-9,.,-]+)\W(.*|)/",$line,$email)){
			$status['date']=$email[1];
			$status['server']=$email[2];
			$status['sid']=$email[3];
			$status['remote_hostname']=$email[6];
			$status['remote_ip']=$email[7];
			$status['from']=$email[8];
			$status['to']=$email[9];
			$status['helo']=$email[10];
			$status['status']=$email[4];
			
			if ($email[4] =="warning"){
				$status['status_info']=preg_replace("/(\s+|\'|\")/"," ",$email[11]);
				$status['subject']=preg_replace("/Subject: /","",$email[5]);
				$status['subject']=preg_replace("/(\s+|\'|\")/"," ",$status['subject']);
				$stm_queue.="update mail_queue set subject='".$status['subject']."', status='".$status['status']."', status_info='".$status['info']."', fromm='".$status['from']."',too='".$status['size']."',helo='".$status['helo']."' where sid='".$status['sid']."';\n";
				}
			else{
				$status['status_info']=$email[5].$email[11];
				$stm_queue.="update mail_queue set status='".$status['status']."', status_info='".$status['status_info']."', fromm='".$status['from']."',too='".$status['to']."',helo='".$status['helo']."' where sid='".$status['sid']."';\n";
				}
			}
		#Nov  9 02:14:34 srvchunk01 postfix/smtpd[38129]: NOQUEUE: reject: RCPT from unknown[201.36.98.7]: 450 4.7.1 Client host rejected: cannot find your hostname, [201.36.98.7]; from=<maladireta@esadcursos.com.br> to=<sexec.09vara@go.domain.test.com> proto=ESMTP helo=<capri0.wb.com.br>
		else if(preg_match("/(\w+\s+\d+\s+[0-9,:]+) (\w+) postfix.smtpd\W\d+\W+NOQUEUE:\s+(\w+): (.*); from=\<(.*)\> to=\<(.*)\>.*helo=\<(.*)\>/",$line,$email)){
			$status['sid']='NOQUEUE';
			$status['date']=$email[1];
			$status['server']=$email[2];
			$status['status']=$email[3];
			$status['status_info']=$email[4];
			$status['from']=$email[5];
			$status['to']=$email[6];
			$status['helo']=$email[7];	
			$values="'".$status['date']."','".$status['status']."','".$status['status_info']."','".$status['from']."','".$status['to']."','".$status['helo']."'";
			$stm_noqueue.='insert into mail_noqueue(date,status,status_info,fromm,too,helo) values('.$values.');'."\n";		
		}
		if ($total_lines%1000 == 0){
			#save log in database
			write_db($stm_noqueue."COMMIT;","noqueue");
			write_db($stm_queue."COMMIT;","queue");
			$stm_noqueue="BEGIN;\n";
			$stm_queue="BEGIN;\n";
		}
	if ($total_lines%1000 == 0)
		print "$line\n";
		}
	#save log in database
	write_db($stm_noqueue."COMMIT;","noqueue");
	write_db($stm_queue."COMMIT;","queue");
	$stm_noqueue="BEGIN;\n";
	$stm_queue="BEGIN;\n";
	}
}

function write_db($stm,$table){
	global $postfix_dir,$postfix_db;
	print date("H:i:s") . " writing db...";
	$dbhandle = sqlite_open($postfix_dir.'/'.$postfix_db, 0666, $error);
	$ok = sqlite_exec($dbhandle, $stm, $error);
	if (!$ok)
		die ("Cannot execute query. $error\n$stm\n");
	
	print "ok ";
	$result = sqlite_query($dbhandle, "select count(*) ".$table." from mail_".$table);
	$row = sqlite_fetch_array($result, SQLITE_ASSOC); 
	print $table .":". $row[$table]."\n";
	sqlite_close($dbhandle);
echo "<br>";		
}

function create_db(){
	global $postfix_dir,$postfix_db,$postfix_arg;
	if ($postfix_arg['time']== "-01 day"){
		unlink_if_exists($postfix_dir.'/'.$postfix_db);
		unlink_if_exists($postfix_dir.'/'.$postfix_db."-journal");
		}
	if (! is_dir($postfix_dir))
		mkdir($postfix_dir,0775);
	$new_db=(file_exists($postfix_dir.'/'.$postfix_db)?1:0);
	$dbhandle = sqlite_open($postfix_dir.'/'.$postfix_db, 0666, $error);
	if (!$dbhandle) die ($error);
$stm = <<<EOF
	CREATE TABLE mail_queue(
	"id" INTEGER PRIMARY KEY,
	"sid" TEXT NOT NULL,
    "client" TEXT NOT NULL,
    "msgid" TEXT,
	"fromm" TEXT,
    "too" TEXT,
    "status" TEXT,
    "size" INTEGER,
    "status_info" TEXT,
    "subject" TEXT,
    "smtp" TEXT,
    "delay" TEXT,
    "relay" TEXT,
    "dsn" TEXT,
    "date" TEXT NOT NULL,
    "server" TEXT,
    "helo" TEXT,
    "bounce" TEXT
);
CREATE TABLE "mail_noqueue"(
	"id" INTEGER PRIMARY KEY,
    "date" TEXT NOT NULL,
    "status" INTEGER NOT NULL,
    "status_info" INTEGER NOT NULL,
    "fromm" TEXT NOT NULL,
    "too" TEXT NOT NULL,
    "helo" TEXT NOT NULL
);

CREATE UNIQUE INDEX "queue_sid" on mail_queue (sid ASC);
CREATE INDEX "queue_bounce" on mail_queue (bounce ASC);
CREATE INDEX "queue_relay" on mail_queue (relay ASC);
CREATE INDEX "queue_client" on mail_queue (client ASC);
CREATE INDEX "queue_helo" on mail_queue (helo ASC);
CREATE INDEX "queue_server" on mail_queue (server ASC);
CREATE INDEX "queue_date" on mail_queue (date ASC);
CREATE INDEX "queue_smtp" on mail_queue (smtp ASC);
CREATE INDEX "queue_subject" on mail_queue (subject ASC);
CREATE INDEX "queue_info" on mail_queue (status_info ASC);
CREATE INDEX "queue_status" on mail_queue (status ASC);
CREATE INDEX "queue_msgid" on mail_queue (msgid ASC);
CREATE INDEX "queue_too" on mail_queue (too ASC);
CREATE INDEX "queue_fromm" on mail_queue (fromm ASC);
CREATE INDEX "noqueue_unique" on mail_noqueue (date ASC, fromm ASC, too ASC);
CREATE INDEX "noqueue_helo" on mail_noqueue (helo ASC);
CREATE INDEX "noqueue_too" on mail_noqueue (too ASC);
CREATE INDEX "noqueue_fromm" on mail_noqueue (fromm ASC);
CREATE INDEX "noqueue_info" on mail_noqueue (status_info ASC);
CREATE INDEX "noqueue_status" on mail_noqueue (status ASC);
CREATE INDEX "noqueue_date" on mail_noqueue (date ASC);
EOF;
if ($new_db==0){
	$ok = sqlite_exec($dbhandle, $stm, $error);
	if (!$ok)
		print ("Cannot execute query. $error\n");
	sqlite_close($dbhandle);
	}
}
function print_html($status="CANNOT_BE_NULL"){
 if (is_array($status)){
 	
 }	
}
$postfix_dir="/var/db/postfix/";
$curr_time = time();
#console script call
if ($argv[1]!=""){
switch ($argv[1]){
	case "10min":
		$postfix_arg=array(	'grep' => array(substr(date("H:i",strtotime('-10 min',$curr_time)),0,-1)),
							'time' => '-10 min');
		break;
	case "01hour":
		$postfix_arg=array(	'grep' => array(date("H:",strtotime('-01 hour',$curr_time))),
							'time' => '-01 hour');
		break;
	case "24hours":
		$postfix_arg=array(	'grep' => array('00:','01:','02:','03:','04:','05:','06:','07:','08:','09:','10:','11:',
											'12:','13:','14:','15:','16:','17:','18:','19:','20:','21:','22:','23:'),
							'time' => '-01 day');
		break;
	case "24hours2":
		$postfix_arg=array(	'grep' => array('00:','01:','02:','03:','04:','05:','06:','07:','08:','09:','10:','11:',
											'12:','13:','14:','15:','16:','17:','18:','19:','20:','21:','22:','23:'),
							'time' => '-02 day');
		break;
	case "24hours3":
		$postfix_arg=array(	'grep' => array('00:','01:','02:','03:','04:','05:','06:','07:','08:','09:','10:','11:',
											'12:','13:','14:','15:','16:','17:','18:','19:','20:','21:','22:','23:'),
							'time' => '-03 day');
		break;
		
		default:
		die ("invalid parameters\n");
}
$postfix_db=date("Y-m-d",strtotime($postfix_arg['time'],$curr_time)).".db";
grep_log();	
}

#http client call
if ($_REQUEST['files']!= ""){
	#do search
	$queue=($_REQUEST['queue']=="QUEUE"?"mail_queue":"mail_noqueue");
	$limit_prefix=(preg_match("/\d+/",$_REQUEST['limit'])?"limit ":"");
	$limit=(preg_match("/\d+/",$_REQUEST['limit'])?$_REQUEST['limit']:"");
	$files= explode(",", $_REQUEST['files']);
	$stm_fetch=array();
	$total_result=0;
	foreach ($files as $postfix_db)
		if (file_exists($postfix_dir.'/'.$postfix_db)){ 
			$last_next="";
			$dbhandle = sqlite_open($postfix_dir.'/'.$postfix_db, 0666, $error);
			$stm='select * from '.$queue;
			if ($_REQUEST['from']!= ""){
				$next=($last_next==" and "?" and ":" where ");
				$last_next=" and ";
				$stm .=$next."fromm in('".$_REQUEST['from']."')";
				}
			if ($_REQUEST['to']!= ""){
				$next=($last_next==" and "?" and ":" where ");
				$last_next=" and ";
				$stm .=$next."too in('".$_REQUEST['to']."')";
				}
			if ($_REQUEST['sid']!= ""){
				$next=($last_next==" and "?" and ":" where ");
				$last_next=" and ";
				$stm .=$next."sid in('".$_REQUEST['sid']."')";
				}
			if ($_REQUEST['subject']!= ""){
				$next=($last_next==" and "?" and ":" where ");
				$last_next=" and ";
				$stm .=$next."subject like '%".$_REQUEST['subject']."%'";
				}
			if ($_REQUEST['msgid']!= ""){
				$next=($last_next==" and "?" and ":" where ");
				$last_next=" and ";
				$stm .=$next."msgid = '".$_REQUEST['msgid']."'";
				}
		if ($_REQUEST['status']!= ""){
				$next=($last_next==" and "?" and ":" where ");
				$last_next=" and ";
				$stm .=$next."status = '".$_REQUEST['status']."'";
				}
			$result = sqlite_query($dbhandle, $stm." order by date desc $limit_prefix $limit ");
			if (preg_match("/\d+/",$_REQUEST['limit'])){
				for ($i = 1; $i <= $limit; $i++) {
					$row = sqlite_fetch_array($result, SQLITE_ASSOC);
					 if (is_array($row))
						$stm_fetch[]=$row;
					}
			}
			else{
				$stm_fetch = sqlite_fetch_all($result, SQLITE_ASSOC);
			}
			sqlite_close($dbhandle);
	}
	$fields= explode(",", $_REQUEST['fields']);
	if ($queue=="mail_noqueue"){
		print '<table class="tabcont" width="100%" border="0" cellpadding="8" cellspacing="0">';
		print '<tr><td colspan="'.count($fields).'" valign="top" class="listtopic">'.gettext("Search Results").'</td></tr>';
		print '<tr>';
		if(in_array("date",$fields))
			print '<td class="listlr"><strong>date</strong></td>';
		if(in_array("from",$fields))
			print '<td class="listlr"><strong>From</strong></td>';
		if(in_array("to",$fields))
			print '<td class="listlr"><strong>to</strong></td>';
		if(in_array("helo",$fields))
			print '<td class="listlr"><strong>Helo</strong></td>';
		if(in_array("status",$fields))
			print '<td class="listlr"><strong>Status</strong></td>';
		if(in_array("status_info",$fields))
			print '<td class="listlr"><strong>Status Info</strong></td>';
		print '</tr>';
		foreach ($stm_fetch as $mail){
			print '<tr>';
		if(in_array("date",$fields))
			print '<td class="listlr">'.$mail['date'].'</td>';
		if(in_array("from",$fields))
			print '<td class="listlr">'.$mail['fromm'].'</td>';
		if(in_array("to",$fields))
			print '<td class="listlr">'.$mail['too'].'</td>';
		if(in_array("helo",$fields))
			print '<td class="listlr">'.$mail['helo'].'</td>';
		if(in_array("status",$fields))
			print '<td class="listlr">'.$mail['status'].'</td>';
		if(in_array("status_info",$fields))
			print '<td class="listlr">'.$mail['status_info'].'</td>';
			print '</tr>';
			$total_result++;
		}
	}
  else{
  		print '<table class="tabcont" width="100%" border="0" cellpadding="8" cellspacing="0">';
		print '<tr><td colspan="'.count($fields).'" valign="top" class="listtopic">'.gettext("Search Results").'</td></tr>';
		print '<tr>';
		if(in_array("date",$fields))
			print '<td class="listlr" ><strong>Date</strong></td>';
		if(in_array("from",$fields))
			print '<td class="listlr" ><strong>From</strong></td>';
		if(in_array("to",$fields))
			print '<td class="listlr" ><strong>to</strong></td>';
		if(in_array("subject",$fields))
			print '<td class="listlr" ><strong>Subject</strong></td>';
		if(in_array("delay",$fields))
			print '<td class="listlr" ><strong>Delay</strong></td>';
		if(in_array("status",$fields))
			print '<td class="listlr" ><strong>Status</strong></td>';
		if(in_array("status_info",$fields))
			print '<td class="listlr" ><strong>Status Info</strong></td>';
		if(in_array("size",$fields))
			print '<td class="listlr" ><strong>Size</strong></td>';
		if(in_array("helo",$fields))
			print '<td class="listlr" ><strong>Helo</strong></td>';
		if(in_array("sid",$fields))
			print '<td class="listlr" ><strong>SID</strong></td>';
		if(in_array("msgid",$fields))
			print '<td class="listlr" ><strong>MSGID</strong></td>';
		if(in_array("bounce",$fields))
			print '<td class="listlr" ><strong>Bounce</strong></td>';
		if(in_array("relay",$fields))
			print '<td class="listlr" ><strong>Relay</strong></td>';
		print '</tr>';
		foreach ($stm_fetch as $mail){
			print '<tr>';
			if(in_array("date",$fields))
				print '<td class="listlr">'.$mail['date'].'</td>';
			if(in_array("from",$fields))
				print '<td class="listlr">'.$mail['fromm'].'</td>';
			if(in_array("to",$fields))
				print '<td class="listlr">'.$mail['too'].'</td>';
			if(in_array("subject",$fields))
				print '<td class="listlr">'.$mail['subject'].'</td>';
			if(in_array("delay",$fields))
				print '<td class="listlr">'.$mail['delay'].'</td>';
			if(in_array("status",$fields))
				print '<td class="listlr">'.$mail['status'].'</td>';
			if(in_array("status_info",$fields))
				print '<td class="listlr">'.$mail['status_info'].'</td>';
			if(in_array("size",$fields))
				print '<td class="listlr">'.$mail['size'].'</td>';
			if(in_array("helo",$fields))
				print '<td class="listlr">'.$mail['helo'].'</td>';
			if(in_array("sid",$fields))
				print '<td class="listlr">'.$mail['sid'].'</td>';
			if(in_array("msgid",$fields))
				print '<td class="listlr">'.$mail['msgid'].'</td>';
			if(in_array("bounce",$fields))
				print '<td class="listlr">'.$mail['bounce'].'</td>';
			if(in_array("relay",$fields))
				print '<td class="listlr">'.$mail['relay'].'</td>';
			print '</tr>';
			$total_result++;
		}
  }
	print '<tr>';
	print '<td ><strong>Total:</strong></td>';
	print '<td ><strong>'.$total_result.'</strong></td>';
	print '</tr>';	
	print '</table>';					
}
?>