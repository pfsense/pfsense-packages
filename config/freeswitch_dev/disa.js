include("/usr/local/freeswitch/scripts/config.js");

 //var admin_pin = ""; //don't require a pin
   //if you choose not to require a pin then then you may want to add a dialplan condition for a specific caller id
 var predefined_destination = ""; //example: 9999
 //predefined_destination leave empty in most cases
   //Use this to define a single destination
 var digitmaxlength = 0;
 var timeoutpin = 7500;
 var timeouttransfer = 7500;

 function mycb( session, type, obj, arg ) {
    try {
        if ( type == "dtmf" ) {
          console_log( "info", "digit: "+obj.digit+"\n" );
          if ( obj.digit == "#" ) {
            //console_log( "info", "detected pound sign.\n" );
            exit = true;
            return( false );
          }

          dtmf.digits += obj.digit;

          if ( dtmf.digits.length >= digitmaxlength ) {
            exit = true;
            return( false );
          }
        }
    } catch (e) {
        console_log( "err", e+"\n" );
    }
    return( true );
 } //end function mycb


 //console_log( "info", "DISA Request\n" );

 var dtmf = new Object( );
 dtmf.digits = "";

 if ( session.ready( ) ) {
   session.answer( );

   if (admin_pin.length > 0) {
      digitmaxlength = 6;
      session.streamFile( "/usr/local/freeswitch/sounds/custom/8000/please_enter_the_pin_number.wav", mycb, "dtmf");
      session.collectInput( mycb, dtmf, timeoutpin );
      //console_log( "info", "DISA pin: " + dtmf.digits + "\n" );
   }

   if (dtmf.digits == admin_pin || admin_pin.length == 0) {

      //console_log( "info", "DISA pin is correct\n" );

      us_ring = session.getVariable("us-ring");
      session.execute("set", "ringback="+us_ring);          //set to ringtone
      session.execute("set", "transfer_ringback="+us_ring); //set to ringtone
      session.execute("set", "hangup_after_bridge=true");

      if (predefined_destination.length == 0) {
         dtmf.digits = ""; //clear dtmf digits to prepare for next dtmf request
         digitmaxlength = 11;
         session.streamFile( "/usr/local/freeswitch/sounds/custom/8000/please_enter_the_phone_number.wav", mycb, "dtmf");
         session.collectInput( mycb, dtmf, timeouttransfer );
         console_log( "info", "DISA Transfer: " + dtmf.digits + "\n" );
         session.execute("transfer", dtmf.digits + " XML default");
      }
      else {
         session.execute("transfer", predefined_destination + " XML default");
      }

   }
   else {
      session.streamFile( "/usr/local/freeswitch/sounds/custom/8000/your_pin_number_is_incorect_goodbye.wav", mycb, "dtmf");
      console_log( "info", "DISA Pin: " + dtmf.digits + " is incorrect\n" );
   }

 }
