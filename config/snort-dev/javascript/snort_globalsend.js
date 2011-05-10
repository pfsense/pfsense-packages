jQuery.noConflict();

//prepare the form when the DOM is ready 
jQuery(document).ready(function() { 

		jQuery(".icon_click").live('mouseover', function() {
			jQuery(this).css('cursor', 'pointer');
		});

  // -------------------------- START cancel form code ------------------------------------------- 
  //jQuery('#cancel').click(function() {
  jQuery('#cancel').live('click', function() {
    
    location.reload();
    
  });

  
// ------------------------------- START add row element ------------------------------------------
  
  jQuery(".icon_plus").live('click', function() {
	  
	  	var NewRow_UUID = genUUID();
	  	var rowNumCount = jQuery("#address").length;
	  	
	  	if (rowNumCount > 0)
	  	{
	  		// stop empty
		    var prevAddressAll_ck = jQuery('tr[id^=maintable_]');
			var prevAddress_ck = prevAddressAll_ck[prevAddressAll_ck.length-1].id;
			var prevAddressEmpty_ck = jQuery('#' + prevAddress_ck + ' #address').val();
			var prevAddressEmpty_ck = jQuery.trim(prevAddressEmpty_ck);
		  	
			if (prevAddressEmpty_ck == '') 
			{
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
    jQuery(".icon_x").live('click', function() {
        
        var elem = getBaseElement(this.id); // this.id gets id of .icon_x

        // window.RemoveRow_UUID = jQuery("#rowlist_" + elem.index).data("options").rowuuid;
        window.RemoveRow_UUID = elem.index;        
        window.RemoveRow_Table = jQuery("#maintable_" + window.RemoveRow_UUID).data("options").pagetable;
        window.RemoveRow_DB = jQuery("#maintable_" + window.RemoveRow_UUID).data("options").pagedb;
        window.RemoveRow_POST = jQuery("#maintable_" + window.RemoveRow_UUID).data("options").DoPOST;       
        
        if (window.RemoveRow_POST == 'true')  // snort_interfaces_whitelist
        {
          if(confirm('Do you really want to delete this list? (e.g. snort rules will fall back to the default list)!')) {
        	  
            jQuery("#maintable_" + window.RemoveRow_UUID).fadeOut("fast");           
            function removeRow() 
            {
              jQuery("#maintable_" + window.RemoveRow_UUID).remove();
            }
                      
            setTimeout(removeRow, 600);
            jQuery(this).ajaxSubmit(optionsRMlist); // call POST     
            return false;
          }
        }
        
        // remove element NO post
        if (window.RemoveRow_POST == 'false')
        {
                  
          jQuery("#maintable_" + window.RemoveRow_UUID).fadeOut("fast");       
          function removeRow()
          {
            jQuery("#maintable_" + window.RemoveRow_UUID).remove();
          }          
          setTimeout(removeRow, 600);
          
          return false;   
          
        }
        
    });
    
  // declare variable for whitelist delete
  var optionsRMlist = {
            beforeSubmit:  showRequestRMlist,
            dataType:      'json', 
            success:       showResponseRMlist,
            type:          'POST',
            data:          { RMlistDelRow: '1', RMlistDB: RMlistDBDelCall, RMlistTable: RMlistTableDelCall, RMlistUuid: RMlistUuidDelCall },
            url:           './snort_json_post.php'
        }; 

	function RMlistDBDelCall() {
		return RemoveRow_DB;
	}
  
    function RMlistTableDelCall() {
        return RemoveRow_Table;
    }
  
    function RMlistUuidDelCall() {
        return RemoveRow_UUID;
    }
  
    // pre-submit callback 
    function showRequestRMlist(formData, jqForm, optionsWhitelist) { 
      
        var queryString = jQuery.param(formData);
     
        //alert('About to submit: \n\n' + queryString); 
        
        // call false to prevent form reload
        return true; 
    }
    
    // post-submit callback if snort_json_post.php returns true or false
    function showResponseRMlist(data) { 
      
    	//alert('test');
      
    }
    
    function getBaseElement(elem)
    {
      elem = elem + "";
      var len     = elem.length;
      var lPos    = elem.lastIndexOf("_") * 1;
      var baseElem  = elem.substr(0, lPos);
      var index   = elem.substr(lPos+1, len);
      
      return {"base": baseElem, "index": index};
      
    }
  // STOP remove row element
	
// ------------------- START iform Submit/RETURN code ---------------------------------------------
	
	/* general form */
	//jQuery('#iform').submit(function() { 
	jQuery('#iform').live('submit', function() {

		jQuery(this).ajaxSubmit(options);

        return false; 
    });
	
	/* general form2 */
	jQuery('#iform2').submit(function() { 

		jQuery(this).ajaxSubmit(options); 

        return false; 
    });	

	/* general form3 */
	jQuery('#iform3').submit(function() { 

		jQuery(this).ajaxSubmit(options); 

        return false; 
    });
	
	// declare variable for iform
	var options = {
            beforeSubmit:  showRequest,
            dataType:      'json', 
            success:       showResponse,
            type:          'POST',
            url:           './snort_json_post.php'
        }; 
	
}); 

// pre-submit callback 
function showRequest(formData, jqForm, options) { 
	
    var queryString = jQuery.param(formData); 
    
    // Please wait code
	function showLoading() {
		  jQuery("#loadingWaiting").show();
		}
	// call to please wait	
	showLoading();
 
    alert('About to submit: \n\n' + queryString); 
    
    // call false to prevent the form
    return true; 
} 
 
// post-submit callback 
function showResponse(data, responseText, statusText, xhr, $form)  { 
    
	
	function snortUnhideTabsCall() {
		// unhide tabs for iface edit
		if (data.snortUnhideTabs == 'true')
		{
			jQuery('.hide_newtabmenu').show();
		}
	};
	
	function hideLoading() {
		  jQuery("#loadingWaiting").hide();
	};	
	
	// START of fill call to user
	if (responseText == 'success') {
		
		// snort logs download success
		if (data.downloadfilename != '' && data.snortdownload == 'success') {
			function downloadsnortlogs(){
				jQuery('.hiddendownloadlink').append('<iframe width="1" height="1" frameborder="0" src="./snort_json_get.php?snortlogdownload=1&snortlogfilename=' + data.downloadfilename + '></iframe>');
				var appendElem = jQuery('<br> <span>success...<span>');
				appendElem.appendTo('.loadingWaitingMessage');
				setTimeout(hideLoading, 3000);
			}
		downloadsnortlogs();
		}
		
		// succsess display
		if (data.snortgeneralsettings == 'success' || data.snortdelete == 'success' || data.snortreset == 'success') 
		{
			var appendElem = jQuery('<br> <span>success...<span>');
			appendElem.appendTo('.loadingWaitingMessage');
			
			// After Save Calls display
			function finnish() {
			snortUnhideTabsCall();
			hideLoading();
			appendElem.remove();
			updatestarted = 1;
			};			
			setTimeout(finnish, 2000);
			
			if (data.snortreset) {location.reload();}; // hard refresh
			
		}
		
	
	// END of fill call to user
	}else{
		// On FAIL get some info back
		alert('responseText: \n' + data.responseText + 'FAIL');
	}
} 
// END iform code



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

//-----------------STOP Misc----------------------------------------------










