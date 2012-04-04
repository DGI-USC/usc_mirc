<?php

/**
 * array_filter callback to find filenames which (should) represent a video file.
 */
function _usc_mirc_is_video_filename($filename) {
  static $exts = NULL;
  if (empty($exts)) {
    explode(
      ' ', 
      variable_get(
        'usc_mirc_streaming_extensions',
        '.m4v .mp4 .flv .mov'
      )
    );
  }
  
  return in_array(substr($filename, -4), $exts);
}

/**
 *  Get an array of all the files in this directory (including subdirectories 
 *    if $recurse is TRUE).
 */
function _usc_mirc_traverse_directory($url_base, $filesystem_base, $sub_dir = '.', $recurse = TRUE) {
  $videos = array();
  
  $dir = dir("$filesystem_base/$sub_dir");
  while ($entry = $dir->read()) {
    $this_entry = "$sub_dir/$entry";
    if ($recurse && is_dir("$filesystem_base/$this_entry")) {
      $videos = array_merge($videos, _usc_mirc_traverse_directory($url_base, $filesystem_base, $this_entry, $recurse));
    }
    elseif ($string === '*' || strpos($entry, $string) === 0) { //XXX:  Should probably get rid of the wildcard.
      $videos["$url_base/$this_entry"] = $entry;
    }
  }
  $dir->close();
  
  return $videos;
}

/**
 * autocomplete callback used to obtain streaming URLs.
 */
function usc_mirc_autocomplete_video_url($url_base, $filesystem_base, $string) {
  $videos = _usc_mirc_traverse_directory(
    $url_base,  //variable_get('usc_mirc_streaming_url', 'http://dvr-streaming.mirc.sc.edu'),
    $filesystem_base, //variable_get('usc_mirc_streaming_folder', '/mnt/mirc/mezz/access')
  );
  $videos = array_filter($videos, '_usc_mirc_is_video_filename');
  
  ksort($videos);
  drupal_json($videos);

  return;
}
