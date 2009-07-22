var uuid = argv[0];
var sipuri = argv[1];
var extension = argv[2];
var caller_announce = argv[3];
var caller_id_name = argv[4];
var caller_id_number = argv[5];
var tmp_sipuri;

caller_id_name = caller_id_name.replace("+", " ");
//console_log( "info", "caller_announce: "+caller_announce+"\n" );

function originate (session, sipuri, extension, caller_announce, caller_id_name, caller_id_number) {

	var dtmf = new Object();
	var cid;
	dtmf.digits = "";
	cid = ",origination_caller_id_name="+caller_id_name+",origination_caller_id_number="+caller_id_number;
	
	new_session = new Session("{ignore_early_media=true"+cid+"}"+sipuri);
	new_session.execute("set", "call_timeout=30");
		
	if ( new_session.ready() ) {

		console_log( "info", "followme: new_session uuid "+new_session.uuid+"\n" );
		console_log( "info", "followme: no dtmf detected\n" );

		digitmaxlength = 1;
		while (new_session.ready()) {
			
			if (caller_announce.length > 0) {
				new_session.streamFile( "/tmp/"+caller_announce);
			}
			new_session.streamFile( "/usr/local/freeswitch/sounds/custom/8000/press_1_to_accept_2_to_reject_or_3_for_voicemail.wav");
			if (new_session.ready()) {
				if (dtmf.digits.length == 0) {
					dtmf.digits +=  new_session.getDigits(1, "#", 10000); // 10 seconds
					if (dtmf.digits.length == 0) {
						
					}
					else {
						break; //dtmf found end the while loop
					}
				}
			}
		}

		if ( dtmf.digits.length > "0" ) {
			if ( dtmf.digits == "1" ) {
				console_log( "info", "followme: call accepted\n" ); //accept
				new_session.execute("fifo", extension+"@${domain_name} out nowait");
				return true;
			}
			else if ( dtmf.digits == "2" ) {
				console_log( "info", "followme: call rejected\n" ); //reject
				new_session.hangup;
				return false;
			}
			else if ( dtmf.digits == "3" ) {
				console_log( "info", "followme: call sent to voicemail\n" ); //reject
				new_session.hangup;
				exit;
				return true;
			}
			
		}
		else {
			console_log( "info", "followme: no dtmf detected\n" ); //reject
			new_session.hangup;
			return false;
		}

	}
}

sipuri_array = sipuri.split(",");
for (i = 0; i < sipuri_array.length; i++){
	tmp_sipuri = sipuri_array[i];
	console_log("info", "tmp_sipuri: "+tmp_sipuri);
	result = originate (session, tmp_sipuri, extension, caller_announce, caller_id_name, caller_id_number);
	if (result) {
		break;
		exit;
	}
}