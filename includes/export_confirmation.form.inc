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
  // Basic confirm form.
  $form = confirm_form(
    array(),
    t(
      "Are you sure you want to export PBCORE datastreams in @coll?",
      array('@coll' => $obj->label)
    ),
    'islandora/object/' . $obj->id . '/manage',
    t('Records will be exported to @dir', array('@dir' => variable_get('usc_mirc_pbcore_export_dir', '/mnt/mirc/mezz/PBCore_Watch')))
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

  // Get the cmodel from preservation master records.
  $vro_cmodel = variable_get('usc_mirc_cmodel_vro', 'usc:vroCModel');

  // Query to count preservation masters that are members of this collection.
  $query = <<<EOQ
SELECT ?obj
FROM <#ri>
WHERE {
  $obj <fedora-rels-ext:isMemberOfCollection> <info:fedora/{$obj->id}> ;
       <fedora-model:hasModel> <info:fedora/{$vro_cmodel}> .
}
EOQ;
  $tuque = islandora_get_tuque_connection();
  $total = $tuque->repository->ri->countQuery($query, 'sparql');

  // Process the export as a batch if there's records to export.  Otherwise
  // just redirect back with a message.
  if (isset($total) && $total) {
    // Get the output directory from the database and naively sanitize it.
    $output_dir = variable_get('usc_mirc_pbcore_export_dir', '/mnt/mirc/mezz/PBCore_Watch');
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
        'progress_message' => t("Exporting PBCORE's"),
        'error_message' => t("There was an error exporting the PBCORE's"),
        'operations' => array(
          array(
            'usc_mirc_pbcore_export_batch_operation',
            array(
              $output_dir,
              $obj,
              100,
              0,
              $total,
            ),
          ),
        ),
        'finished' => 'usc_mirc_pbcore_export_batch_finished',
        'file' => drupal_get_path('module', 'usc_mirc') . '/includes/export_confirmation.form.inc',
      );

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
    drupal_set_message(check_plain("There are no preservation masters in {$obj->id} to export!"));
  }
}

/**
 * The export operation for a single iteration of the batch.
 *
 * Exports the pbcores for preservation master children of the supplied
 * collection.
 *
 * @param string $output_dir
 *   Output directory where exported pbcore xml will reside.
 *
 * @param AbstractObject $coll
 *   Collection in Fedora whose children we're exporting.  Defaults to NULL
 *   (for the actual cron job), but is used for descriptive logging when a
 *   user initiates the export.
 *
 * @param int $limit
 *   The number of pbcores to process in this iteration.
 *
 * @param int $offset
 *   Offset for the ITQL query to select the correct children.  We need this
 *   since we can't rely on setting the last export time as we go (as in the
 *   cron initiated batch).
 *
 * @param int $total
 *   Total number of pbcores to export.  Used to set the batch's progress bar.
 *
 * @param array $context
 *   The context array for the current batch being processed.
 */
function usc_mirc_pbcore_export_batch_operation($output_dir, $coll, $limit, $offset, $total, &$context) {
  module_load_include('inc', 'usc_mirc', 'includes/utilities');

  // Grab the offset out of the context if it exists.
  if (isset($context['sandbox']['offset'])) {
    $offset = $context['sandbox']['offset'];
  }

  // Set the number of processed items to zero if we're just beginning.
  if (!isset($context['sandbox']['processed'])) {
    $context['sandbox']['processed'] = 0;
  }

  // Get the cmodel from preservation master records.
  $vro_cmodel = variable_get('usc_mirc_cmodel_vro', 'usc:vroCModel');

  // Query for all preservation masters that are members of this collection.
  $query = <<<EOQ
SELECT ?obj ?time
FROM <#ri>
WHERE {
  ?obj <fedora-rels-ext:isMemberOfCollection> <info:fedora/{$coll->id}> ;
       <fedora-model:hasModel> <info:fedora/{$vro_cmodel}> ;
       <fedora-view:disseminates> ?ds .
  ?ds <fedora-view:disseminationType> <info:fedora/*/PBCORE> ;
      <fedora-view:lastModifiedDate> ?time .
}
ORDER by ?time
LIMIT $limit
OFFSET $offset
EOQ;
  $tuque = islandora_get_tuque_connection();
  $results = $tuque->repository->ri->sparqlQuery($query);

  if (!empty($results)) {
    // Open the log file.
    $log_path = $output_dir . 'pbcore_export.log';
    $log_fp = fopen($log_path, 'a');

    try {
      // Export each PBCORE record.
      foreach ($results as $result) {
        $pid = $result['obj']['value'];
        $time = $result['time']['value'];
        $output_path = $output_dir . str_replace(":", "_", $pid) . '_pbcore.xml';
        usc_mirc_export_pbcore($pid, $output_path);
        usc_mirc_log_pbcore_export_record($log_fp, $output_path);
        $context['sandbox']['processed']++;
      }

      // Set some vars in the context to use next iteration, or to hack out
      // later in the finished handler.
      $context['results']['log_path'] = $log_path;
      $context['results']['coll'] = $coll;
      $context['sandbox']['offset'] = $offset + $limit;

      // Let the batch know it's not finished yet.
      $context['finished'] = ($context['sandbox']['progress'] >= $total) ? 1 : $context['sandbox']['progress'] / $total;

      // Update the mesage for appropriate user feedback.
      $context['message'] = t('Exported @proc out of @tot', array('@proc' => $context['sandbox']['processed'], '@tot' => $total));

      $_SESSION['http_request_count']++;

      // Close the log file.
      fclose($log_fp);
    }
    catch (Exception $e) {
      fclose($log_fp);
      throw($e);
    }
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
 *   Is used to grab the collection and log path for the final message in the
 *   export manifest log.
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
