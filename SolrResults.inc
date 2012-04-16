<?php 

module_load_include('inc', 'islandora_solr_search', 'IslandoraSolrResults');

class USC_Results extends IslandoraSolrResults {
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
      
      $title = 'pb_title_Main_ms';
      $title = (array)$doc->$title;
      $title = $title[0];
      $desc = 'pb_description_custom_ms';
      $desc = (array)$doc->$desc;
      $desc = $desc[0];
      
      $output['data'] .= l(theme('image', "fedora/repository/{$doc->PID}/TN", $title, $title, array(
        //'style' => 'float: left;',
      ), FALSE), "fedora/repository/{$doc->PID}", array(
        'attributes' => array(
          //'style' => 'float: left;',
        ),
        'html' => TRUE
      ));
      
      $text_stuff = array(
        '#type' => 'markup',
        '#prefix' => '<div class="usc-mirc-text-block">',
        '#suffix' => '</div>',
        array(
          '#type' => 'item',
          '#title' => t('Title'),
          '#value' => l($title, "fedora/repository/{$doc->PID}", array('attributes' => array(
            //'style' => 'float: left;',
          ))),
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
      //$output['data'] .= "<p>$desc</p>";
      //$output['data'] .= '</div>';
      $output_list[] = $output;
    }
    
    $output = theme('item_list', $output_list, '', 'ol', array(
      'class' => 'usc-mirc-solr-results'
    ));
    
    return $output;
  }
}
