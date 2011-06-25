/* $Id$ */
/*

 part of pfSense
 All rights reserved.
 
 Pfsense snort GUI 
 Copyright (C) 2008-2012 Robert Zelaya.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.

 3. Neither the name of the pfSense nor the names of its contributors 
 may be used to endorse or promote products derived from this software without 
 specific prior written permission.

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

jQuery.noConflict();

//prepare the form when the DOM is ready 
jQuery(document).ready(function() {	
	
		jQuery(".icon_click").live('mouseover', function() {
			jQuery(this).css('cursor', 'pointer');
		});

	//-------------------START Misc-------------------------------------------
		
		
		/*! Needs to be watched not my code <- IMPORTANT
		* JavaScript UUID Generator, v0.0.1
		*
		* Copyright (c) 2009 Massimo Lombardo.
		* Dual licensed under the MIT and the GNU GPL licenses.
		*/			
		
		function genUUID() {
		    var uuid = (function () {
		        var i,
		            c = "89ab",
		            u = [];
		        for (i = 0; i < 36; i += 1) {
		            u[i] = (Math.random() * 16 | 0).toString(16);
		        }
		        u[8] = u[13] = u[18] = u[23] = "";
		        u[14] = "4";
		        u[19] = c.charAt(Math.random() * 4 | 0);
		        return u.join("");
		    })();
		    return {
		        toString: function () {
		            return uuid;
		        },
		        valueOf: function () {
		            return uuid;
		        }
		    };
		}

		//-------------------START Misc GLOBAL WINDOW-------------------------------------------
		// NOTE: try not to add to manny of thses
		
		/*
		 * Gives you even true or false on even numbers
		 */
		window.isEven = function(someNumber) {

			return (someNumber%2 == 0) ? true : false;
						
		};	
		
		/*
		 * Loop through object with timeout.
		 * NOTE: IE9 still has issues. Example : deleted rules (6000+ sigs). 
		 * Break up heavy javascript intensive processing into smaller parts. Used to stop "browser Stop responding" warnings.
		 */
		
		/*
		function processLoop( actionFunc, numTimes, numWait, doneFunc ) {
		  var i = 0;
		  var f = function () {
		    if (i < numTimes) {
		      actionFunc( i++ );  // closure on i
		      setTimeout( f, numWait );
		    } 
		    else if (doneFunc) { 
		      doneFunc();
		    }
		  };
		  f();
		}
		*/

		window.incrementallyProcess = function(workerCallback, data, chunkSize, timeout, completionCallback) {
			  var i = 0;
			  (function() {
			    var remainingDataLength = (data.length - i);
			    var currentChunkSize = (remainingDataLength >= chunkSize) ? chunkSize : remainingDataLength;
			    if(i < data.length) {
			      while(currentChunkSize--) {
			        workerCallback(i++);
			      }
			      setTimeout(arguments.callee, timeout);
			    } else if(completionCallback) {
			      completionCallback();
			    }
			  })();
			};
		
		// Please wait code
		window.hideLoading = function(thisLocation){
			  jQuery(thisLocation).hide();
		};
		
	    // Please wait code
		window.showLoading = function(thisLocation){
			  jQuery(thisLocation).show();
		};
		
		// this was cp from stackoverflow dot com help question
		// used to center snort modals
		jQuery.fn.centerModal = function () {
		    this.css("position","absolute");
		    this.css("top", 70 + "px");
		    this.css("left", ((jQuery(window).width() - this.outerWidth()) / 2) + jQuery(window).scrollLeft() + "px");
		    return this;
		}
		
		
  //--------------------------- START select all code ---------------------------
		
		jQuery('#select_all').live('click', function(){					    
			checkAll(jQuery('.domecheck'));					    
		});
		
		jQuery('#deselect_all').live('click', function(){					    
			uncheckAll(jQuery('.domecheck'));					    
		});

			function checkAll(field){				
				for (i = 0; i < field.length; i++){
					field[i].checked = true;
				}
			}

			function uncheckAll(field){
				for (i = 0; i < field.length; i++){
					field[i].checked = false;
				}
			}
		
		
  // -------------------------- START cancel form code ------------------------------------------- 
  //jQuery('#cancel').click(function() {
  jQuery('#cancel').live('click', function(){
    
    location.reload();
    
  });

  
// ------------------------------- START add row element ------------------------------------------
  
  jQuery(".icon_plus").live('click', function() {
	  
	var NewRow_UUID = genUUID();
	var rowNumCount = jQuery("#address").length;

	if (rowNumCount > 0){
		// stop empty
		var prevAddressAll_ck = jQuery('tr[id^=maintable_]');
		var prevAddress_ck = prevAddressAll_ck[prevAddressAll_ck.length-1].id;
		var prevAddressEmpty_ck = jQuery.trim(jQuery('#' + prevAddress_ck + ' #address').val());

		if (prevAddressEmpty_ck === ''){
			return false;    
		}	
	}
					jQuery('#listloopblock').append(
						"\n" + '<tr id="maintable_' +  NewRow_UUID + '" ' + 'data-options=\'{"pagetable":"SnortWhitelist", "pagedb":"snortDB", "DoPOST":"false"}\' >' +
						'<td>' +
						'<input class="formfld2" name="list[' + NewRow_UUID + '][ip]" type="text" id="address" size="30" value="" />' +
						'</td>' +
						'<td>' +
						'<input class="formfld2" name="list[' + NewRow_UUID + '][description]" type="text" id="detail" size="50" value="" />' +
						'</td>' +
						'<td>' +
						'<img id="icon_x_' + NewRow_UUID + '" class="icon_click icon_x" src="/themes/nervecenter/images/icons/icon_x.gif" width="17" height="17" border="0" title="delete list" >' +
						'</td>' +
						'<input name="list[' + NewRow_UUID + '][uuid]" value="EmptyUUID" type="hidden">' +
						'</tr>' + "\n"					
					);				

	});   
  

// ------------------------------- START remove row element ---------------------------------------
  

	function removeRow(){
		jQuery("#maintable_" + window.RemoveRow_UUID).remove();
	}
	
    jQuery(".icon_x").live('click', function(){
        
        var elem = getBaseElement(this.id); // this.id gets id of .icon_x

        // window.RemoveRow_UUID = jQuery("#rowlist_" + elem.index).data("options").rowuuid;
        window.RemoveRow_UUID = elem.index;        
        window.RemoveRow_Table = jQuery("#maintable_" + window.RemoveRow_UUID).data("options").pagetable;
        window.RemoveRow_DB = jQuery("#maintable_" + window.RemoveRow_UUID).data("options").pagedb;
        window.RemoveRow_POST = jQuery("#maintable_" + window.RemoveRow_UUID).data("options").DoPOST;       
        
        // snort_interfaces_whitelist
        if (window.RemoveRow_POST === 'true'){
          if(confirm('Do you really want to delete this list? (e.g. snort rules will fall back to the default list)!')) {

            jQuery("#maintable_" + window.RemoveRow_UUID).fadeOut("fast"); 

            setTimeout(removeRow, 600);
            jQuery(this).ajaxSubmit(optionsRMlist); // call POST     
            return false;
          }
        }
        
        // remove element NO post
        if (window.RemoveRow_POST === 'false'){
                  
          jQuery("#maintable_" + window.RemoveRow_UUID).fadeOut("fast");       
         
          setTimeout(removeRow, 600);
          
          return false;   
          
        }
        
    });

    
	function RMlistDBDelCall(){
		return RemoveRow_DB;
	}
  
    function RMlistTableDelCall(){
        return RemoveRow_Table;
    }
  
    function RMlistUuidDelCall(){
        return RemoveRow_UUID;
    }
  
    // pre-submit callback 
    function showRequestRMlist(formData, jqForm, optionsWhitelist) { 
      
        var queryString = jQuery.param(formData);
        
        // call false to prevent form reload
        return true; 
    }
    
    // post-submit callback if snort_json_post.php returns true or false
    function showResponseRMlist(data){ 
      
    }
    
    function getBaseElement(elem){
      elem = elem + "";
      var len     = elem.length;
      var lPos    = elem.lastIndexOf("_") * 1;
      var baseElem  = elem.substr(0, lPos);
      var index   = elem.substr(lPos+1, len);
      
      return {"base": baseElem, "index": index};
      
    }
   
    
  // declare variable for whitelist delete
  var optionsRMlist = {
            beforeSubmit:  showRequestRMlist,
            dataType:      'json', 
            success:       showResponseRMlist,
            type:          'POST',
            data:          { RMlistDelRow: '1', RMlistDB: RMlistDBDelCall, RMlistTable: RMlistTableDelCall, RMlistUuid: RMlistUuidDelCall },
            url:           './snort_json_post.php'
        }; 


  // STOP remove row element
	
// ------------------- START iform Submit/RETURN code ---------------------------------------------
	
	/* general form */
	//jQuery('#iform').submit(function() { 
	jQuery('#iform, #iform2, #iform3').live('submit', function(){

		jQuery(this).ajaxSubmit(options);

        return false; 
    });

	// pre-submit callback 
	function showRequest(formData, jqForm, options) { 
		
	    var queryString = jQuery.param(formData); 
	    
		// call to please wait
	    showLoading('#loadingWaiting');
	    jQuery('.snortModal').centerModal();
	 
	    //alert('About to submit: \n\n' + queryString); 
	    
	    // call false to prevent the form
	    return true; 
	}
	

	
	function downloadsnortlogs(data){
		jQuery('.hiddendownloadlink').append('<iframe width="1" height="1" frameborder="0" src="/snort/snort_json_get.php?snortlogdownload=1&snortlogfilename=' + data.downloadfilename + '" ></iframe>');
		
		var appendElem = jQuery('<br> <span>success...<span>');
		appendElem.appendTo('.loadingWaitingMessage');
		setTimeout(hideLoading('#loadingWaiting'), 3000);
	}	

	// After Save Calls display
	var appendElem = jQuery('<br> <span>success...<span>');
	function finnish(){
		// hold msg for a min
	    setTimeout(function(){ 
			hideLoading('#loadingWaiting');
			appendElem.remove();
			updatestarted = 1;
	    }, 1200 );		
	}
	
	function showResponse(data, responseText, statusText, xhr, $form){

		// START of fill call to user
		if (responseText === 'success') {
			
			// snort logs download success
			if (data.downloadfilename !== '' && data.snortdownload === 'success'){
				downloadsnortlogs(data);		
			}
			
			// succsess display
			if (data.snortgeneralsettings === 'success' || data.snortdelete === 'success' || data.snortreset === 'success'){
				// sucsses msg
				appendElem.appendTo('.loadingWaitingMessage');
				
				// Clean up Waiting code
				finnish();
				
				if (data.snortUnhideTabs === 'true'){
					jQuery('.hide_newtabmenu').show();
				}
				
				if (data.snortreset) {location.reload();} // hard refresh
				
			}			
		
		// END of fill call to user
		}else{
			// On FAIL get some info back
			//alert('responseText: \n' + data.responseText + 'FAIL');
		}
	} 
	// END iform code	

	// declare variable for iform
	var options = {
            beforeSubmit:  showRequest,
            dataType:      'json', 
            success:       showResponse,
            type:          'POST',
            url:           './snort_json_post.php'
        };
	
}); // end of document ready

