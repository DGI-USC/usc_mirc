<?php

module_load_include('inc', 'usc_mirc', 'displays/base_video_display');

class IslandoraMezz extends BaseVideoDisplay {
  protected function _getPBCore() { //Get the PBCore from the parent.
    //Get the PBCore from the parent object. (VRO)?
    if(count($results = self::_perform_sparql_query(<<<EOQ
PREFIX fre: <info:fedora/fedora-system:def/relations-external#>
PREFIX fm: <info:fedora/fedora-system:def/model#>
SELECT ?object
WHERE {
  ?subject fre:{$this->def_rel} ?object ;
           fm:state fm:Active .
  ?object fm:state fm:Active .
  FILTER(sameTerm(?subject, <info:fedora/{$this->pid}>)) .
}
EOQ
)) > 0) {
      $pid = $results[0]['object'];
      
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
      $pid = $d['subject'];
      
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
      $tabset['view_player'] = array(
        '#type' => 'tabpage',
        '#title' => t('Play'),
        '#selected' => TRUE,
        '#weight' => $i++,
        '#tab_name' => 'view-player',
        'player' => $this->_player($this->_getAccessVideoPaths()),
        'metadata' => $this->_shortMetadata()
      );
      
      //1. Show the metadata about this object
      $tabset['view_metadata'] = array(
        '#type' => 'tabpage',
        '#title' => t('Metadata'),
        '#selected' => TRUE,
        'content' => $this->_renderPBCore(),
        '#weight' => $i++,
        '#tab_name' => 'view-metadata'
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
      $d_form = _usc_mirc_get_unrendered_form('usc_mirc_switch_mezzanine_collection_form', $this->pid);
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