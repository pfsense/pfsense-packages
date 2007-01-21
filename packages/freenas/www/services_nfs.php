<?php
/* $Id$ */
/* ========================================================================== */
/*
    services_nfs.php
    part of pfSense (http://www.pfSense.com)
    Copyright (C) 2006 Daniel S. Haischt <me@daniel.stefan.haischt.name>
    All rights reserved.

    Based on FreeNAS (http://www.freenas.org)
    Copyright (C) 2005-2006 Olivier Cochard-Labbé <olivier@freenas.org>.
    All rights reserved.

    Based on m0n0wall (http://m0n0.ch/wall)
    Copyright (C) 2003-2006 Manuel Kasper <mk@neon1.net>.
    All rights reserved.
                                                                              */
/* ========================================================================== */
/*
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
/* ========================================================================== */

$pgtitle = array(gettext("Services"),
                 gettext("NFS"));

require_once("freenas_config.inc");
require_once("guiconfig.inc");
require_once("freenas_guiconfig.inc");
require_once("freenas_functions.inc");

if (!is_array($freenas_config['nfs']))
{
  $freenas_config['nfs'] = array();
}

$pconfig['enable'] = isset($freenas_config['nfs']['enable']);
$pconfig['mapall'] = $freenas_config['nfs']['mapall'];
$pconfig['bindto'] = $freenas_config['nfs']['bindto'];

if (! empty($_POST))
{
  /* hash */
  unset($error_bucket);
  /* simple error list */
  unset($input_errors);
  $pconfig = $_POST;
  
  /* input validation */
  $reqdfields = explode(" ", "authnetworks bindto");
  $reqdfieldsn = explode(",", "Destination network, IP address to bind to");
  
  do_input_validation($_POST, $reqdfields, $reqdfieldsn, &$input_errors);
  
  if (isset($_POST['authnetworks']) && is_array($_POST['authnetworks'])) {
    foreach ($_POST['authnetworks'] as $netel) {
      list($_POST['network'], $_POST['network_subnet']) = explode('/', $netel);
      
      if (($_POST['network'] && !is_ipaddr($_POST['network']))) {
        $error_bucket[] = array("error" => gettext("A valid network must be specified."),
                                "field" => "network");
      }
      
      if (($_POST['network_subnet'] && !is_numeric($_POST['network_subnet']))) {
        $error_bucket[] = array("error" => gettext("A valid network bit count must be specified."),
                                "field" => "network_subnet");
      }
      
      $osn['nfsnetwork'][] = gen_subnet($_POST['network'], $_POST['network_subnet']) . "/" . $_POST['network_subnet'];
    }
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
    $freenas_config['nfs'] = $osn;
    $freenas_config['nfs']['enable'] = $_POST['enable'] ? true : false;
    $freenas_config['nfs']['mapall'] = $_POST['mapall'];
    $freenas_config['nfs']['bindto'] = $_POST['bindto'];
    write_config();
    
    $retval = 0;
    if (!file_exists($d_sysrebootreqd_path))
    {
      /* nuke the cache file */
      config_lock();
      services_nfs_configure();	
      config_unlock();
    }
    $savemsg = get_std_save_message($retval);
  }
}

/* if ajax is calling, give them an update message */
if(isAjax())
  print_info_box_np($savemsg);

include("head.inc");
/* put your custom HTML head content here        */
/* using some of the $pfSenseHead function calls */

$addressTransString = gettext("Address");
$plusimgDescTransString = gettext("add to network list");
$minusimgDescTransString = gettext("remove from network list");
$currentnetTransString = gettext("Current networks");
$networksTypehintTransString = gettext("Network that is authorised to access NFS shares");

$networkCount = count($freenas_config['nfs']['nfsnetwork']);
$generatedWANSubnet = gen_subnet($config['interfaces']['wan']['ipaddr'],
                                 $config['interfaces']['wan']['subnet']);
$generatedLANSubnet = gen_subnet($config['interfaces']['lan']['ipaddr'],
                                 $config['interfaces']['lan']['subnet']);

$jscriptstr = <<<EOD
<script type="text/javascript">
<!--
function network_exists(myValue) {
  for (i = 0; i < $('authnetworks').options.length; i++) {
    if ($('authnetworks').options[i].value == myValue) {
      return true;
    }
  }
  
  return false;
}
function selectnetel() {
  for (i = 0; i < $('authnetworks').options.length; i++) {
    $('authnetworks').options[i].selected = true;
  }
}
function get_selected_listitems() {
  var selected = new Array();
  
  if (!$('authnetworks')) { return; }
  
  for (i = 0; i < $('authnetworks').options.length; i++) {
    if ($('authnetworks').childNodes[i].selected == true) {
      selected.push($('authnetworks').options[i]);
    }
  }

  return selected;
}
  
function add_selnetwork(net, mask) {
  var newOption = document.createElement("option");
  var newOptionText = document.createTextNode(net + '/' + mask);
  var selectSize = $('authnetworks').size;
  
  if (!net || !mask) { return; }
  
  if (network_exists(net + '/' + mask)) {
    alert('Element already exist in the network list');
    return;
  }
  
  newOption.setAttribute('value', net + '/' + mask);
  newOption.appendChild(newOptionText);
  
  /* add the option to the select element */
  $('authnetworks').appendChild(newOption);
  $('authnetworks').setAttribute('size', '3');
  
  /* clear values from HTML fields */
  if ($('network')) { $('network').value = ''; }
  if ($('network_subnet')) { $('network_subnet').selectedIndex = 0; }
}

function remove_selnetwork() {
  var selectedItems = get_selected_listitems();
  
  if (selectedItems.length <= 0) {
    alert('No element selected!');
    return;
  }
  
  for (i = 0; i < selectedItems.length; i++) {
    $('authnetworks').removeChild(selectedItems[i]);
  }
}

function create_authnetworks_tr() {
  var newTR = document.createElement("tr");
  newTR.setAttribute('id', 'authNetworksTR');
  
  var newImageTR = document.createElement("tr");
  newImageTR.setAttribute('id', 'authNetworksImageTR');
  
  var descTD = document.createElement("td");
  descTD.setAttribute('align', 'left');
  descTD.setAttribute('valign', 'top');
  descTD.setAttribute('style', 'padding-top: 10px; border-top: solid 1px grey;');
  
  var selectTD = document.createElement("td");
  selectTD.setAttribute('align', 'left');
  selectTD.setAttribute('valign', 'middle');
  selectTD.setAttribute('style', 'padding-top: 10px; border-top: solid 1px grey;');
  
  var typehintTD = document.createElement("td");
  typehintTD.setAttribute('align', 'left');
  typehintTD.setAttribute('valign', 'middle');
  typehintTD.setAttribute('colspan', '2');
  typehintTD.setAttribute('style', 'padding-top: 10px; border-top: solid 1px grey;');
  
  var blankTD = document.createElement("td");
  blankTD.setAttribute('align', 'left');
  blankTD.setAttribute('valign', 'top');
  
  var imageTD = document.createElement("td");
  imageTD.setAttribute('align', 'left');
  imageTD.setAttribute('valign', 'middle');
  imageTD.setAttribute('style', 'vertical-align: middle;');
  imageTD.setAttribute('colspan', '3');
  
  var newSpan = document.createElement("span");
  newSpan.setAttribute('style', 'padding-left: 5px; vertical-align: middle;');
  
  var newTypehintSpan = document.createElement("span");
  newTypehintSpan.setAttribute('style', 'padding-left: 5px; vertical-align: middle;');
  
  var newDescription = document.createTextNode('{$currentnetTransString}:');
  var newImgDescription = document.createTextNode('{$minusimgDescTransString}');
  var typehint = document.createTextNode('{$networksTypehintTransString}');
  
  var newSelect = document.createElement("select");
  newSelect.setAttribute('name', 'authnetworks[]');
  newSelect.setAttribute('class', 'formselect');
  newSelect.setAttribute('id', 'authnetworks');
  newSelect.setAttribute('multiple', 'multiple');
  newSelect.setAttribute('size', '{$networkCount}');
  
  /* divs are used to achieve proper alignement */
  newImageDiv = document.createElement("div");
  newImageDiv.setAttribute('style', 'float: left;');
  
  newImageDescDiv = document.createElement("div");
  newImageDescDiv.setAttribute('style', 'padding-top: 1px;');
  
  /* try to add each network to the select element */
  
EOD;

if (is_array($freenas_config['nfs']['nfsnetwork'])) {
  foreach ($freenas_config['nfs']['nfsnetwork'] as $networkel) {
    list($netaddress, $netmask) = explode('/', $networkel);
    $networkSanitized = htmlspecialchars($netaddress);
      
    $jscriptstr .= <<<EOD
  var newOption = document.createElement("option");
  var newOptionText = document.createTextNode('{$networkSanitized}/{$netmask}');
    
  newOption.setAttribute('value', '{$networkSanitized}/{$netmask}');
  newOption.appendChild(newOptionText);
    
  /* add the option to the select element */
  newSelect.appendChild(newOption);

EOD;
  } // end if
} // end foreach

$jscriptstr .= <<<EOD
  var newImage = document.createElement("img");
  newImage.setAttribute('src', '/themes/{$g['theme']}/images/misc/bullet_toggle_minus.png');
  newImage.setAttribute('alt', 'remove network');
  newImage.setAttribute('border', '0');
  newImage.setAttribute('style', 'margin-right: 5px; border: solid 1px silver; cursor: pointer;');
  newImage.setAttribute('onclick', 'remove_selnetwork();');
  
  /* assemble everything */
  newTypehintSpan.appendChild(typehint);
  
  descTD.appendChild(newDescription);
  selectTD.appendChild(newSelect);
  typehintTD.appendChild(newTypehintSpan);

  newImageDiv.appendChild(newImage);
  newImageDescDiv.appendChild(newImgDescription);

  imageTD.appendChild(newImageDiv);
  imageTD.appendChild(newImageDescDiv);
  
  newTR.appendChild(descTD);
  newTR.appendChild(selectTD);
  newTR.appendChild(typehintTD);
  
  newImageTR.appendChild(blankTD);
  newImageTR.appendChild(imageTD);
  
  $('networkopttab').appendChild(newTR);
  $('networkopttab').appendChild(newImageTR);
}

function create_network_tr() {
  var newTR = document.createElement("tr");
  newTR.setAttribute('id', 'typeDetailsTR');
  
  var descTD = document.createElement("td");
  descTD.setAttribute('align', 'left');
  descTD.setAttribute('valign', 'top');
  
  var inputTD = document.createElement("td");
  inputTD.setAttribute('align', 'left');
  inputTD.setAttribute('valign', 'top');
  
  var selectTD = document.createElement("td");
  selectTD.setAttribute('align', 'left');
  selectTD.setAttribute('valign', 'middle');
  
  var imageTD = document.createElement("td");
  imageTD.setAttribute('align', 'left');
  imageTD.setAttribute('valign', 'middle');
  
  var newDescription = document.createTextNode('{$addressTransString}:');
  var newImgDescription = document.createTextNode('{$plusimgDescTransString}');
  
  var newInput = document.createElement("input");
  newInput.setAttribute('name', 'network');
  newInput.setAttribute('type', 'text');
  newInput.setAttribute('class', 'formfld host');
  newInput.setAttribute('id', 'network');
  newInput.setAttribute('size', '20');
  newInput.setAttribute('value', '');
  
  var newSelect = document.createElement("select");
  newSelect.setAttribute('name', 'network_subnet');
  newSelect.setAttribute('class', 'formselect');
  newSelect.setAttribute('id', 'network_subnet');
  
  var newImage = document.createElement("img");
  newImage.setAttribute('src', '/themes/{$g['theme']}/images/misc/bullet_toggle_plus.png');
  newImage.setAttribute('alt', 'add network');
  newImage.setAttribute('border', '0');
  newImage.setAttribute('style', 'margin-right: 5px; border: solid 1px silver; cursor: pointer;');
  newImage.setAttribute('onclick', 'add_selnetwork($("network").value, $("network_subnet").value);');
  
  /* divs are used to achieve proper alignement */
  newImageDiv = document.createElement("div");
  newImageDiv.setAttribute('style', 'float: left;');
  
  newImageDescDiv = document.createElement("div");
  newImageDescDiv.setAttribute('style', 'padding-top: 1px;');
  
  /* add options to select */
  for (i = 31; i > 0; i--) {
    var newOption = document.createElement("option");
    var newOptionText = document.createTextNode(i.toString());
    
    newOption.setAttribute('value', i);
    newOption.appendChild(newOptionText);
    
    /* add the option to the select element */
    newSelect.appendChild(newOption);
  }
  
  /* assemble everything */
  descTD.appendChild(newDescription);
  inputTD.appendChild(newInput);
  selectTD.appendChild(newSelect);
  
  newImageDiv.appendChild(newImage);
  newImageDescDiv.appendChild(newImgDescription);
  
  imageTD.appendChild(newImageDiv);
  imageTD.appendChild(newImageDescDiv);
  
  newTR.appendChild(descTD);
  newTR.appendChild(inputTD);
  newTR.appendChild(selectTD);
  newTR.appendChild(imageTD);

  $('networkopttab').appendChild(newTR);
}

function create_wan_tr() {
  var wanSubnet = '{$config['interfaces']['wan']['subnet']}';
  
  var newTR = document.createElement("tr");
  newTR.setAttribute('id', 'typeDetailsTR');
  
  var descTD = document.createElement("td");
  descTD.setAttribute('align', 'left');
  descTD.setAttribute('valign', 'top');
  
  var nettextTD = document.createElement("td");
  nettextTD.setAttribute('align', 'left');
  nettextTD.setAttribute('valign', 'top');
  
  var imageTD = document.createElement("td");
  imageTD.setAttribute('align', 'left');
  imageTD.setAttribute('valign', 'middle');
  imageTD.setAttribute('colspan', '2');
  
  var newDescription = document.createTextNode('{$addressTransString}:');
  var newWanDescription = document.createTextNode('{$generatedWANSubnet}/' + wanSubnet);
  
  var newNetTextSpan = document.createElement("span");
  newNetTextSpan.setAttribute('style', 'font-weight: bold; font-style: italic; vertical-align: middle;');
  
  var newImage = document.createElement("img");
  newImage.setAttribute('src', '/themes/{$g['theme']}/images/misc/bullet_toggle_plus.png');
  newImage.setAttribute('alt', 'add network');
  newImage.setAttribute('border', '0');
  newImage.setAttribute('style', 'margin-right: 5px; border: solid 1px silver; cursor: pointer;');
  newImage.setAttribute('onclick', 'add_selnetwork("{$generatedWANSubnet}", ' + wanSubnet + ');');
  
  /* divs are used to achieve proper alignement */
  newImageDiv = document.createElement("div");
  newImageDiv.setAttribute('style', 'float: left;');
  
  newImageDescDiv = document.createElement("div");
  newImageDescDiv.setAttribute('style', 'padding-top: 1px;');
  
  var newImgDescription = document.createTextNode('{$plusimgDescTransString}');
  
  /* assemble everything */
  newNetTextSpan.appendChild(newWanDescription);
  
  descTD.appendChild(newDescription);
  nettextTD.appendChild(newNetTextSpan);
  
  newImageDiv.appendChild(newImage);
  newImageDescDiv.appendChild(newImgDescription);
  
  imageTD.appendChild(newImageDiv);
  imageTD.appendChild(newImageDescDiv);
  
  newTR.appendChild(descTD);
  newTR.appendChild(nettextTD);
  newTR.appendChild(imageTD);
  
  $('networkopttab').appendChild(newTR);
}

function create_lan_tr() {
  var lanSubnet = '{$config['interfaces']['lan']['subnet']}';
  
  var newTR = document.createElement("tr");
  newTR.setAttribute('id', 'typeDetailsTR');
  
  var descTD = document.createElement("td");
  descTD.setAttribute('align', 'left');
  descTD.setAttribute('valign', 'top');
  
  var nettextTD = document.createElement("td");
  nettextTD.setAttribute('align', 'left');
  nettextTD.setAttribute('valign', 'top');
  
  var imageTD = document.createElement("td");
  imageTD.setAttribute('align', 'left');
  imageTD.setAttribute('valign', 'middle');
  imageTD.setAttribute('colspan', '2');
  
  var newDescription = document.createTextNode('{$addressTransString}:');
  var newLanDescription = document.createTextNode('{$generatedLANSubnet}/' + lanSubnet);
  
  var newNetTextSpan = document.createElement("span");
  newNetTextSpan.setAttribute('style', 'font-weight: bold; font-style: italic; vertical-align: middle;');
  
  var newImage = document.createElement("img");
  newImage.setAttribute('src', '/themes/{$g['theme']}/images/misc/bullet_toggle_plus.png');
  newImage.setAttribute('alt', 'add network');
  newImage.setAttribute('border', '0');
  newImage.setAttribute('style', 'margin-right: 5px; border: solid 1px silver; cursor: pointer;');
  newImage.setAttribute('onclick', 'add_selnetwork("{$generatedLANSubnet}", ' + lanSubnet + ');');
  
  /* divs are used to achieve proper alignement */
  newImageDiv = document.createElement("div");
  newImageDiv.setAttribute('style', 'float: left;');
  
  newImageDescDiv = document.createElement("div");
  newImageDescDiv.setAttribute('style', 'padding-top: 1px;');
  
  var newImgDescription = document.createTextNode('{$plusimgDescTransString}');
  
  /* assemble everything */
  newNetTextSpan.appendChild(newLanDescription);
  
  descTD.appendChild(newDescription);
  nettextTD.appendChild(newNetTextSpan);
  
  newImageDiv.appendChild(newImage);
  newImageDescDiv.appendChild(newImgDescription);
  
  imageTD.appendChild(newImageDiv);
  imageTD.appendChild(newImageDescDiv);
  
  newTR.appendChild(descTD);
  newTR.appendChild(nettextTD);
  newTR.appendChild(imageTD);
  
  $('networkopttab').appendChild(newTR);
}

function get_optnetwork() {
  var slashIndex= $('opt_iface_desc').firstChild.nodeValue.indexOf('/');
  var myNetwork = $('opt_iface_desc').firstChild.nodeValue.substring(0, slashIndex);

  return myNetwork;
}

function get_optsubnet() {
  var slashIndex= $('opt_iface_desc').firstChild.nodeValue.indexOf('/');
  var mySubnet = $('opt_iface_desc').firstChild.nodeValue.substring(slashIndex + 1, $('opt_iface_desc').firstChild.nodeValue.length);
  
  return mySubnet;
}

function create_opt_tr() {
  var newTR = document.createElement("tr");
  newTR.setAttribute('id', 'typeDetailsTR');
  
  var descTD = document.createElement("td");
  descTD.setAttribute('align', 'left');
  descTD.setAttribute('valign', 'top');
  
  var nettextTD = document.createElement("td");
  nettextTD.setAttribute('align', 'left');
  nettextTD.setAttribute('valign', 'top');
  
  var imageTD = document.createElement("td");
  imageTD.setAttribute('align', 'left');
  imageTD.setAttribute('valign', 'middle');
  imageTD.setAttribute('colspan', '2');
  
  var newDescription = document.createTextNode('{$addressTransString}:');
  var newImgDescription = document.createTextNode('{$plusimgDescTransString}');
  
  var newImage = document.createElement("img");
  newImage.setAttribute('src', '/themes/{$g['theme']}/images/misc/bullet_toggle_plus.png');
  newImage.setAttribute('alt', 'add network');
  newImage.setAttribute('border', '0');
  newImage.setAttribute('style', 'margin-right: 5px; border: solid 1px silver; cursor: pointer;');
  newImage.setAttribute('onclick', 'add_selnetwork(get_optnetwork(), get_optsubnet());');
  
  /* divs are used to achieve proper alignement */
  newImageDiv = document.createElement("div");
  newImageDiv.setAttribute('style', 'float: left;');
  
  newImageDescDiv = document.createElement("div");
  newImageDescDiv.setAttribute('style', 'padding-top: 1px;');
  
  /* add options to select */

EOD;

for ($i = 1; isset($config['interfaces']['opt' . $i]); $i++) {
  $optSubnet = $config['interfaces']['opt' . $i]['subnet'];
  $generatedOPTSubnet = gen_subnet($config['interfaces']['opt' . $i]['ipaddr'],
                                   $config['interfaces']['opt' . $i]['subnet']);
  
  if (empty($optSubnet) || empty($generatedOPTSubnet)) { continue; }

  $jscriptstr .= <<<EOD
  var selOptIndex = $('authnettype').selectedIndex;
  var optNumber = $('authnettype').options[selOptIndex].value.substr(3, $('authnettype').options[selOptIndex].value.length);
  var newOptDescription = null;
  var newNetTextSpan = null;

  if (optNumber == {$i}) {
    newOptDescription = document.createTextNode('{$generatedOPTSubnet}/{$optSubnet}');
    
    newNetTextSpan = document.createElement("span");
    newNetTextSpan.setAttribute('id', 'opt_iface_desc');
    newNetTextSpan.setAttribute('style', 'font-weight: bold; font-style: italic; vertical-align: middle;');
  }

EOD;
}
  
$jscriptstr .= <<<EOD
  /* assemble everything */
  if (newNetTextSpan)
    newNetTextSpan.appendChild(newOptDescription);
  
  descTD.appendChild(newDescription);
  if (newNetTextSpan)
    nettextTD.appendChild(newNetTextSpan);
  
  newImageDiv.appendChild(newImage);
  newImageDescDiv.appendChild(newImgDescription);
  
  imageTD.appendChild(newImageDiv);
  imageTD.appendChild(newImageDescDiv);
  
  newTR.appendChild(descTD);
  newTR.appendChild(nettextTD);
  newTR.appendChild(imageTD);
  
  $('networkopttab').appendChild(newTR);
}

function authnet_change() {
  if ($('typeDetailsTR')) { $('networkopttab').removeChild($('typeDetailsTR')); }
  if ($('authNetworksTR')) { $('networkopttab').removeChild($('authNetworksTR')); }
  if ($('authNetworksImageTR')) { $('networkopttab').removeChild($('authNetworksImageTR')); }
  
  switch ($('authnettype').selectedIndex) {
    case 0:
      /* Network */
      create_network_tr();
      break;
    case 1:
      /* WAN subnet */
      create_wan_tr();
      break;
    case 2:
      /* LAN subnet */
      create_lan_tr();
      break;

EOD;

for ($i = 1; isset($config['interfaces']['opt' . $i]); $i++) {
  $case_count = 2 + $i;

  $jscriptstr .= <<<EOD
    case {$case_count}:
      /* OPT subnet */
      create_opt_tr();
      break;

EOD;

} // end for

$jscriptstr .= <<<EOD
  } // end switch
  
  create_authnetworks_tr();
} // end function authnet_change

function enable_change(enable_change) {
  var endis;
  
  endis = !(document.iform.enable.checked || enable_change);
  endis ? color = '#D4D0C8' : color = '#FFFFFF';
  
  document.iform.mapall.disabled = endis;
  document.iform.authnettype.disabled = endis;
  /* color adjustments */
  document.iform.mapall.style.backgroundColor = color;
  document.iform.authnettype.style.backgroundColor = color;
}
//-->
</script>

EOD;

$pfSenseHead->addScript($jscriptstr);
echo $pfSenseHead->getHTML();

?>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC" onload="<?= $jsevents["body"]["onload"] ?>">
<?php include("fbegin.inc"); ?>

<?php if ($input_errors) print_input_errors($input_errors); ?>
<?php if ($savemsg) print_info_box($savemsg); ?>
  <div id="inputerrors"></div>
  <form id="iform" name="iform" action="services_nfs.php" method="post">
    <table width="100%" border="0" cellpadding="6" cellspacing="0">
      <tr>
        <td width="100%" valign="middle" class="listtopic" colspan="2">
          <span style="vertical-align: middle; position: relative; left: 0px;"><?=gettext("NFS Server");?></span>
          <span style="vertical-align: middle; position: relative; left: 84%;">
            <input name="enable" type="checkbox" value="yes" <?php if ($pconfig['enable']) echo "checked=\"checked\""; ?> onClick="enable_change(false)" />&nbsp;<?= gettext("Enable"); ?>
          </span>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncellreq"><?=gettext("Bind to IP address");?></td>
        <td width="78%" class="vtable" align="left" valign="middle">
          <select name="bindto" id="bindto" class="formselect">
            <option value="<?= $config['interfaces']['lan'][ipaddr] ?>">
              <?= $config['interfaces']['wan'][ipaddr] ?>
            </option>
            <option value="<?= $config['interfaces']['lan'][ipaddr] ?>">
              <?= $config['interfaces']['lan'][ipaddr] ?>
            </option>
            <?php
              for ($i = 1; isset($config['interfaces']['opt' . $i]); $i++):
            ?>
            <option value="<?= $config['interfaces']['opt' . $i][ipaddr] ?>">
              <?= $config['interfaces']['opt' . $i][ipaddr] ?>
            </option>
            <?php endfor; ?>
          </select>
          <br />
          <?= gettext("Use an address from the list to make nfsd and rpcbind bind to a specific address."); ?>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncellreq"><?=gettext("map all user to root");?></td>
        <td width="78%" class="vtable">
          <select name="mapall" class="formselect" id="mapall">
            <?php
              $types = explode(",", "Yes,No");
              $vals = explode(" ", "yes no");
              $j = 0;
              
              for ($j = 0; $j < count($vals); $j++):
            ?>
            <option value="<?=$vals[$j];?>" <?php if ($vals[$j] == $pconfig['mapall']) echo "selected=\"selected\"";?>> 
            <?=htmlspecialchars($types[$j]);?>
            </option>
            <?php endfor; ?>
          </select>
          <br />
          <?= gettext("All users will have the root privilege."); ?>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top" class="vncellreq"><?=gettext("Authorised network");?></td>
        <td width="78%" class="vtable" align="left" valign="middle">
          <table border="0" cellspacing="0" cellpadding="4" id="networkopttab">
            <tr>
              <td align="left" valign="middle"><?=gettext("Type");?>:</td>
              <td align="left" valign="middle" colspan="4">
                <select name="authnettype" id="authnettype" class="formselect" onchange="authnet_change();">
                  <option value="network">
                    <?=gettext("Network")?>
                  </option>
                  <option value="wan">
                    <?=gettext("WAN subnet");?>
                  </option>
                  <option value="lan">
                    <?=gettext("LAN subnet");?>
                  </option>
                  <?php
                    for ($i = 1; isset($config['interfaces']['opt' . $i]); $i++):
                  ?>
                  <option value="opt<?=$i;?>">
                    <?=htmlspecialchars($config['interfaces']['opt' . $i]['descr']);?>
                    <?=gettext("subnet");?>
                  </option>
                  <?php endfor; ?>
                </select>
              </td>
            </tr>
          </table>
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top">&nbsp;</td>
        <td width="78%">
          <!-- Note: Cause Prototype is observing the onclick event, we are using onmousedown and onkeydown instead -->
          <input id="submit" name="Submit" type="submit" class="formbtn" value="<?=gettext("Save");?>" onmousedown="selectnetel();" onkeydown="selectnetel();" />
        </td>
      </tr>
      <tr>
        <td width="22%" valign="top">&nbsp;</td>
        <td width="78%">
          <span class="red">
            <strong><?= gettext("NOTE"); ?>:</strong>
          </span>
          <br />
          <ul>
            <li>
              <span class="vexpl">
                <?= gettext("The name of each exported directory is: /mnt/sharename"); ?>
              </span>
            </li>
            <li>
              <span class="vexpl">
                <?= gettext("Try adding networks to the 'current networks' list to authorize each particular network."); ?>
              </span>
            </li>
            <li>
              <span class="vexpl">
                <?= gettext("Use ctrl-click (or command-click on the Mac) to select and de-select elements from the 'current networks' list."); ?>
              </span>
            </li>
          </ul>
        </td>
      </tr>
    </table>
  </form>
<?php include("fend.inc"); ?>
<?= checkForInputErrors(); ?>
<script type="text/javascript">
<!--
enable_change(false);
authnet_change();
//-->
</script>
</body>
</html>
