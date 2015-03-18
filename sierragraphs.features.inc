<?php
/**
 * @file
 * sierragraphs.features.inc
 */

/**
 * Implements hook_ctools_plugin_api().
 */
function sierragraphs_ctools_plugin_api($module = NULL, $api = NULL) {
  if ($module == "strongarm" && $api == "strongarm") {
    return array("version" => "1");
  }
}

/**
 * Implements hook_paragraphs_info().
 */
function sierragraphs_paragraphs_info() {
  $items = array(
    'sierragraphs' => array(
      'name' => 'Sierragraphs',
      'bundle' => 'sierragraphs',
      'locked' => '1',
    ),
  );
  return $items;
}
