<?php
/* $Id$ */
/* ========================================================================== */
/*
    classdefs.inc
    part of pfSense (http://www.pfSense.com)
    Copyright (C) 2007 Daniel S. Haischt <me@daniel.stefan.haischt.name>
    All rights reserved.

    Based on m0n0wall (http://m0n0.ch/wall)
    Copyright (C) 2003-2006 Manuel Kasper <mk@neon1.net>.
    All rights reserved.
                                                                              */
/* ========================================================================== */
/*
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

     1. Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

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
/* ========================================================================== */

if (! class_exists("Object")) {
    class Object {
        function Object() {
            $args = func_get_args();
            if (method_exists($this, '__destruct')) {
                        register_shutdown_function(array(&$this, '__destruct'));
            }
            call_user_func_array(array(&$this, '__construct'), $args);
        }

        function __construct() {
        }
    }
}

class Column extends Object {
    var $defaultClass = "listhdrr";
    var $class = null;
    var $name = null;
    var $sortID = null;

    function __construct($name) {
        // Perform object initialization here.
        parent::__construct();
        $this->name = $name;
    }

    function setClass($class) {
        $this->class = $class;
    }

    function getClass() {
        return $this->class;
    }

    function setSortID($id) {
        $this->sortID = $id;
    }

    function getSortID() {
        return $this->sortID;
    }

    function toHTML() {
        global $_GET;

        $classString = $this->class != null ? $this->class : $this->defaultClass;
        $allString = isset($_GET['all']) ? ", {$_GET['all']}" : "";
        return "<td class='{$classString}'><a id='{$this->sortID}' href='#' onclick='sortTable(this{$allString});'>{$this->name}</a></td>";
    }
}

class Table extends Object {
    var $columns = null;
    var $data = null;

    function __construct() {
        // Perform object initialization here.
        parent::__construct();
        $this->columns = array();
        $this->data = array();
    }

    function addColumn($column, $id = null, $class = null) {
        $column = new Column($column);

        if ($id != null) {
            $column->setSortID($id);
        }
        if ($class != null) {
            $column->setClass($class);
        }

        $this->columns[] = $column;
    }

    function addDataSet($dataSet) {
        if (is_array($dataSet)) {
            if (count($dataSet) == count($this->columns)) {
                $this->data[] = $dataSet;
            } else {
                trigger_error("Dataset does not match column size.");
            }
        } else {
            trigger_error("Adding non arrays as datasets to a table is not supported.");
        }
    }

    function setData(&$data) {
        $this->data = $data;
    }

    function getData() {
        return $this->data;
    }

    function getColumnHTML() {
        $htmlString = "";

        foreach($this->columns as $column) {
            $htmlString .= $column->toHtml();
        }

        return "<tr>{$htmlString}</tr>";
    }
}

?>