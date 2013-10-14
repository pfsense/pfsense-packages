<?php
/* $Id$ */
/*
	ossec.inc
	Copyright (C) 2012 Lance Leger
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

require_once('globals.inc');
require_once('config.inc');
require_once('util.inc');
require_once('pfsense-utils.inc');
require_once('pkg-utils.inc');
require_once('service-utils.inc');

if(!function_exists("filter_configure")) 
	require_once("filter.inc");

define('OSSEC_BASE', '/usr/pbi/ossec-hids-i386/ossec-hids/');
define('OSSEC_RULE_DIR', OSSEC_BASE . 'rules/');
define('OSSEC_ETC_DIR', OSSEC_BASE . 'etc/');

function ossec_install_command() {

}

function ossec_deinstall_command() {
        exec("rm -rf /usr/local/ossec-hids");
        exec("pw user del ossec");
        exec("pw user del ossecm");
        exec("pw user del ossecr");
        filter_configure();
}

// Courtesy of TJ at devnet http://recursive-design.com/blog/2007/04/05/format-xml-with-php/
function ossec_format_xml($xml) {

	// add marker linefeeds to aid the pretty-tokeniser (adds a linefeed between all tag-end boundaries)
	$xml = preg_replace('/(>)(<)(\/*)/', "$1\n$2$3", $xml);

	// now indent the tags
	$token      = strtok($xml, "\n");
	$result     = ''; // holds formatted version as it is built
	$pad        = 0; // initial indent
	$matches    = array(); // returns from preg_matches()

	// scan each line and adjust indent based on opening/closing tags
	while ($token !== false) :

	// test for the various tag states

	// 1. open and closing tags on same line - no change
	if (preg_match('/.+<\/\w[^>]*>.?$/', $token, $matches)) :
	$indent=0;
	// 2. closing tag - outdent now
	elseif (preg_match('/^<\/\w/', $token, $matches)) :
	$pad-=4;
	$indent = 0;
	// 3. opening tag - don't pad this one, only subsequent tags
	elseif (preg_match('/^<\w[^>]*[^\/]>.*$/', $token, $matches)) :
	$indent=4;
	// 4. no indentation needed
	else :
	$indent = 0;
	endif;

	// pad the line with the required number of leading spaces
	$line    = str_pad($token, strlen($token)+$pad, ' ', STR_PAD_LEFT);
	$result .= $line . "\n"; // add to the cumulative result, with linefeed
	$token   = strtok("\n"); // get the next token
	$pad    += $indent; // update the pad size for subsequent lines
	endwhile;

	return $result;
}

// Courtesy of Dan at calcatraz http://www.calcatraz.com/blog/php-topological-sort-function-384
function ossec_topological_sort($nodeids, $edges) {

	// initialize variables
	$L = $S = $nodes = array();

	// remove duplicate nodes
	$nodeids = array_unique($nodeids);

	// remove duplicate edges
	$hashes = array();
	foreach($edges as $k=>$e) {
		$hash = md5(serialize($e));
		if (in_array($hash, $hashes)) {
			unset($edges[$k]);
		}
		else { $hashes[] = $hash;
		};
	}

	// Build a lookup table of each node's edges
	foreach($nodeids as $id) {
		$nodes[$id] = array('in'=>array(), 'out'=>array());
		foreach($edges as $e) {
			if ($id==$e[0]) {
				$nodes[$id]['out'][]=$e[1];
			}
			if ($id==$e[1]) {
				$nodes[$id]['in'][]=$e[0];
			}
		}
	}

	// While we have nodes left, we pick a node with no inbound edges,
	// remove it and its edges from the graph, and add it to the end
	// of the sorted list.
	foreach ($nodes as $id=>$n) {
		if (empty($n['in'])) $S[]=$id;
	}
	while (!empty($S)) {
		$L[] = $id = array_shift($S);
		foreach($nodes[$id]['out'] as $m) {
			$nodes[$m]['in'] = array_diff($nodes[$m]['in'], array($id));
			if (empty($nodes[$m]['in'])) {
				$S[] = $m;
			}
		}
		$nodes[$id]['out'] = array();
	}

	// Check if we have any edges left unprocessed
	foreach($nodes as $n) {
		if (!empty($n['in']) or !empty($n['out'])) {
			return null; // not sortable as graph is cyclic
		}
	}
	return $L;
}

function ossec_sanitize_xml($xml) {
        $safe_xml = '';
        $multiline_comment = 0;

        foreach(preg_split('/((\r?\n)|(\n?\r))/', $xml) as $line) {
                if($multiline_comment == 0) {
                        if(preg_match('/^[ \t]*<!--.*-->[ \t]*$/', $line, $null)) {
                                continue;
                        }
                        elseif(preg_match('/^[ \t]*<!--/', $line, $null)) {
                                $multiline_comment = 1;
                        }
                        elseif(preg_match('/^[ \t]*<\/?\w*.*>[ \t]*$/', $line, $null)) {
                                if(preg_match('/(<\w*(regex|match)[^>]*>)(.*[&<>"\']+.*)(<\/[^>]+>)/', $line, $match)) {
                                        $match[3] = str_replace('&','&#38;',$match[3]);
                                        $match[3] = str_replace('<','&#60;',$match[3]);
                                        $match[3] = str_replace('>','&#62;',$match[3]);
                                        $match[3] = str_replace('"','&#34;',$match[3]);
                                        $match[3] = str_replace('\'','&#39;',$match[3]);

                                        $safe_xml .= (string)$match[1] . (string)$match[3] . (string)$match[4] . "\n";
                                } else {
                                        $safe_xml .= $line . "\n";
                                }
                        }
                } else {
                        if(preg_match('/^[ \t]*-->/', $line, $null)) {
                                $multiline_comment = 0;
                        }
                }
        }

        return $safe_xml;
}

function ossec_get_real_interface_address($interface) {
	$interface = convert_friendly_interface_to_real_interface_name($interface);
	$line = trim(shell_exec("ifconfig $interface | grep inet | grep -v inet6 | awk '{ print \$2, \$4 }'"));
	list($ip, $netmask) = explode(" ", $line);

	return array($ip, long2ip(hexdec($netmask)));
}

function ossec_get_log_files() {
	$log_dir = OSSEC_BASE . "/logs/";
	$log_matches = explode("\n", shell_exec("cd $log_dir && find . -maxdepth 2 -name '*.log'"));
	
	$log_files = array();
	
	foreach($log_matches as $log_match) {
		$log_file = array();
		
		$log_file['name'] = basename($log_match);
		$log_file['path'] = substr($log_match, 2);
		
		if(!empty($log_file['name']) && !empty($log_file['path']))
			array_push($log_files, $log_file);
	}
	
	$sort = array();
	
	foreach($log_files as $k=>$v) {
		$sort['name'][$k] = $v['name'];
	}
	
	array_multisort($sort['name'], SORT_ASC, $log_files);
	
	return $log_files;
}

function ossec_generate_agent_key() {
	// Generate first md5 hash
	$urandom_bit_length = 128;
	$fp = @fopen('/dev/urandom','rb');
	if ($fp !== FALSE) {
		$random1 = substr(base64_encode(@fread($fp,($urandom_bit_length + 7) / 8)), 0, (($urandom_bit_length + 5) / 6)  - 2);
		@fclose($fp);
	} else {
		$random1 = microtime(true).mt_rand(10000,90000) . uniqid();
	}
	$hash1 = md5($random1);

	// Generate second md5 hash
	$hash2 = md5($hash1 . trim(shell_exec("netstat -i")));

	return $hash1 . $hash2;
}

function ossec_control_agent($action, $agent_id) {
	switch ($action) {
		case "extract_key":
			if(!empty($agent_id))
	   			$command = OSSEC_BASE . "bin/manage_agents -e $agent_id";
	   		break;
		case "start_checks":
			if(!empty($agent_id))
	    		$command = OSSEC_BASE . "bin/agent_control -r -u $agent_id";
			else
				$command = OSSEC_BASE . "bin/agent_control -r -a";
	    	break;
		case "restart_agent":
			if(!empty($agent_id))
	   			$command = OSSEC_BASE . "bin/agent_control -R $agent_id";
			break;
		default:
			return "ERROR: Agent action not valid!";
	}
	
	$output = trim(shell_exec($command));
	return $output;
}

function ossec_load_rule_file($rule_file) {
	global $config;
	
	$rules = array();
	
	if(file_exists(OSSEC_RULE_DIR . $rule_file)) {
		$rules_xml = file_get_contents(OSSEC_RULE_DIR . $rule_file);
		$rules_xml = "<rules>\n" . $rules_xml . "\n</rules>";
		$rules_objects = simplexml_load_string(ossec_sanitize_xml($rules_xml));
	
		foreach($rules_objects as $rule_element) {
			if($rule_element->getName() == "group") {		
				foreach($rule_element->children() as $rule_sub_element) {
					$rule = array();
					
					$parameters = "";
					foreach($rule_sub_element->children() as $parameter)
						$parameters .= $parameter->asXML() . "\n";
					
					$rule['filename'] = $rule_file;
					$rule['ruleid'] = (string)$rule_sub_element->attributes()->id;
					$rule['level'] = (string)$rule_sub_element->attributes()->level;
					$rule['parameters'] = $parameters;
					$rule['groups'] = (string)$rule_element->attributes()->name;
					$rule['maxsize'] = (string)$rule_sub_element->attributes()->maxsize;
					$rule['frequency'] = (string)$rule_sub_element->attributes()->frequency;
					$rule['timeframe'] = (string)$rule_sub_element->attributes()->timeframe;
					$rule['ignore'] = (string)$rule_sub_element->attributes()->ignore;
					$rule['overwrite'] = (string)$rule_sub_element->attributes()->overwrite;
					$rule['description'] = (string)$rule_sub_element->description;
					
					array_push($rules, $rule);
				}
			}
			elseif($rule_element->getName() == "rule") {
				$rule = array();
				
				$parameters = "";
				foreach($rule_element->children() as $parameter)
					$parameters .= $parameter->asXML() . "\n";
				
				$rule['filename'] = $rule_file;
				$rule['ruleid'] = (string)$rule_element->attributes()->id;
				$rule['level'] = (string)$rule_element->attributes()->level;
				$rule['parameters'] = $parameters;
				$rule['groups'] = "";
				$rule['maxsize'] = (string)$rule_element->attributes()->maxsize;
				$rule['frequency'] = (string)$rule_element->attributes()->frequency;
				$rule['timeframe'] = (string)$rule_element->attributes()->timeframe;
				$rule['ignore'] = (string)$rule_element->attributes()->ignore;
				$rule['overwrite'] = (string)$rule_element->attributes()->overwrite;
				$rule['description'] = (string)$rule_element->description;
				
				array_push($rules, $rule);
			}
		}
	}
	
	if(!empty($rules)) {
		$sort = array();
	
		foreach($rules as $k=>$v) {
			$sort['ruleid'][$k] = $v['ruleid'];
		}
	
		array_multisort($sort['ruleid'], SORT_ASC, $rules);
	}
	
	return $rules;
}

function ossec_load_decoder_file($decoder_file) {
	global $config;
	
	$decoders = array();
	
	if(file_exists(OSSEC_ETC_DIR . $decoder_file)) {
		$decoder_xml = file_get_contents(OSSEC_ETC_DIR . $decoder_file);
		$decoder_xml = "<decoders>\n" . $decoder_xml . "\n</decoders>";
		$decoder_objects = simplexml_load_string(ossec_sanitize_xml($decoder_xml));
	
		foreach($decoder_objects as $decoder_element) {
			$decoder = array();

			$parameters = "";
			foreach($decoder_element->children() as $parameter)
				$parameters .= $parameter->asXML() . "\n";

			$decoder['filename'] = $decoder_file;
			$decoder['name'] = (string)$decoder_element->attributes()->name;
			$decoder['parameters'] = $parameters;
			$decoder['description'] = "";

			array_push($decoders, $decoder);
		}
	}
	
	if(!empty($decoders)) {
		$sort = array();
	
		foreach($decoders as $k=>$v) {
			$sort['name'][$k] = $v['name'];
		}
	
		array_multisort($sort['name'], SORT_ASC, $decoders);
	}
	
	return $decoders;
}

function ossec_detect_duplicate_object($object_name, $field_name, $value, $index) {
	global $config;
	
	$object_array = array_values($config['installedpackages'][$object_name]['config']);
	$object_array_pos = 0;
	
	$duplicate_detected = 0;
	
	foreach($object_array as $object_element) {
		if($object_element[$field_name] == $value) {
			if($object_array_pos != $index)
				$duplicate_detected = 1;
			
			break;
		}
			
		$object_array_pos++;
	}
	
	return $duplicate_detected;
}

function ossec_validate_general($post, $input_errors) {
	global $config;
	
	if(!($post['email_alert_level'] > 0) || !($post['email_alert_level'] <= 16))
		$input_errors[] = 'The value specified in \'E-mail Alert Level\' must be in the range 1-16';
	
	if(!($post['log_alert_level'] > 0) || !($post['log_alert_level'] <= 16))
		$input_errors[] = 'The value specified in \'Log Alert Level\' must be in the range 1-16';
	
	if(empty($post['interfaces'])) {
		$input_errors[] = 'You must select at least one interface in \'Interfaces\' field';
	} else {
		$post['interfaces'] = implode(",", $post['interfaces']);
	}
	
	if(!is_port($post['port']))
		$input_errors[] = 'You must enter a valid port number in the \'Port\' field';
	
	$sockstat = trim(shell_exec("sockstat -l -P tcp" . " -p " . $post['port'] . " | grep -v ^USER | grep -v ossec"));
	if(!empty($sockstat))
		$input_errors[] = 'The port specified in the \'Port\' field is already in use';
}

function ossec_validate_rule_files($post, $input_errors) {
	if(ossec_detect_duplicate_object('ossecrulefiles', 'name', $post['name'], $post['id']))
		$input_errors[] = 'The value specified in \'Name\' already exists';
	
	if(!file_exists(OSSEC_RULE_DIR . $post['name']))
		$input_errors[] = 'The file name specified in \'Name\' does not exist in the rules directory';
}

function ossec_validate_syschecks($post, $input_errors) {
	if(ossec_detect_duplicate_object('ossecsyschecks', 'name', $post['name'], $post['id']))
		$input_errors[] = 'The value specified in \'Name\' already exists';
}

function ossec_validate_rootchecks($post, $input_errors) {
	if(ossec_detect_duplicate_object('ossecrootchecks', 'name', $post['name'], $post['id']))
		$input_errors[] = 'The value specified in \'Name\' already exists';
}

function ossec_validate_active_response($post, $input_errors) {
	if(ossec_detect_duplicate_object('ossecactiveresponse', 'name', $post['name'], $post['id']))
		$input_errors[] = 'The value specified in \'Name\' already exists';
}

function ossec_validate_commands($post, $input_errors) {
	if(ossec_detect_duplicate_object('osseccommands', 'name', $post['name'], $post['id']))
		$input_errors[] = 'The value specified in \'Name\' already exists';
}

function ossec_validate_local_rules($post, $input_errors) {
	if(ossec_detect_duplicate_object('osseclocalrules', 'ruleid', $post['ruleid'], $post['id']))
		$input_errors[] = 'The value specified in \'Rule ID\' already exists';
		
	if(!($post['ruleid'] >= 1) || !($post['ruleid'] < 110000))
		$input_errors[] = 'The value specified in \'Rule ID\' must be in the range 1 - 109999';
	elseif((!($post['ruleid'] > 99999) || !($post['ruleid'] <= 109999)) && $post['overwrite'] != 'on')
		$input_errors[] = 'The value specified in \'Rule ID\' must be in the range 100000 - 109999 unless \'Overwrite\' is checked';
		
	if(!($post['level'] >= 0) || !($post['level'] <= 16))
		$input_errors[] = 'The value specified in \'Level\' must be in the range 0-16';
}

function ossec_validate_local_decoders($post, $input_errors) {
    $decoder_matches = ossec_search_decoder_files($post['name'], 'name');
	
	if(ossec_detect_duplicate_object('osseclocaldecoders', 'name', $post['name'], $post['id'])) {
		$input_errors[] = 'The value specified in \'Name\' already exists';
	} elseif(!empty($decoder_matches)) {
		foreach($decoder_matches as $decoder_match) {
			if($decoder_match['filename'] != 'local_decoder.xml')
				$input_errors[] = 'The value specified in \'Name\' is reserved any may not be used';
		}
	}
}

function ossec_validate_local_files($post, $input_errors) {
	if(ossec_detect_duplicate_object('osseclocalfiles', 'location', $post['location'], $post['id']))
		$input_errors[] = 'The value specified in \'Location\' already exists';
}

function ossec_validate_local_variables($post, $input_errors) {
	if(ossec_detect_duplicate_object('osseclocalvariables', 'name', $post['name'], $post['id']))
		$input_errors[] = 'The value specified in \'Name\' already exists';
}

function ossec_validate_agents($post, $input_errors) {
	if(ossec_detect_duplicate_object('ossecagents', 'agentid', $post['agentid'], $post['id']))
		$input_errors[] = 'The value specified in \'Agent ID\' already exists';
		
	if(ossec_detect_duplicate_object('ossecagents', 'name', $post['name'], $post['id']))
		$input_errors[] = 'The value specified in \'Name\' already exists';
	
	if(empty($post['agentkey'])) {
		$input_errors[] = 'No key specified in \'Agent Key\'. The following key has been automatically generated: ' . ossec_generate_agent_key();
	}
}

function ossec_toggle_rule_file($rule_file) {
	global $config;
	
	$rulefiles = (array)$config['installedpackages']['ossecrulefiles']['config'];
	
	$config_index = array_search($rule_file, $rulefiles);
	
	conf_mount_rw();
	
	if($config_index !== FALSE)
		unset($config['installedpackages']['ossecrulefiles']['config'][$config_index]);
	elseif(file_exists(OSSEC_RULE_DIR . $rule_file))
		$config['installedpackages']['ossecrulefiles']['config'][] = $rule_file;
	
	write_config();
	conf_mount_ro();
}

function ossec_get_rule_files() {
	$rule_files = array();

	if ($handle = opendir(OSSEC_RULE_DIR)) {
		while (false !== ($entry = readdir($handle))) {
			if (!is_dir($entry) && preg_match('/rules.xml$/', $entry)) {
				array_push($rule_files, $entry);
			}
		}
		closedir($handle);
	}
	
	sort($rule_files);
	
	return $rule_files;
}

function ossec_sort_rule_files($rule_files) {
	$rule_files_list = implode(" ", $rule_files);
	$rule_dir = OSSEC_RULE_DIR;
	
	$rule_depends = array();
	
	$output = trim(shell_exec("cd $rule_dir && grep '<if.*sid>' $rule_files_list | tr -d ' \\t' | sed 's/\\(.*\\):.*>\\(.*\\)<.*/\\1,\\2/g' | awk -F ',' '{ for(i=2;i<=NF;i++) { print \$i,\$1 } }' | sort -nu"));
	$rule_sid_depends = explode("\n", $output);
	
	foreach($rule_sid_depends as $rule_sid_depend) {
		list($sid, $rule_file) = explode(" ", $rule_sid_depend);
		
		$sid_sources = array();
		
		if($sid != $last_sid) {
			$output = trim(shell_exec("cd $rule_dir && grep -E '<rule.*id=\"$sid\"' $rule_files_list | awk -F ':' '{ print \$1 }' | sort -u"));
			$sid_sources = explode("\n", $output);
		}
		
		foreach($sid_sources as $sid_source) {
			if(!empty($sid_source) && ($sid_source != $rule_file)) {
					array_push($rule_depends, array($sid_source, $rule_file));
			}
		}
		
		$last_sid = $sid;
	}
	
	$output = trim(shell_exec("cd $rule_dir && grep '<if.*group>' $rule_files_list | tr -d ' \\t' | sed 's/\\(.*\\):.*>\\(.*\\)<.*/\\1,\\2/g' | awk -F ',' '{ for(i=2;i<=NF;i++) { print \$i,\$1 } }' | sort -u"));
	$rule_group_depends = explode("\n", $output);
	
	foreach($rule_group_depends as $rule_group_depend) {
		list($group, $rule_file) = explode(" ", $rule_group_depend);
		
		$group_sources = array();
		
		if($group != $last_group) {
			$output = trim(shell_exec("cd $rule_dir && grep -E '<group.*$group" . "[,<]' $rule_files_list | awk -F ':' '{ print \$1 }' | sort -u"));
			$group_sources = explode("\n", $output);
		}
		
		foreach($group_sources as $group_source) {
			if(!empty($group_source) && ($group_source != $rule_file)) {
				array_push($rule_depends, array($group_source, $rule_file));
			}
		}
		
		$last_group = $group;
	}
	
	return ossec_topological_sort($rule_files, $rule_depends);
}

function ossec_get_rule_ids($rule_file) {
	$rule_file = OSSEC_RULE_DIR . $rule_file;
	$rule_ids = array();
	
	exec("grep '<rule ' $rule_file | sed 's/.*id=\"\([0-9]*\)\".*/\\1/g' | sed 's/^0*//g' | sort -n", $rule_ids);
	
	return $rule_ids;
}

function ossec_search_rule_files($search, $field) {
	$rules_found = array();
	$rule_files = ossec_get_rule_files();
	
	foreach($rule_files as $rule_file) {
		$rules = ossec_load_rule_file($rule_file);
		
		foreach($rules as $rule) {
			if(empty($field)) {
				foreach($rule as $k=>$v) {
					if(stripos($v, $search) !== false) {
						array_push($rules_found, $rule);
						break;
					}
				}
			} else {
				if($rule[$field] == $search) {
					array_push($rules_found, $rule);
				}
			}
		}
		
	}
	
	if(!empty($rules_found)) {
		$sort = array();
	
		foreach($rules_found as $k=>$v) {
			$sort['ruleid'][$k] = $v['ruleid'];
		}
	
		array_multisort($sort['ruleid'], SORT_ASC, $rules_found);
	}
	
	return $rules_found;
}

function ossec_get_decoder_files() {
	$decoder_files = array();

	if ($handle = opendir(OSSEC_ETC_DIR)) {
		while (false !== ($entry = readdir($handle))) {
			if (!is_dir($entry) && preg_match('/decoder\.xml$/', $entry)) {
				array_push($decoder_files, $entry);
			}
		}
		closedir($handle);
	}
	
	sort($decoder_files);

	return $decoder_files;
}

function ossec_search_decoder_files($search, $field) {
	$decoders_found = array();
	$decoder_files = ossec_get_decoder_files();

	foreach($decoder_files as $decoder_file) {
		$decoders = ossec_load_decoder_file($decoder_file);

		foreach($decoders as $decoder) {
			if(empty($field)) {
				foreach($decoder as $k=>$v) {
					if(stripos($v, $search) !== false) {
						array_push($decoders_found, $decoder);
						break;
					}
				}
			} else {
				if($decoder[$field] == $search) {
					array_push($decoders_found, $decoder);
				}
			}
		}

	}

	if(!empty($decoders_found)) {
		$sort = array();

		foreach($decoders_found as $k=>$v) {
			$sort['name'][$k] = $v['name'];
		}

		array_multisort($sort['name'], SORT_ASC, $decoders_found);
	}

	return $decoders_found;
}

function ossec_build_conf_files() {
	global $config;
	
	$settings = $config['installedpackages']['ossec']['config'][0];
	$rulefiles = $config['installedpackages']['ossecrulefiles']['config'];
	$syschecks = $config['installedpackages']['ossecsyschecks']['config'];
	$rootchecks = $config['installedpackages']['ossecrootchecks']['config'];
	$activeresponses = $config['installedpackages']['ossecactiveresponse']['config'];
	$commands = $config['installedpackages']['osseccommands']['config'];
	$localrules = $config['installedpackages']['osseclocalrules']['config'];
	$localdecoders = $config['installedpackages']['osseclocaldecoders']['config'];
	$localfiles = $config['installedpackages']['osseclocalfiles']['config'];
	$localvariables = $config['installedpackages']['osseclocalvariables']['config'];
	$agents = $config['installedpackages']['ossecagents']['config'];
	
	$agent_conf = array();
	
	// Build agent.conf
	
	ksort($agent_conf);
	
	$conf = "<!-- This file is automatically generated by pfSense -->\n";
	$conf .= "<!-- Do not edit manually !-->\n";
	
	foreach(array_keys($agent_conf) as $agent_conf_key) {
		if(!empty($agent_conf_key))
			$conf .= "<agent_config " . $agent_conf_key . ">\n";
		else
			$conf .= "<agent_config>\n";
		$conf .= $agent_conf[$agent_conf_key];
		$conf .= "</agent_config>\n";
	}
	
	file_put_contents(OSSEC_ETC_DIR . 'shared/agent.conf', ossec_format_xml($conf));
	
	// Build client.keys file
	
	if(!empty($agents)) {
		$sort = array();
		
		$conf = NULL;
		
		foreach($agents as $k=>$v) {
			$sort['agentid'][$k] = $v['agentid'];
		}
		
		array_multisort($sort['agentid'], SORT_ASC, $agents);
		
		foreach($agents as $agent) {
			$conf .= $agent['agentid'] . " " . $agent['name'] . " " . $agent['ip'] . " " . $agent['agentkey'] . "\n";
		}
		
		file_put_contents(OSSEC_ETC_DIR . 'client.keys', $conf);
	}
	
	// Build local rule file
	
	if(!empty($localrules)) {
		$sort = array();
	
		$conf = "<!-- This file is automatically generated by pfSense -->\n";
		$conf .= "<!-- Do not edit manually !-->\n";
	
		foreach($localrules as $k=>$v) {
			$sort['ruleid'][$k] = $v['ruleid'];
			$sort['groups'][$k] = $v['groups'];
		}
	
		array_multisort($sort['groups'], SORT_ASC, $sort['ruleid'], SORT_ASC, $localrules);
	
		$previous_groups = '';
	
		foreach($variables as $variable) {
			$conf .= "<var name=\"" . $variable['name'] . "\">" . base64_decode($variable['value']) . "</var>\n";
		}
	
		foreach($localrules as $localrule) {
			$rule_attributes = "id=\"" . $localrule['ruleid'] . "\" level=\"" . $localrule['level'] . "\"";
			if(!empty($localrule['maxsize']))
				$rule_attributes .= " maxsize=\"" . $localrule['maxsize'] . "\"";
			if(!empty($localrule['frequency']))
				$rule_attributes .= " frequency=\"" . $localrule['frequency'] . "\"";
			if(!empty($localrule['timeframe']))
				$rule_attributes .= " timeframe=\"" . $localrule['timeframe'] . "\"";
			if(!empty($localrule['ignore']))
				$rule_attributes .= " ignore=\"" . $localrule['ignore'] . "\"";
			if($localrule['overwrite'] == 'on')
				$rule_attributes .= " overwrite=\"yes\"";
				
			if(!empty($previous_groups) && ($localrule['groups'] != $previous_groups)) {
				$conf .= "</group>\n";
			}
				
			if(!empty($localrule['groups'])) {
				if($localrule['groups'] != $previous_groups) {
					$conf .= "<group name=\"" . $localrule['groups'] . "\">\n";
					$conf .= "<rule " . $rule_attributes . ">\n";
					$conf .= base64_decode($localrule['parameters']) . "\n";
					$conf .= "</rule>\n";
				} else {
					$conf .= "<rule " . $rule_attributes . ">\n";
					$conf .= base64_decode($localrule['parameters']) . "\n";
					$conf .= "</rule>\n";
				}
			} else {
				$conf .= "<rule " . $rule_attributes . ">\n";
				$conf .= base64_decode($localrule['parameters']) . "\n";
				$conf .= "</rule>\n";
			}
				
			$previous_groups = $localrule['groups'];
		}
	
		if(!empty($previous_groups)) {
			$conf .= "</group>\n";
		}
	
		file_put_contents(OSSEC_RULE_DIR . 'local_rules.xml', ossec_format_xml($conf));
	} else {
		file_put_contents(OSSEC_RULE_DIR  . 'local_rules.xml', NULL);
	}
	
	// Build local decoder file
	
	if(!empty($localdecoders)) {
		$sort = array();
	
		$conf = "<!-- This file is automatically generated by pfSense -->\n";
		$conf .= "<!-- Do not edit manually !-->\n";
			
		foreach($localdecoders as $k=>$v) {
			$sort['name'][$k] = $v['name'];
		}
	
		array_multisort($sort['name'], SORT_ASC, $localdecoders);
	
		foreach($localdecoders as $localdecoder) {
			$conf .= "<decoder name=\"" . $localdecoder['name'] . "\">\n";
			$conf .= base64_decode($localdecoder['parameters']) . "\n";
			$conf .= "</decoder>\n";
		}
	
		file_put_contents(OSSEC_ETC_DIR . 'local_decoder.xml', ossec_format_xml($conf));
	} else {
		file_put_contents(OSSEC_ETC_DIR . 'local_decoder.xml', NULL);
	}
	
	// Build ossec.conf
	
	$conf = "<!-- This file is automatically generated by pfSense -->\n";
	$conf .= "<!-- Do not edit manually !-->\n";
	
	$conf .= "<ossec_config>\n";
	
	if($settings['email_notification'] == 'on') {
		$conf .= "<global>\n";
		$conf .= "<email_notification>yes</email_notification>\n";
		$conf .= "<email_to>" . $settings['email_to'] . "</email_to>\n";
		$conf .= "<email_from>" . $settings['email_from'] . "</email_from>\n";
		$conf .= "<smtp_server>" . $settings['smtp_server'] . "</smtp_server>\n";
		$conf .= "</global>\n";
	}
	
	if(!empty($rulefiles)) {
		$conf .= "<rules>\n";
		
		$conf .= "<include>rules_config.xml</include>\n";
		
		// Rule files have to be loaded in a specific order (if_sid and if_group dependencies)	
		$rulefiles = ossec_sort_rule_files($rulefiles);
		
		foreach($rulefiles as $rulefile) {
			if(file_exists(OSSEC_RULE_DIR . $rulefile) && (filesize(OSSEC_RULE_DIR . $rulefile) != 0))
				$conf .= "<include>" . $rulefile . "</include>\n";
		}

		$conf .= "</rules>\n";
	}
	
	if(!empty($syschecks)) {
		foreach($syschecks as $syscheck) {
			$syscheck_conf = "<syscheck>\n";
			$syscheck_conf .= base64_decode($syscheck['parameters']) . "\n";
			$syscheck_conf .= "\n</syscheck>\n";
				
			if(empty($syscheck['agent_config'])) {
				$conf .= $syscheck_conf;
			} else {
				$agent_conf[$syscheck['agent_identifier']] .= $syscheck_conf;
			}
		}
	}
	
	if(!empty($rootchecks)) {
		foreach($rootchecks as $rootcheck) {
			$rootcheck_conf = "<rootcheck>\n";
			$rootcheck_conf .= base64_decode($rootcheck['parameters']) . "\n";
			$rootcheck_conf .= "\n</rootcheck>\n";
				
			if(empty($rootcheck['agent_config'])) {
				$conf .= $rootcheck_conf;
			} else {
				$agent_conf[$rootcheck['agent_identifier']] .= $rootcheck_conf;
			}
		}
	}
	
	if(!empty($settings['white_list'])) {
		$conf .= "<global>\n";

		foreach(explode(',',$settings['white_list']) as $white_list)
			$conf .= "<white_list>" . $white_list . "</white_list>\n";
		
		$conf .= "</global>\n";
	}
	
	if($settings['remote_connections'] == 'on') {
		$conf .= "<remote>\n";
		$conf .= "<connection>" . $settings['connection'] . "</connection>\n";
		$conf .= "<port>" . $settings['port'] . "</port>\n";
		$conf .= "<protocol>" . $settings['protocol'] . "</protocol>\n";
		$interface_ip = ossec_get_real_interface_address($settings['interface']);
		$conf .= "<local_ip>" . $interface_ip[0] . "</local_ip>\n";
		$conf .= "</remote>\n";
	}
	
	$conf .= "<alerts>\n";
	$conf .= "<log_alert_level>" . $settings['log_alert_level'] . "</log_alert_level>\n";
	$conf .= "<email_alert_level>" . $settings['email_alert_level'] . "</email_alert_level>\n";
	$conf .= "</alerts>\n";
	
	if(!empty($commands)) {
		foreach($commands as $command) {
			$conf .= "<command>\n";
			$conf .= "<name>" . $command['name'] . "</name>\n";
			$conf .= "<executable>" . $command['executable'] . "</executable>\n";
			$conf .= "<expect>" . $command['expect'] . "</expect>\n";
			if($command['timeout_allowed'] == 'on')
				$conf .= "<timeout_allowed>yes</timeout_allowed>\n";
			$conf .= "</command>\n";
		}
	}
	
	if(!empty($activeresponses)) {
		foreach($activeresponses as $activeresponse) {
			$conf .= "<active-response>\n";
			$conf .= base64_decode($activeresponse['parameters']) . "\n";
			$conf .= "\n</active-response>\n";
		}
	}
	
	if(!empty($localfiles)) {
		foreach($localfiles as $localfile) {
			$localfile_conf = "<localfile>\n";
			$localfile_conf .= "<location>" . $localfile['location'] . "</location>\n";
			$localfile_conf .= "<log_format>" . $localfile['log_format'] . "</log_format>\n";
			if(!empty($localfile['command']))
				$localfile_conf .= "<command>" . $localfile['command'] . "</command>\n";
			if(!empty($localfile['command_alias']))
				$localfile_conf .= "<alias>" . $localfile['command_alias'] . "</alias>\n";
			if(!empty($localfile['frequency']))
				$localfile_conf .= "<frequency>" . $localfile['frequency'] . "</frequency>\n";
			if($localfile['check_diff'] == 'on')
				$localfile_conf .= "<check_diff>yes</check_diff>\n";
			$localfile_conf .= "</localfile>\n";
			
			if(empty($localfile['agent_config'])) {
				$conf .= $localfile_conf;
			} else {
				$agent_conf[$localfile['agent_identifier']] .= $localfile_conf;
			}
		}
	}
	
	$conf .= "</ossec_config>\n";
	
	file_put_contents(OSSEC_ETC_DIR . 'ossec.conf', ossec_format_xml($conf));
	
	$errors = trim(shell_exec(OSSEC_BASE .'/bin/ossec-logtest -t 2>&1'));
		
}

function ossec_generate_rules($type) {
	global $config;
	
	$settings = $config['installedpackages']['ossec']['config'][0];
	
	$interfaces = ($settings['interfaces'] ? $settings['interfaces'] : 'lan');
	$port = ($settings['port'] ? $settings['port'] : 1514);
	$protocol = ($settings['protocol'] ? $settings['protocol'] : 'udp');
        $remote_connections = ($settings['remote_connections'] ? $settings['remote_connections'] : 'off');	

        $interfaces = explode(',', $settings['interfaces']);
        $interfaces = array_map('convert_friendly_interface_to_real_interface_name', $interfaces);

	$rules = "";

        if($remote_connections == 'on') {
	        switch($type) {
                        case 'nat':
                                // Nothing here yet.
                                break;
                        case 'filter':
		        case 'rule':
			        foreach ($interfaces as $interface) {
				        $rules .= "pass in quick on $interface proto $protocol from any to ($interface) port $port keep state label \"ossec remote connection rule\"\n";
			        }
			        break;
                        default:
                                break;
	        }
        }

	return $rules;
}
	
function ossec_resync() {
	global $config;
	conf_mount_rw();
	
	$settings = $config['installedpackages']['ossec']['config'][0];
	
	if(!isset($settings['enable']))
		$settings['enable'] = 'off';
	if(!isset($settings['log_alert_level']))
		$settings['log_alert_level'] = '1';
	if(!isset($settings['email_alert_level']))
		$settings['email_alert_level'] = '7';
	if(!isset($settings['remote_connections']))
		$settings['remote_connections'] = 'off';
	if(!isset($settings['interfaces']))
		$settings['interfaces'] = 'lan';
	if(!isset($settings['connection']))
		$settings['connection'] = 'secure';
	if(!isset($settings['protocol']))
		$settings['protocol'] = 'udp';
	if(!isset($settings['port']))
		$settings['port'] = 1514;
	if(!isset($settings['email_notification']))
		$settings['email_notification'] = 'off';
	
	$config['installedpackages']['ossec']['config'][0] = $settings;

	write_config();
	conf_mount_ro();
	
	ossec_build_conf_files();
	
	if($settings['enable'] == 'on') {
		ossec_write_rcfile();
	
		if(!is_service_running('ossec')) {
			log_error("Starting ossec");
			exec("/usr/local/etc/rc.d/ossec.sh start");
		} else {
			log_error("Reloading ossec for configuration sync");
			exec("/usr/local/etc/rc.d/ossec.sh restart");
		}
	
		// Sleep for a couple seconds to give ossec a chance to fire up fully.
		for ($i=0; $i < 10; $i++) {
			if(!is_service_running('ossec'))
				sleep(1);
		}
	} else {
		if(is_service_running('ossec')) {
			log_error("Stopping ossec");
			exec("/usr/local/etc/rc.d/ossec.sh stop");
		}
		
		unlink_if_exists("/usr/local/etc/rc.d/ossec.sh");
	}
	
	filter_configure();
}

function ossec_write_rcfile() {
	$rc = array();
	$fts_queue = OSSEC_BASE . "/queue/fts/fts-queue";
	$ig_queue = OSSEC_BASE . "/queue/fts/ig-queue";
	$control_bin = OSSEC_BASE . "/bin/ossec-control";
	$rc['file'] = 'ossec.sh';
	$rc['start'] = <<<EOD

		# These files are not created by the daemons with the correct
		# ownership, so create them here before starting up the system,
		# if they don't already exist. This is only done for the "local" and
		# "server" installation types.

		if [ ! -e ${fts_queue} ]; then
			touch ${fts_queue}
			chown ossec:ossec ${fts_queue}
			chmod 640 ${fts_queue}
		fi
		if [ ! -e ${ig_queue} ]; then
			touch ${ig_queue}
			chown ossec:ossec ${ig_queue}
			chmod 640 ${ig_queue}
		fi

		${control_bin} start
EOD;
	
	$rc['stop'] = <<<EOD

		${control_bin} stop
EOD;
    
	$rc['restart'] = <<<EOD

		${control_bin} restart
EOD;
    
	conf_mount_rw();
	write_rcfile($rc);
}
?>
