<?php 

module_load_include('inc', 'usc_mirc', 'displays/base_cmodel');

class IslandoraVideoCollection extends BaseContentModel {
  protected $pager_name;
  protected $per_page;
  public function __construct($pid) {
    parent::__construct($pid);
    $this->pager_name = 0;
    $this->per_page = 20;
  }
  
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
  
  private static function _hack_pager($pager_name, $per_page, $total_items) {
    //Pager hacking...  Huzzah!
    global $pager_total, $pager_page_array;
    $pager_total[$pager_name] = ceil($total_items / $per_page);
    
    $page_info = explode(',', isset($_GET['page']) ? $_GET['page'] : '');
    $page = $page_info[$pager_name];
    if ($page < 0) {
      $page = 0;
    }
    
    if (!isset($pager_page_array)) {
      $pager_page_array = pager_load_array($page, $pager_name, $page_info);
    }
    else {
      $pager_page_array = pager_load_array($page, $pager_name, $pager_page_array);
    }
    
    $page = $pager_page_array[$pager_name];
    return $page;
  }
  
  private function _preservation_master_collection_view(&$weight) {
    $collection = array(
      '#type' => 'fieldset',
      '#title' => t('Collection Members'),
      '#weight' => $weight++,
      'members' => array(
        '#type' => 'markup',
        '#weight' => $weight++,
      )
    );
    
    //1.  Draw paged list of mezzanines videos.
    
    //Pager hacking...  Huzzah!
    $pager_name = 1;
    $page = self::_hack_pager($pager_name, $this->per_page, $this->_get_number_of_children(FALSE));
    //Done with pager hacking...

    
    $i = $page * $this->per_page + 1;
    $table_header = array(
      t(''),
      t('Thumbnail'),
      t('Info'),
    );
    $table_data = array();
    foreach ($this->_get_collection_set($page, $this->per_page, FALSE) as $item) {
      $path = "fedora/repository/{$item['vro']}";
      $table_data[] = array(
        //$i++,
        'data' => l(self::_get_image_markup($item['vro'], $item['vro_title'], $item['vro_title']), $path, array('html' => TRUE , 'attributes' => array('class' => 'item-image-link'))) . l($item['vro_title'], $path, array('attributes' => array('class' => 'item-title-text'))),
        'class' => 'collection-members',
      );
    }
    
    if (count($table_data) === 0) {
      $collection['members'] = array(
        '#type' => 'markup',
        '#value' => t('No preservation masters in this collection.'),
      );
    }
    else {
      $collection['members']['#value'] = theme('item_list', $table_data, '', 'ul', array(
        'class' => 'usc-collection-view',
      ));
      $collection['pager'] = array(
        '#type' => 'markup',
        '#value' => theme('pager', array(), $this->per_page, $pager_name),
        '#weight' => $weight++,
      );
    }
    
    return $collection;
  }
  
  private function _collection_info_fieldset(&$weight) {
    $mods = $this->item->get_datastream_dissemination('MODS');
    $to_return = array(
      '#type' => 'fieldset',
      '#title' => t('Collection Metadata'),
      //'#value' => 'TODO: Render MODS here...',
      '#prefix' => '<div>',
      '#suffix' => '</div>',
      '#weight' => $weight++,
    );
    
    $image = array(
      '#type' => 'markup',
      '#value' => self::_get_image_markup($this->pid, '', '', 'featured_content_image'),
      '#sorted' => TRUE,
    );
    
    foreach (_usc_mirc_get_unrendered_form('Collection Desc Display', $mods) as $key => $value) {
      if ($key === 'title_info') {
        $to_return['content']['collection_image'] = $image;
      }
      $to_return['content'][$key] = $value;
    }

    return $to_return;
  }
  
  private function _get_collection_set($page = 0, $number = 20, $mezzanines = TRUE) {
    $offset = $page * $number;
    $collection_relation = variable_get('usc_mirc_collection_relationship', 'isMemberOfCollection');
    $derivative_relation = variable_get('usc_mirc_relationship', 'isDerivativeOf');
    $vro_cmodel = variable_get('usc_mirc_cmodel_vro', 'usc:test-vro');
    $mezzanine_cmodel = variable_get('usc_mirc_cmodel_mezzanine', 'usc:test-mezzanine');
    
    $query = "
PREFIX fre: <info:fedora/fedora-system:def/relations-external#>
PREFIX fm: <info:fedora/fedora-system:def/model#>
PREFIX dc: <http://purl.org/dc/elements/1.1/>
SELECT ?vro " . ($mezzanines ? " ?mezzanine ?mezzanine_title" : "?vro_title") . " 
WHERE {
?vro fre:$collection_relation <info:fedora/{$this->pid}> ;
     fm:hasModel <info:fedora/$vro_cmodel> ; " .
     ((!$mezzanines) ? " dc:title ?vro_title ; " : '') . "
     fm:state fm:Active . " . ($mezzanines ? "
OPTIONAL{
?mezzanine fre:$derivative_relation ?vro ;
           fm:hasModel <info:fedora/$mezzanine_cmodel> ;
           fm:state fm:Active ;
           dc:title ?mezzanine_title .
}" : '') . "
}
ORDER BY " . ($mezzanines? "?mezzanine_title" : "?vro_title") . "
LIMIT $number
OFFSET $offset";
    //dsm($query, 'Query');
    //dsm($mezzanines, 'Mezz?');
    return self::_perform_sparql_query($query);
  }
  
  protected function _get_number_of_children($mezzanines = TRUE) {
    $collection_relation = variable_get('usc_mirc_collection_relationship', 'isMemberOfCollection');
    $derivative_relation = variable_get('usc_mirc_relationship', 'isDerivativeOf');
    $vro_cmodel = variable_get('usc_mirc_cmodel_vro', 'usc:test-vro');
    $mezzanine_cmodel = variable_get('usc_mirc_cmodel_mezzanine', 'usc:test-mezzanine');
      
      
    $results = self::_perform_sparql_query("
PREFIX fre: <info:fedora/fedora-system:def/relations-external#>
PREFIX fm: <info:fedora/fedora-system:def/model#>
PREFIX dc: <http://purl.org/dc/elements/1.1/>
SELECT ?vro
WHERE {
  ?vro fre:$collection_relation <info:fedora/{$this->pid}> ;
       fm:hasModel <info:fedora/$vro_cmodel> ;
       fm:state fm:Active . " . ($mezzanines ? "
  ?mezzanine fre:$derivative_relation ?vro ;
             fm:hasModel <info:fedora/$mezzanine_cmodel> ;
             fm:state fm:Active . " : '') . "
}");

    return count($results);
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
  
  protected static function _get_image_markup($for_pid, $alt = '', $title = '', $imagecache_preset = NULL) {
    $fedora_image = self::_get_image($for_pid);
    $image = NULL;
    $fedora_base = variable_get('fedora_base_url', 'http://localhost:8080/fedora');
    $image_path = NULL;
    
    if ($imagecache_preset === NULL) {
      $imagecache_preset = variable_get('usc_mirc_imagecache_thumbnail_preset', 'usc_mirc_thumbnail');
    }
    
    if (empty($fedora_image)) {
      //TODO: $image gets some decent default image
    }
    else {
      list($pid, $dsid) = $fedora_image;
      $image = "fedora/repository/$pid/$dsid";
    }
    
    if ($image) {
      if (module_exists('imagecache_external') && is_callable('theme_imagecache_external_image')) {
        return theme('imagecache_external_image', $imagecache_preset, $image, $alt, $title);
      }
      else {
        return theme('image', $image, $alt, $title, array(), FALSE);
      }
    }
    else {
      return 'Makey image worky.';
    }
  }
  
  private function _collection_view(&$weight) {
    $collection = array(
      '#type' => 'fieldset',
      '#title' => t('Collection Members'),
      '#weight' => $weight++,
      'members' => array(
        '#type' => 'markup',
        '#weight' => $weight++,
      )
    );
    
    //1.  Draw paged list of mezzanines videos.
    
    //Pager hacking...  Huzzah!
    $pager_name = 0;
    $page = self::_hack_pager($pager_name, $this->per_page, $this->_get_number_of_children(TRUE));
    //Done with pager hacking...

    
    $i = $page * $this->per_page + 1;
    $table_header = array(
      t(''),
      t('Thumbnail'),
      t('Info'),
    );
    $table_data = array();
    foreach ($this->_get_collection_set($page, $this->per_page) as $item) {
      if (!(empty($item['mezzanine']) || empty($item['mezzanine_title']))) {
        $path = "fedora/repository/{$item['mezzanine']}";
        $table_data[] = array(
          //$i++,
          'data' => l(self::_get_image_markup($item['mezzanine'], $item['mezzanine_title'], $item['mezzanine_title']), $path, array('html' => TRUE, 'attributes' => array('class' => 'item-image-link'))) . l($item['mezzanine_title'], $path, array('attributes' => array('class' => 'item-title-text'))),
          'class' => 'collection-members',
        );
      }
    }
    
    if (count($table_data) === 0) {
      $collection['members'] = array(
        '#type' => 'markup',
        '#value' => t('No public video information in this collection, yet.'),
      );
    }
    else {
      $collection['members']['#value'] = theme('item_list', $table_data, '', 'ul', array(
        'class' => 'usc-collection-view',
      ));
      $collection['pager'] = array(
        '#type' => 'markup',
        '#value' => theme('pager', array(), $this->per_page, $pager_name),
        '#weight' => $weight++,
      );
    }
    
    return $collection;
  }
}
