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

    function write_summary()
    {
        global $summary,$top,$day,$hour,$month;

        $trx = $summary['totalrx']*1024+$summary['totalrxk'];
        $ttx = $summary['totaltx']*1024+$summary['totaltxk'];

        //
        // build array for write_data_table
        //
        $sum['hour']['act'] = 1;
        $sum['hour']['rx'] = $hour[0]['rx'];
        $sum['hour']['tx'] = $hour[0]['tx'];

        $sum['day']['act'] = 1;
        $sum['day']['rx'] = $day[0]['rx'];
        $sum['day']['tx'] = $day[0]['tx'];

        $sum['month']['act'] = 1;
        $sum['month']['rx'] = $month[0]['rx'];
        $sum['month']['tx'] = $month[0]['tx'];

        $sum['total']['act'] = 1;
        $sum['total']['rx'] = $trx;
        $sum['total']['tx'] = $ttx;

        print json_encode($sum);
    }


    get_vnstat_data(false);

    header('Content-type: application/json; charset=utf-8');
    $graph_params = "if=$iface&amp;page=$page&amp;style=$style";
    if ($page == 's')
    {
        write_summary();
    }
    else if ($page == 'h')
    {
      print json_encode(array('hours' => $hour));
    }
    else if ($page == 'd')
    {
      print json_encode(array('days' => $day));
    }
    else if ($page == 'm')
    {
      print json_encode(array('months' => $month));
    }
    ?>