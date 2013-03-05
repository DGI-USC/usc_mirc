<?php

function usc_mirc_admin_settings($form, &$form_state) {
  $image_styles = image_styles();
  foreach ($image_styles as $style_name => $values) {
    $image_styles[$style_name] = $style_name;
  }
  $form = array(
    'usc_mirc_number_per_page' => array(
      '#type' => 'textfield',
      '#title' => t('Collection items per page'),
      '#description' => t('The number of items which will appear in the mezzanine and preservation master collection displays. (A number)'),
      '#default_value' => variable_get('usc_mirc_number_per_page', '15'),
    ),
    'usc_mirc_imagecache_thumbnail_preset' => array(
      '#type' => 'select',
      '#title' => t('Collection Thumbnail Imagecache preset'),
      '#description' => t('The preset with which thumbnails will be created.'),
      '#default_value' => variable_get('usc_mirc_imagecache_thumbnail_preset', 'usc_mirc_thumbnail'),
      '#options' => $image_styles,
    ),
   'usc_mirc_solr_imagecache_thumbnail_preset' => array(
      '#type' => 'select',
      '#title' => t('Solr Result Thumbnail Imagecache preset'),
      '#description' => t('The preset with which thumbnails will be created.'),
      '#default_value' => variable_get('usc_mirc_solr_imagecache_thumbnail_preset', 'usc_mirc_solr_imagecache_thumbnail_preset'),
      '#options' => $image_styles,
    ),
  );
  return system_settings_form($form);
}
