<?php
/* $Id$ */
/*
	disks_manage_edit.php
	part of FreeNAS (http://www.freenas.org)
	Copyright (C) 2005-2006 Olivier Cochard-Labbé <olivier@freenas.org>.
	All rights reserved.
	
	Based on m0n0wall (http://m0n0.ch/wall)
	Copyright (C) 2003-2006 Manuel Kasper <mk@neon1.net>.
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

$pgtitle = array(gettext("Services"),
                 gettext("RSYNCD"),
                 gettext("Client"));

require_once("guiconfig.inc");
require_once("freenas_guiconfig.inc");
require_once("freenas_functions.inc");

/* TODO: use pfSense users/groups. */
if (!is_array($freenas_config['access']['user']))
	$freenas_config['access']['user'] = array();
	
users_sort();

$a_user = &$freenas_config['access']['user'];

if (!is_array($freenas_config['rsync']))
{
	$freenas_config['rsync'] = array();
}

$a_months = array("January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December");
$a_weekdays = array("Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday");

$pconfig['readonly'] = $freenas_config['rsyncd']['readonly'];
$pconfig['port'] = $freenas_config['rsyncd']['port'];
$pconfig['motd'] = $freenas_config['rsyncd']['motd'];
$pconfig['maxcon'] = $freenas_config['rsyncd']['maxcon'];
$pconfig['rsyncd_user'] = $freenas_config['rsyncd']['rsyncd_user'];
$pconfig['enable'] = isset($freenas_config['rsyncd']['enable']);

if (! empty($_POST))
{
  /* hash */
  unset($error_bucket);
  /* simple error list */
  unset($input_errors);
  unset($do_format);
  $pconfig = $_POST;

  /* input validation */
  if ($_POST['enable']){
    $reqdfields = array_merge($reqdfields, explode(" ", "rsyncserverip sharetosync"));
    $reqdfieldsn = array_merge($reqdfieldsn, explode(",", "Rsyncserverip,Sharetosync"));
  }
  
  do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);
	
	if ($_POST['enable']) {
		if (!is_port($_POST['port']))
      $error_bucket[] = array("error" => gettext("The TCP port must be a valid port number."),
                              "field" => "port");
		else if (!is_numericint($_POST['maxcon']))
      $error_bucket[] = array("error" => gettext("The value provided by the maximum connections field is not a number"),
                              "field" => "maxcon");
	}
  
  if (!is_array($freenas_config['mounts']['mount'])) {
    $error_bucket[] = array("error" => gettext("You must configure mount point first."),
                            "field" => "none");
  }

  if (is_array($error_bucket))
    foreach($error_bucket as $elem)
      $input_errors[] =& $elem["error"];
  
  /* if this is an AJAX caller then handle via JSON */
  if(isAjax() && is_array($error_bucket)) {
      input_errors2Ajax(NULL, $error_bucket);       
      exit;   
  }
  
	if (!$input_errors)
	{
		$freenas_config['rsyncclient']['opt_delete'] = $_POST['opt_delete'] ? true : false;;
  	$freenas_config['rsyncclient']['rsyncserverip'] = $_POST['rsyncserverip'];
  	$freenas_config['rsyncclient']['minute'] = $_POST['minutes'];
  	$freenas_config['rsyncclient']['hour'] = $_POST['hours'];
  	$freenas_config['rsyncclient']['day'] = $_POST['days'];
  	$freenas_config['rsyncclient']['month'] = $_POST['months'];
  	$freenas_config['rsyncclient']['weekday'] = $_POST['weekdays'];
  	$freenas_config['rsyncclient']['rsyncshare'] = $_POST['rsyncshare'];
  	$freenas_config['rsyncclient']['enable'] = $_POST['enable'] ? true : false;
  	$freenas_config['rsyncclient']['sharetosync'] = $_POST['sharetosync'];
  	$freenas_config['rsyncclient']['all_mins'] = $_POST['all_mins'];
  	$freenas_config['rsyncclient']['all_hours'] = $_POST['all_hours'];
  	$freenas_config['rsyncclient']['all_days'] = $_POST['all_days'];
  	$freenas_config['rsyncclient']['all_months'] = $_POST['all_months'];
  	$freenas_config['rsyncclient']['all_weekdays'] = $_POST['all_weekdays'];
		
		write_config();
		
		$retval = 0;
  
  	if (!file_exists($d_sysrebootreqd_path)){
  		/* nuke the cache file */
  		config_lock();
  		services_rsyncclient_configure();
  		services_cron_configure();
  		config_unlock();
  	}
  		
  	$savemsg = get_std_save_message($retval);
	}
}

if (is_array($freenas_config['mounts']['mount'])) {
 	mount_sort();
  $a_mount = &$freenas_config['mounts']['mount'];
  
	$pconfig['opt_delete'] = isset($freenas_config['rsyncclient']['opt_delete']);
	$pconfig['enable'] = isset($freenas_config['rsyncclient']['enable']);
	$pconfig['rsyncserverip'] = $freenas_config['rsyncclient']['rsyncserverip'];
	$pconfig['rsyncshare'] = $freenas_config['rsyncclient']['rsyncshare'];
	$pconfig['minute'] = $freenas_config['rsyncclient']['minute'];
	$pconfig['hour'] = $freenas_config['rsyncclient']['hour'];
	$pconfig['day'] = $freenas_config['rsyncclient']['day'];
	$pconfig['month'] = $freenas_config['rsyncclient']['month'];
	$pconfig['weekday'] = $freenas_config['rsyncclient']['weekday'];
	$pconfig['sharetosync'] = $freenas_config['rsyncclient']['sharetosync'];
	$pconfig['all_mins'] = $freenas_config['rsyncclient']['all_mins'];
	$pconfig['all_hours'] = $freenas_config['rsyncclient']['all_hours'];
	$pconfig['all_days'] = $freenas_config['rsyncclient']['all_days'];
	$pconfig['all_months'] = $freenas_config['rsyncclient']['all_months'];
	$pconfig['all_weekdays'] = $freenas_config['rsyncclient']['all_weekdays'];
	
  if ($pconfig['all_mins'] == 1){
   $all_mins_all = " checked";
  } else {
   $all_mins_selected = " checked";
  }
  
  if ($pconfig['all_hours'] == 1){
   $all_hours_all = " checked";
  } else {
   $all_hours_selected = " checked";
  }
      
  if ($pconfig['all_days'] == 1){
   $all_days_all = " checked";
  } else {
   $all_days_selected = " checked";
  }
  
  if ($pconfig['all_months'] == 1){
   $all_months_all = " checked";
  } else {
   $all_months_selected = " checked";
  }
  
  if ($pconfig['all_weekdays'] == 1){
   $all_weekdays_all = " checked";
  } else {
   $all_weekdays_selected = " checked";
  }
}

include("head.inc");
/* put your custom HTML head content here        */
/* using some of the $pfSenseHead function calls */

$jscriptstr = <<<EOD
<script type="text/javascript">
<!--
function enable_change(enable_change) {
	var endis;
	
	endis = !(document.iform.enable.checked || enable_change);
  endis ? color = '#D4D0C8' : color = '#FFFFFF';

EOD;

$i = 0;

if (is_array($a_mount)) {
  foreach ($a_mount as $mountv) {
    $jscriptstr .= "  document.iform.share_" . $i . ".disabled = endis;\n";
    $i++;
  }
}

$jscriptstr .= <<<EOD

	document.iform.rsyncserverip.disabled = endis;
	document.iform.minutes1.disabled = endis;
	document.iform.minutes2.disabled = endis;
	document.iform.minutes3.disabled = endis;
	document.iform.hours1.disabled = endis;
	document.iform.days1.disabled = endis;
	document.iform.days2.disabled = endis;
	document.iform.months.disabled = endis;
	document.iform.weekdays.disabled = endis;
	document.iform.all_mins1.disabled = endis;
	document.iform.all_mins2.disabled = endis;
	document.iform.all_hours1.disabled = endis;
	document.iform.all_hours2.disabled = endis;
	document.iform.all_days1.disabled = endis;
	document.iform.all_days2.disabled = endis;
	document.iform.all_months1.disabled = endis;
	document.iform.all_months2.disabled = endis;
	document.iform.all_weekdays1.disabled = endis;
	document.iform.all_weekdays2.disabled = endis;
	document.iform.opt_delete.disabled = endis;
  /* color adjustments */	
	document.iform.rsyncserverip.style.backgroundColor = color;
	document.iform.minutes1.style.backgroundColor = color;
	document.iform.minutes2.style.backgroundColor = color;
	document.iform.minutes3.style.backgroundColor = color;
	document.iform.hours1.style.backgroundColor = color;
	document.iform.days1.style.backgroundColor = color;
	document.iform.days2.style.backgroundColor = color;
	document.iform.months.style.backgroundColor = color;
	document.iform.weekdays.style.backgroundColor = color;
	document.iform.all_mins1.style.backgroundColor = color;
	document.iform.all_mins2.style.backgroundColor = color;
	document.iform.all_hours1.style.backgroundColor = color;
	document.iform.all_hours2.style.backgroundColor = color;
	document.iform.all_days1.style.backgroundColor = color;
	document.iform.all_days2.style.backgroundColor = color;
	document.iform.all_months1.style.backgroundColor = color;
	document.iform.all_months2.style.backgroundColor = color;
	document.iform.all_weekdays1.style.backgroundColor = color;
	document.iform.all_weekdays2.style.backgroundColor = color;
	document.iform.opt_delete.style.backgroundColor = color;
}
//-->
</script>

EOD;

$pfSenseHead->addScript($jscriptstr);
echo $pfSenseHead->getHTML();

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<?php include("fbegin.inc"); ?>

<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>
<div id="inputerrors"></div> 
<table width="100%" border="0" cellspacing="0" cellpadding="0">
  <tr>
    <td>
<?php
  $tab_array = array();
	$tab_array[0] = array(gettext("Server"), false, "services_rsyncd.php");
	$tab_array[1] = array(gettext("Client"), true,  "services_rsyncd_client.php");
  display_top_tabs($tab_array);
?>
    </td>
  </tr>
  <tr>
    <td>
      <div id="mainarea">
      <form id="iform" name="iform" action="services_rsyncd_client.php" method="post">
      <table class="tabcont" align="center" width="100%" border="0" cellpadding="6" cellspacing="0">
          <tr>
            <td width="100%" valign="middle" class="listtopic" colspan="2">
              <span style="vertical-align: middle; position: relative; left: 0px;"><?=gettext("Rsync Client Synchronization");?></span>
              <span style="vertical-align: middle; position: relative; left: 70%;">
                <input name="enable" type="checkbox" value="yes" <?php if ($pconfig['enable']) echo "checked=\"checked\""; ?> onClick="enable_change(false)" />&nbsp;<?= gettext("Enable"); ?>
              </span>
            </td>
          </tr>
          <tr>
            <td width="16%" valign="top" class="vncellreq"><?= gettext("Remote RSYNC Server"); ?></td>
            <td width="84%" class="vtable">
              <input name="rsyncserverip" id="rsyncserverip" type="text" class="formfld unknown" size="20" value="<?=htmlspecialchars($pconfig['rsyncserverip']);?>" /> 
              <br />
              <?= gettext("IP address of remote RSYNC server"); ?>
            </td>
          </tr>
          <tr>
            <td width="16%" valign="top" class="vncellreq"><?= gettext("RSYNC Options"); ?></td>
            <td width="84%" class="vtable">
              <input name="opt_delete" id="opt_delete" type="checkbox" value="yes" <?php if ($pconfig['opt_delete']) echo "checked=\"checked\""; ?> />
              <?= gettext("Delete files that don't exist on sender."); ?>
            </td>
          </tr>
          <tr>
            <td width="16%" valign="top" class="vncellreq"><?= gettext("Shares to be synchronized"); ?></td>
            <td width="84%" class="vtable">
              <?php 
                $i=0;
                if (is_array($freenas_config['mounts']['mount'])) {
                	foreach ($a_mount as $mountv) {  
                		echo "<input name=\"sharetosync[]\" id=\"share_" . $i  . "\" type=\"checkbox\" value=\"" . $mountv['sharename'] . "\"";
                		
                		if (in_array($mountv['sharename'], $pconfig['sharetosync']))
                	 		echo " checked=\"checked\"";
                		echo" />";
                		echo $mountv['sharename'] . " (" . $mountv['desc'] . ")<br />\n";
                		$i++;
                	}
                }
                else
                	echo "You must configure mount point before!";
              ?>
              <br />
              <?= gettext("This same local share must be present on the Remote Server."); ?>
            </td>
          </tr>
          <tr>
            <td width="16%" valign="top" class="vncellreq"><?= gettext("Synchronization Time"); ?></td>
            <td width="84%" class="vtable">
              <table width="100%" border="1" cellpadding="4" cellspacing="0">
                <tr>
                  <td align="left" valign="top" class="listtopic"><?= gettext("minutes"); ?></td>
                  <td align="left" valign="top" class="listtopic"><?= gettext("hours"); ?></td>
                  <td align="left" valign="top" class="listtopic"><?= gettext("days"); ?></td>
                  <td align="left" valign="top" class="listtopic"><?= gettext("months"); ?></td>
                  <td align="left" valign="top" class="listtopic"><?= gettext("week days"); ?></td>
                </tr>
                <tr>
                  <td align="left" valign="top" class="vncell" nowrap="nowrap">
                    <div id="all_min_rdbtns" style="padding-bottom: 10px;" >
                      <input type="radio" name="all_mins" id="all_mins1" value="1"<?php echo $all_mins_all;?> />
                      <label for="all_mins1">All</label><br />
                      <input type="radio" name="all_mins" id="all_mins2" value="0"<?php echo $all_mins_selected;?> />
                      <label for="all_mins2">Selected ...</label>
                    </div>
                    <div id="all_min_select" style="vertical-align: top;">
											<select multiple="multible" class="formselect" size="24" name="minutes[]" id="minutes1" style="vertical-align: top;">
                        <?php
													$i = 0;
											  	while ($i <= 23){					 
														if (isset($pconfig['minute'])) {
                              if (in_array($i, $pconfig['minute'])) {
                                $is_selected = " selected";
  														} else {
  															$is_selected = "";
  														}
														}
																		
													  echo "<option value=\"" . $i . "\"" . $is_selected . ">" . $i . "\n";
													  $i++;
													}
												?>
                      </select>
											<select multiple="multible" class="formselect" size="24" name="minutes[]" id="minutes2" style="vertical-align: top;">
                        <?php
													$i = 24;
												  while ($i <= 47) {
														if (isset($pconfig['minute'])) {
  													  if (in_array($i, $pconfig['minute'])) {
                                $is_selected = " selected";
  														} else {
  															$is_selected = "";
  														}
														}
																		
														echo "<option value=\"" . $i . "\"" . $is_selected . ">" . $i . "\n";
														$i++;
													}
												?>
                      </select>
                      <select multiple="multible" class="formselect" size="12" name="minutes[]" id="minutes3" style="vertical-align: top;">
                        <?php
                          $i = 48;
                          while ($i <= 59) {
                            if (isset($pconfig['minute'])) {
                              if (in_array($i, $pconfig['minute'])) {
                                $is_selected = " selected";
                              } else {
                                $is_selected = "";
                              }
                            }
                          
                            echo "<option value=\"" . $i . "\"" . $is_selected . ">" . $i . "\n";
                            $i++;
                          }
                        ?>
                      </select>
                    </div>
                  </td>
                  <td align="left" valign="top" class="vncell" nowrap="nowrap">
                    <div id="all_hours_rdbtns" style="padding-bottom: 10px;" >
                      <input type="radio" name="all_hours" id="all_hours1" value="1"<?php echo $all_hours_all;?> />
                      <label for="all_hours1">All</label><br />
                      <input type="radio" name="all_hours" id="all_hours2" value="0"<?php echo $all_hours_selected;?> />
                      <label for="all_hours2">Selected ...</label>
                    </div>
                    <div id="all_hours_select" style="vertical-align: top;">
                      <select multiple size="24" name="hours[]" id="hours1" style="vertical-align: top;">
                        <?php
                          $i = 0;
                          while ($i <= 23) {                           
                            if (isset($pconfig['hour'])) {
                              if (in_array($i, $pconfig['hour'])) {
                                $is_selected = " selected";
                              } else {
                                $is_selected = "";
                              }
                            }
                            
                            echo "<option value=\"" . $i . "\"" . $is_selected . ">" . $i . "\n";
                            $i++;
                          }
                        ?>
                      </select>
                    </div>
                  </td>
                  <td align="left" valign="top" class="vncell" nowrap="nowrap">
                    <div id="all_days_rdbtns" style="padding-bottom: 10px;" >
                      <input type="radio" name="all_days" id="all_days1" value="1" <?php echo $all_days_all;?> />
                      <label for="all_days1">All</label><br />
                      <input type="radio" name="all_days" id="all_days2" value="0"<?php echo $all_days_selected;?> />
                      <label for="all_days2">Selected ...</label>
                    </div>
                    <div id="all_days_select" style="vertical-align: top;">
                      <select multiple size="24" name="days[]" id="days1" style="vertical-align: top;">
                        <?php
                          $i = 1;
                          while ($i <= 24) { 
                            if (isset($pconfig['day'])) {
                              if (in_array($i, $pconfig['day'])) {
                                $is_selected = " selected";
                              } else {
                                $is_selected = "";
                              }
                            }
                            
                            echo "<option value=\"" . $i . "\"" . $is_selected . ">" . $i . "\n";
                            $i++;
                          }
                        ?>
                      </select>
                      <select multiple size="7" name="days[]" id="days2" style="vertical-align: top;">
                        <?php
                          $i = 25;
                          while ($i <= 31) {                             
                            if (isset($pconfig['day'])) {
                              if (in_array($i, $pconfig['day'])) {
                                $is_selected = " selected";
                              } else {
                                $is_selected = "";
                              }
                            }
                            
                            echo "<option value=\"" . $i . "\"" . $is_selected . ">" . $i . "\n";
                            $i++;
                          }
                        ?>
                      </select>
                    </div>
                  </td>
                  <td align="left" valign="top" class="vncell">
                    <div id="all_months_rdbtns" style="padding-bottom: 10px;" >
                      <input type="radio" name="all_months" id="all_months1" value="1"<?php echo $all_months_all;?> />
                      <label for="all_months1">All</label><br />
                      <input type="radio" name="all_months" id="all_months2" value="0"<?php echo $all_months_selected;?> />
                      <label for="all_months2">Selected ...</label>
                    </div>
                    <div id="all_months_select" style="vertical-align: top;">
                      <select multiple size="12" name="months[]" id="months" style="vertical-align: top;">                      
                        <?php
                          $i=1;
                          foreach ($a_months as $monthv) { 
                            if (isset($pconfig['month'])) {
                              if (in_array($i, $pconfig['month'])) {
                                $is_selected = " selected";
                              } else {
                                $is_selected = "";
                              }
                            }
                            
                            echo "<option value=\"" . $i . "\"" . $is_selected . ">" . $monthv . "\n";
                            $i++;
                          }
                        ?>
                      </select>
                    </div>
                  </td>
                  <td align="left" valign="top" class="vncell">
                    <div id="all_weekdays_rdbtns" style="padding-bottom: 10px;" >
                      <input type="radio" name="all_weekdays" id="all_weekdays1" value="1"<?php echo $all_weekdays_all;?> />
                      <label for="all_weekdays1">All</label><br />
                      <input type="radio" name="all_weekdays" id="all_weekdays2" value="0"<?php echo $all_weekdays_selected;?> />
                      <label for="all_weekdays2">Selected ...</label>
                    </div>
                    <div id="all_weekdays_select" style="vertical-align: top;">
                      <select multiple size="7" name="weekdays[]" id="weekdays" style="vertical-align: top;">
                        <?php
                          $i=0;
                          foreach ($a_weekdays as $weekdayv) {
                            if (isset($pconfig['weekday'])){
                              if (in_array($i, $pconfig['weekday'])) {
                                $is_selected = " selected";
                              } else {
                                $is_selected = "";
                              }
                            }
                            
                            echo "<option value=\"" . $i . "\"" . $is_selected . ">" . $weekdayv . "\n";
                            $i++;
                          }
                        ?>
                      </select>
                    </div>
                  </td>
                </tr>
                <tr>
                  <td align="left" valign="top" class="vncell" colspan="5">
                    <?= gettext("Note: Ctrl-click (or command-click on the Mac) to select and de-select minutes, hours, days and months."); ?>
                  </td>
                </tr>
              </table>
            </td>
          </tr>
          <tr>
            <td width="16%" valign="top">&nbsp;</td>
            <td width="84%">
              <input id="submit" name="Submit" type="submit" class="formbtn" value="<?=gettext("Save");?>" />
            </td>
          </tr>
        </table>
        </form>
      </div>
    </td>
	</tr>
</table>
<?php include("fend.inc"); ?>
<?= checkForInputErrors(); ?>
<script type="text/javascript">
<!--
enable_change(false);
//-->
</script>
</body>
</html>
