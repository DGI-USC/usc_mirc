<?php

abstract class BaseVideoDisplay {
  protected $pid;
  
  public function __construct($pid) {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    $this->pid = $pid;
  }
  
  protected function _addDerivativeTab($content_models = array()) {
    //Add a tab which permits the addition of mezzanine (or access) variants to the current video resource object (or mezzanine).
    return _usc_mirc_get_unrendered_form('fedora_repository_ingest_form', $this->pid, 'Get the label', $content_models);
    /*return array(
      '#type' => 'markup',
      '#value' => drupal_get_form('fedora_repository_ingest_form', $this->pid, 'Get the label', $content_models)
    );*/
  }
  
  protected function _renderPBCore() {
    //Get the PBCore (as a DOMDocument)
    $pbcore = $this->_getPBCore();
    
    if ($pbcore === FALSE) {
      return array(
        array(
          '#type' => 'item',
          '#title' => t('Failure!'),
          '#value' => t('Failed to load the PBCore; XML could not be loaded!')
        )
      );
    }
    elseif ($pbcore === NULL) {
      return array(
        array(
          '#type' => 'item',
          '#title' => t('Failure!'),
          '#value' => t('Failed to load the PBCore; Datastream does not exist?')
        )
      );
    }
    else {
      $pbcore = new DOMXPath($pbcore);
      // Register the php: namespace (required)
      $pbcore->registerNamespace('php', 'http://php.net/xpath');

      // Register PHP functions (has_multiple only)
      $pbcore->registerPHPFunctions(array(
        'usc_mirc_create_person_string',
        'usc_mirc_create_date_string'
      ));
      
      //Parameters with which to process the DOM Document/XPath...
      $i = 0;
      $functional_mapping = array(
        array(
          '#title' => t('Title'),
          '#type' => 'usc_mirc_item',
          '#user_data' => array(
            'pbcore' => $pbcore,
            'xpath' => '/pb:pbcoreDescriptionDocument/pb:pbcoreTitle[@titleType="Main"]/text()'
          ),
          '#weight' => $i++,
        ),
        array(
          '#title' => t('Item Title'),
          '#type' => 'usc_mirc_item',
          '#user_data' => array(
            'pbcore' => $pbcore,
            'xpath' => '/pb:pbcoreDescriptionDocument/pb:pbcoreTitle[@titleType="Item"]/text()'
          ),
          '#weight' => $i++,
        ),
        array(  //NOTE:  There can be multiple!
          '#title' => t('Other Titles'),
          '#type' => 'usc_mirc_item',
          '#user_data' => array(
            'pbcore' => $pbcore,
            'xpath' => '/pb:pbcoreDescriptionDocument/pb:pbcoreTitle[@titleType="Alternative" or @titleType="alternative"]/text()'
          ),
          '#weight' => $i++,
        ),
        array( 
          '#title' => t('Original Language Title'),
          '#type' => 'usc_mirc_item',
          '#user_data' => array(
            'pbcore' => $pbcore,
            'xpath' => '/pb:pbcoreDescriptionDocument/pb:pbcoreTitle[@titleType="International"]/text()'
          ),
          '#weight' => $i++,
        ),
        array(
          '#title' => t('Series Title'),
          '#type' => 'usc_mirc_item',
          '#user_data' => array(
            'pbcore' => $pbcore,
            'xpath' => '/pb:pbcoreDescriptionDocument/pb:pbcoreTitle[@titleType="Series"]/text()'
          ),
          '#weight' => $i++,
        ),
        array(
          '#title' => t('OCLC Number'),
          '#type' => 'usc_mirc_item',
          '#user_data' => array(
            'pbcore' => $pbcore,
            'xpath' => '/pb:pbcoreDescriptionDocument/pb:pbcoreIdentifier[@source="oclc"]/text()'
          ),
          '#weight' => $i++,
        ),
        array(
          '#title' => t('Production dates'),
          '#type' => 'usc_mirc_item',
          '#user_data' => array(
            'pbcore' => $pbcore,
            'xpath' => 'php:functionString("usc_mirc_create_date_string", /pb:pbcoreDescriptionDocument/pb:pbcoreAssetDate[@dateType="createdStart"]/text(), /pb:pbcoreDescriptionDocument/pb:pbcoreAssetDate[@dateType="createdEnd"]/text())'
          ),
          '#weight' => $i++,
        ),
        array(
          '#title' => t('Publication dates'),
          '#type' => 'usc_mirc_item',
          '#user_data' => array(
            'pbcore' => $pbcore,
            'xpath' => 'php:functionString("usc_mirc_create_date_string", /pb:pbcoreDescriptionDocument/pb:pbcoreAssetDate[@dateType="publishedStart"]/text(), /pb:pbcoreDescriptionDocument/pb:pbcoreAssetDate[@dateType="publishedEnd"]/text())'
          ),
          '#weight' => $i++,
        ),
        array(
          '#title' => t('Description'),
          '#type' => 'usc_mirc_item',
          '#user_data' => array(
            'pbcore' => $pbcore,
            'xpath' => '/pb:pbcoreDescriptionDocument/pb:pbcoreDescription[@descriptionType="summary"]/text()'
          ),
          '#weight' => $i++,
        ),
        array(
          '#title' => t('Physical Properties: Sound'),
          '#type' => 'usc_mirc_item',
          '#user_data' => array(
            'pbcore' => $pbcore,
            'xpath' => '/pb:pbcoreDescriptionDocument/pb:pbcoreDescription[@descriptionType="sound content"]/text()'
          ),
          '#weight' => $i++,
        ),
        array(
          '#title' => t('Keywords'),
          '#type' => 'usc_mirc_item',
          '#user_data' => array(
            'pbcore' => $pbcore,
            'xpath' => '/pb:pbcoreDescriptionDocument/pb:pbcoreSubject[@subjectType="keyword"]/text()'
          ),
          '#weight' => $i++,
        ),
        array(
          '#title' => t('Geographical Coverage'),
          '#type' => 'usc_mirc_item',
          '#user_data' => array(
            'pbcore' => $pbcore,
            'xpath' => '/pb:pbcoreDescriptionDocument/pb:pbcoreCoverage[pb:coverageType="spatial"]/pb:coverage/text()'
          ),
          '#weight' => $i++,
        ),
        array(
          '#title' => t('Temporal Coverage'),
          '#type' => 'usc_mirc_item',
          '#user_data' => array(
            'pbcore' => $pbcore,
            'xpath' => '/pb:pbcoreDescriptionDocument/pb:pbcoreCoverage[pb:coverageType="temporal"]/pb:coverage/text()'
          ),
          '#weight' => $i++,
        ),
        array(
          '#title' => t('Publisher'),
          '#type' => 'usc_mirc_item',
          '#user_data' => array(
            'pbcore' => $pbcore,
            'context_xpath' => '/pb:pbcoreDescriptionDocument/pb:pbcorePublisher',
            'xpath' => 'php:functionString("usc_mirc_create_person_string", pb:publisher/text(), pb:publisherRole/text())'
          ),
          '#weight' => $i++,
        ),
        array(
          '#title' => t('Creator'),
          '#type' => 'usc_mirc_item',
          '#user_data' => array(
            'pbcore' => $pbcore,
            'context_xpath' => '/pb:pbcoreDescriptionDocument/pb:pbcoreCreator',
            'xpath' => 'php:functionString("usc_mirc_create_person_string", pb:creator/text(), pb:creatorRole/text())'
          ),
          '#weight' => $i++,
        ),
        array(
          '#title' => t('Credits'),
          '#type' => 'usc_mirc_item',
          '#user_data' => array(
            'pbcore' => $pbcore,
            'context_xpath' => '/pb:pbcoreDescriptionDocument/pb:pbcoreContributor',
            'xpath' => 'php:functionString("usc_mirc_create_person_string", pb:contributor/text(), pb:contributorRole/text())'
          ),
          '#weight' => $i++,
        ),
        array(
          '#title' => t('Genre'),
          '#type' => 'usc_mirc_item',
          '#user_data' => array(
            'pbcore' => $pbcore,
            'xpath' => '/pb:pbcoreDescriptionDocument/pb:pbcoreGenre'
          ),
          '#weight' => $i++,
        ),
        array(
          '#title' => t('Subjects'),
          '#type' => 'usc_mirc_item',
          '#user_data' => array(
            'pbcore' => $pbcore,
            'xpath' => '/pb:pbcoreDescriptionDocument/pb:pbcoreSubject[not(@subjectType)]'
          ),
          '#weight' => $i++,
        ),
        array(
          '#title' => t('Rights Information'),
          '#type' => 'usc_mirc_item',
          '#user_data' => array(
            'pbcore' => $pbcore,
            'xpath' => '/pb:pbcoreDescriptionDocument/pb:pbcoreRightsSummary/pb:rightsSummary[not(@source="donor_info")]'
          ),
          '#weight' => $i++,
        ),
        array(
          '#title' => t('Donor'),
          '#type' => 'usc_mirc_item',
          '#user_data' => array(
            'pbcore' => $pbcore,
            'xpath' => '/pb:pbcoreDescriptionDocument/pb:pbcoreRightsSummary/pb:rightsSummary[@source="donor_info"]'
          ),
          '#weight' => $i++,
        ),
        array(
          '#title' => t('From collection'),
          '#type' => 'usc_mirc_item',
          '#user_data' => array(
            'pbcore' => $pbcore,
            'xpath' => '/pb:pbcoreDescriptionDocument/pb:pbcoreRelation[@relationType="isMemberOfCollection"]'
          ),
          '#weight' => $i++,
        ),
        /*array( //Seems like this could go a little differently...  As in, get the name of the related titles, instead of the PID/ID which should be here
          '#title' => t('Related Titles'),
          '#user_data' => array(
            'pbcore' => $pbcore,
            'xpath' => '/pb:pbcoreDescriptionDocument/pb:pbcoreRelation[not(@relationType="isMemberOfCollection")]'
          ),
          '#weight' => $i++,
        ),*/
        /*array(
          '#title' => t('Instantiation info'),
          '#type' => 'item',
          '#value' => '(Not quite sure how to show it...)',
          '#user_data' => array(
            'pbcore' => $pbcore,
            'xpath' => '/pb:pbcoreDescriptionDocument/pb:pbcoreRelation[@relationType="isMemberOfCollection"]' 
          ),
          '#weight' => $i++,
        ),*/
      );
      
      return $functional_mapping;
    }
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
      foreach ($result->children() as $element) { //Breaks here 'Node no longer exists'
        $val = NULL;
        
        $attrs = $element->attributes();
        
        if (!empty($attrs['uri'])) {
          $val = $attrs['uri'];
        }
        else {
          $val = $element;
        }
        
        $r[$element->getName()] = (string) $val;
      }
      $results[] = $r;
    }
    return $results;
  }
  protected static function _perform_itql_query($query, $limit = -1, $offset = 0) {
    return self::_perform_ri_query($query, 'itql', $limit, $offset);
  }
  protected static function _perform_sparql_query($query, $limit = -1, $offset = 0) {
    return self::_perform_ri_query($query, 'sparql', $limit, $offset);
  }

  protected static function _pid_uri_to_bare_pid($uri) {
    return substr(strstr($uri, '/'), 1);
  }
 
  protected function _getDerivatives($subject = NULL) {
    if ($subject === NULL) {
      $subject = "?subject fre:isDerivativeOf <info:fedora/{$this->pid}>";
    }
    //perform query
    $results = self::_perform_sparql_query(<<<EOQ
PREFIX fre: <info:fedora/fedora-system:def/relations-external#>
PREFIX fm: <info:fedora/fedora-system:def/model#>
SELECT ?subject ?label
WHERE {
  $subject ;
           fm:label ?label ;
           fm:state fm:Active .
}
EOQ
);
    //return
    return $results;
  }
  protected function _renderDerivatives($derivatives) {
    $ds = array();
    foreach ($derivatives as $d) {
      $pid = self::_pid_uri_to_bare_pid($d['subject']);
      $ds[] = l($d['label'], "fedora/repository/$pid");
    }
    return theme('item_list', $ds);
  }
  
  protected function _player(array $videos) {
    $mod_path = drupal_get_path('module', 'usc_mirc');
    drupal_add_js("$mod_path/lib/flowplayer/flowplayer-3.2.8.min.js", 'module');
    $mod_path = url($mod_path);
    $player_class = 'test_flowplayer';
    drupal_add_js(<<<EOJS
$(function() {
  $('.$player_class').flowplayer("$mod_path/lib/flowplayer/flowplayer.commercial-3.2.8.swf", {

    //Product key (for sc.edu)
    key: "#$60fe88f4c446bc34a1c",
    
    // configure clip to use "lighthttpd" plugin for providing video data
    clip: {
      provider: 'lighttpd',
      scaling: 'fit'
    },

    // streaming plugins are configured normally under the plugins node
    plugins: {
      lighttpd: {
        url: 'flowplayer.pseudostreaming-3.2.8.swf'
      }
    }
  })
});
EOJS
, 'inline');

    
    $output =  array(
      '#type' => 'fieldset',
      '#title' => t('Player'),
      '#attributes' => array(
        'class' => 'usc-mirc-player',
        'style' => 'float: left'
      ),
    );
    if (count($videos) > 0) {
      $output['player'] = array(
        '#type' => 'markup',
        '#attributes' => array(
          'class' => $player_class
        ),
        '#value' => "<a class='$player_class' href='{$videos[0]}' style='display: block; width: 480px; height: 360px'><img src='$mod_path/images/playbuttonblack.jpg' alt='Video placeholder' width='480' height='360'></img></a>",
      );
      
      $output['carousel'] = array(
        '#type' => 'markup',
        '#prefix' => '<!--',
        '#suffix' => '-->',
        '#value' => "<p>Make the carousel/thumbnail scrubber dealio.</p>",
      );
    }
    else {
      $output['no_player'] = array(
        '#type' => 'item',
        '#value' => 'No video!'
      );
    }
    
    return $output;
  }
  
  protected function _shortMetadata() {
    $pbcore = $this->_getPBCore();
    
    $output = array(
      '#type' => 'fieldset',
        '#title' => t('Basic Metadata'),
        '#attributes' => array(
          'class' => 'usc-mirc-metadata',
          'style' => 'float: left'
        ),
    );
    if ($pbcore === FALSE || $pbcore === NULL) {
      //Error
      $output[] = array(
        '#type' => 'markup',
        '#value' => '<p>Need to load some PBCore...</p>'
      );
    }
    elseif ($pbcore === NULL) {
      //Another error, to be accounted for later... (Doesn't exist?)
      return array();
    }
    else {   
      $pbcore = new DOMXPath($pbcore);
      $i = 0;
      $output['title'] = array(
        '#title' => t('Title'),
        '#type' => 'usc_mirc_item',
        '#user_data' => array(
          'pbcore' => $pbcore,
          'xpath' => '/pb:pbcoreDescriptionDocument/pb:pbcoreTitle[@titleType="Main"]/text()'
        ),
        '#weight' => $i++,
      );
      /*$output['date'] = array(
        '#type' => 'item',
        '#title' => t('Date'),
        '#value' => 'A date or sommat'
      );*/
      $output['item_title'] = array(
        '#title' => t('Item Title'),
        '#type' => 'usc_mirc_item',
        '#user_data' => array(
          'pbcore' => $pbcore,
          'xpath' => '/pb:pbcoreDescriptionDocument/pb:pbcoreTitle[@titleType="Item"]/text()'
        ),
        '#weight' => $i++,
      );
      $output['description'] = array(
        '#type' => 'usc_mirc_item',
        '#title' => t('Description'),
        '#user_data' => array(
          'pbcore' => $pbcore,
          'xpath' => '/pb:pbcoreDescriptionDocument/pb:pbcoreDescription[@descriptionType="summary"]/text()'
        ),
        '#weight' => $i++,
      );
    }
    
    return $output;
  }
  
  //Add in the relevant tab...  Is this supposed to show take a 'page' variable?
  abstract public function showFieldSets();
  abstract protected function _getPBCore();
}

class IslandoraVRO extends BaseVideoDisplay {
  protected function _getPBCore() {
    //Get the PBCore from this object.
    $url = url(variable_get('fedora_base_url', 'http://localhost:8080/fedora') . "/objects/{$this->pid}/datastreams/PBCORE/content");
    self::_user_agent_hack();
    return DOMDocument::load($url);
  }

  public function showFieldSets() {
    $tabset = array();
    $i = 0;
    
    //1. Show the metadata about this object
    if (TRUE) {
      $tabset['view_metadata'] = array(
        '#type' => 'tabpage',
        '#title' => t('Metadata'),
        '#selected' => TRUE,
        'content' => array_merge(array(
          '#type' => 'fieldset',
          '#title' => 'PBCore'
          ),
          $this->_renderPBCore()
        ),
        '#weight' => $i++,
        '#tab_name' => 'view_metadata'
      );
    }
    
    if (user_access('edit fedora meta data')) {
      //$form = drupal_get_form('fedora_repository_edit_qdc_form', $this->pid, 'PBCORE');
      $form = _usc_mirc_get_unrendered_form('fedora_repository_edit_qdc_form', $this->pid, 'PBCORE');
    
      $tabset['edit_metadata'] = array(
        '#type' => 'tabpage',
        '#title' => t('Edit Metadata'),
        //'#selected' => $showEdit,
        'content' => $form,
        //'#content' => $form,
        '#weight' => $i++,
        '#tab_name' => 'edit_metadata'
      );
    }
    //2.1 Perform a query to get the list of all mezzanines of the current VRO.  If none, don't draw the tab.
    if (user_access('view fedora collection')) {
      $derivatives = $this->_getDerivatives();
      if (count($derivatives) > 0) {
        $tabset['tab-' . $i] = array(
          '#type' => 'tabpage',
          '#title' => t('Mezzanines'), 
          '#selected' => FALSE,
          '#content' => $this->_renderDerivatives($derivatives),
          '#weight' => $i++
        );
      }
    }
    
    //2.2 Create a tab for the list of access copies
    if (user_access('view detailed list of content')) {
      //This seems like a stupid way to do this, but I want to get the list of access copies...  Blargh.
      $derivatives = $this->_getDerivatives(<<<EOQ
?sub fre:isDerivativeOf <info:fedora/{$this->pid}> ;
     fm:state fm:Active .
?subject fre:isDerivativeOf ?sub
EOQ
);
      if (count($derivatives) > 0) {
        $tabset['tab-' . $i] = array(
          '#type' => 'tabpage',
          '#title' => t('Access copies'), 
          '#selected' => FALSE,
          '#content' => $this->_renderDerivatives($derivatives),
          '#weight' => $i++
        );
      }
    }
    
    //3. Allow the addition of variants (mezzanines; show according to permissions)
    if (user_access('ingest new fedora objects')) {
      $d_form = $this->_addDerivativeTab('usc:test-mezzanine');
      //dsm($d_form);
      
      $tabset['tab-' . $i] = array(
        '#type' => 'tabpage',
        '#title' => t('Add a mezzanine'),
        '#selected' => FALSE,
        '#weight' => $i++,
        'content' => $d_form
      );
    }
    
    return $tabset;
  }
}

class IslandoraMezz extends BaseVideoDisplay {
  protected function _getPBCore() { //Get the PBCore from the parent.
    //Get the PBCore from the parent object. (VRO)?
    if(count($results = self::_perform_sparql_query(<<<EOQ
PREFIX fre: <info:fedora/fedora-system:def/relations-external#>
PREFIX fm: <info:fedora/fedora-system:def/model#>
SELECT ?object
WHERE {
  ?subject fre:isDerivativeOf ?object ;
           fm:state fm:Active .
  ?object fm:state fm:Active .
  FILTER(sameTerm(?subject, <info:fedora/{$this->pid}>)) .
}
EOQ
)) > 0) {
      $pid = self::_pid_uri_to_bare_pid($results[0]['object']);
      
      $item = new Fedora_Item($this->pid);
      
      $url = url(variable_get('fedora_base_url', 'http://localhost:8080/fedora') . "/objects/{$pid}/datastreams/PBCORE/content"); //Get from the parent by default.
     /* 
     //Get the PBCore from this (mezzanine) object, if it exists.
     if (array_key_exists('PBCORE', $item->get_datastreams_list_as_array())) { //Get from the current object if it exists.
        $url = url(variable_get('fedora_base_url', 'http://localhost:8080/fedora') . "/objects/{$this->pid}/datastreams/PBCORE/content");
      }*/
      
      self::_user_agent_hack();
      return DOMDocument::load($url);
    }
  }
  
  protected function _addDerivativeTab() {
    return _usc_mirc_get_unrendered_form('usc_mirc_redirect_form', $this->pid);
  }
  
  protected function _getAccessVideoPaths() {
    $videos = array();
    //Need to get a list of the child access videos and create array of paths
    foreach ($this->_getDerivatives() as $d) {
      $pid = self::_pid_uri_to_bare_pid($d['subject']);
      
      $videos[] = url("fedora/repository/$pid/VIDEO", array(
        'absolute' => TRUE
      ));
      /*
      //This should work, but is less desirable?
      $videos[] = variable_get('fedora_base_url', 'http://localhost:8080/fedora') . "objects/$pid/datastreams/VIDEO/content";
      */
    }
    //dsm($videos);
    return $videos;
  }
  
  public function showFieldSets() {
    $tabset = array();
    $i = 0;
    
    if (user_access('view fedora collection')) {
      $tabset['tab-' . $i++] = array(
        '#type' => 'tabpage',
        '#title' => t('Play'),
        '#selected' => TRUE,
        'player' => $this->_player($this->_getAccessVideoPaths()),
        'metadata' => $this->_shortMetadata()
      );
      
      //1. Show the metadata about this object
      $tabset['tab-' . $i] = array(
        '#type' => 'tabpage',
        '#title' => t('Metadata'),
        '#selected' => TRUE,
        'content' => $this->_renderPBCore(),
        '#weight' => $i++
      );
    }
    
    //2. Perform a query to get the list of all access copies of the current mezzanine.  If none, don't draw the tab.
    if (user_access('view detailed list of content')) {
      $derivatives = $this->_getDerivatives();
      if (count($derivatives) > 0) {
        $tabset['tab-' . $i] = array(
          '#type' => 'tabpage',
          '#title' => t('Access variants'), 
          '#selected' => FALSE,
          '#content' => $this->_renderDerivatives($derivatives),
          '#weight' => $i++
        );
      }
    }
    
    //3. Allow the addition of variants (show according to permissions)
    if (user_access('ingest new fedora objects')) {
      $d_form = $this->_addDerivativeTab();
      //dsm($d_form);
      $tabset['tab-' . $i] = array(
        '#type' => 'tabpage',
        '#title' => t('Add an access variant'),
        '#selected' => FALSE,
        'content' => $d_form,
        '#weight' => $i++
      );
    
      //Allow migration of mezzanines to their proper collection
      $d_form = _usc_mirc_get_unrendered_form('usc_mirc_switch_mezzanine_collection', $this->pid);
      //dsm($d_form);
      $tabset['tab-' . $i] = array(
        '#type' => 'tabpage',
        '#title' => t('Migrate to collection'),
        '#selected' => FALSE,
        'content' => $d_form,
        '#weight' => $i++
      );
    }
    
    return $tabset;
  }
}

class IslandoraAccess extends BaseVideoDisplay {
  protected function _getPBCore() {
    //Get the PBCore from the parent's (mezz) parent object. (VRO)
  }

  public function showFieldSets() {
    $tabset = array();
    
    $i = 0;
    //2. Show extended metadata about this object...  Check if it contains it, first?
    $tabset['tab-' . $i++] = array(
      '#type' => 'tabpage',
      '#title' => t('Metadata'),
      //'#selected' => TRUE,
      'content' => $this->_renderPBCore()
    );
    
    return $tabset;
  }
}
