<?php

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
