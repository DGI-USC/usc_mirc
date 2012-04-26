<?php

module_load_include('inc', 'usc_mirc', 'displays/base_cmodel');

abstract class BaseVideoDisplay extends BaseContentModel {
  protected $def_rel;
  
  public function __construct($pid) {
    parent::__construct($pid);
    $this->def_rel = variable_get('usc_mirc_relationship', 'isDerivativeOf');
  }
  
  protected function _addDerivativeTab($content_models = array()) {
    //Add a tab which permits the addition of mezzanine (or access) variants to the current video resource object (or mezzanine).
    return _usc_mirc_get_unrendered_form('fedora_repository_ingest_form', $this->pid, 'Get the label', $content_models);
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
      $functional_mapping = _usc_mirc_get_unrendered_form('Mezzanine Display', $pbcore->saveXML());
      return $functional_mapping;
    }
  }
  
  protected function _getDerivatives($subject = NULL) {
    if ($subject === NULL) {
      $subject = "?subject fre:{$this->def_rel} <info:fedora/{$this->pid}>";
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
      $pid = $d['subject'];
      $ds[] = l($d['label'], "fedora/repository/$pid");
    }
    return theme('item_list', $ds);
  }
  
  protected function _player(array $videos) {
    $mod_path = drupal_get_path('module', 'usc_mirc');
    drupal_add_js("$mod_path/lib/flowplayer/flowplayer-3.2.8.min.js", 'module');

    $player_class = 'test_flowplayer';
    drupal_add_js(<<<EOJS
$(function() {
  $('.$player_class').flowplayer("/$mod_path/lib/flowplayer/flowplayer.commercial-3.2.8.swf", {

    //Product key (for sc.edu)
    key: "#\$60fe88f4c446bc34a1c",
    
    clip: {
      provider: 'lighttpd', // configure clip to use "lighthttpd" plugin for providing video data
      scaling: 'fit', //Maintain aspect ratio when scaling
    },
    canvas: { //Use a full-black background
      background: '#000000',
      backgroundGradient: 'none',
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

    $i = -100;
    $output =  array(
      '#type' => 'fieldset',
      '#title' => t('Player'),
      '#attributes' => array(
        'class' => 'usc-mirc-player',
        'style' => 'float: left'
      ),
      '#weight' => $i++,
    );
    if (count($videos) > 0) {
      $output['player'] = array(
        '#type' => 'markup',
        '#attributes' => array(
          //'class' => $player_class,
        ),
        //'#prefix' => "<a class='$player_class' href='{$videos[0]}' style='display: block; width: 480px; height: 360px'>",
        //'#suffix' => '</a>',
        '#value' => l(theme('image', "$mod_path/images/playbuttonblack.jpg", 'Video placeholder'), $videos[0], array(
          'html' => TRUE,
          'attributes' => array(
            'class' => $player_class,
            'style' => 'display: block; width: 480px; height: 360px', //XXX:  Should probably be done via a style sheet...
          ),
        )), 
        '#weight' => $i++,
      );
      
      $output['carousel'] = array(
        '#type' => 'markup',
        '#prefix' => '<!--',
        '#suffix' => '-->',
        '#value' => "<span>Make the carousel/thumbnail scrubber dealio.</span>",
        '#weight' => $i++,
      );
    }
    else {
      $output['no_player'] = array(
        '#type' => 'markup',
        '#value' => theme('image', "$mod_path/images/NoVideoAvailable.jpg"),
        '#weight' => $i++,
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
        '#weight' => 0
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
      $output['temporal_coverage'] = array(
        '#title' => t('Date'),
        '#type' => 'usc_mirc_item',
        '#user_data' => array(
          'pbcore' => $pbcore,
          'xpath' => '/pb:pbcoreDescriptionDocument/pb:pbcoreCoverage[pb:coverageType/text()="temporal"]/pb:coverage/text()'
        ),
        '#weight' => $i++,
      );
      $output['sound_content'] = array(
        '#title' => t('Silent or Sound'),
        '#type' => 'usc_mirc_item',
        '#user_data' => array(
          'pbcore' => $pbcore,
          'xpath' => '/pb:pbcoreDescriptionDocument/pb:pbcoreDescription[@descriptionType="sound content"]/text()'
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
