<?php
//var_dump('BEING INCLUDED');

function sample_on_load_event(&$widgets)
{
	/* Called when the form is just loaded. You receive the array widgets as
	 * argument, and can modify it to alter the behavior of the form. */
	//var_dump('BEING LOADED');
	//$widget =& $widgets[2];
	$widget =& get_widget($widgets, 'mytextarea');
	$widget->value='This example shows how to change the text inside a text box from within the include file, when the form is loaded.';
}

function sample_on_sync_event()
{
	/* This is called when the form is posted. You generally want to sync the
	 * configurations in the XML file to settings in files spread across the file
	 * system or maybe restart a service here. */
	//var_dump('BEING SYNCED');
}

function sample_on_apply_event()
{
	/* This is called when a configuration has been changed and the user is
	 * prompted to apply the changes and clicks "Apply changes". */
	 var_dump('BEING APPLIED');
}

function sample_on_submit_event()
{
	/* This allows you to perform an action when an specific submit button is
	 * clicked. This should only be used when you have more than one submit
	 * button in the same form. Otherwise, <on_load_event> performs better and
	 * is cleaner. */
	//var_dump('BEING SUBMITTED');
}
?>
