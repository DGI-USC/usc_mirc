<?php

module_load_include('inc', 'usc_mirc', 'displays/base_video_display');

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
?sub fre:{$this->def_rel} <info:fedora/{$this->pid}> ;
     fm:state fm:Active .
?subject fre:$def_rel ?sub
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
  
  protected function _addDerivativeTab($model) {
    return _usc_mirc_get_unrendered_form('usc_mirc_add_mezzanine_form', $this->pid);
  }
}
