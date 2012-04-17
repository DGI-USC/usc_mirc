<?php 

module_load_include('inc', 'islandora_solr_search', 'IslandoraSolrResults');

class USC_Results extends IslandoraSolrResults {
  private static function _get_image_path($doc) {
    $imagecache_preset = variable_get('usc_mirc_imagecache_thumbnail_preset', 'test_thumbnail');
    
    $image_dsid = '';
    $potential_dsids = array(
      'IMG',
      'JPG',
      'TN',
    );
    
    $intersected_dsids = array_intersect($potential_dsids, $doc->fedora_active_datastream_state);
    if ($intersected_dsids) {
      $image_dsid = $intersected_dsids[0];
    }
    
    $path = '';
    if ($image_dsid) {
      $path = "fedora/repository/{$doc->PID}/$image_dsid";
      
      if (module_exists('imagecache_external')) {
        $path = imagecache_external_generate_path($path, $imagecache_preset);
      }
    }
    
    if (empty($path)) {
      $path = imagecache_external_generate_path('http://upload.wikimedia.org/wikipedia/commons/1/12/Crystal_Clear_mimetype_video.png', $imagecache_preset);
    }
    
    //XXX:  At the time of writing, imagecache_external_generate_path generates a path which starts with a '/', which isn't appreciated by theme('image')...  So...  Strip it!
    if (strpos($path, '/') === 0) {
      $path = substr($path, 1);
    }
    
    return $path;
  }
  
  function printResults($results) {
    if( (int)$results->response->numFound === 0 ) {
      $output .= "<p>Sorry, but your search returned no results.</p>";
      return $output;
    }
    
    $mod_path = drupal_get_path('module', 'usc_mirc');
    drupal_add_css("$mod_path/css/usc_mirc_solr_results.css");
    
    $output_list = array();
    foreach ($results->response->docs as $key => $doc) {
      $classes = (($key % 2 === 0) ? 'odd' : 'even'); //May look weird (due to starting at 0)
      
      $output = array();
      $output['class'] = $classes;
      //$output = "<div class='usc-mirc-solr-result $classes'>";
      
      $title = 'pb_title_s';
      $title = (array)$doc->$title;
      $title = $title[0];
      $desc = 'pb_parent_description_custom_ms';
      $desc = (array)$doc->$desc;
      $desc = $desc[0];
      
      $output['data'] .= l(theme('image', self::_get_image_path($doc), $title, $title, array(), FALSE), "fedora/repository/{$doc->PID}", array(
        'html' => TRUE
      ));
      
      $text_stuff = array(
        '#type' => 'markup',
        '#prefix' => '<div class="usc-mirc-text-block">',
        '#suffix' => '</div>',
        array(
          '#type' => 'item',
          '#title' => t('Title'),
          '#value' => l($title, "fedora/repository/{$doc->PID}"),
          '#weight' => 1,
        ),
        array(
          '#type' => 'item',
          '#title' => t('Abstract'),
          '#value' => (!empty($desc)?$desc:'(Empty abstract)'),
          '#weight' => 2,
        )
      );
      $output['data'] .= drupal_render($text_stuff);

      $output_list[] = $output;
    }
    
    $output = theme('item_list', $output_list, '', 'ol', array(
      'class' => 'usc-mirc-solr-results'
    ));
    
    return $output;
  }
}
