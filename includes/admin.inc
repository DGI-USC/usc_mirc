<?php
/**
 * @file
 * Admin form for usc_mirc.
 *
 */

/**
 * Admin settings form for the MIRC@SC module.
 *
 * @param array $form
 * @param type $form_state
 * @return type
 */
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
    'autocomplete' => array(
      '#type' => 'fieldset',
      '#title' => t('Autocompletion'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      'access_autocomplete' => array(
        '#type' => 'fieldset',
        '#title' => t('Access autocompletion'),
        '#collapsible' => TRUE,
        'usc_mirc_streaming_url' => array(
          '#type' => 'textfield',
          '#title' => t('Streaming base URL'),
          '#description' => t('URL (possibly with port and path), without a terminiating slash. (Used during access copy autocompletion)'),
          '#default_value' => variable_get('usc_mirc_streaming_url', 'http://dvr-streaming.mirc.sc.edu'),
        ),
      ' usc_mirc_streaming_folder' => array(
          '#type' => 'textfield',
          '#title' => t('Streaming folder'),
          '#description' => t('Mountpoint/folder on server under which the streaming access copies are located. (Used during access copy autocompletion)'),
          '#default_value' => variable_get('usc_mirc_streaming_folder', '/mnt/mirc/mezz/access'),
        ),
      ),
      'mezzanine_autocomplete' => array(
        '#type' => 'fieldset',
        '#title' => t('Mezzanine autocompletion'),
        '#collapsible' => TRUE,
        'usc_mirc_mezzanine_url' => array(
          '#type' => 'textfield',
          '#title' => t('Mezzanine base URL'),
          '#description' => t('URL (possibly with port and path), without a terminiating slash. (Used during mezzanine autocompletion)'),
          '#default_value' => variable_get('usc_mirc_mezzanine_url', '/mnt/mirc/mezz/mezzanine'),
        ),
        'usc_mirc_mezzanine_folder' => array(
          '#type' => 'textfield',
          '#title' => t('Mezzanine folder'),
          '#description' => t('Mountpoint/folder on server under which the streaming access copies are located. (Used during mezzanine autocompletion)'),
          '#default_value' => variable_get('usc_mirc_mezzanine_folder', '/mnt/mirc/mezz/mezzanine'),
        ),
        'usc_mirc_mezzanine_type_vocab' => array(
          '#type' => 'textfield',
          '#title' => t('Type Taxonomy vocabulary'),
          '#description' => t('A taxonomy to use for the select box in the mezzanine ingest/edit form. (There is a minimal default if it does not exist.)'),
          '#default_value' => variable_get('usc_mirc_mezzanine_type_vocab', 'usc_mirc_mezzanine_type_vocab'),
        ),
        'usc_mirc_mezzanine_format_vocab' => array(
          '#type' => 'textfield',
          '#title' => t('Format Taxonomy vocabulary'),
          '#description' => t('A taxonomy to use for the select box in the mezzanine ingest/edit form. (There is a minimal default if it does not exist.)'),
          '#default_value' => variable_get('usc_mirc_mezzanine_format_vocab', 'usc_mirc_mezzanine_format_vocab'),
        ),
      ),
      'usc_mirc_streaming_extensions' => array(
        '#type' => 'textfield',
        '#title' => t('File extensions'),
        '#description' => t('Space-separated list of file extensions to autocomplete'),
        '#default_value' => variable_get('usc_mirc_streaming_extensions', '.m4v .mp4 .flv .mov'),
      ),
    ),
    'breakables' => array(
      '#type' => 'fieldset',
      '#title' => t('Breakables (CAUTION!)'),
      '#description' => t('Settings which probably should not be touched after the initial installation.'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      'usc_mirc_relationship' => array(
        '#type' => 'textfield',
        '#title' => t('Relationship'),
        '#description' => t('RELS-EXT relationship used to relate mezzanines to VROs and access copies to mezzanines. Probably a bad idea to change this after objects have already been created, as existing objects will not be changed.'),
        '#default_value' => variable_get('usc_mirc_relationship', 'isDerivativeOf'),
      ),
      'cmodels' => array(
        '#type' => 'fieldset',
        '#title' => t('Content Models'),
        '#description' => t('Content Models used for different purposes. Probably a bad idea to change any after objects have been created with them (existing objects may be orphaned). Fields may be hidden?'),
        'usc_mirc_cmodel_vro' => array(
          '#type' => 'textfield',
          '#title' => t('VRO Content Model'),
          '#description' => t('Content Model used for Video Resource Objects.'),
          '#default_value' => variable_get('usc_mirc_cmodel_vro', 'usc:test-vro'),
        ),
        'usc_mirc_cmodel_mezzanine' => array(
          '#type' => 'textfield',
          '#title' => t('Mezzanine Content Model'),
          '#description' => t('Content Model used for mezzanine objects.'),
          '#default_value' => variable_get('usc_mirc_cmodel_mezzanine', 'usc:test-mezzanine'),
        ),
        'usc_mirc_cmodel_access' => array(
          '#type' => 'textfield',
          '#title' => t('Access Content Model'),
          '#description' => t('Content Model used for access copy objects.'),
          '#default_value' => variable_get('usc_mirc_cmodel_access', 'usc:test-access'),
        ),
      ),
    ),
  );
  return system_settings_form($form);
}
