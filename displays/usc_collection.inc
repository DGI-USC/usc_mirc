<?php 

module_load_include('inc', 'usc_mirc', 'displays/base_cmodel');

class IslandoraVideoCollection extends BaseContentModel {
  /*public function __construct($pid) {
    parent::__construct($pid);
  }*/
  
  public function showFieldSets($page = 0) {
    $tabset = array();
    $i = -100;
    //1. Show collection view to those permitted.
    if (user_access('view fedora collection')) {
      $tabset['collection-view'] = array(
        '#type' => 'tabpage',
        '#title' => t('Collection View'),
        '#weight' => $i++,
        '#tab_name' => 'collection-view',
        'mods' => $this->_collection_info_fieldset($i),
        'content' => $this->_collection_view($i, $page),
      );
    }
    
    //2. Show "Preservation Master" tab to permitted individuals/roles
    if (user_access('access preservation masters')) {
      $tabset['preservation-masters'] = array(
        '#type' => 'tabpage',
        '#title' => t('Preservation Masters'),
        '#weight' => $i++,
        '#tab_name' => 'preservation-masters',
        'content' => $this->_preservation_master_collection_view($i, $page),
      );
    }
    
    //3. Allow those permitted to add new preservation masters.
    if (user_access('access preservation masters') && user_access('ingest new fedora objects')) {
      $tabset['preservation-master-ingest'] = array(
        '#type' => 'tabpage',
        '#title' => t('Add New Preservation Master'),
        '#weight' => $i++,
        '#tab_name' => 'preservation-master-ingest',
        'content' => _usc_mirc_get_unrendered_form('fedora_repository_ingest_form', $this->pid, 'collection', variable_get('usc_mirc_cmodel_vro', 'usc:test-vro')),
      );
    }
    
    if (user_access('edit fedora meta data')) {
      $form = _usc_mirc_get_unrendered_form('fedora_repository_edit_qdc_form', $this->pid, 'MODS');
    
      $tabset['edit_metadata'] = array(
        '#type' => 'tabpage',
        '#title' => t('Edit Collection Metadata'),
        //'#selected' => $showEdit,
        'content' => $form,
        //'#content' => $form,
        '#weight' => $i++,
        '#tab_name' => 'edit-metadata'
      );
    }

    return $tabset;
  }
  
  private function _preservation_master_collection_view(&$weight, $page) {
    $collection = array(
      '#type' => 'markup',
      '#weight' => $weight++,
      '#prefix' => '<ol>',
      '#suffix' => '</ol>',
    );
    //1.  Draw paged list of mezzanines videos.
    foreach ($this->_get_collection_set($page) as $item) {
      $collection[$item['vro']] = array(
        '#type' => 'markup',
        '#weight' => $weight++,
        '#prefix' => '<li>',
        '#suffix' => '</li>',
        'image' => array(
          '#type' => 'markup',
          //'#value' => theme('image', ),
          '#value' => self::_get_imagecache_external_markup($item['vro']),
          '#weight' => $weight++,
        ),
        'title_link' => array(
          '#type' => 'markup',
          '#value' => l($item['vro_title'], "fedora/repository/{$item['vro']}"),
          '#weight' => $weight++,
        ),
      );
    }
    
    return $collection; 
  }
  
  private function _collection_info_fieldset(&$weight) {
    $mods = $this->item->get_datastream_dissemination('MODS');
    return array(
      '#type' => 'fieldset',
      '#title' => t('Collection Metadata'),
      //'#value' => 'TODO: Render MODS here...',
      '#prefix' => '<div>',
      '#suffix' => '</div>',
      '#weight' => $weight++,
      'content' => _usc_mirc_get_unrendered_form('Collection Desc Display', $mods),
    );
  }
  
  private function _get_collection_set($page = 0, $number = 20) {
    static $cache = array();  //This'll occasionally be used twice per request...  Slight optimization?
    
    $offset = $page * $number;
    if (empty($cache[$offset])) {
      $collection_relation = variable_get('usc_mirc_collection_relationship', 'isMemberOfCollection');
      $derivative_relation = variable_get('usc_mirc_relationship', 'isDerivativeOf');
      
      $cache[$offset] = self::_perform_sparql_query(<<<EOQ
PREFIX fre: <info:fedora/fedora-system:def/relations-external#>
PREFIX fm: <info:fedora/fedora-system:def/model#>
PREFIX dc: <http://purl.org/dc/elements/1.1/>
SELECT ?vro ?vro_title ?mezzanine ?mezzanine_title 
WHERE {
  ?vro fre:$collection_relation <info:fedora/{$this->pid}> ;
       dc:title ?vro_title ;
       fm:state fm:Active .
  OPTIONAL{
  ?mezzanine fre:$derivative_relation ?vro ;
             fm:state fm:Active ;
             dc:title ?mezzanine_title .
  }
}
ORDER BY ?mezzanine_title
LIMIT $number
OFFSET $offset
EOQ
);
    }
    
    return $cache[$offset];  
  }
  
  /**
   * @return array($pid, $dsid)
   */
  protected static function _get_image($pid) {
    static $collection_relation = NULL; 
    static $derivative_relation = NULL;
    static $supported_MIMEs = NULL;
    
    if ($collection_relation === NULL) { //XXX: Not so necessary if not happening recursively
      $collection_relation = variable_get('usc_mirc_collection_relationship', 'isMemberOfCollection');
      
      $derivative_relation = variable_get('usc_mirc_relationship', 'isDerivativeOf');
      
      $supported_MIMEs = array( //Is limited to this set in imagecache_external.module
        'image/jpeg',
        'image/png',
        'image/gif',
      );
    }
      
    //Instantiate the fedora_item,
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    $item = new Fedora_Item($pid);
    
    //Look for an image in the list datastream
    foreach ($item->get_datastreams_list_as_array() as $dsid => $info) {
      if (in_array($info['MIMEType'], $supported_MIMEs)) {
        return array($pid, $dsid);
      }
    }
    
    //Default...
    return array('usc:mirc', 'TN');
  }
  
  protected static function _get_imagecache_external_markup($for_pid, $alt = '', $title = '') {
    $fedora_image = self::_get_image($for_pid);
    $image = NULL;
    $fedora_base = variable_get('fedora_base_url', 'http://localhost:8080/fedora');
    $image_path = NULL;
    
    if (empty($fedora_image)) {
      //TODO: $image gets some decent default image
    }
    else {
      list($pid, $dsid) = $fedora_image;
      $image_path = "$fedora_base/objects/$pid/datastreams/$dsid/content";
    }
    
    if (is_callable('imagecache_external_generate_path')) {
      $image = imagecache_external_generate_path($image_path, variable_get('usc_mirc_imagecache_thumbnail_preset', 'test_thumbnail'));
    }
    
    if (strpos($image, '/') === 0) {
      $image = substr($image, 1); //XXX:  the imagecache path generated starts with a /, which theme('image') doesn't seem to like...
    }
    
    if ($image) {
      return theme('image', $image, $alt, $title);
    }
    else {
      return 'Makey image worky.';
    }
  }
  
  private function _collection_view(&$weight, $page = 0) {
    $collection = array(
      '#type' => 'fieldset',
      '#title' => t('Collection Members'),
      '#weight' => $weight++,
      'members' => array(
        '#type' => 'markup',
        '#prefix' => '<ol>',
        '#suffix' => '</ol>',
      )
    );
    
    //1.  Draw paged list of mezzanines videos.
    foreach ($this->_get_collection_set($page) as $item) {
      if (!(empty($item['mezzanine']) || empty($item['mezzanine_title']))) {
        $collection['members'][$item['mezzanine']] = array(
          '#type' => 'markup',
          '#weight' => $weight++,
          '#prefix' => '<li>',
          '#suffix' => '</li>',
          'image' => array(
            '#type' => 'markup',
            //'#value' => theme('image', ),
            '#value' => self::_get_imagecache_external_markup($item['mezzanine'], $item['mezzanine_title'], $item['mezzanine_title']),
            '#weight' => $weight++,
          ),
          'title_link' => array(
            '#type' => 'markup',
            '#value' => l($item['mezzanine_title'], "fedora/repository/{$item['mezzanine']}"),
            '#weight' => $weight++,
          ),
        );
      }
    }
    
    if (count(element_children($collection['members'])) === 0) {
      $collection['members'] = array(
        '#type' => 'markup',
        '#value' => t('No public video information in this collection, yet.'),
      );
    }
    
    $collection['pager'] = array(
      '#type' => 'markup',
      '#value' => theme('pager'),
      '#weight' => $weight++,
    );
    
    return $collection;
  }
}
