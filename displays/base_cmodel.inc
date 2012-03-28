<?php

abstract class BaseContentModel {
  protected $pid;
  
  public __construct($pid, $page) {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    $this->pid = $pid;
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
   * FIXME: Could probably made more fail-safe (avoiding passing directly from the curl call to oadXML, for example.)
   *
   * @param String $query
   * @param Integer $limit
   * @param Integer $offset
   * @return DOMDocument
   */
  protected static function _perform_ri_query($query, $type = 'itql', $limit = -1, $offset = 0) {
    self::_user_agent_hack();
    
    //Setup the query options...
    $options = array(
      'type' => 'tuples',
      'flush' => TRUE,
      'format' => 'Sparql',
      'lang' => $type,
      'query' => $query
    );
    if ($limit > 0) {
      $options['limit'] = $limit;
    }
    if ($offset > 0) {
      $options['offset'] = $offset;
    }
    
    //Actually construct the query URL.
    $queryUrl = url(variable_get('fedora_repository_url', 'http://localhost:8080/fedora/risearch'), array('query' => $options));
    
    //Perform the query, within a SimpleXML element.
    $doc = new SimpleXMLElement($queryUrl, 0, TRUE, 'http://www.w3.org/2001/sw/DataAccess/rf1/result');
    $results = array();
    foreach ($doc->results->children() as $result) {
      $r = array();
      foreach ($result->children() as $element) {
        $val = NULL;
        
        $attrs = $element->attributes();
        
        if (!empty($attrs['uri'])) {
          $val = $attrs['uri'];
        }
        else {
          $val = $element;
        }
        
        $r[$element->getName()] = self::_pid_uri_to_bare_pid((string)$val);
      }
      $results[] = $r;
    }
    return $results;
  }
  public static function _perform_itql_query($query, $limit = -1, $offset = 0) {
    return self::_perform_ri_query($query, 'itql', $limit, $offset);
  }
  public static function _perform_sparql_query($query, $limit = -1, $offset = 0) {
    return self::_perform_ri_query($query, 'sparql', $limit, $offset);
  }

  protected static function _pid_uri_to_bare_pid($uri) {
    $chunk = 'info:fedora/';
    $pos = strrpos($uri, $chunk);
    if ($pos !== FALSE) { //Remove info:fedora/ chunk
      return substr($uri, strlen($chunk));
    }
    else { //Doesn't start with info:fedora/ chunk...
      return $uri;
    }
  }
}
