<?php
/* $Id$ */
/*
  dspam-analysis-graph.php

  Copyright (C) 2006 Daniel S. Haischt.
  All rights reserved.
*/

/* required because of system_groupmanager.php */
$pgtitle = array(gettext("Services"),
                 gettext("DSPAM"),
                 gettext("Analysis"),
                 gettext("Graph"));

require_once 'Image/Graph.php';
require("guiconfig.inc");
include("/usr/local/pkg/dspam.inc");

if (! $_GET ||
    strlen($_SERVER['QUERY_STRING']) == 0) {
  return;
}

$FORM =& ReadParse($_SERVER['QUERY_STRING']);

list($spam, $nonspam, $period) = split('_', $FORM['data']);
$spam_day = split(',', $spam);
$nonspam_day = split(',', $nonspam);
$period = split(',', $period);

// create the graph
$Graph =& Image_Graph::factory('graph', array(725, 300));

// add a TrueType font
$Font =& $Graph->addNew('font', 'Verdana');
// set the font size to 11 pixels
$Font->setSize(8);

$Graph->setFont($Font);

// setup the plotarea, legend and their layout
$Graph->add(
   Image_Graph::vertical(
      Image_Graph::factory('title', array('', 12)),
      Image_Graph::vertical(
         $Plotarea = Image_Graph::factory('plotarea'),
         $Legend = Image_Graph::factory('legend'),
         88
      ),
      5
   )
);

// link the legend with the plotares
$Legend->setPlotarea($Plotarea);

// create the two datasets
$i = 0;
$spamds =& Image_Graph::factory('dataset');
foreach($spam_day as $el){
  $spamds->addPoint(strval($period[$i]), intval($el));
  $i++;
}

$i = 0;
$hamds =& Image_Graph::factory('dataset');
foreach($nonspam_day as $el){
  $hamds->addPoint(strval($period[$i]), intval($el));
  $i++;
}

// set the name/title of each dataset
$spamds->setName('SPAM');
$hamds->setName('Good');

// put each dataset in a singel ds array
$Datasets = array($spamds, $hamds);

// create the plot as line chart using the dataset
$Plot =& $Plotarea->addNew('Image_Graph_Plot_Line', array($Datasets,'normal'));

// set a line color
$LineArray =& Image_Graph::factory('Image_Graph_Line_Array');
$LineArray->addColor('red');
$LineArray->addColor('green');

// set a standard line style
$Plot->setLineStyle($LineArray);

/* set axis labels */
$XAxis =& $Plotarea->getAxis(IMAGE_GRAPH_AXIS_X);
$XAxis->setTitle("{$FORM['x_label']}");
$YAxis =& $Plotarea->getAxis(IMAGE_GRAPH_AXIS_Y);
$YAxis->setTitle('Number of Messages', 'vertical');

// output the Graph
$Graph->done();

function &ReadParse($URI = "") {
  if ($URI == "") {
    return NULL;
  }

  $pairs = preg_split('/&/', $URI);
  $FORM = array();

  foreach($pairs as $pair){
    list($name, $value) = preg_split('/\=/', $pair);
    $pattern = '/%([a-fA-F0-9][a-fA-F0-9])/';

    $name = preg_replace('/\+/', ' ', $name);
    $name = preg_replace_callback(
      $pattern,
      create_function(
        '$matches',
        'return pack("C", hexdec($matches[1]));'
      ),
      $name
    );

    $value = preg_replace('/\+/', ' ', $value);
    $value = preg_replace_callback(
      $pattern,
      create_function(
        '$matches',
        'return pack("C", hexdec($matches[1]));'
      ),
      $value
    );

    $FORM[$name] = $value;
  } // end foreach

  return $FORM;
}
?>
