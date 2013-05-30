<?php

/**
 * @file
 * Simple confirmation form for a user initiated PBCORE export.
 */

/**
 * Form funtion for confirming a pbcore export for a specific collection.
 *
 * @param arrray $form
 *   The form's renderable array.
 *
 * @param array $form_state
 *   Array of form state variables.
 *
 * @return array
 *   A drupal renderable array.  The $form variable, in this instance.
 */
function usc_mirc_export_collection_confirm_form($form, &$form_state, $obj) {
  // Basic confirm form
  $form = confirm_form(
    array(),
    t(
      "Are you sure you want to export PBCORE datastreams in @coll?",
      array('@coll' => $obj->label)
    ),
    'islandora/object/' . $obj->id . '/manage'
  );

  // Stuff the collection into the form so we can pull it out and use it in the
  // submit handler.
  $form['obj'] = array(
    '#type' => 'value',
    '#value' => $obj,
  );

  return $form;
}

/**
 * Submit handler for the pbcore export confirmation form.
 *
 * Exports all pbcore docs from children of the supplied collection.
 * Does NOT set the last export time in the db.  Setting the time based on user
 * input for only a single collection will cause the next cron run to miss
 * other objects that may have been udated in different collections.
 *
 * @param arrray $form
 *   The form's renderable array.
 *
 * @param array $form_state
 *   Array of form state variables.
 */
function usc_mirc_export_collection_confirm_form_submit($form, &$form_state) {
  module_load_include('inc', 'usc_mirc', 'includes/utilities');

  // Hack the object out of storage.
  $obj = $form_state['values']['obj'];

  // Get the time of last export, defaulting to the beginning of the epoch if
  // this is the first export.  The time is formatted as a string that mulgara
  // can work with.
  $last_export_time = variable_get("usc_mirc_last_pbcore_export", usc_mirc_format_time(0));

  // Query for all preservation masters that have been updated since last
  // export that are members of this collection.
  $query = 'select $obj $time
            from <#ri>
            where $obj <fedora-rels-ext:isMemberOfCollection> <info:fedora/' . $obj->id . '>
                  and $obj <info:fedora/fedora-system:def/view#lastModifiedDate> $time
                  and $time <mulgara:after> \'' . $last_export_time . '\'^^<xml-schema:dateTime> in <#xsd>';
  $tuque = islandora_get_tuque_connection();
  $results = $tuque->repository->ri->itqlQuery($query, 'unlimited');

  // Process the export as a batch if there's records to export.  Otherwise
  // just redirect back with a message.
  if (!empty($results)) {
    // Get the output directory from the database and naively sanitize it.
    $output_dir = variable_get('usc_mirc_pbcore_export_dir', '/mnt/mircmezz/PBCore_Watch');
    if (substr($output_dir, -1) != '/') {
      $output_dir .= '/';
    }

    // Open a pointer to the logfile.
    $log_path = $output_dir . 'pbcore_export.log';
    $log_fp = fopen($log_path, 'a');

    try {
      // Log the manifest header.
      usc_mirc_log_pbcore_export_header($log_fp, $obj);

      // Set up the batch.
      $batch = array(
        'title' => t("Exporting PBCORE records."),
        'init_message' => t("The export is beginning."),
        'progress_message' => t('Exported @current out of @total.'),
        'error_message' => t("There was an error exporting the PBCORE records."),
        'operations' => array(),
        'finished' => 'usc_mirc_pbcore_export_batch_finished',
        'file' => drupal_get_path('module', 'usc_mirc') . '/includes/export_confirmation.form.inc',
      );

      // Add the batch operations.
      foreach ($results as $result) {
        $batch['operations'][] = array(
          'usc_mirc_pbcore_export_batch_operation',
          array(
            $result['obj']['value'],
            $output_dir,
            $obj,
          ),
        );
      }

      // Start the batch.
      batch_set($batch);

      // Close the log pointer.
      fclose($log_fp);
    }
    catch (Exception $e) {
      fclose($log_fp);
      throw $e;
    }
  }
  else {
    // Redirect back to the manage overlay.
    $redirect = array('islandora/object/' . $obj->id . '/manage');
    $form_state['redirect'] = $redirect;
    drupal_set_message("No PBCORE records in {$obj->id} have been updated since the last export on $last_export_time");
  }
}

/**
 * The export operation for a single record in the batch.
 *
 * Exports the pbcore for the object with the supplied pid and writes the 
 * appropriate entry in the export manifest log.
 *
 * @param string $pid
 *   Pid of the object whose PBCORE datastream we wish to export.
 *
 * @param string $output_dir
 *   Output directory where exported pbcore xml will reside.
 *
 * @param AbstractObject $coll
 *   Collection in Fedora whose children we're exporting.  Defaults to NULL
 *   (for the actual cron job), but is used for descriptive logging when a
 *   user initiates the export.
 *
 * @param array $context
 *   The context array for the current batch being processed.
 */
function usc_mirc_pbcore_export_batch_operation($pid, $output_dir, $coll, &$context) {
  module_load_include('inc', 'usc_mirc', 'includes/utilities');

  dd("PROCESSING BATCH OPERATION");

  // Open the log file.
  $log_path = $output_dir . 'pbcore_export.log';
  $log_fp = fopen($log_path, 'a');

  try {
    // Export the PBCORE record.
    $output_path = $output_dir . str_replace(":", "_", $pid) . '_pbcore.xml';
    usc_mirc_export_pbcore($pid, $output_path);
    usc_mirc_log_pbcore_export_record($log_fp, $output_path);

    // Set some vars in the context to hack out later in the finished handler.
    if (!isset($context['results']['log_path'])) {
      $context['results']['log_path'] = $log_path;
    }
    if (!isset($context['results']['coll']) && $coll) {
      $context['results']['coll'] = $coll;
    }

    // Close the log file.
    fclose($log_fp);
  }
  catch (Exception $e) {
    fclose($log_fp);
    throw($e);
  }
}

/**
 * Callback function for when the export batch finishes processing.
 *
 * Closes the export manifest log for the batch that just ended.
 *
 * @param bool $success
 *   Boolean indicating whether or not there were errors during the batch 
 *   processing.
 *
 * @param array $results
 *   Array containing values that are set during the batch operation function.
 *   Is used to grab the collection and log path for the final message in the export manifest log.
 *
 * @param array $operations
 *   Array of unprocessed operations if there was an error.
 */
function usc_mirc_pbcore_export_batch_finished($success, $results, $operations) {
  module_load_include('inc', 'usc_mirc', 'includes/utilities');

  // Hack out the log path and collection from the results array.
  $log_path = $results['log_path'];

  // Open the log.
  $log_fp = fopen($log_path, 'a');

  try {
    // Finish the manifest and redirect the user.
    usc_mirc_log_pbcore_export_footer($log_fp);
    fclose($log_fp);
    if (isset($results['coll'])) {
      $coll = $results['coll'];
      drupal_goto("islandora/object/{$coll->id}/manage");
    }
  }
  catch (Exception $e) {
    fclose($log_fp);
    throw($e);
  }
}

