<?php

/**
 * Post render callback for tabpages containing pagers.
 * 
 * @param $content string
 * @param $element array
 * @return string
 */
function usc_mirc_fix_pager_links($content, $element) {
  if ($element['#type'] != 'tabpage') {
  	watchdog('usc_mirc', 'Called usc_mirc_fix_pager_links on something which is not a tabpage, which does not make sense!');
  	return $content;
  }
  
  $content_dom = new DOMDocument('1.0', 'utf-8');
  if (!$content_dom->loadHTML('<?xml encoding="utf-8"?>' . $content)) {
    //Content isn't parsable!?...  Whatever.
    return $content;
  }
  $content_xpath = new DOMXPath($content_dom);
  
  //Find all hrefs in the pager...  Add fragment id with id of this element...
  $pager_links = $content_xpath->query('//ul[contains(@class, "pager")]//a');
  foreach ($pager_links as $link) {
  	$parsed_href = parse_url($link->getAttribute('href'));

  	$new_href = $parsed_href['path'] . '?' . $parsed_href['query'] . '#' . $element['#tab_name'];
  	
  	$link->setAttribute('href', $new_href);
  }
  
  return $content_xpath->document->saveHTML();
}

abstract class BaseContentModel {
  protected $pid;
  protected $item;
  
  public function __construct($pid) {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    $this->pid = $pid;
    $this->item = new Fedora_Item($pid);
  }
  
  /**
   * XXX: Little hack to allow libxml to access HTTP stuff directly,
   * instead of having to first load it with curl.
   */
  protected static function _user_agent_hack() {
    static $hacked = FALSE;
    if (!$hacked) {
      $opts = array(
          'http' => array(
            'user_agent' => 'PHP libxml agent',
          )
      );

      $context = stream_context_create($opts);
      libxml_set_streams_context($context);
      $hacked = TRUE;
    }
  }
 
  /**
   * Performs the given RI query.
   * Might be duplicating code from the Fedora API (I seem to recall something
   * but with a weird name).
   *
   * FIXME: Could probably made more fail-safe (avoiding passing directly from
   *   the curl call to loadXML, for example.)
   *
   * @param String $query
   * @param Integer $limit
   * @param Integer $offset
   * @return DOMDocument
   */
  protected static function _perform_ri_query($query, $type = 'itql', $limit = -1, $offset = 0) {
    module_load_include('inc', 'fedora_repository', 'ObjectHelper');
    return ObjectHelper::performRiQuery($query, $type, $limit, $offset);
  }
  public static function _perform_itql_query($query, $limit = -1, $offset = 0) {
    return self::_perform_ri_query($query, 'itql', $limit, $offset);
  }
  public static function _perform_sparql_query($query, $limit = -1, $offset = 0) {
    return self::_perform_ri_query($query, 'sparql', $limit, $offset);
  }
}
