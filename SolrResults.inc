<?php 

module_load_include('inc', 'islandora_solr_search', 'IslandoraSolrResults');

class USC_Results extends IslandoraSolrResults {
  private static function _get_image_path($doc) {
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
    }
    
    if (empty($path)) {
      $path = 'http://upload.wikimedia.org/wikipedia/commons/1/12/Crystal_Clear_mimetype_video.png';
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
      
      $image = self::_get_image_path($doc);
      if (module_exists('imagecache_external') && is_callable('theme_imagecache_external_image')) {
        $preset = $imagecache_preset = variable_get('usc_mirc_solr_imagecache_thumbnail_preset', 'usc_mirc_solr_thumbnail');
        $image = theme('imagecache_external_image', $preset, $image, $title, $title);
      }
      else {
        $image = theme('image', $image, $title, $title, array(), FALSE);
      }
      $output['data'] .= l($image, "fedora/repository/{$doc->PID}", array(
        'html' => TRUE
      ));
      
      $text_stuff = array(
        '#type' => 'markup',
        '#prefix' => '<div class="usc-mirc-text-block">',
        '#suffix' => '</div>',
      );
      $i = 1;
      foreach ($this->resultFieldArray as $key => $display) {
      	$value = $doc->$key;
      	$value = (array)$value;
        $value = $value[0];
        if (!$value) {
          $value = t('No value for field %field.', array('%field' => $key));
        }
      	$value = l($value, "fedora/repository/{$doc->PID}", array('html' => TRUE));
      	
      	$text_stuff[] = array(
      	  '#type' => 'item',
      	  '#title' => $display,
          '#value' => $value,
      	  '#weight' => $i++,
      	);
      }
      $output['data'] .= drupal_render($text_stuff);

      $output_list[] = $output;
    }

    //$start = $results->response->start;
    $start = 5;
    $output = theme('item_list', $output_list, '', 'ol', array(
      'class' => 'usc-mirc-solr-results',
      'style' => 'counter-reset: item ' . $start . ';',
    ));
    
    return $output;
  }
}
