<?php
/**
 * @file
 * Stores all the handler functionality for OAI responses for USC.
 */

/**
 * Responds to the ListSets request.
 *
 * @param array $params
 *   An array containing:
 *   -token (object): A resumption token object specifying information about
 *   a previous request, or a new empty one.
 *   -max_records (int): The maximum number of records to return per request.
 *
 * @return array
 *   An array containing:
 *   -total_records (int): The total amount of records for the query.
 *   -records (array): An array containing:
 *     -pid (string): The pid of the result
 *     -label (string): The label of the result.
 *
 * @see hook_islandora_oai_identify_request_handler()
 */
function usc_mirc_oai_retrieve_sets($params) {
  module_load_include('inc', 'islandora_oai', 'includes/handler');
  $connection = islandora_get_tuque_connection();
  $records = array();
  $filters = array(
    'sameTerm(?model, <info:fedora/usc:collectionCModel>)',
  );
  $required = array(
    'statements' => array(
      '?object <fedora-rels-ext:isMemberOfCollection> <info:fedora/usc:mirc>',
    ),
  );
  $sparql_query = islandora_oai_construct_sparql_query_for_sets($filters, $required);
  $total_records = $connection->repository->ri->countQuery($sparql_query, 'sparql');
  $sparql_query .= <<<EOQ
  LIMIT {$params['max_records']}
  OFFSET {$params['token']->deliveredrecords}
EOQ;
  $query_results = $connection->repository->ri->sparqlQuery($sparql_query);
  foreach ($query_results as $result) {
    $pid = $result['object']['value'];
    $object = islandora_object_load($pid);
    $records[$pid] = array(
      'pid' => $pid,
      'label' => $result['title']['value'],
    );
    $description = FALSE;
    if (isset($object['DC'])) {
      $description = $object['DC']->content;
    }
    if ($description) {
      $records[$pid]['description'] = $description;
    }
  }
  return array(
    'total_records' => $total_records,
    'records' => $records,
  );
}

/**
 * Constructs the object response XML depending on configuration options.
 *
 * @param array $params
 *   An array containing:
 *   -metadata_prefix (string): The metadata prefix of the request being
 *   executed.
 *   -pid (string): The pid of the record that we are building for.
 *
 * @return null|string
 *   XML string output of the record depending on configuration, NULL if an
 *   error was encountered.
 *
 * @see hook_islandora_oai_identify_request_handler()
 */
function usc_mirc_oai_object_response_xml($params) {
  $object = islandora_object_load($params['pid']);

  if ($object) {
    // Mezzanine object.
    if (in_array('usc:mezzanineCModel', $object->models)) {
      if (isset($object['PBCORE']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object['PBCORE'])) {
        module_load_include('inc', 'usc_mirc', 'includes/utilities');
        $mezz_pbcore_xml = new SimpleXMLElement($object['PBCORE']->content);
        $mezz_pbcore_xml->registerXpathNamespace('pb', 'http://www.pbcore.org/PBCore/PBCoreNamespace.html');
        $mezz_defined_mappings = array(
          'dc:identifier' => 'pb:instantiationIdentifier[@source="hdl"]/text()',
        );
        $mezz_dc = usc_mirc_oai_perform_xpath($mezz_defined_mappings, $mezz_pbcore_xml, 'mezz');
        $mezz_parent = usc_mirc_get_mezzanine_parent($object);
        $mezz_parent_object = islandora_object_load($mezz_parent);
        // Get the collection for use in dcterms.
        $collection_rels = $mezz_parent_object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOfCollection');
        if ($collection_rels) {
          $member = reset($collection_rels);
          $collection = $member['object']['value'];
          $collection_object = islandora_object_load($collection);
          if ($collection_object && islandora_object_access(ISLANDORA_VIEW_OBJECTS, $collection_object)) {
            if (!isset($mezz_dc['dcterms:isPartOf'])) {
              $mezz_dc['dcterms:isPartOf'] = array();
            }
            $mezz_dc['dcterms:isPartOf'][] = $collection_object->id;
          }
        }
        // Will only be a member of one preservation master.
        $pres_rels = $object->relationships->get(FEDORA_RELS_EXT_URI, 'isDerivativeOf');
        $pres_rels = reset($pres_rels);
        $pres_object = islandora_object_load($pres_rels['object']['value']);
        $pres_dc = array();
        if ($pres_object && isset($pres_object['PBCORE']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $pres_object['PBCORE'])) {
          $pres_pbcore_xml = new SimpleXMLElement($pres_object['PBCORE']->content);
          $pres_pbcore_xml->registerXpathNamespace('pb', 'http://www.pbcore.org/PBCore/PBCoreNamespace.html');
          $pres_defined_mappings = array(
            'dc:title' => 'pb:pbcoreTitle[@titleType="Main" and normalize-space(text())]',
            'dcterms:alternative' => 'pb:pbcoreTitle[not(@titleType="Main") and normalize-space(text())]',
            'dc:subject' => 'pb:pbcoreSubject[text()] | pb:pbcoreGenre[text()]',
            'dc:description' => 'pb:pbcoreDescription[@descriptionType="summary" and text()]',
            'dc:coverage' => 'pb:pbcoreCoverage[pb:coverageType[text() = "Spatial"]]/pb:coverage[text()]/text()',
            'dc:format' => 'pb:pbcoreDescription[@descriptionType="sound content"]/text()',
            'dc:rights' => 'pb:pbcoreRightsSummary/pb:rightsSummary[@source="copyright_statement"]/text()',
          );
          $pres_dc = usc_mirc_oai_perform_xpath($pres_defined_mappings, $pres_pbcore_xml, 'pres');
        }

        $dc_xml = array_filter(array_merge_recursive($pres_dc, $mezz_dc));
        if (!isset($dc_xml['dc:type'])) {
          $dc_xml['dc:type'] = array();
        }
        $dc_xml['dc:type'][] = 'MovingImage';
        if (!isset($dc_xml['dc:publisher'])) {
          $dc_xml['dc:publisher'] = array();
        }
        $dc_xml['dc:publisher'][] = 'University of South Carolina. Moving Image Research Collections';
        // Sort the array for prettyness sake.
        ksort($dc_xml);
        $writer = new XMLWriter();
        $writer->openMemory();
        $writer->startElement('oai_dc:dc');
        $writer->writeAttribute('xmlns:dc', 'http://purl.org/dc/elements/1.1/');
        $writer->writeAttribute('xmlns:dcterms', 'http://purl.org/dc/terms/');
        $writer->writeAttribute('xmlns:oai_dc', 'http://www.openarchives.org/OAI/2.0/');
        foreach ($dc_xml as $key => $values) {
          foreach ($values as $value) {
            $writer->writeElement($key, $value);
          }
        }
        $writer->endElement();
        return $writer->outputMemory();
      }
    }
    // Collection object.
    else {
      if (isset($object['MODS'])) {
        $xslt = new XSLTProcessor();
        $xsl = new DOMDocument();
        $xsl->load(drupal_get_path('module', 'usc_mirc') . '/transforms/usc_mods_to_oai_dc.xsl');
        $xslt->importStylesheet($xsl);
        $mods_xml = new DOMDocument();
        $mods_xml->loadXML($object['MODS']->content);
        return $xslt->transformToXml($mods_xml);
      }
      else {
        return $object['DC']->content;
      }
    }
  }
  return NULL;
}

/**
 * Gets the setSpec membership information for a record.
 *
 * @param array $params
 *   An array containing:
 *   -record: An array containing:
 *     -pid: A string representation of the PID of the record.
 *     -date: A string representation of the date timestamp of the record.
 *     -solr_doc: An array containing the Solr doc for the record.
 *
 * @return array
 *   An array containing all set pids that this record belongs to.
 *
 * @see hook_islandora_oai_identify_request_handler()
 */
function usc_mirc_oai_get_membership($params) {
  $sets = array();
  $object = islandora_object_load($params['record']['pid']);
  if ($object) {
    if (in_array('usc:mezzanineCModel', $object->models)) {
      // Will only be a member of one preservation master.
      $pres_rels = $object->relationships->get(FEDORA_RELS_EXT_URI, 'isDerivativeOf');
      $pres_rels = reset($pres_rels);
      $pres_object = islandora_object_load($pres_rels['object']['value']);
      if ($pres_object) {
        $collection_rels = $pres_object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOfCollection');
        $collection_rels = reset($collection_rels);
        $sets[] = $collection_rels['object']['value'];
      }
    }
  }
  return $sets;
}

/**
 * Responds to the ListRecords or ListIdentifier request.
 *
 * @param array $params
 *   An array containing:
 *   -set: String of the PID of the set being searched for, FALSE if no set
 *   argument.
 *   -from: String of the from time starting point, FALSE if no from argument.
 *   -until: String of the until ending point, FALSE if no until argument.
 *   -token: Object representing a token.
 *
 * @return array
 *   An array describing the record retrieved from Solr.
 */
function usc_mirc_oai_retrieve_records_or_identifiers($params) {
  module_load_include('inc', 'islandora_oai', 'includes/handler');
  $set = $params['set'];
  $from = $params['from'];
  $until = $params['until'];
  $token = $params['token'];
  $query = NULL;
  $date_query = NULL;

  if ($from || $until) {
    $field = variable_get('islandora_oai_date_field', 'fgs_lastModifiedDate_dt');

    if ($from) {
      $solr_from = islandora_oai_datetime_to_solr_time($from);
    }
    else {
      $solr_from = '*';
    }

    if ($until) {
      $solr_until = islandora_oai_datetime_to_solr_time($until, TRUE);
    }
    else {
      $solr_until = '*';
    }
    $date_query = "$field:[$solr_from TO $solr_until]";
  }

  if (!isset($set)) {
    $set = NULL;
  }
  $results = usc_mirc_oai_query_solr($token->deliveredrecords, $set, $date_query);
  $records = array();
  foreach ($results['objects'] as $solr_object) {
    $records[] = islandora_oai_construct_record_from_solr_response($solr_object);
  }
  return array(
    'total_records' => $results['numFound'],
    'records' => $records,
  );
}

/**
 * Function that performs the queries to Solr.
 *
 * @param int $start
 *   The starting point for Solr queries, mainly used for resumption tokens.
 * @param bool $set
 *   Whether we are querying down for a set which requires walking in ITQL land.
 * @param bool $date_query
 *   Whether this is a date query or not.
 *
 * @return array
 *   The results generated from the crafted Solr query.
 */
function usc_mirc_oai_query_solr($start = 0, $set = NULL, $date_query = NULL) {
  global $user;
  $query_processor = new IslandoraSolrQueryProcessor();
  $query_processor->buildQuery('*:*');
  $query_processor->solrParams['fl'] = '*';

  // Need to modify one of the FQs USC uses for Solr display results. This can
  // have two possible values depending on the whether the user is logged in or
  // not. Harvesters should crawl as anonymous but let's handle both cases
  // anyway.
  $pres_search = array('administrator', 'Creator', 'Editor', 'Manager');
  $accepted_role = array_intersect($pres_search, $user->roles);

  if (empty($accepted_role)) {
    $fq_to_remove = "+rels_hasModel_uri_ms:(usc\:mezzanineCModel usc\:collectionCModel) +(pb_parent_title_Main_ms:[* TO *] mods_title_s:[* TO *])";
  }
  else {
    $fq_to_remove = "+rels_hasModel_uri_ms:(usc\:mezzanineCModel usc\:collectionCModel usc\:vroCModel) +(pb_title_Main_ms:[* TO *] pb_parent_title_Main_ms:[* TO *] mods_title_s:[* TO *])";
  }
  $fq = $query_processor->solrParams['fq'];
  $fq_remove_key = array_search($fq_to_remove, $fq);

  if ($fq_remove_key) {
    unset($fq[$fq_remove_key]);
  }
  $query_processor->solrParams['fq'] = $fq;
  if ($date_query) {
    $query_processor->solrParams['fq'][] = $date_query;
  }
  if ($set) {
    $parent_solr_field = variable_get('usc_mirc_parent_collection_solr_field', 'usc_parent_collection_pid_ms');
    // OAI uses : to separate multiple setSpecs so we had to replace it
    // from the pids earlier.
    $sets = explode(':', $set);
    $set_fq = array();
    foreach ($sets as $key => $value) {
      // Replace the first occurrence of _ with a : so we are back to a
      // valid pid.
      $value = preg_replace('/_/', ':', $value, 1);
      $value = trim($value);
      $set_fq[] = $parent_solr_field . ':("' . $value . '")';
    }
    $query_processor->solrParams['fq'][] = '(' . implode(' OR ', $set_fq) . ')';
    $query_processor->solrParams['fq'][] = '+rels_hasModel_uri_ms:"usc:mezzanineCModel"';
  }
  else {
    // Limit to just mezzanines or collections if not inside a set.
    $query_processor->solrParams['fq'][] = '+rels_hasModel_uri_ms:("usc:mezzanineCModel" OR "usc:collectionCModel")';
  }
  $query_processor->solrStart = $start;
  $query_processor->solrLimit = variable_get('islandora_oai_max_size', '20');
  try {
    $query_processor->executeQuery(FALSE);
    $solr_results = $query_processor->islandoraSolrResult['response'];
  }
  catch (Exception $e) {
    drupal_set_message(t('error searching @message', array('@message' => $e->getMessage())), 'error');
  }
  return $solr_results;
}

/**
 * Performs the given XPATHs on the source document.
 *
 * @param array $xpaths
 *   An associative array where the key is the DC element and the value is the
 *   XPATH to be performed.
 * @param SimpleXMLElement $source
 *   The source XML document that is being XPATHed.
 * @param string $type
 *   The source of the XPaths, either mezz or pres respectively.
 *
 * @return array
 *   An array containing the DC key values mapped to their text values or FALSE
 *   if they did not exist.
 */
function usc_mirc_oai_perform_xpath($xpaths, $source, $type) {
  $dc_mappings = array();
  foreach ($xpaths as $dc => $xpath) {
    $xpath_result = $source->xpath($xpath);
    $dc_value = FALSE;
    foreach ($xpath_result as $result) {
      $dc_value[] = trim((string) $result);
    }
    $dc_mappings[$dc] = $dc_value;
  }

  if ($type == 'pres') {
    // Handle some additional special cases for USC.
    // PBCore Creator.
    $creator_xpath = $source->xpath('pb:pbcoreCreator');
    foreach ($creator_xpath as $creator) {
      $creator_value = (string) $creator->creator;
      if (isset($creator->creatorRole)) {
        $creator_role = (string) $creator->creatorRole;
      }
      else {
        $creator_role = FALSE;
      }
      $creator_string = format_string('!creator_value (!creator_role)', array(
        '!creator_value' => $creator_value,
        '!creator_role' => $creator_role ? $creator_role : 'No Role Provided',
      ));
      $dc_mappings['dc:creator'][] = $creator_string;
    }

    $contributor_xpath = $source->xpath('pb:pbcoreContributor');
    foreach ($contributor_xpath as $contributor) {
      $contributor_value = (string) $contributor->contributor;
      if (isset($contributor->contributorRole)) {
        $contributor_role = (string) $contributor->contributorRole;
      }
      else {
        $contributor_role = FALSE;
      }
      $contributor_string = format_string('!contributor_value (!contributor_role)', array(
        '!contributor_value' => $contributor_value,
        '!contributor_role' => $contributor_role ? $contributor_role : 'No Role Provided',
      ));
      $dc_mappings['dc:contributor'][] = $contributor_string;
    }

    $coverage_xpath = $source->xpath('pb:pbcoreCoverage');
    foreach ($coverage_xpath as $coverage) {
      if (isset($coverage->coverageType)) {
        $coverage_type = (string) $coverage->coverageType;
        $coverage_type = strtolower($coverage_type);
        $coverage_value = (string) $coverage->coverage;
        $dc_mappings["dcterms:$coverage_type"][] = $coverage_value;
      }
    }

    $collapse_dates = function($source, $date_xpaths) {
      $date_parts = array();
      foreach ($date_xpaths as $date) {
        $date_xpath = $source->xpath($date);
        if ($date_xpath) {
          $date_string = (string) reset($date_xpath);
          if (!isset($date_parts[$date_string])) {
            $date_parts[$date_string] = $date_string;
          }
        }
      }
      return implode('-', $date_parts);
    };

    $dc_mappings['dc:date'][] = $collapse_dates($source, array(
      'pb:pbcoreAssetDate[@dateType="createdStart"]',
      'pb:pbcoreAssetDate[@dateType="createdEnd"]',
    ));
    $dc_mappings['dc:date'][] = $collapse_dates($source, array(
      'pb:pbcoreAssetDate[@dateType="publishedStart"]',
      'pb:pbcoreAssetDate[@dateType="publishedEnd"]',
    ));
  }
  elseif ($type == 'mezz') {
    $type_xpath = $source->xpath('pb:instantiationGenerations/text()');
    if ($type_xpath) {
      $type = (string) reset($type_xpath);
      if ($type == 'Edited') {
        $dc_mappings['dc:type'][] = 'This video has been edited by MIRC staff.';
      }
      elseif ($type == 'Part') {
        $dc_mappings['dc:type'][] = 'This video constitutes a part of the moving image described.';
      }
    }
    $dc_mappings['dcterms:medium'][] = 'Film';
  }
  return $dc_mappings;
}
