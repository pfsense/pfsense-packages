<?php
/* $Id$ */
/* ========================================================================== */
/*
    authng_classdefs.xml
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
require("headjs.inc");

/* if user has selected a custom template, use it.
 * otherwise default to pfsense tempalte
 */
if($config['theme'] <> "") {
    $g['theme'] = $config['theme'];
} else {
    $g['theme'] = "pfsense";
}

// navigation level separator string
$navlevelsep = ": ";

function gentitle($title) {
    global $navlevelsep;

    if(!is_array($title)) {
        return $title;
    }

    return join($navlevelsep, $title);
}

function genhtmltitle($title) {
    global $config;
    //return $config['system']['hostname'] . "." . $config['system']['domain'] . " - " . gentitle($title);
    return gentitle($title);
}

$scriptName = split("/", $_SERVER["SCRIPT_FILENAME"]);
$scriptElms = count($scriptName);
$scriptName = $scriptName[$scriptElms-1];

$pfSenseHead = new pfSenseHTMLHead();
$pfSenseHead->setCloseHead(true);
$pfSenseHead->setTitle(genhtmltitle($pgtitle));

/* all.css has to be treated a bit different, compared to generic stylesheets */
$allID = $pfSenseHead->addLink("<link rel=\"stylesheet\" type=\"text/css\" href=\"/themes/" . $g['theme'] . "/all.css\" media=\"all\" />\n");
$pfSenseHead->setAllCssID($allID);

$pfSenseHead->addLink("<link rel='shortcut icon' href='/themes/{$g['theme']}/images/icons/favicon.ico' />\n");
$pfSenseHead->addScript("<script type='text/javascript'>\nvar theme = '" . $g['theme'] . "';\nvar dontUseCustomBGColor = true;\n</script>\n", 1);
$pfSenseHead->addScript("<script type='text/javascript' src=\"/themes/" . $g['theme'] . "/loader.js\"></script>\n", 2);
//TODO: if ((($_POST || $_GET || isAjax()) &&
if ((($_POST || $_GET) &&
    is_array($error_bucket)) ||
    strpos($_SERVER['SCRIPT_NAME'], "wizard.php") !== false) {
  $pfSenseHead->addScript("<script type='text/javascript' src='/javascript/domTT/domLib.js'></script>", 500);
  $pfSenseHead->addScript("<script type='text/javascript' src='/javascript/domTT/domTT.js'></script>", 510);
  $pfSenseHead->addScript("<script type='text/javascript' src='/javascript/domTT/behaviour.js'></script>", 520);
  $pfSenseHead->addScript("<script type='text/javascript' src='/javascript/domTT/fadomatic.js'></script>", 530);
}

/*
 * Find all javascript files that need to be included
 * for this page ... from the arrays ... :)
 * Coded by: Erik Kristensen
 */
$scriptWeight = 100;

$dir  = trim(basename($_SERVER["SCRIPT_FILENAME"]), '.php');
$path = "/usr/local/www/javascript/" . $dir . "/";
if (is_dir($path)) {
  if ($dh = opendir($path)) {
    while (($file = readdir($dh)) !== false) {
      if (is_dir($file)) { continue; }
      if (strpos($file, ".js") === false) { continue; }

      $pfSenseHead->addScript("<script type='text/javascript' src='/javascript/{$dir}/{$file}'></script>\n", $scriptWeight);
      $scriptWeight++;
    }
    closedir($dh);
  }
}

/*
 * Find all JavaScript files that may be provided by the current theme
 * TODO: Commented because this pulls in PHP5 specific stuff from the theme.
 *
 */
//$path = "/usr/local/www/themes/{$g['theme']}/javascript/";

//if (is_dir($path)) {
//  if ($dh = opendir($path)) {
//    while (($file = readdir($dh)) !== false) {
//      if (is_dir($file)) { continue; }
//      if (strpos($file, ".js") !== false) {
//        $pfSenseHead->addScript("<script type='text/javascript' src='/themes/{$g['theme']}/javascript/{$file}'></script>\n", $scriptWeight);
//      } else if (strpos($file, ".php") !== false &&
//                 strpos($file, "-head") !== false &&
//                 strpos($file, ".disabled") === false) {
//        $filename = ucfirst(trim(trim($file, '.php'), '-head'));
//        require_once("themes/{$g['theme']}/javascript/{$file}");

//        if (function_exists("{$g['theme']}{$filename}GetHeadJS")) {
//          $jsfunction = "{$g['theme']}{$filename}GetHeadJS";
//          $jscript = $jsfunction();
//          $pfSenseHead->addScript("<script type='text/javascript'>\n<!--\n{$jscript}\n-->\n</script>\n", $scriptWeight);
//        }
//      } else {
//        continue;
//      }

//      $scriptWeight++;
//    }
//    closedir($dh);
//  }
//}

/*
 * Find all JavaScript events that may be provided by the current theme
 *
 */
$path = "/usr/local/www/themes/{$g['theme']}/jsevents/";
if (is_dir($path)) {
  if ($dh = opendir($path)) {
    while (($file = readdir($dh)) !== false) {
      if (is_dir($file)) { continue; }
      if (strpos($file, ".def") !== false) {
        if (empty($jsevents)) { $jsevents = array(); }

        switch ($file) {
          case "body.def":
            $contents = file_get_contents("/usr/local/www/themes/{$g['theme']}/jsevents/{$file}");
            $contents_a = split("\n", $contents);
            foreach ($contents_a as $line) {
              if (strpos($line, "#") === 0) { continue; }
              if (strpos($line, "!") !== false) {
                $events_forbidden_pages = split("!", $line);
                $keyval = split("=", $events_forbidden_pages[0]);

                if (strpos($events_forbidden_pages[1], basename($_SERVER['SCRIPT_NAME'])) !== false) { continue; }
              } else {
                $keyval = split("=", $line);
              }
              $jsevents["body"][$keyval[0]] = $keyval[1];
            }
            break;
        }
      } else {
        continue;
      }
    }
    closedir($dh);
  }
}

/*
 * Find all CSS files that may be provided by the current theme
 * TODO: Not needed right now.
 */
//$path = "/usr/local/www/themes/{$g['theme']}/styles/";
//if (is_dir($path)) {
//  if ($dh = opendir($path)) {
//    while (($file = readdir($dh)) !== false) {
//      if (is_dir($file)) { continue; }
//      if (strpos($file, ".css") === false) { continue; }

//      $pfSenseHead->addLink("<link rel='stylesheet' type='text/css' href='/themes/{$g['theme']}/styles/{$file}' media='all' />\n");
//    }
//    closedir($dh);
//  }
//}

if ($oSajax) {
$pfSenseHead->addScript("<script type='text/javascript'>\n" .
                        $oSajax->sajax_get_javascript() . "\n</script>\n", ++$scriptWeight);
}

// TODO: This line needs to be commented if any PHP calls
// $pfSenseHead->getHTML(); on its own.
//echo $pfSenseHead->getHTML();

/**
 * pfSenseHTMLHead
 *
 * @package www
 * @author Daniel S. Haischt <me@daniel.stefan.haischt.name>
 * @copyright Copyright (c) 2006
 * @version $Id$
 * @access public
 **/
class pfSenseHTMLHead
{
  var $xmlHead = "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n";
  var $docType = "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\"\n  \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n";
  var $title = "UNSET";
  var $meta = array();
  var $link = array();
  var $script = array();
  var $style = array();
  var $html = "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">\n<head>\n<title>%TITLE%</title>\n%META%\n%STYLE%\n%LINK%\n%SCRIPT%";
  var $closehead = true;
  var $returnedHTML = false;
  var $allCSSID = "NOT-SET";

  /**
   * pfSenseHTMLHead::pfSenseHTMLHead()
   *
   * Class Konstructor
   **/
  function pfSenseHTMLHead() {
    $this->meta[] = array("meta" => "  <meta http-equiv=\"Content-Type\" " .
                                    "content=\"text/html; charset=iso-8859-1\" />",
                          "ID" => "meta-" . strval(microtime()));
    $this->link[] = array("link" => "  <link rel=\"stylesheet\" type=\"text/css\" " .
                                    "href=\"/niftycssprintCode.css\" media=\"print\" />",
                          "ID" => "link-" . strval(microtime()));
    $this->script[] = array("script" => "  <script type=\"text/javascript\" " .
                                        "src=\"/javascript/scriptaculous/prototype.js\"></script>",
                            "weight" => 3,
                            "ID" => "script-" . strval(microtime()));
    $this->script[] = array("script" => "  <script type=\"text/javascript\" " .
                                        "src=\"/javascript/scriptaculous/scriptaculous.js\"></script>",
                            "weight" => 4,
                            "ID" => "script-" . strval(microtime()));
//    $this->script[] = array("script" => "  <script type=\"text/javascript\">\n<!--\n" . getHeadJS() . "\n//-->\n</script>",
//                            "weight" => 5,
//                            "ID" => "script-" . strval(microtime()));
  }

  /**
   * pfSenseHTMLHead::getAllCssID()
   *
   * Allows to store the ID associated with the all CSS file.
   * @return an ID
   **/
  function getAllCssID() {
    return $this->allCSSID;
  }

  /**
   * pfSenseHTMLHead::setAllCssID()
   *
   * Allows to set the ID associated with the all CSS file.
   * @param mixed $myID a string representing an ID that was already generated.
   **/
  function setAllCssID($myID = "") {
    if ($myID == "") { return; }

    $this->allCSSID = $myID;
  }

  /**
   * pfSenseHTMLHead::setCloseHead()
   *
   * Should the HTML &lt;head /&gt; element be closed by the class or
   * do you want to close it manually?
   * @param mixed $myCloseHead Boolean value which indicates whether &lt;head /&gt; should be closed by the class
   * @return
   **/
  function setCloseHead($myCloseHead = true) {
    $this->closehead = $myCloseHead;
  }

  /**
   * pfSenseHTMLHead::setTitle()
   *
   * Set the HTML &lt;title /&gt; element.
   * @param string $myTitle The title (without any markup)
   * @return NULL
   **/
  function setTitle($myTitle = "") {
    $this->title = $myTitle;
  }

  /**
   * pfSenseHTMLHead::addStyle()
   *
   * Allows to add a complete HTML &lt;style /&gt; element to the current
   * meta element array. You can provide an ID if you want to access your
   * particular element at a later time, for example to delete it from the
   * array etc.. If you don't provide an ID, a random ID will be generated
   * and returned.
   * @param string $myStyleElement an HTML string that represents a &lt;style /&gt; tag.
   * @param string $myID an ID that identifies this element.
   * @return the ID that identifies the particular element that you've just added.
   **/
  function addStyle($myStyleElement = "", $myID = "") {
    if ($myID == "") { $myID = "style-" . strval(microtime()); }

    $this->style[] = array("style" => $myStyleElement,
                           "ID" => $myID);

    return $myID;
  }

  /**
   * pfSenseHTMLHead::getStyleArray()
   *
   * @return a reference to the meta element array.
   **/
  function &getStyleArray() {
    return $this->style;
  }

  /**
   * pfSenseHTMLHead::getStyleByID()
   *
   * Returns a reference to an array element that is identified by an ID.
   * Can be used for example to manipulate an array element after it was
   * already stored in the array.
   * @param string $myID an ID that identifies the element that should be retrieved.
   * @return a reference to an array element or NULL if the element does not exist.
   **/
  function &getStyleByID($myID = "") {
    foreach($this->style as $styleel){
      if ($styleel["ID"] == $myID) {
        return $styleel;
      }
    }
    return NULL;
  }

  /**
   * pfSenseHTMLHead::removeStyleByID()
   *
   * Provides a way to delete an element from an HTML element array.
   * You must provide an ID which identifies the element to be deleted.
   * @param string $myID an ID the identifies the element.
   * @return 1 if the element was found or 0 if it does not exist.
   **/
  function removeStyleByID($myID = "") {
    foreach($this->style as $styleel){
      if ($styleel["ID"] == $myID) {
        unset($styleel);
        return 1;
      }
    }
    return 0;
  }

  /**
   * pfSenseHTMLHead::addMeta()
   *
   * Allows to add a complete HTML &lt;meta /&gt; element to the current
   * meta element array. You can provide an ID if you want to access your
   * particular element at a later time, for example to delete it from the
   * array etc.. If you don't provide an ID, a random ID will be generated
   * and returned.
   * @param string $myMetaElement an HTML string that represents a &lt;meta /&gt; tag.
   * @param string $myID an ID that identifies this element.
   * @return the ID that identifies the particular element that you've just added.
   **/
  function addMeta($myMetaElement = "", $myID = "") {
    if ($myID == "") { $myID = "meta-" . strval(microtime()); }

    $this->meta[] = array("meta" => $myMetaElement,
                          "ID" => $myID);

    return $myID;
  }

  /**
   * pfSenseHTMLHead::getMetaArray()
   *
   * @return a reference to the meta element array.
   **/
  function &getMetaArray() {
    return $this->meta;
  }

  /**
   * pfSenseHTMLHead::getMetaByID()
   *
   * Returns a reference to an array element that is identified by an ID.
   * Can be used for example to manipulate an array element after it was
   * already stored in the array.
   * @param string $myID an ID that identifies the element that should be retrieved.
   * @return a reference to an array element or NULL if the element does not exist.
   **/
  function &getMetaByID($myID = "") {
    foreach($this->meta as $metael){
      if ($metael["ID"] == $myID) {
        return $metael;
      }
    }
    return NULL;
  }

  /**
   * pfSenseHTMLHead::removeMetaByID()
   *
   * Provides a way to delete an element from an HTML element array.
   * You must provide an ID which identifies the element to be deleted.
   * @param string $myID an ID the identifies the element.
   * @return 1 if the element was found or 0 if it does not exist.
   **/
  function removeMetaByID($myID = "") {
    foreach($this->meta as $metael){
      if ($metael["ID"] == $myID) {
        unset($metael);
        return 1;
      }
    }
    return 0;
  }

  /**
   * pfSenseHTMLHead::addLink()
   *
   * Allows to add a complete HTML &lt;link /&gt; element to the current
   * link element array. You can provide an ID if you want to access your
   * particular element at a later time, for example to delete it from the
   * array etc.. If you don't provide an ID, a random ID will be generated
   * and returned.
   * @param string $myLinkElement an HTML string that represents a &lt;link /&gt; tag.
   * @param string $myID an ID that identifies this element.
   * @return the ID that identifies the particular element that you've just added.
   **/
  function addLink ($myLinkElement = "", $myID = "") {
    if ($myID == "") { $myID = "link-" . strval(microtime()); }

    $this->link[] = array("link" => $myLinkElement,
                          "ID" => $myID);

    return $myID;
  }

  /**
   * pfSenseHTMLHead::getLinkArray()
   *
   * @return a reference to the link element array.
   **/
  function &getLinkArray() {
    return $this->link;
  }

  /**
   * pfSenseHTMLHead::getLinkByID()
   *
   * Returns a reference to an array element that is identified by an ID.
   * Can be used for example to manipulate an array element after it was
   * already stored in the array.
   * @param string $myID an ID that identifies the element that should be retrieved.
   * @return a reference to an array element or NULL if the element does not exist.
   **/
  function &getLinkByID($myID = "") {
    foreach($this->link as $linkel){
      if ($linkel["ID"] == $myID) {
        return $linkel;
      }
    }
    return NULL;
  }

  /**
   * pfSenseHTMLHead::removeLinkByID()
   *
   * Provides a way to delete an element from an HTML element array.
   * You must provide an ID which identifies the element to be deleted.
   * @param string $myID an ID the identifies the element.
   * @return 1 if the element was found or 0 if it does not exist.
   **/
  function removeLinkByID($myID = "") {
    foreach($this->link as $linkel){
      if ($linkel["ID"] == $myID) {
        unset($linkel);
        return 1;
      }
    }
    return 0;
  }

  /**
   * pfSenseHTMLHead::replaceLinkByID()
   *
   * Provides a way to replace an element from an HTML element array.
   * You must provide an ID which identifies the element to be replace.
   * @param string $myID an ID the identifies the element.
   * @return 1 if the element was found or 0 if it does not exist.
   **/
  function replaceLinkByID($myID = "", $byWhat = "") {
    for ($i = 0; $i < count($this->link); $i++) {
      $linkel =& $this->link[$i];
      if ($linkel["ID"] == $myID) {
        $linkel["link"] = $byWhat;
        return 1;
      }
    }

    return 0;
  }

  /**
   * pfSenseHTMLHead::addScript()
   *
   * Allows to add a complete HTML &lt;link /&gt; element to the current
   * link element array. You can provide an ID if you want to access your
   * particular element at a later time, for example to delete it from the
   * array etc.. If you don't provide an ID, a random ID will be generated
   * and returned.
   *
   * The <code>weight</code> parameter can be used to force the &lt;script /&gt;
   * element to appear at the beginning of the HTML &lt;head /&gt; element or at
   * its end. The greater the value for weight, the later the &lt;script /&gt;
   * element will appear within the HTML &lt;head /&gt; element.
   * @param string $myScriptElement an HTML string that represents a &lt;script /&gt; tag.
   * @param integer $weight allows to position this element within the HTML &lt;head /&gt;
   * @param string $myID an ID that identifies this element.
   * @return the ID that identifies the particular element that you've just added.
   **/
  function addScript($myScriptElement = "", $weight = 1000000, $myID = "") {
    if ($myID == "") { $myID = "script-" . strval(microtime()); }

    $this->script[] = array("script" => $myScriptElement,
                            "weight" => $weight,
                            "ID" => $myID);
  }

  /**
   * pfSenseHTMLHead::getScriptArray()
   *
   * @return a reference to the script element array.
   **/
  function &getScriptArray() {
    return $this->script;
  }

  /**
   * pfSenseHTMLHead::getScriptByID()
   *
   * Returns a reference to an array element that is identified by an ID.
   * Can be used for example to manipulate an array element after it was
   * already stored in the array.
   * @param string $myID an ID that identifies the element that should be retrieved.
   * @return a reference to an array element or NULL if the element does not exist.
   **/
  function &getScriptByID($myID = "") {
    foreach($this->script as $scriptel){
      if ($scriptel["ID"] == $myID) {
        return $scriptel;
      }
    }
    return NULL;
  }

  /**
   * pfSenseHTMLHead::removeScriptByID()
   *
   * Provides a way to delete an element from an HTML element array.
   * You must provide an ID which identifies the element to be deleted.
   * @param string $myID an ID the identifies the element.
   * @return 1 if the element was found or 0 if it does not exist.
   **/
  function removeScriptByID($myID = "") {
    foreach($this->script as $scriptel){
      if ($scriptel["ID"] == $myID) {
        unset($scriptel);
        return 1;
      }
    }
    return 0;
  }

  /**
   * pfSenseHTMLHead::getHTML()
   *
   * This function finally renders the HTML string representation of the
   * HTML document header that is represented by this class. If you did
   * specify to not close the HTML &lt;head /&gt; element via <code>closehead</code>
   * you need to close it manually.
   *
   * @return a string that contains a HTML &lt;head /&gt; element.
   **/
  function getHTML () {
    $language = $GLOBALS['config']['system']['language'];

    if ($this->returnedHTML) {
      return;
    } else {
      $metastr = "";
      if (is_array($this->meta) && count($this->meta) > 0)
        foreach($this->meta as $metael) {
          $metastr .= $metael["meta"] . "\n";
        }

      $linkstr = "";
      if (is_array($this->link) && count($this->link) > 0)
        foreach($this->link as $linkel) {
          $linkstr .= $linkel["link"] . "\n";
        }

      $stylestr = "";
      if (is_array($this->style) && count($this->style) > 0)
        foreach($this->style as $styleel) {
          $stylestr .= $styleel["style"] . "\n";
        }

      $scriptstr = "";
      usort($this->script, "sortScriptArray");
      if (is_array($this->script) && count($this->script) > 0)
        foreach($this->script as $scriptel) {

          $scriptstr .= $scriptel["script"] . "\n";
        }

      $this->html = str_replace("%TITLE%", $this->title, $this->html);
      $this->html = str_replace("%META%", $metastr, $this->html);
      $this->html = str_replace("%STYLE%", $stylestr, $this->html);
      $this->html = str_replace("%LINK%", $linkstr, $this->html);
      $this->html = str_replace("%SCRIPT%", $scriptstr, $this->html);
      //$this->html = str_replace("%LANG%", $language, $this->html);
      $this->html = $this->xmlHead . $this->docType . $this->html;

      $this->returnedHTML = true;

      return $this->closehead ? ($this->html . "</head>") : ($this->html);
    }
  }
}

/**
 * sortScriptArray()
 *
 * Sorts the script array according of the weight of a particular
 * script element.
 * @param mixed $a
 * @param mixed $b
 * @return
 **/
function sortScriptArray($a, $b) {
  if ($a["weigth"] == $b["weight"]) {
    return 0;
  }

  return (intval($a["weight"]) > intval($b["weight"])) ? 1 : -1;
}

?>
