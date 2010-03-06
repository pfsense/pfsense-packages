<?php

/*
    status_ospfd.php
    Copyright (C) 2010 Nick Buraglio; nick@buraglio.com
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
    INClUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
    AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
    OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
*/


echo "<h2>Basic OSPFd statistics </h2><br>";

echo "<h3> OSPF Summary </h3>";
echo '<pre>';

$summary = system('/usr/local/sbin/ospfctl show summary', $summary);

echo "<h3> OSPF Neighbors </h3>";
echo '<pre>';

$neighbor = system('/usr/local/sbin/ospfctl show neighbor', $neighbor);

echo "<h3> FIB </h3>";
echo '<pre>';

$rib = system('/usr/local/sbin/ospfctl show fib', $rib);

echo "<h3> RIB </h3>";
echo '<pre>';

$fib = system('/usr/local/sbin/ospfctl show rib', $fib);

echo "<h3> OSPF Interfaces </h3>";
echo '<pre>';

$interfaces = system('/usr/local/sbin/ospfctl show interfaces', $interfaces);

echo "<h3> OSPF Database </h3>";
echo '<pre>';

$database = system('/usr/local/sbin/ospfctl show database', $database);


?>

