<?php
/* $Id$ */
/*
  dspam-admin-graph.php

  Copyright (C) 2006 Daniel S. Haischt.
  All rights reserved.
*/

/* required because of system_groupmanager.php */
$pgtitle = array(gettext("Services"),
                 gettext("DSPAM"),
                 gettext("System Status"),
                 gettext("Graph"));

require_once 'Image/Graph.php';
require_once 'Image/Canvas.php';
require("guiconfig.inc");
include("/usr/local/pkg/dspam.inc");

if (! $_GET ||
    strlen($_SERVER['QUERY_STRING']) == 0 ||
    ! isDSPAMAdmin($HTTP_SERVER_VARS['AUTH_USER'])) {
  return;
}

$FORM =& ReadParse($_SERVER['QUERY_STRING']);

list($spam, $nonspam, $sm, $fp, $inoc, $wh, $period) = split('_', $FORM['data']);
$spam = split(',', $spam);
$nonspam = split(',', $nonspam);
$sm = split(',', $sm);
$fp = split(',', $fp);
$inoc = split(',', $inoc);
$wh = split(',', $wh);
$period = split(',', $period);

// create a PNG canvas and enable antialiasing (canvas implementation)
$Canvas =& Image_Canvas::factory('png', array('width' => 725,
                                              'height' => 450,
                                              'antialias' => 'native'));

// create the graph
$Graph =& Image_Graph::factory('graph', $Canvas);
// add a TrueType font
$Font =& $Graph->addNew('font', 'Verdana');
// set the font size to 8 pixels
$Font->setSize(8);

$Graph->setFont($Font);

// setup the plotarea, legend and their layout
$Graph->add(
   Image_Graph::vertical(
      Image_Graph::factory('title', array('', 12)),
      Image_Graph::horizontal(
         $Plotarea = Image_Graph::factory('plotarea'),
         $Legend = Image_Graph::factory('legend'),
         80
      ),
      0
   )
);

// add grids
$Grid =& $Plotarea->addNew('line_grid', IMAGE_GRAPH_AXIS_Y);
$Grid->setLineColor('silver');

// link the legend with the plotares
$Legend->setPlotarea($Plotarea);

// create the two datasets
$i = 0;
$spamds =& Image_Graph::factory('dataset');
foreach($spam as $el){
  $spamds->addPoint(strval($period[$i]), intval($el));
  $i++;
}

$i = 0;
$hamds =& Image_Graph::factory('dataset');
foreach($nonspam as $el){
  $hamds->addPoint(strval($period[$i]), intval($el));
  $i++;
}

$i = 0;
$smds =& Image_Graph::factory('dataset');
foreach($sm as $el){
  $smds->addPoint(strval($period[$i]), intval($el));
  $i++;
}

$i = 0;
$fpds =& Image_Graph::factory('dataset');
foreach($fp as $el){
  $fpds->addPoint(strval($period[$i]), intval($el));
  $i++;
}

$i = 0;
$inocds =& Image_Graph::factory('dataset');
foreach($inoc as $el){
  $inocds->addPoint(strval($period[$i]), intval($el));
  $i++;
}

$i = 0;
$whds =& Image_Graph::factory('dataset');
foreach($wh as $el){
  $whds->addPoint(strval($period[$i]), intval($el));
  $i++;
}

// set the name/title of each dataset
$spamds->setName('SPAM');
$hamds->setName('Nonspam');
$smds->setName('Spam Misses');
$fpds->setName('False Positives');
$inocds->setName('Inoculations');
$whds->setName('Auto-Whitelisted');

// put each dataset in a singel ds array
$Datasets = array($inocds, $whds, $spamds, $hamds, $smds, $fpds);

// create the plot as line chart using the dataset
$Plot =& $Plotarea->addNew('Image_Graph_Plot_Bar', array($Datasets,'stacked'));

// set a fill color
$FillArray =& Image_Graph::factory('Image_Graph_Fill_Array');
$FillArray->addColor('#000000');
$FillArray->addColor('#BF00BF');
$FillArray->addColor('#BF0000');
$FillArray->addColor('#00BF00');
$FillArray->addColor('#BFBF00');
$FillArray->addColor('#FF7F00');

// set a standard fill style
$Plot->setFillStyle($FillArray);

/* set axis labels */
$XAxis =& $Plotarea->getAxis(IMAGE_GRAPH_AXIS_X);
$XAxis->setTitle("{$FORM['x_label']}", array('size' => 8, 'angle' => 0));
$XAxis->setFontAngle(60);
$XAxis->setLabelOptions(array('offset'   => intval($FORM['offset']),
                              'showtext' => true,
                              'position' => 'outside'), 1);


$YAxis =& $Plotarea->getAxis(IMAGE_GRAPH_AXIS_Y);
$YAxis->setTitle('Number of Messages', array('size' => 8, 'angle' => 90));

// create a Y data value marker
$Marker =& $Plot->addNew('Image_Graph_Marker_Value', IMAGE_GRAPH_VALUE_Y);
$Marker->setFontSize(6);

// and use the marker on the 1st plot
$Plot->setMarker($Marker);
$Plot->setDataSelector(Image_Graph::factory('Image_Graph_DataSelector_NoZeros'));

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
