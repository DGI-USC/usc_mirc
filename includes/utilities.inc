<?php
/**
 * @file
 * Utilities used in the MIRC@SC module.
 */

/**
 * Retrieves the image markup for an object.
 *
 * @param string $pid
 *   The PID of the object.
 *
 * @param string $alt
 *   Text to be displayed as an alt.
 *
 * @param string $title
 *   Title of the image.
 *
 * @param string $imagecache_preset
 *   The Imagecache External preset we are to use.
 *
 * @return string
 *   Markup for the generated image.
 */
function usc_mirc_get_image_markup($pid, $alt = '', $title = '', $imagecache_preset = NULL) {
  $object = islandora_object_load($pid);
  $imagecache_preset = variable_get('usc_mirc_imagecache_thumbnail_preset', 'usc_mirc_thumbnail');

  $dsid = '';
  if ($object['IMG']) {
    $dsid = 'IMG';
  }
  else {
    $dsid = 'TN';
  }
  $image = usc_mirc_get_imagecache_token_url($object, $dsid);

  if (module_exists('imagecache_external') && is_callable('theme_imagecache_external')) {
    return theme('imagecache_external', array(
      'path' => url($image, array('absolute' => TRUE)),
      'style_name' => $imagecache_preset,
      'alt' => 'Collection image')
    );
  }
  else {
    return theme('image', array(
      'path' => $image,
      'alt' => $alt,
      'title' => $title,
      'attributes' => array(),
      )
    );
  }
}

/**
 * Builds and returns a query for VRO objects based upon parameters.
 *
 * @param FedoraObject $object
 *   A FedoraObject.
 *
 * @param bool $count
 *   Whether this is a count query or not.
 *
 * @param int $page
 *   The page number we are to start at.
 *
 * @param int $number
 *   Number of items per page.
 *
 * @return array
 *   Results from the collection set query.
 */
function usc_mirc_get_vro_query($object, $count = FALSE, $page = NULL, $number = NULL) {
  $collection_relation = variable_get('usc_mirc_collection_relationship', 'isMemberOfCollection');
  $vro_cmodel = variable_get('usc_mirc_cmodel_vro', 'usc:vroCModel');

  $query = "PREFIX fre: <info:fedora/fedora-system:def/relations-external#>
  PREFIX fm: <info:fedora/fedora-system:def/model#>
  PREFIX dc: <http://purl.org/dc/elements/1.1/>
  PREFIX is: <http://islandora.ca/ontology/relsext#>
  SELECT ?vro ?vro_title
  WHERE {
  ?vro fre:$collection_relation <info:fedora/$object->id> ;
       fm:hasModel <info:fedora/$vro_cmodel> ;
       fm:state fm:Active ;
       dc:title ?vro_title .
   OPTIONAL {
      ?vro is:isViewableByUser ?usr .
      ?vro is:isViewableByRole ?role .
    }";
  $query .= usc_mirc_append_xacml_filter();
  $query .= "ORDER BY ?vro_title";

  if (!$count) {
    $offset = $page * $number;
    $query .= "
    LIMIT $number
    OFFSET $offset";
  }
  return $query;
}

/**
 * Performs a SPARQL query to go out and get the VRO set.
 *
 * @param FedoraObject $object
 *   A FedoraObject.
 *
 * @param int $page
 *   The page to start on.
 *
 * @param int $number
 *   The number of items per page.
 *
 * @return array
 *   An array containing the results of the VRO query.
 */
function usc_mirc_get_vro_set($object, $page = 0, $number = 20) {
  $query = usc_mirc_get_vro_query($object, FALSE, $page, $number);
  $query_array = array(
    'query' => $query,
    'type' => 'sparql',
    'page_size' => $page,
    'page_number' => $number,
  );

  $results = $object->repository->ri->query($query_array['query'], $query_array['type']);
  return $results;
}

/**
 * Performs a SPARQL query to retrieve the number of children for VROs.
 *
 * @param FedoraObject $object
 *   A FedoraObject.
 *
 * @return int
 *   The number of VRO children.
 */
function usc_mirc_get_vro_children($object) {
  $query = usc_mirc_get_vro_query($object, TRUE);
  $query_array = array(
    'query' => $query,
    'type' => 'sparql',
  );
  $results = $object->repository->ri->query($query_array['query'], $query_array['type']);
  return count($results);
}

/**
 * Builds and returns a query for mezzanine objects based upon parameters.
 *
 * @param FedoraObject $object
 *   A FedoraObject.
 *
 * @param bool $count
 *   Whether this is a count query or not.
 *
 * @param int $page
 *   The page number we are to start at.
 *
 * @param int $number
 *   Number of items per page.
 *
 * @return array
 *   Results from the collection set query.
 */
function usc_mirc_get_mezzanine_query($object, $count = FALSE, $page = NULL, $number = NULL) {
  $collection_relation = variable_get('usc_mirc_collection_relationship', 'isMemberOfCollection');
  $derivative_relation = variable_get('usc_mirc_relationship', 'isDerivativeOf');
  $vro_cmodel = variable_get('usc_mirc_cmodel_vro', 'usc:vroCModel');
  $mezzanine_cmodel = variable_get('usc_mirc_cmodel_mezzanine', 'usc:mezzanineCModel');

  $query = "PREFIX fre: <info:fedora/fedora-system:def/relations-external#>
  PREFIX fm: <info:fedora/fedora-system:def/model#>
  PREFIX dc: <http://purl.org/dc/elements/1.1/>
  PREFIX is: <http://islandora.ca/ontology/relsext#>
  SELECT ?mezzanine ?mezzanine_title
  WHERE {
  ?vro fre:$collection_relation <info:fedora/$object->id> ;
       fm:hasModel <info:fedora/$vro_cmodel> ;
       fm:state fm:Active .

  ?mezzanine fre:$derivative_relation ?vro ;
             fm:hasModel <info:fedora/$mezzanine_cmodel> ;
             fm:state fm:Active ;
             dc:title ?mezzanine_title .
    OPTIONAL {
      ?mezzanine is:isViewableByUser ?usr .
      ?mezzanine is:isViewableByRole ?role .
    }";
  $query .= usc_mirc_append_xacml_filter();
  $query .= "ORDER BY ?mezzanine_title";

  if (!$count) {
    $offset = $page * $number;
    $query .= "
    LIMIT $number
    OFFSET $offset";
  }
  return $query;
}

/**
 * Performs a SPARQL query to go out and get the mezzanine set.
 *
 * @param FedoraObject $object
 *   A FedoraObject.
 *
 * @param int $page
 *   The page to start on.
 *
 * @param int $number
 *   The number of items per page.
 *
 * @return array
 *   An array containing the results of the VRO query.
 */
function usc_mirc_get_mezzanine_set($object, $page = 0, $number = 20) {
  $query = usc_mirc_get_mezzanine_query($object, FALSE, $page, $number);
  $query_array = array(
    'query' => $query,
    'type' => 'sparql',
    'page_size' => $page,
    'page_number' => $number,
  );

  $results = $object->repository->ri->query($query_array['query'], $query_array['type']);
  return $results;
}

/**
 * Performs a SPARQL query to retrieve the number of children for mezzaniness.
 *
 * @param FedoraObject $object
 *   A FedoraObject.
 *
 * @return int
 *   The number of VRO children.
 */
function usc_mirc_get_mezzanine_children($object) {
  $query = usc_mirc_get_mezzanine_query($object, TRUE);
  $query_array = array(
    'query' => $query,
    'type' => 'sparql',
  );
  $results = $object->repository->ri->query($query_array['query'], $query_array['type']);
  return count($results);
}

/**
 * Performs a SPARQL query to retrieve the parent of a mezzanine object.
 *
 * @param FedoraObject $object
 *   A FedoraObject.
 *
 * @return mixed
 *   A bool if it has no parent, or the parent pid otherwise.
 */
function usc_mirc_get_mezzanine_parent($object) {
  global $user;
  $vro_cmodel = 'usc:vroCModel';
  $def_rel = variable_get('usc_mirc_relationship', 'isDerivativeOf');
  $query = "PREFIX fre: <info:fedora/fedora-system:def/relations-external#>
  PREFIX fm: <info:fedora/fedora-system:def/model#>
  PREFIX is: <http://islandora.ca/ontology/relsext#>
  SELECT ?object
  WHERE {
    ?subject fre:$def_rel ?object ;
             fm:state fm:Active .
    ?object fm:state fm:Active ;
            fm:hasModel <info:fedora/$vro_cmodel> .
    OPTIONAL {
      ?object is:isViewableByUser ?usr .
      ?object is:isViewableByRole ?role .
    }
    FILTER(sameTerm(?subject, <info:fedora/$object->id>)) . ";
  $query .= usc_mirc_append_xacml_filter();
  $query_array = array(
    'query' => $query,
    'type' => 'sparql',
  );
  $results = $object->repository->ri->query($query_array['query'], $query_array['type']);

  if (count($results) > 0) {
    return $results[0]['object']['value'];
  }
  else {
    return FALSE;
  }
}

/**
 * Adds the flowplayer markup to the current page.
 *
 * @global type $base_url
 *
 * @param array $videos
 *   An array of videos that are returned via query. We will only markup the
 *   first one.
 *
 * @return string
 *   Flowplayer markup.
 */
function usc_mirc_mezzanine_player_markup($videos) {
  global $base_url;
  $mod_path = drupal_get_path('module', 'usc_mirc');
  drupal_add_js("$mod_path/lib/flowplayer/flowplayer-3.2.8.min.js", array('group' => JS_LIBRARY));

  $player_class = 'usc-flowplayer';
  $flowplayer_js = <<<EOJS
  jQuery(function() {
    jQuery('.$player_class').flowplayer("$base_url/$mod_path/lib/flowplayer/flowplayer.commercial-3.2.8.swf", {

      //Product key (for sc.edu)
      key: "#\$60fe88f4c446bc34a1c",

      clip: {
        provider: 'lighttpd', // configure clip to use "lighthttpd" plugin for providing video data
        scaling: 'fit', //Maintain aspect ratio when scaling
      },
      canvas: { //Use a full-black background
        background: '#000000',
        backgroundGradient: 'none',
      },

      // streaming plugins are configured normally under the plugins node
      plugins: {
        lighttpd: {
          url: 'flowplayer.pseudostreaming-3.2.8.swf'
        }
      }
    })
  });
EOJS;
  drupal_add_js($flowplayer_js, array('type' => 'inline'));

  if (count($videos) > 0) {
    $output = array(
      '#markup' => l(theme('image', array(
        "path" => "$mod_path/images/playbuttonblack.jpg",
        'alt' => 'Video placeholder',
        'attributes' => array(),
        )), $videos[0],
        array(
          'html' => TRUE,
          'attributes' => array(
            'class' => array($player_class),
          ),
        )),
    );
  }
  else {
    $output = array(
      '#markup' => theme('image', array("path" => "$mod_path/images/NoVideoAvailable.jpg", 'attributes' => array())),
    );
  }
  return drupal_render($output);
}

/**
 * Performs a SPARQL query to retrieve derivatives of an object.
 *
 * @param FedoraObject $object
 *   A FedoraObject.
 *
 * @param string $subject
 *   The subject we are searching for in SPARQL.
 *
 * @return array
 *   The result set.
 */
function usc_mirc_get_derivatives($object, $subject = NULL) {
  global $user;
  if ($subject === NULL) {
    $def_rel = variable_get('usc_mirc_relationship', 'isDerivativeOf');
    $subject = "?subject fre:$def_rel <info:fedora/$object->id>";
  }
  // Perform query.
  $query = "PREFIX fre: <info:fedora/fedora-system:def/relations-external#>
  PREFIX fm: <info:fedora/fedora-system:def/model#>
  PREFIX is: <http://islandora.ca/ontology/relsext#>
  SELECT ?subject ?label
  WHERE {
  $subject ;
         fm:label ?label ;
         fm:state fm:Active .";
  $query .= "OPTIONAL {
      ?subject is:isViewableByUser ?usr .
      ?subject is:isViewableByRole ?role .
    }";
  $query .= usc_mirc_append_xacml_filter();
  $query_array = array(
    'query' => $query,
    'type' => 'sparql',
  );
  $results = $object->repository->ri->query($query_array['query'], $query_array['type']);
  return $results;
}

/**
 * Performs a SPARQL query to build a breadcrumb link back to the parent object.
 *
 * @param FedoraObject $object
 *   A FedoraObject.
 *
 * @return mixed
 *   Markup to the of the object or FALSE if no parents are visible to the user.
 */
function usc_mirc_build_breadcrumb($object) {
  $vro_cmodel = variable_get('usc_mirc_cmodel_vro', 'usc:vroCModel');
  $mezz_cmodel = variable_get('usc_mirc_cmodel_mezzanine', 'usc:mezzanineCModel');
  $pid = $object->id;
  $query = "PREFIX fre: <info:fedora/fedora-system:def/relations-external#>
  PREFIX fm: <info:fedora/fedora-system:def/model#>
  PREFIX is: <http://islandora.ca/ontology/relsext#>
  SELECT ?parent
  WHERE {
    {
      ?this fre:isDerivativeOf ?vro ;
            fm:hasModel <info:fedora/$mezz_cmodel> ;
            fm:state fm:Active .
      ?vro fre:isMemberOfCollection ?parent ;
           fm:hasModel <info:fedora/$vro_cmodel> ;
           fm:state fm:Active .
      OPTIONAL {
        ?parent is:isViewableByUser ?usr .
        ?parent is:isViewableByRole ?role .
      }
    }
    UNION
    {
      ?this fre:isMemberOfCollection ?parent ;
            fm:hasModel ?model ;
            fm:state fm:Active .
      FILTER(!sameterm(?model, <info:fedora/$vro_cmodel>)) .
      OPTIONAL {
        ?parent is:isViewableByUser ?usr .
        ?parent is:isViewableByRole ?role .
      }
    }
    FILTER(sameterm(?this, <info:fedora/$pid>))";
  $query .= usc_mirc_append_xacml_filter();

  $query_array = array(
    'query' => $query,
    'type' => 'sparql',
  );
  $results = $object->repository->ri->query($query_array['query'], $query_array['type']);
  if ($results) {
    $parent_object = islandora_object_load($results[0]['parent']['value']);
    $title = $parent_object->label;
    return l($title, "islandora/object/$parent_object->id");
  }
  return FALSE;
}

/**
 * Retrieves a list of mezzanine formats from the taxonomy.
 *
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   An array containing the options retrieved from the taxonomy.
 */
function usc_mirc_get_mezzanine_formats(&$form_state) {
  $vocabs = taxonomy_get_vocabularies();
  foreach ($vocabs as $id => $voc) {
    if ($voc->machine_name == variable_get('usc_mirc_mezzanine_format_vocab', 'usc_mirc_mezzanine_format_vocab')) {
      $format_options = array();
      $format_tree = taxonomy_get_tree($voc->vid);
      foreach ($format_tree as $term) {
        $format_options[$term->name] = $term->name;
      }
      return array(
        '#type' => 'select',
        '#title' => t('Derivative format'),
        '#description' => t('Whether this mezzanine is edge-to-edge, cropped, or so on. (This list can be edited/expanded on via <a href=@link>derivative formats</a>)', array('@link' => url("admin/structure/taxonomy/$voc->machine_name/add"))),
        '#options' => $format_options,
      );
    }
  }
}

/**
 * Retrieves the types of mezzanines from the taxonomy.
 *
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   An array containing the options retrieved from the taxonomy.
 */
function usc_mirc_get_mezzanine_types(&$form_state) {
  $vocabs = taxonomy_get_vocabularies();
  foreach ($vocabs as $id => $voc) {
    if ($voc->machine_name == variable_get('usc_mirc_mezzanine_type_vocab', 'usc_mirc_mezzanine_type_vocab')) {
      $type_options = array();
      $type_tree = taxonomy_get_tree($voc->vid);
      foreach ($type_tree as $term) {
        $type_options[$term->name] = $term->name;
      }
      return array(
        '#type' => 'select',
        '#title' => t('Derivative type'),
        '#description' => t('Whether this mezzanine is edited, or so on. (This list can be edited/expanded on via <a href=@link>derivative types</a>)', array('@link' => url("admin/structure/taxonomy/$voc->machine_name/add"))),
        '#options' => $type_options,
      );
    }
  }
}

/**
 * Constructs and returns the XACML filter for the current user.
 *
 * @return string
 *   The XACML filter.
 */
function usc_mirc_append_xacml_filter() {
  global $user;
  $filter_string = '';
  $role_count = 0;
  foreach ($user->roles as $role) {
    if ($role_count === 0) {
      $filter_string .= "FILTER(!bound(?role) || (bound(?role) && (sameTerm(?role, '$role')";
    }
    else {
      $filter_string .= " || sameTerm(?role, '$role')";
    }
    $role_count++;
  }
  $filter_string .= "))";
  if ($user->uid != 0) {
    $user_name = $user->name;
  }
  else {
    $user_name = 'anonymous';
  }
  $filter_string .= " || (!bound(?usr) || (bound(?usr) && sameTerm(?usr, '$user_name'))))\n}";
  return $filter_string;
}

/**
 * Generates PBCore and DC for an object and applies them to the same object.
 *
 * @param string $path
 *   Path to the video file on the USC's file system.
 * @param AbstractObject $object
 *   A FedoraObject that is being created or modified.
 * @param array $params
 *   An associate array of parameters.
 */
function usc_mirc_generate_video_metadata($path, AbstractObject $object, array $params) {
  if (!(is_file($path) && is_readable($path))) {
    throw new Exception("The file at $path does not seem to exist or is not readable!");
  }
  else {
    $old_level = error_reporting(-1);

    try {
      // Grab media info output for the given file...
      $command = 'mediainfo --Full --Output=XML ' . escapeshellarg($path);
      $descriptor = array(
        1 => array('pipe', 'w'),
      );
      $pipes = array();
      $command = escapeshellcmd($command);
      $media_proc = proc_open($command, $descriptor, $pipes);
      $mediainfo_output = '';

      if (!is_resource($media_proc)) {
        throw new Exception(t('Error extracting metadata with Mediainfo for @object_id.', array(
          '@object_id' => $object->id,
        )));
      }
      else {
        $mediainfo_output = stream_get_contents($pipes[1]);
        fclose($pipes[1]);
        $return_value = proc_close($media_proc);
      }

      $mediainfo_doc = new DOMDocument();
      $mediainfo_doc->loadXML($mediainfo_output);

      if (!$mediainfo_doc) {
        throw new Exception(t('Mediainfo output failed to parse with DOMDocument for @object_id!', array(
          '@object_id' => $object->id,
        )));
      }

      // Transform the mediainfo file to pbcore.
      $pbcore_transform = new DOMDocument();
      $pbcore_transform->load(drupal_get_path('module', 'usc_mirc') . '/transforms/mediainfo_to_pbcoreInstantiation.xsl');

      if ($pbcore_transform === FALSE) {
        throw new Exception(t('Failed to load transform for @object_id!', array(
          '@object_id' => $object->id,
        )));
      }
      $transformer = new XSLTProcessor();
      $transformer->importStylesheet($pbcore_transform);

      $intermediate_pbcore = $transformer->transformToDoc($mediainfo_doc);
      if (!$intermediate_pbcore) {
        throw new Exception(t('Failed transforming Mediainfo XML to PBCore for @object_id!', array(
          '@object_id' => $object->id,
        )));
      }
      // No longer need this (have the transformed version), free it up.
      unset($mediainfo_doc);

      $pbcore_xpath = new DOMXPath($intermediate_pbcore);
      $pbcore_doc = $pbcore_xpath->document;
      $pbcore_xpath->registerNamespace('pb', 'http://www.pbcore.org/PBCore/PBCoreNamespace.html');

      $title = $params['label'];
      $title_el = $pbcore_doc->createElement('instantiationAnnotation', $title);
      $title_el->setAttribute('annotationType', 'instantiation_title');

      foreach ($pbcore_xpath->query('//pb:instantiationEssenceTrack[pb:essenceTrackType/text()="Video"]/pb:essenceTrackFrameSize') as $format_el) {
        $format_el->setAttribute('annotation', $params['derivative_format']);
      }

      $type_el = $pbcore_doc->createElement('instantiationGenerations', $params['derivative_type']);

      $doc_el = $pbcore_doc->documentElement;
      $doc_el->appendChild($title_el);
      $doc_el->appendChild($type_el);

      $pbcore_doc->normalizeDocument();
      $pbcore = $pbcore_doc->saveXML();

      // Create DC transformation of PBCore.
      $dc_transform = new DOMDocument();
      $dc_transform->load(drupal_get_path('module', 'usc_mirc') . '/transforms/pbcore_to_dc.xsl');
      if (!$dc_transform) {
        throw new Exception(t('Failed loading PBCore to DC transform for @object_id.', array(
          '@object_id' => $object->id,
        )));
      }
      $transformer = new XSLTProcessor();
      $transformer->importStyleSheet($dc_transform);

      $pbcore_doc = new DOMDocument();
      $pbcore_doc->loadXML($pbcore);
      // XXX:  This seems to be necessary for some reason...  It seems to fail
      // to get the newly added elements for the transform otherwise.
      // DOMDocument, y u no work!?
      $dc = $transformer->transformToXML($pbcore_doc);
      if (!$dc) {
        throw new Exception(t('Failed transforming PBCore XML to DC for @object_id.', array(
          '@object_id' => $object->id,
        )));
      }
      // No longer need this (have it in a string), free it up.
      unset($pbcore_doc);
    }
    catch (Exception $e) {
      // Log the error.
      watchdog_exception('usc_mirc', $e);
    }
    error_reporting($old_level);
  }

  // If we've properly created the metadata, create the Fedora Object.
  if ($pbcore && $dc) {
    if (!isset($object['DC'])) {
      $dc_ds = $object->constructDatastream('DC', 'X');
      $dc_ds->label = 'DC Record';
      $dc_ds->mimetype = 'text/xml';
      $dc_ds->setContentFromString($dc);
      $object->ingestDatastream($dc_ds);
    }
    else {
      $object['DC']->content = $dc;
    }

    $pbcore_ds = $object->constructDatastream('PBCORE', 'X');
    $pbcore_ds->label = 'PBCORE Record';
    $pbcore_ds->mimetype = 'text/xml';
    $pbcore_ds->setContentFromString($pbcore);
    $object->ingestDatastream($pbcore_ds);
  }
  else {
    watchdog('usc_mirc', 'PBCore or DC metadata not present for @object_id!', array(
      '@object_id' => $object->id,
    ), WATCHDOG_ERROR);
  }
}

/**
 * XPATHs PBCore to construct an array of parameters used elsewhere.
 *
 * @see usc_mirc_generate_video_metadata()
 *
 * @param AbstractObject $object
 *   A Fedora object.
 *
 * @return mixed
 *   An array of parameters or FALSE if an XPATH fails to retrieve metadata.
 */
function usc_mirc_retrieve_pbcore_fields($object) {
  $params = array(
    'label' => $object->label,
  );

  if (!isset($object['PBCORE'])) {
    watchdog('usc_mirc', 'The object "@object_id" does not have any PBCore!', array(
      '@object_id' => $object->id,
    ), WATCHDOG_ERROR);
    return FALSE;
  }

  $pbcore_xml = simplexml_load_string($object['PBCORE']->content);
  $pbcore_xml->registerXPathNamespace('pb', 'http://www.pbcore.org/PBCore/PBCoreNamespace.html');
  $type_result = $pbcore_xml->xpath('//pb:instantiationGenerations/text()');

  if (!empty($type_result)) {
    $params['derivative_type'] = (string) reset($type_result);
  }

  $format_result = $pbcore_xml->xpath('//pb:instantiationEssenceTrack[pb:essenceTrackType/text()="Video"]/pb:essenceTrackFrameSize/@annotation');
  if (!empty($format_result)) {
    $params['derivative_format'] = (string) reset($format_result);
  }

  if (isset($params['derivative_format']) && isset($params['derivative_type'])) {
    return $params;
  }
  else {
    return FALSE;
  }
}

/**
 * Utility funciton to convert a seconds to a mulgara date string.
 *
 * @param int $time
 *   Seconds since the last epoch
 *
 * @return string
 *   Date string in a format mulgara finds acceptable.
 */
function usc_mirc_format_time($time) {
  return date('Y-m-d\TH:i:s\Z', $time);
}

/**
 * Utility function to export pbcore for a preservation master.
 *
 * @param string $pid
 *   Pid for the preservation master.
 *
 * @param string $output_path
 *   Output directory where exported pbcore xml will reside.
 */
function usc_mirc_export_pbcore($pid, $output_path) {
  $obj = islandora_object_load($pid);
  $pbcore = $obj['PBCORE'];
  $pbcore->getContent($output_path);
}

/**
 * Utility function to write the PBCORE export manifest header to a log file.
 *
 * @param resource $log_fp
 *   File pointer resource for the log file.
 *
 * @param AbstractObject $collection
 *   Collection in Fedora whose children we're exporting.  Defaults to NULL
 *   (for the actual cron job), but is used for descriptive logging when a
 *   user initiates the export.
 */
function usc_mirc_log_pbcore_export_header($log_fp, $collection = NULL) {
  fwrite($log_fp, "!!!\n");
  $header_line = "Beginning PBCORE export";
  if ($collection) {
    $header_line .= " for " . $collection->label . "\n";
  }
  else {
    $header_line .= "\n";
  }
  fwrite($log_fp, $header_line);
  fwrite($log_fp, "Time: " . usc_mirc_format_time(time()) . "\n");
  fwrite($log_fp, "!!!\n");
  fwrite($log_fp, "\n");
  fwrite($log_fp, "Export manifest:\n");
}

/**
 * Utility function to write a PBCORE export manifest entry to a log file.
 *
 * @param resource $log_fp
 *   File pointer resource for the log file.
 *
 * @param string $output_path
 *   The output path of the exported PBCORE datastream.
 */
function usc_mirc_log_pbcore_export_record($log_fp, $output_path) {
  fwrite($log_fp, "\t" . $output_path . "\n");
}

/**
 * Utility function to write a manifest entry when there are no records.
 *
 * @param resource $log_fp
 *   File pointer resource for the log file.
 */
function usc_mirc_log_pbcore_export_no_records($log_fp) {
  fwrite($log_fp, "\tNo records to export!\n");
}

/**
 * Utility function to write a PBCORE export manifest footer to a log file.
 *
 * @param resource $log_fp
 *   File pointer resource for the log file.
 */
function usc_mirc_log_pbcore_export_footer($log_fp) {
  fwrite($log_fp, "Export successfully finished at " . usc_mirc_format_time(time()) . "\n");
  fwrite($log_fp, "\n");
}

/**
 * Retrieves the image url with token attached for the specified datastream.
 *
 * @param AbstractObject $object
 *   An AbstractObject representing and object within Fedora.
 * @param string $dsid
 *   The datastream we are retrieving.
 *
 * @return string
 *   The URL to the image with token appended.
 */
function usc_mirc_get_imagecache_token_url($object, $dsid) {
  module_load_include('inc', 'islandora', 'includes/authtokens');
  $token = islandora_get_object_token($object->id, $dsid, '2');
  $image = url("islandora/object/$object->id/datastream/$dsid/view", array(
    'absolute' => TRUE,
    'query' => array('token' => $token),
  ));
  return $image;
}

/**
 * Goes out and XPATHs the preservation master for use in CSV export.
 *
 * @param string $pid
 *   The pid of the preservation master we are going to XPATH through.
 */
function usc_mirc_extract_metadata_from_preservation_master($pid) {
  $object = islandora_object_load($pid);
  $pres_metadata = array();
  if (!isset($object['PBCORE'])) {
    return array();
  }
  else {
    $pbcore = simplexml_load_string($object['PBCORE']->content);
    $pbcore->registerXPathNamespace('pb', 'http://www.pbcore.org/PBCore/PBCoreNamespace.html');

    // Title will only be used if the pid being exported as actually a
    // preservation master.
    $title_xpath = $pbcore->xpath('/pb:pbcoreDescriptionDocument/pb:pbcoreTitle[@annotation="Title supplied by cataloger."]/text()');
    $title = (string) reset($title_xpath);
    $pres_metadata['title'] = $title;

    // Item title.
    $item_title_xpath = $pbcore->xpath('/pb:pbcoreDescriptionDocument/pb:pbcoreTitle[@titleType="Item"]');
    $item_title = (string) reset($item_title_xpath);
    $pres_metadata['item_title'] = $item_title;

    // Temporal coverage.
    $temporal_coverage_xpath = $pbcore->xpath('/pb:pbcoreDescriptionDocument/pb:pbcoreCoverage[pb:coverageType="Temporal"]/pb:coverage');
    $temporal_coverage = (string) reset($temporal_coverage_xpath);
    $pres_metadata['temporal_coverage'] = $temporal_coverage;

    // Description.
    $description_xpath = $pbcore->xpath('/pb:pbcoreDescriptionDocument/pb:pbcoreDescription[@descriptionType="summary"]');
    $description = (string) reset($description_xpath);
    $pres_metadata['description'] = $description;

    // Silent or sound.
    $sound_xpath = $pbcore->xpath('/pb:pbcoreDescriptionDocument/pb:pbcoreDescription[@descriptionType="sound content"]/text()');
    $sound = (string) reset($sound_xpath);
    $pres_metadata['sound'] = $sound;

    // Production date(s).
    $start_xpath = $pbcore->xpath('/pb:pbcoreDescriptionDocument/pb:pbcoreAssetDate[@dateType="createdStart"]');
    $start_date = (string) reset($start_xpath);
    $end_xpath = $pbcore->xpath('/pb:pbcoreDescriptionDocument/pb:pbcoreAssetDate[@dateType="createdEnd" and not(text()=/pb:pbcoreDescriptionDocument/pb:pbcoreAssetDate[@dateType="createdStart"]/text())]');
    $end_date = (string) reset($end_xpath);
    if ($end_date) {
      $date_string = $start_date . ' to ' . $end_date;
    }
    else {
      $date_string = $start_date;
    }
    $pres_metadata['production_date'] = $date_string;

    // Donor.
    $donor_xpath = $pbcore->xpath('/pb:pbcoreDescriptionDocument/pb:pbcoreRightsSummary/pb:rightsSummary[@source="donor_info"]');
    $donor = (string) reset($donor_xpath);
    $pres_metadata['donor'] = $donor;

    // Preservation statement.
    $preservation_statement_xpath = $pbcore->xpath('/pb:pbcoreDescriptionDocument/pb:pbcoreRightsSummary/pb:rightsSummary[@source="preservedBy"]');
    $preservation_statement = (string) reset($preservation_statement_xpath);
    $pres_metadata['preservation_statement'] = $preservation_statement;

    // Copyright statement.
    $copyright_statement_xpath = $pbcore->xpath('/pb:pbcoreDescriptionDocument/pb:pbcoreRightsSummary/pb:rightsSummary[@source="copyright_statement"]');
    $copyright_statement = (string) reset($copyright_statement_xpath);
    $pres_metadata['copyright_statement'] = $copyright_statement;

    return $pres_metadata;
  }
}

/**
 * Goes out and XPATHs an access variant or mezzanine for its title.
 *
 * @param string $pid
 *   The pid of the object we are going to XPATH through.
 */
function usc_mirc_extract_title_from_pbcore($pid) {
  $title = '';
  $object = islandora_object_load($pid);
  if (isset($object['PBCORE'])) {
    $pbcore = simplexml_load_string($object['PBCORE']->content);
    $pbcore->registerXPathNamespace('pb', 'http://www.pbcore.org/PBCore/PBCoreNamespace.html');
    $pbcore_xpath = $pbcore->xpath('/pb:pbcoreInstantiationDocument/pb:instantiationAnnotation[@annotationType="instantiation_title"]/text()');
    $pbcore_title = (string) reset($pbcore_xpath);
    $title = $pbcore_title;
  }
  return $title;
}
