<?php
    //
    // vnStat PHP frontend (c)2006-2010 Bjorge Dijkstra (bjd@jooz.net)
    //
    // This program is free software; you can redistribute it and/or modify
    // it under the terms of the GNU General Public License as published by
    // the Free Software Foundation; either version 2 of the License, or
    // (at your option) any later version.
    //
    // This program is distributed in the hope that it will be useful,
    // but WITHOUT ANY WARRANTY; without even the implied warranty of
    // MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    // GNU General Public License for more details.
    //
    // You should have received a copy of the GNU General Public License
    // along with this program; if not, write to the Free Software
    // Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
    //
    //
    // see file COPYING or at http://www.gnu.org/licenses/gpl.html 
    // for more information.
    //
    require 'config.php';
    require 'localize.php';
    require 'vnstat.php';

    validate_input();

    require "./themes/$style/theme.php";

    function svg_create($width, $height)
    {
	header('Content-type: image/svg+xml');
	print "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";
	print "<svg width=\"$width\" height=\"$height\" version=\"1.2\" baseProfile=\"tiny\" xmlns=\"http://www.w3.org/2000/svg\">\n";
	print "<g style=\"shape-rendering: crispEdges\">\n";
    }

    function svg_end()
    {
	print "</g>\n";
	print "</svg>\n";
    }

    function svg_options($options)
    {
	foreach ($options as $key => $value) {
	    print "$key=\"$value\" ";
	}
    }

    function svg_group($options)
    {
	print "<g ";
	svg_options($options);
	print ">\n";
    }

    function svg_group_end()
    {
	print "</g>\n";
    }

    function svg_text($x, $y, $text, $options = array()) 
    {	
	printf("<text x=\"%F\" y=\"%F\" ", $x, $y);
	svg_options($options);
	print ">$text</text>\n";
    }

    function svg_line($x1, $y1, $x2, $y2, $options = array())
    {
	printf("<line x1=\"%F\" y1=\"%F\" x2=\"%F\" y2=\"%F\" ", $x1, $y1, $x2, $y2);
	svg_options($options);
	print "/>\n";
    }

    function svg_rect($x, $y, $w, $h, $options = array()) 
    {
	printf("<rect x=\"%F\" y=\"%F\" width=\"%F\" height=\"%F\" ", $x, $y, $w, $h);
	svg_options($options);
	print "/>\n";
    }

    function svg_poly($points, $options = array())
    {
       print "<polygon points=\"";
       for ($p = 0; $p < count($points); $p += 2) {
	  printf("%F,%F ", $points[$p], $points[$p+1]);
       }
       svg_options($options);
       print "\"/>\n";
    }

    function allocate_color($colors)
    {
	$col['rgb'] = sprintf("#%02X%02X%02X", $colors[0], $colors[1], $colors[2]);
	$col['opacity'] = sprintf("%F", (127 - $colors[3]) / 127);
	return $col;
    }
            
    function init_image()
    {
        global $xlm, $xrm, $ytm, $ybm, $iw, $ih,$graph, $cl, $iface, $colorscheme, $style;

        if ($graph == 'none')
            return;

        //
        // image object
        //    
        $xlm = 70;
        $xrm = 20;
        $ytm = 35;
        $ybm = 60;
        if ($graph == 'small')
        {
            $iw = 300 + $xrm + $xlm;
            $ih = 100 + $ytm + $ybm;    
        }
        else
        {
            $iw = 600 + $xrm + $xlm;
            $ih = 200 + $ytm + $ybm;
        }
	
	svg_create($iw, $ih);

        //
        // colors
        //
	$cs = $colorscheme;
	$cl['image_background'] = allocate_color($cs['image_background']);
	$cl['background'] = allocate_color($cs['graph_background']);
	$cl['background_2'] = allocate_color($cs['graph_background_2']);
        $cl['grid_stipple_1'] = allocate_color($cs['grid_stipple_1']);
        $cl['grid_stipple_2'] = allocate_color($cs['grid_stipple_2']);
        $cl['text'] = allocate_color($cs['text']);
        $cl['border'] = allocate_color($cs['border']);
        $cl['rx'] = allocate_color($cs['rx']);
        $cl['rx_border'] = allocate_color($cs['rx_border']);
        $cl['tx'] = allocate_color($cs['tx']);
        $cl['tx_border'] = allocate_color($cs['tx_border']);
	
        svg_rect(0, 0, $iw, $ih, array( 'stroke' => 'none', 'stroke-width' => 0, 'fill' => $cl['image_background']['rgb']) );
	svg_rect($xlm, $ytm, $iw-$xrm-$xlm, $ih-$ybm-$ytm, array( 'stroke' => 'none', 'stroke-width' => 0, 'fill' => $cl['background']['rgb']) );
	
	$depth = 12;
	svg_group( array( 'stroke' => 'none', 'stroke-width' => 0, 'fill' => $cl['background_2']['rgb'], 'fill-opacity' => $cl['background_2']['opacity']) );
	svg_poly(array($xlm, $ytm, $xlm, $ih - $ybm, $xlm - $depth, $ih - $ybm + $depth, $xlm - $depth, $ytm + $depth));
	svg_poly(array($xlm, $ih - $ybm, $xlm - $depth, $ih - $ybm + $depth, $iw - $xrm - $depth, $ih - $ybm  + $depth, $iw - $xrm, $ih - $ybm));
	svg_group_end();

	// draw title
	$text = T('Traffic data for')." $iface";
	svg_text($iw / 2, ($ytm / 2), $text, array( 'stroke' => $cl['text'], 'fill' => $cl['text']['rgb'],'stroke-width' => 0, 'font-family' => SVG_FONT, 'font-weight' => 'bold', 'text-anchor' => 'middle' ));
    }

    function draw_border()
    {
        global $cl, $iw, $ih;
	svg_rect(1, 1, $iw-2, $ih-2, array( 'stroke' => $cl['border']['rgb'], 'stroke-opacity' => $cl['border']['opacity'], 'stroke-width' => 1, 'fill' => 'none') );
    }
    
    function draw_grid($x_ticks, $y_ticks)
    {
        global $cl, $iw, $ih, $xlm, $xrm, $ytm, $ybm;
        $x_step = ($iw - $xlm - $xrm) / $x_ticks;
        $y_step = ($ih - $ytm - $ybm) / $y_ticks;
	
	$depth = 12;

	svg_group( array( 'stroke' => $cl['grid_stipple_1']['rgb'], 'stroke-opacity' => $cl['grid_stipple_1']['opacity'], 'stroke-width' => '1px', 'stroke-dasharray' => '1,1' ) );
        for ($i = $xlm; $i <= ($iw - $xrm); $i += $x_step)
        {
	    svg_line($i, $ytm, $i, $ih-$ybm);
            svg_line($i, $ih-$ybm, $i-$depth, $ih-$ybm+$depth);
        }
        for ($i = $ytm; $i <= ($ih - $ybm); $i += $y_step)
        {
            svg_line($xlm, $i, $iw - $xrm, $i); 
	    svg_line($xlm, $i, $xlm - $depth, $i + $depth);
        }
	svg_group_end();

	svg_group( array( 'stroke' => $cl['border']['rgb'], 'stroke-width' => '1px', 'stroke-opacity' => $cl['border']['opacity'] ) );
        svg_line($xlm, $ytm, $xlm, $ih - $ybm);
        svg_line($xlm, $ih - $ybm, $iw - $xrm, $ih - $ybm);
	svg_group_end();
    }
    
    
    function draw_data($data)
    {
        global $cl,$iw,$ih,$xlm,$xrm,$ytm,$ybm;

        sort($data);

        $x_ticks = count($data);
        $y_ticks = 10;
        $y_scale = 1;
        $prescale = 1;
        $unit = 'K';
        $offset = 0;
        $gr_h = $ih - $ytm - $ybm;
        $x_step = ($iw - $xlm - $xrm) / $x_ticks;
        $y_step = ($ih - $ytm - $ybm) / $y_ticks;
        $bar_w = ($x_step / 2) ;

        //
        // determine scale
        //
        $low = 99999999999;
        $high = 0;
        for ($i=0; $i<$x_ticks; $i++)
        {
            if ($data[$i]['rx'] < $low)
            $low = $data[$i]['rx'];
            if ($data[$i]['tx'] < $low)
            $low = $data[$i]['tx'];
            if ($data[$i]['rx'] > $high)
            $high = $data[$i]['rx'];
            if ($data[$i]['tx'] > $high)
            $high = $data[$i]['tx'];
        }

        while ($high > ($prescale * $y_scale * $y_ticks))
        {
            $y_scale = $y_scale * 2;
            if ($y_scale >= 1024)
            {
            $prescale = $prescale * 1024;
            $y_scale = $y_scale / 1024;
            if ($unit == 'K') 
                $unit = 'M';
            else if ($unit == 'M')
                $unit = 'G';
            else if ($unit == 'G')
                $unit = 'T';
            }
        }

        draw_grid($x_ticks, $y_ticks);
	
        //
        // graph scale factor (per pixel)
        //
        $sf = ($prescale * $y_scale * $y_ticks) / $gr_h;

        if ($data[0] == 'nodata')
        {
            $text = 'no data available';
	    svg_text($iw/2, $ytm + 80, $text, array( 'stroke' => $cl['text']['rgb'], 'fill' => $cl['text']['rgb'], 'stroke-width' => 0, 'font-family' => SVG_FONT, 'font-size' => '16pt', 'text-anchor' => 'middle') );
        }
        else
        {
            //
            // draw bars
            //      
            for ($i=0; $i<$x_ticks; $i++)
            {
        	$x = $xlm + ($i * $x_step);
        	$y = $ytm + ($ih - $ytm - $ybm) - (($data[$i]['rx'] - $offset) / $sf);
		
		$depth = ($x_ticks < 20) ? 8 : 6;
		$space = 0;
		
		$x1 = (int)$x;
		$y1 = (int)$y;
		$w = (int)($bar_w - $space);
		$h = (int)($ih - $ybm - $y);
		$x2 = (int)($x + $bar_w - $space);
		$y2 = (int)($ih - $ybm);
		
		svg_group( array( 'stroke' => $cl['rx_border']['rgb'], 'stroke-opacity' => $cl['rx_border']['opacity'], 
				  'stroke-width' => 1, 'stroke-linejoin' => 'round',
			          'fill' => $cl['rx']['rgb'], 'fill-opacity' => $cl['rx']['opacity'] ) );
        	svg_rect($x1, $y1, $w, $h);
		svg_rect($x1 - $depth, $y1 + $depth, $w, $h);
		svg_poly(array($x1, $y1, $x2, $y1, $x2 - $depth, $y1 + $depth, $x1 - $depth, $y1 + $depth));
		svg_poly(array($x2, $y1, $x2, $y2, $x2 - $depth, $y2 + $depth, $x2 - $depth, $y1 + $depth));
		svg_group_end();

        	$y1 = (int)($ytm + ($ih - $ytm - $ybm) - (($data[$i]['tx'] - $offset) / $sf));
		$x1 = (int)($x1 + $bar_w);
		$x2 = (int)($x2 + $bar_w);
		$w = (int)($bar_w - $space);
		$h = (int)($ih - $ybm - $y1 - 1);

		svg_group( array( 'stroke' => $cl['tx_border']['rgb'], 'stroke-opacity' => $cl['tx_border']['opacity'], 
				  'stroke-width' => 1, 'stroke-linejoin' => 'round',
			          'fill' => $cl['tx']['rgb'], 'fill-opacity' => $cl['tx']['opacity'] ) );
        	svg_rect($x1, $y1, $w, $h);
		svg_rect($x1 - $depth, $y1 + $depth, $w, $h);
		svg_poly(array($x1, $y1, $x2, $y1, $x2 - $depth, $y1 + $depth, $x1 - $depth, $y1 + $depth));
		svg_poly(array($x2, $y1, $x2, $y2, $x2 - $depth, $y2 + $depth, $x2 - $depth, $y1 + $depth));
		svg_group_end();
            }
    
            //
            // axis labels
            //
	    svg_group( array( 'fill' => $cl['text']['rgb'], 'fill-opacity' => $cl['text']['opacity'], 'stroke-width' => '0', 'font-family' => SVG_FONT, 'font-size' => '10pt', 'text-anchor' => 'end' ) );
            for ($i=0; $i<=$y_ticks; $i++)
            {
                $label = ($i * $y_scale).$unit;
		$tx = $xlm - 16;
		$ty = (int)(($ih - $ybm) - ($i * $y_step) + 8 + $depth);
		svg_text($tx, $ty, $label);
            }
	    svg_group_end();

	    svg_group( array( 'fill' => $cl['text']['rgb'], 'fill-opacity' => $cl['text']['opacity'], 'stroke-width' => '0', 'font-family' => SVG_FONT, 'font-size' => '10pt', 'text-anchor' => 'middle' ) );
            for ($i=0; $i<$x_ticks; $i++)
            {
                $label = $data[$i]['img_label'];
		svg_text($xlm + ($i * $x_step) + ($x_step / 2) - $depth - 4, $ih - $ybm + 20 + $depth, $label);
            }
	    svg_group_end();
        }

        draw_border();


        //
        // legend
        //
        svg_rect($xlm, $ih-$ybm+39, 8, 8, array( 'stroke' => $cl['text']['rgb'], 'stroke-width' => 1, 'fill' => $cl['rx']['rgb']) );
	svg_text($xlm+14, $ih-$ybm+48, T('bytes in'), array( 'fill' => $cl['text']['rgb'], 'stroke-width' => 0, 'font-family' => SVG_FONT, 'font-size' => '8pt') );

        svg_rect($xlm+120 , $ih-$ybm+39, 8, 8, array( 'stroke' => $cl['text']['rgb'], 'stroke-width' => 1, 'fill' => $cl['tx']['rgb']) );
	svg_text($xlm+134, $ih-$ybm+48, T('bytes out'), array( 'fill' => $cl['text']['rgb'], 'stroke-width' => 0, 'font-family' => SVG_FONT, 'font-size' => '8pt') );
    }

    function output_image()
    {
        global $page,$hour,$day,$month,$iface;

        if ($page == 'summary')
            return;

        init_image();

        if ($page == 'h')
        {
            draw_data($hour);
        }
        else if ($page == 'd')
        {
            draw_data($day);
        }
        else if ($page == 'm')
        {
            draw_data($month);
        }

	svg_end();
    }

    get_vnstat_data();
    output_image();
?>        
