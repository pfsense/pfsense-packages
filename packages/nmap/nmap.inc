
function nmap_custom_add_php_command() {
	$nmap_options = "";
	if($_POST['option'] == "-sT: TCP connect() scan.") $nmap_options .= " -sT";
	if($_POST['option'] == "-sP: Ping scanning") $nmap_options .= " -sP";
	if($_POST['option'] == "-sU: UDP Scanning") $nmap_options .= " -sU";
	if($_POST['noping']) $nmap_options .= " -P0";
	if($_POST['servicever']) $nmap_options .= " -sV";
	if($_POST['osdetect']) $nmap_options .= " -O";
	    $nmap_options .= " " . $_POST['hostname'];
	    system("/usr/local/bin/nmap" . $nmap_options);
}


