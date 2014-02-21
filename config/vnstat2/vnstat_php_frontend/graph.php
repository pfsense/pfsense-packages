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

    function allocate_color($im, $colors)
    {
	return imagecolorallocatealpha($im, $colors[0], $colors[1], $colors[2], $colors[3]);
    }
            
    function init_image()
    {
        global $im, $xlm, $xrm, $ytm, $ybm, $iw, $ih,$graph, $cl, $iface, $colorscheme, $style;

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

        $im = imagecreatetruecolor($iw,$ih);

        //
        // colors
        //
	$cs = $colorscheme;
	$cl['image_background'] = allocate_color($im, $cs['image_background']);
	$cl['background'] = allocate_color($im, $cs['graph_background']);
	$cl['background_2'] = allocate_color($im, $cs['graph_background_2']);
        $cl['grid_stipple_1'] = allocate_color($im, $cs['grid_stipple_1']);
        $cl['grid_stipple_2'] = allocate_color($im, $cs['grid_stipple_2']);
        $cl['text'] = allocate_color($im, $cs['text']);
        $cl['border'] = allocate_color($im, $cs['border']);
        $cl['rx'] = allocate_color($im, $cs['rx']);
        $cl['rx_border'] = allocate_color($im, $cs['rx_border']);
        $cl['tx'] = allocate_color($im, $cs['tx']);
        $cl['tx_border'] = allocate_color($im, $cs['tx_border']);
	
        imagefilledrectangle($im,0,0,$iw,$ih,$cl['image_background']);
	imagefilledrectangle($im,$xlm,$ytm,$iw-$xrm,$ih-$ybm, $cl['background']);
	
	$x_step = ($iw - $xlm - $xrm) / 12;
	$depth = ($x_step / 8) + 4;
	imagefilledpolygon($im, array($xlm, $ytm, $xlm, $ih - $ybm, $xlm - $depth, $ih - $ybm + $depth, $xlm - $depth, $ytm + $depth), 4, $cl['background_2']);
	imagefilledpolygon($im, array($xlm, $ih - $ybm, $xlm - $depth, $ih - $ybm + $depth, $iw - $xrm - $depth, $ih - $ybm  + $depth, $iw - $xrm, $ih - $ybm), 4, $cl['background_2']);

	// draw title
	$text = T('Traffic data for')." $iface";
 	$bbox = imagettfbbox(10, 0, GRAPH_FONT, $text);
	$textwidth = $bbox[2] - $bbox[0];
	imagettftext($im, 10, 0, ($iw-$textwidth)/2, ($ytm/2), $cl['text'], GRAPH_FONT, $text);
		
    }

    function draw_border()
    {
        global $im,$cl,$iw,$ih;

        imageline($im,     0,    0,$iw-1,    0, $cl['border']);
        imageline($im,     0,$ih-1,$iw-1,$ih-1, $cl['border']);
        imageline($im,     0,    0,    0,$ih-1, $cl['border']);  
        imageline($im, $iw-1,    0,$iw-1,$ih-1, $cl['border']);
    }
    
    function draw_grid($x_ticks, $y_ticks)
    {
        global $im, $cl, $iw, $ih, $xlm, $xrm, $ytm, $ybm;
        $x_step = ($iw - $xlm - $xrm) / $x_ticks;
        $y_step = ($ih - $ytm - $ybm) / $y_ticks;
	
	$depth = 10;//($x_step / 8) + 4;

        $ls = array($cl['grid_stipple_1'],$cl['grid_stipple_2']);
        imagesetstyle($im, $ls);
        for ($i=$xlm;$i<=($iw-$xrm); $i += $x_step)
        {
            imageline($im, $i, $ytm, $i, $ih - $ybm, IMG_COLOR_STYLED);
	    imageline($im, $i, $ih - $ybm, $i - $depth, $ih - $ybm + $depth, IMG_COLOR_STYLED);
        }
        for ($i=$ytm;$i<=($ih-$ybm); $i += $y_step)
        {
            imageline($im, $xlm, $i, $iw - $xrm, $i, IMG_COLOR_STYLED); 
	    imageline($im, $xlm, $i, $xlm - $depth, $i + $depth, IMG_COLOR_STYLED);
        }
        imageline($im, $xlm, $ytm, $xlm, $ih - $ybm, $cl['border']);
        imageline($im, $xlm, $ih - $ybm, $iw - $xrm, $ih - $ybm, $cl['border']);
    }
    
    
    function draw_data($data)
    {
        global $im,$cl,$iw,$ih,$xlm,$xrm,$ytm,$ybm;

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
	imagesetthickness($im, 1);
        $sf = ($prescale * $y_scale * $y_ticks) / $gr_h;

        if ($data[0] == 'nodata')
        {
            $text = 'no data available';
	    $bbox = imagettfbbox(10, 0, GRAPH_FONT, $text);
	    $textwidth = $bbox[2] - $bbox[0];
	    imagettftext($im, 10, 0, ($iw-$textwidth)/2, $ytm + 80, $cl['text'], GRAPH_FONT, $text);
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
		
		$depth = $x_step / 8;
		$space = 0;
		
		$x1 = $x;
		$y1 = $y;
		$x2 = $x + $bar_w - $space;
		$y2 = $ih - $ybm;
		
        	imagefilledrectangle($im, $x1, $y1, $x2, $y2, $cl['rx']);
		imagerectangle($im, $x1, $y1, $x2, $y2, $cl['rx_border']);
		
		imagefilledrectangle($im, $x1 - $depth, $y1 + $depth, $x2 -$depth, $y2 + $depth, $cl['rx']);
		imagerectangle($im, $x1 - $depth, $y1 + $depth, $x2 - $depth, $y2 + $depth, $cl['rx_border']);
		
		imagefilledpolygon($im, array($x1, $y1, $x2, $y1, $x2 - $depth, $y1 + $depth, $x1 - $depth, $y1 + $depth), 4, $cl['rx']);
		imagepolygon($im, array($x1, $y1, $x2, $y1, $x2 - $depth, $y1 + $depth, $x1 - $depth, $y1 + $depth), 4, $cl['rx_border']);
		imagefilledpolygon($im, array($x2, $y1, $x2, $y2, $x2 - $depth, $y2 + $depth, $x2 - $depth, $y1 + $depth), 4, $cl['rx']);
		imagepolygon($im, array($x2, $y1, $x2, $y2, $x2 - $depth, $y2 + $depth, $x2 - $depth, $y1 + $depth), 4, $cl['rx_border']);

        	$y1 = $ytm + ($ih - $ytm - $ybm) - (($data[$i]['tx'] - $offset) / $sf);
		$x1 = $x1 + $bar_w;
		$x2 = $x2 + $bar_w;

        	imagefilledrectangle($im, $x1, $y1, $x2, $y2, $cl['tx']);
		imagerectangle($im, $x1, $y1, $x2, $y2, $cl['tx_border']);
		
        	imagefilledrectangle($im, $x1 - $depth, $y1 + $depth, $x2 - $depth, $y2 + $depth, $cl['tx']);
		imagerectangle($im, $x1 - $depth, $y1 + $depth, $x2 - $depth, $y2 + $depth, $cl['tx_border']);		
		
		imagefilledpolygon($im, array($x1, $y1, $x2, $y1, $x2 - $depth, $y1 + $depth, $x1 - $depth, $y1 + $depth), 4, $cl['tx']);
		imagepolygon($im, array($x1, $y1, $x2, $y1, $x2 - $depth, $y1 + $depth, $x1 - $depth, $y1 + $depth), 4, $cl['tx_border']);
		imagefilledpolygon($im, array($x2, $y1, $x2, $y2, $x2 - $depth, $y2 + $depth, $x2 - $depth, $y1 + $depth), 4, $cl['tx']);
		imagepolygon($im, array($x2, $y1, $x2, $y2, $x2 - $depth, $y2 + $depth, $x2 - $depth, $y1 + $depth), 4, $cl['tx_border']);
            }
    
            //
            // axis labels
            //
            for ($i=0; $i<=$y_ticks; $i++)
            {
                $label = ($i * $y_scale).$unit;
		$bbox = imagettfbbox(8, 0, GRAPH_FONT, $label);
		$textwidth = $bbox[2] - $bbox[0];
		imagettftext($im, 8, 0, $xlm - $textwidth - 16, ($ih - $ybm) - ($i * $y_step) + 8 + $depth, $cl['text'], GRAPH_FONT, $label);
            }

            for ($i=0; $i<$x_ticks; $i++)
            {
                $label = $data[$i]['img_label'];
		$bbox = imagettfbbox(9, 0, GRAPH_FONT, $label);
		$textwidth = $bbox[2] - $bbox[0];
		imagettftext($im, 9, 0, $xlm + ($i * $x_step) + ($x_step / 2) - ($textwidth / 2) - $depth - 4, $ih - $ybm + 20 + $depth, $cl['text'], GRAPH_FONT, $label);
            }
        }

        draw_border();


        //
        // legend
        //
        imagefilledrectangle($im, $xlm, $ih-$ybm+39, $xlm+8,$ih-$ybm+47,$cl['rx']);
        imagerectangle($im, $xlm, $ih-$ybm+39, $xlm+8,$ih-$ybm+47,$cl['text']);
	imagettftext($im, 8,0, $xlm+14, $ih-$ybm+48,$cl['text'], GRAPH_FONT,'bytes in');

        imagefilledrectangle($im, $xlm+120 , $ih-$ybm+39, $xlm+128,$ih-$ybm+47,$cl['tx']);
        imagerectangle($im, $xlm+120, $ih-$ybm+39, $xlm+128,$ih-$ybm+47,$cl['text']);
	imagettftext($im, 8,0, $xlm+134, $ih-$ybm+48,$cl['text'], GRAPH_FONT,'bytes out'); 
    }

    function output_image()
    {
        global $page,$hour,$day,$month,$im,$iface;

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
	
        header('Content-type: image/png');	
        imagepng($im);
    }

    get_vnstat_data();
    output_image();
?>        
