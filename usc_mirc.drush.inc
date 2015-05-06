<?php

/**
 * @file
 * Drush command/hook implementation for updating existing citation objects.
 */

/**
 * Implements hook_drush_command().
 */
function usc_mirc_drush_command() {
  $commands = array();

  $commands['usc-mirc-update-mezzanines'] = array(
    'description' => dt('Update existing Mezzanines to generate Handles in PBCore and transform to DC. Note that the --uri option needs to be set to the current site as not specifying one will cause Drupal to use http://default when constructing URLs.'),
    'drupal dependencies' => array(
      'usc_mirc',
      'islandora_handle',
      'islandora',
    ),
    'options' => array(
      'commit' => array(
        'description' => dt('Whether we are doing a test run or not, defaults to FALSE if not specified'),
        'required' => FALSE,
      ),
    ),
    'examples' => array(
      'drush -u 1 --uri=http://mirc.sc.edu usc-mirc-update-mezzanines' => dt('Updating existing citations with PDF derivatives.'),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
  );
  return $commands;
}

/**
 * Command callback to update Mezzanines with Handle.
 */
function drush_usc_mirc_update_mezzanines() {
  batch_set(usc_mirc_update_mezzanines_create_batch());
  drush_backend_batch_process();
}

/**
 * Constructs a batch used to update things via Drush.
 */
function usc_mirc_update_mezzanines_create_batch() {
  $commit = drush_get_option('commit', 'FALSE') === 'TRUE';
  return array(
    'operations' => array(
      array('usc_mirc_update_mezzanine_batch_operation', array($commit)),
    ),
    'title' => t('Updating Mezzanines...'),
    'init_message' => t('Preparing to update Mezzanines with Handles.'),
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaining @estimate.'),
    'error_message' => t('An error has occurred.'),
    'file' => drupal_get_path('module', 'usc_mirc') . '/usc_mirc.drush.inc',
  );
}

/**
 * Constructs and performs the Mezzanine update batch operation.
 *
 * @param bool $commit
 *   Whether we are doing a test run or not.
 * @param array $context
 *   The context of the Drupal batch.
 *
 * @throws Exception
 */
function usc_mirc_update_mezzanine_batch_operation($commit, &$context) {
  $update = 10;
  $sandbox =& $context['sandbox'];

  $query = <<<EOQ
SELECT ?object ?created FROM <#ri>
WHERE {
  ?object <fedora-model:hasModel> <info:fedora/usc:mezzanineCModel> ;
          <fedora-model:createdDate> ?created
  !filter
}
ORDER BY ?created

EOQ;
  $tuque = islandora_get_tuque_connection();
  if (!isset($sandbox['total'])) {
    $sandbox['progress'] = 0;

    $sandbox['total'] = $tuque->repository->ri->countQuery(format_string($query, array(
      '!filter' => '',
    )), 'sparql');
    if ($sandbox['total'] == 0) {
      // Nothing to process.
      $context['finished'] = 1;

      $context['message'] = t('Nothing to fix.');
      return;
    }
    $sandbox['result_stash'] = array();
  }

  if (empty($sandbox['result_stash'])) {
    $limited_query = format_string($query, array(
        '!filter' => (isset($sandbox['last_date']) ?
            "FILTER(?created > \"{$sandbox['last_date']}\"^^<xsd:dateTime>)" :
            ''),
      )) . <<<EOQ
LIMIT $update
EOQ;
    drush_log(dt('Querying for @count objects.', array('@count' => $update)));
    $sandbox['result_stash'] = $tuque->repository->ri->sparqlQuery($limited_query);
    if (empty($sandbox['result_stash'])) {
      // Ran out of items early?
      $context['finished'] = 1;

      $context['message'] = t('Updated @count PID(s) and ran out of items early... Somebody manually updated a document?', array(
        '@count' => $sandbox['total'],
      ));
      return;
    }
  }

  $result = array_shift($sandbox['result_stash']);
  $updating_pid = $result['object']['value'];
  $sandbox['last_date'] = $result['created']['value'];
  $object_to_update = islandora_object_load($updating_pid);

  // See if a Handle is already in the PBCore and or the DC.
  $pbcore_exist = usc_mirc_xpath_pbcore_for_handle($object_to_update);
  if ($pbcore_exist) {
    $dc_exist = usc_mirc_query_ri_for_handle($updating_pid);
    if ($dc_exist) {
      drush_log(dt('The object @pid already has a Handle in the PBCore and the DC.', array('@pid' => $object_to_update->id)));
    }
    // Handle isn't present in the DC, need to go transform the PBCore to DC.
    else {
      if ($commit) {
        $pbcore = $object_to_update['PBCORE']->content;
        $dc_transform = new DOMDocument();
        $dc_transform->load(drupal_get_path('module', 'usc_mirc') . '/transforms/pbcore_to_dc.xsl');
        if (!$dc_transform) {
          throw new Exception(t('Failed loading PBCore to DC transform for @object_id.', array(
            '@object_id' => $object_to_update->id,
          )));
        }
        $transformer = new XSLTProcessor();
        $transformer->importStyleSheet($dc_transform);

        $pbcore_doc = new DOMDocument();
        $pbcore_doc->loadXML($pbcore);
        $dc = $transformer->transformToXML($pbcore_doc);
        $object_to_update['DC']->content = $dc;
      }
      drush_log(dt('The object @pid had a Handle in PBCore but not in DC, updating the DC.', array('@pid' => $object_to_update->id)));
    }
  }
  else {
    // Need to generate a new Handle for this object in the PBCore and DC.
    module_load_include('inc', 'islandora_handle', 'includes/derivatives');
    drush_log(dt('Creating a Handle and updating the PBCore and DC for @pid.', array('@pid' => $object_to_update->id)));
    if ($commit) {
      $handle_creation = islandora_handle_append_handles_from_configuration($object_to_update);
      if ((isset($handle_creation['success']) && $handle_creation['success']) || !$handle_creation) {
      //if ($handle_creation['success']) {
        drush_log(dt('Successfully created a Handle and appended to PBCore for @pid.', array('@pid' => $object_to_update->id)));
        $dc_update = islandora_handle_add_to_dc_datastream($object_to_update);
        if (isset($dc_update['success']) && $dc_update['success']) {
          drush_log(dt('Successfully updated the Dublin Core with a Handle for @pid.', array('@pid' => $object_to_update->id)));
        }
        elseif (isset($dc_update['success']) && !$dc_update['success']) {
          drush_log(dt('Unable to append the Handle to Dublin Core for @pid.', array('@pid' => $object_to_update->id)));
        }
        else {
          drush_log(dt('Handle already exists in the Dublin Core for @pid.', array('@pid' => $object_to_update->id)));
        }
      }
      else {
        drush_log(dt('Unable to create a Handle in PBCore for @pid.', array('@pid' => $object_to_update->id)));
      }
    }
  }
  $sandbox['progress'] = min($sandbox['total'], $sandbox['progress'] + 1);
  $context['finished'] = $sandbox['progress'] / $sandbox['total'];

  $context['message'] = t('Updated @progress of @total.', array(
    '@progress' => $sandbox['progress'],
    '@total' => $sandbox['total'],
  ));
}

/**
 * Queries for the existence of a Handle in the dc.identifier field for a pid.
 *
 * @param string $pid
 *   The PID of the object we are checking.
 *
 * @return bool
 *   TRUE if one exists, FALSE otherwise.
 */
function usc_mirc_query_ri_for_handle($pid) {
  $query = <<<EOQ
ASK WHERE {
  <info:fedora/$pid> <http://purl.org/dc/elements/1.1/identifier> "http://hdl.handle.net/11004/$pid"
}
EOQ;
  $tuque = islandora_get_tuque_connection();
  $results = $tuque->repository->ri->sparqlQuery($query);
  $result = reset($results);
  return $result['k0']['value'] !== 'false';
}

/**
 * XPaths the PBCore for the existence of a Handle.
 *
 * @param AbstractObject $object
 *   An AbstractObject representing an object within Fedora.
 *
 * @return bool
 *   TRUE if the PBCore has a Handle, FALSE otherwise.
 */
function usc_mirc_xpath_pbcore_for_handle(AbstractObject $object) {
  if (!isset($object['PBCORE'])) {
    return FALSE;
  }

  $pbcore_xml = simplexml_load_string($object['PBCORE']->content);
  $pbcore_xml->registerXPathNamespace('pb', 'http://www.pbcore.org/PBCore/PBCoreNamespace.html');
  $result = $pbcore_xml->xpath('//pb:instantiationIdentifier[@source="hdl"]');
  return !empty($result);
}
