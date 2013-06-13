<?php

/**
 * @file
 * Contains functions for drush scripts to be used with the usc_mirc module.
 */

/**
 * Implements hook_drush_command().
 */
function usc_mirc_drush_command() {
  $items = array();
  $items['usc_mirc_cron_export_pbcore'] = array(
    'callback' => 'usc_mirc_cron_export_pbcore',
    'description' => 'Exports all PBCORE datastreams for all preservation masters in response to a cron job.',
    'arguments' => array(
      'limit' => "The number of pbcore records to process per iteration of the batch.",
    ),
    'examples' => array(
      'drush -u 1 -l http://mysite usc_mirc_cron_export_pbcore 250',
    ),
    'boostrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
  );
  return $items;
}

/**
 * Drush command for pbcore exports to be executed in response to cron.
 *
 * Exports all pbcore records for preservation masters that have been updated
 * since the last export.  Uses a batch for processing.
 *
 * @param int $limit
 *   The number of pbcore records to export per iteration of the batch.
 */
function usc_mirc_cron_export_pbcore($limit = 100) {
  module_load_include('inc', 'usc_mirc', 'includes/utilities');

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
      'operations' => array(
        array(
          'usc_mirc_cron_pbcore_export_batch_operation',
          array(
            $output_dir,
            $limit,
          ),
        ),
      ),
      'finished' => 'usc_mirc_cron_pbcore_export_batch_finished',
      'file' => drupal_get_path('module', 'usc_mirc') . '/usc_mirc.drush.inc',
    );

    // Start the batch.
    batch_set($batch);
    drush_backend_batch_process();

    // Close the log pointer.
    fclose($log_fp);
  }
  catch (Exception $e) {
    fclose($log_fp);
    throw $e;
  }
}

/**
 * Exports pbcore records for all recently updated preservation masters.
 *
 * @param string $output_dir
 *   Output directory where exported pbcore xml will reside.
 *
 * @param int $limit
 *   The number of pbcore records to export per iteration of the batch.
 *
 * @param array $context
 *   The context array for the current batch being processed.
 */
function usc_mirc_cron_pbcore_export_batch_operation($output_dir, $limit, &$context) {

  module_load_include('inc', 'usc_mirc', 'includes/utilities');

  // Get the cmodel from preservation master records.
  $vro_cmodel = variable_get('usc_mirc_cmodel_vro', 'usc:vroCModel');

  // Get the time of last export, defaulting to the beginning of the epoch if
  // this is the first export.  The time is formatted as a string that mulgara
  // can work with.
  $last_export_time = variable_get("usc_mirc_last_pbcore_export", usc_mirc_format_time(0));

  // Query for all preservation masters that have been updated since last
  // export.
  $query = 'select $obj $time
            from <#ri>
            where $obj <fedora-model:hasModel> <info:fedora/' . $vro_cmodel . '>
                  and $obj <fedora-view:lastModifiedDate> $time
                  and $time <mulgara:after> \'' . $last_export_time . '\'^^<xml-schema:dateTime> in <#xsd>
            order by $time
            limit ' . $limit;
  $tuque = islandora_get_tuque_connection();
  $results = $tuque->repository->ri->itqlQuery($query, 'unlimited');

  $log_path = $output_dir . 'pbcore_export.log';
  $context['results']['log_path'] = $log_path;

  if (!empty($results)) {
    // Open the log file.
    $log_fp = fopen($log_path, 'a');

    try {
      // Export each PBCORE record.
      foreach ($results as $result) {
        $pid = $result['obj']['value'];
        $time = $result['time']['value'];
        $output_path = $output_dir . str_replace(":", "_", $pid) . '_pbcore.xml';
        usc_mirc_export_pbcore($pid, $output_path);

        // Set the last export time based on the object just in case things go
        // sour.
        variable_set('usc_mirc_last_pbcore_export', $time);

        // Log the export.
        usc_mirc_log_pbcore_export_record($log_fp, $output_path);

      }

      // Let the batch know it's not finished yet.
      $context['finished'] = 0;

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
 *   Is used to grab the log path for the final message in the export manifest
 *   log.
 *
 * @param array $operations
 *   Array of unprocessed operations if there was an error.
 */
function usc_mirc_cron_pbcore_export_batch_finished($success, $results, $operations) {
  module_load_include('inc', 'usc_mirc', 'includes/utilities');

  // Hack out the log path and collection from the results array.
  $log_path = $results['log_path'];

  // Open the log.
  $log_fp = fopen($log_path, 'a');

  try {
    // Finish the manifest and redirect the user.
    usc_mirc_log_pbcore_export_footer($log_fp);
    fclose($log_fp);
  }
  catch (Exception $e) {
    fclose($log_fp);
    throw($e);
  }
}
