<?php 

module_load_include('inc', 'usc_mirc', 'displays/base_video_display');

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
