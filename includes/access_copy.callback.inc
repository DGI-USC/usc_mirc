<?php

/**
 * @file
 * Page callback to create an access copy object, and possibly related
 * functions.
 */

/**
 * Callback from Microservices; create an access copy given a parent and paths.
 *
 * Info should be available in the $_POST, as:
 * - parent: The ID of the parent object.
 * - video_path: The path of the transformed video for the access variant.
 * - thumbnail_path: The path to a thumbnail to use for the access variant.
 */
function usc_mirc_create_access_copy_callback() {
  extract($_POST);

  // Can haz Tuque?
  $tuque = islandora_get_tuque_connection();

  // Create a new access copy object.
  $new_object = $tuque->repository->constructObject('usc');

  // Set the model and parent.
  $new_object->models = variable_get('usc_mirc_cmodel_access', 'usc:accessCModel');
  $new_object->relationships->add(FEDORA_RELS_EXT_URI, variable_get('usc_mirc_relationship', 'isDerivativeOf'), $parent);

  // Build up the URL to the video redirect...
  $streaming_path = variable_get('usc_mirc_streaming_folder', '/mnt/mirc/mezz/access');
  if (strpos($video_path, $streaming_path) === 0) {
    $built_redirect_url = format_string('!streaming_url/!file_part', array(
      '!streaming_url' => variable_get('usc_mirc_streaming_url', 'http://dvr-streaming.mirc.sc.edu'),
      // "+ 1" to get rid of the slash.
      '!file_part' => drupal_substr($video_path, drupal_strlen($streaming_path) + 1),
    ));
  }
  else {
    // Didn't find the video file... Report as much.
    drupal_add_http_header('Status', 404);
    echo('Video not in expected streaming path.');
    drupal_exit();
  }

  // Throw the video_path and object at the mediainfo/PBCore generation bit.
  $parent_object = islandora_object_load($parent);
  if ($parent_object) {
    module_load_include('inc', 'usc_mirc', 'includes/utilities');
    $params = usc_mirc_retrieve_pbcore_fields($parent_object);
    usc_mirc_generate_video_metadata($video_path, $new_object, $params);
  }
  else {
    drupal_add_http_header('Status', 422);
    echo('The parent mezzanine could not be loaded.');
    drupal_exit();
  }

  // ... And add the video redirect to the object.
  $video_datastream = $new_object->constructDatastream('VIDEO', 'R');
  $video_datastream->label = 'Video Redirect';
  $video_datastream->mimetype = 'video/mpeg';
  $video_datastream->url = $built_redirect_url;
  $new_object->ingestDatastream($video_datastream);

  // Add the thumbnail to the access copy.
  $thumbnail_datastream = $new_object->constructDatastream('TN', 'M');
  $thumbnail_datastream->label = 'Thumbnail';
  $thumbnail_datastream->mimetype = 'image/png';
  $thumbnail_datastream->setContentFromFile($thumbnail_path);
  $new_object->ingestDatastream($thumbnail_datastream);

  // Add the XACML to the new access copy if present.
  if (isset($parent_object['POLICY'])) {
    $new_object->ingestDatastream($parent_object['POLICY']);
  }

  // Ingest the access copy.
  islandora_add_object($new_object);

  // Output HTTP 201 if created successfully.
  drupal_add_http_header('Status', 201);
  drupal_add_http_header('Location', url("islandora/object/{$new_object->id}", array('absolute' => TRUE)));
  drupal_exit();
}
