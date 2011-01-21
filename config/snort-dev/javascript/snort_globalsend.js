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


// ------------------------------- START remove row element ------------------------------------------
    jQuery(".icon_x").live('click', function() {
        
        var elem = getBaseElement(this.id); // this.id gets id of .icon_x

        window.RemoveRow_UUID = jQuery("#rowlist_" + elem.index).data("options").rowuuid;
        window.RemoveRow_Table = jQuery("#maintable").data("options").pagetable;
        window.RemoveRow_POST = jQuery("#maintable").data("options").DoPOST;
        
        if (RemoveRow_Table == 'SnortWhitelist')  // snort_interfaces_whitelist
        {
          if(confirm('Do you really want to delete this whitelist? (e.g. snort rules will fall back to the default whitelist)!')) {
                      
            jQuery("#rowlist_" + elem.index).fadeOut("fast");           
            function removeRow() 
            {
              jQuery("#rowlist_" + elem.index).remove();
            }
                      
            setTimeout(removeRow, 600);
            jQuery(this).ajaxSubmit(optionsWhitelist); // call POST     
            return false;
          }
        }
        
        if (RemoveRow_Table == 'SnortWhitelistips')
        { // editing snort_interfaces_whitelist_edit
                  
          jQuery("#rowlist_" + elem.index).fadeOut("fast");       
          function removeRow()
          {
            jQuery("#rowlist_" + elem.index).remove();
          }
          
          setTimeout(removeRow, 600);         
          return false;
        }
        
    });
    
  // declare variable for whitelist delete
  var optionsWhitelist = {
            beforeSubmit:  showRequestWhitelist,
            dataType:      'json', 
            success:       showResponseWhitelist,
            type: 'POST',
            data:         { WhitelistDelRow: '1', WhitelistTable: WhitelistTableDelCall, WhitelistUuid: WhitelistUuidDelCall },
            url:      './snort_json_post.php'
        }; 
    
    function WhitelistTableDelCall() {
        return RemoveRow_Table;
    }
  
    function WhitelistUuidDelCall() {
        return RemoveRow_UUID;
    }
  
    // pre-submit callback 
    function showRequestWhitelist(formData, jqForm, optionsWhitelist) { 
      
        var queryString = jQuery.param(formData);
     
        alert('About to submit: \n\n' + queryString); 
        
        // call false to prevent the form
        return true; 
    }
    
    // pre-submit callback 
    function showResponseWhitelist(data) { 
      
      
    }
    
    function getBaseElement(elem)
    {
      elem = elem + "";
      var len     = elem.length;
      var lPos    = elem.lastIndexOf("_") * 1;
      var baseElem  = elem.substr(0, lPos);
      var index   = elem.substr(lPos+1, len);
      
      //alert(index);
      
      if (checkNumeric(index))
      {
        return {"base": baseElem, "index": index};
      }else{
        return {"base": null, "index": null};
      };
      
      function checkNumeric(value)
      {
              if(isNaN(value)) {
                  return false;
              }else{
                  return true;
              };
      };
      
    };
  // STOP remove row element
	
// ------------------- START iform Submit/RETURN code ---------------------------------------------
	
	/* general form */
	jQuery('#iform').submit(function() { 

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
            success:       showResponse
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
		if (data.snortgeneralsettings == 'success' || data.snortdelete == 'success' || data.snortreset == 'success') {
			var appendElem = jQuery('<br> <span>success...<span>');
			appendElem.appendTo('.loadingWaitingMessage');
			
			// remove display
			function finnish() {
			hideLoading();
			appendElem.remove();
			updatestarted = 1;
			};			
			setTimeout(finnish, 2000);
			if (data.snortreset) {location.reload();}; // hard refresh
			
		};
	
	// END of fill call to user
	}else{
		// On FAIL get some info back
		alert('responseText: \n' + data.responseText + 'FAIL');
	}
} 
// END iform code







