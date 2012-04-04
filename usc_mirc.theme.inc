<?php 

/**
 * theme implementation; returns markup for the given type
 * @deprecated in favour of 'read forms', using 'defaultable_item'
 */
function theme_usc_mirc_item($element) {
  //dsm($element);
  $map = $element['#user_data'];
  $pbcore = $map['pbcore'];
  
  //Obtain a traversable list of contexts
  $context = array($pbcore->document->documentElement);
  if (!empty($map['context_xpath'])) {
    $context = $pbcore->query($map['context_xpath']);
  }

  $temp = array();
  foreach ($context as $con) {
    $results = $pbcore->query($map['xpath'], $con);
    
    foreach($results as $result) {
      //dsm($result, 'result');
      $text = $result->textContent;
      if (!empty($text)) {
        $temp[] = $text;
      }
    }
  }
  
  if (count($temp) === 1) {
    $element['#value'] = $temp[0];
  } 
  elseif (count($temp) > 1) {
    $element['#value'] = theme('item_list', $temp);
  }
  
  if (!empty($element['#value'])) {
    return theme('form_element', $element, $element['#value'] . (!empty($element['#children']) ? $element['#children'] : ''));
  }
}
