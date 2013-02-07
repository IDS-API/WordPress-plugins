<?php

if (!defined('IDS_API_LIBRARY_PATH')) define('IDS_API_LIBRARY_PATH', 'idsapi/');
if (!defined('IDS_API_ENVIRONMENT')) define('IDS_API_ENVIRONMENT', 'generic');

require_once(IDS_API_LIBRARY_PATH . 'idsapi.wrapper.inc');

$idsapi = new IdsApiWrapper;

if (isset($_GET['token_guid'])) {
  $api_key = $_GET['token_guid'];
  $type = (isset($_GET['type']) ?  $_GET['type'] : '');
  $dataset = (isset($_GET['site']) ? $_GET['site'] : '');
  $callback = (isset($_GET['callback']) ? $_GET['callback'] : '');
  $id = (isset($_GET['id']) ? $_GET['id'] : '');
  $url = (isset($_SERVER['SCRIPT_URI']) ? $_SERVER['SCRIPT_URI'] . '?type=' . $type . '&site=' . $dataset . '&token_guid=' . $api_key : '');

  if ($id) {
    // We retrieve the children of the indicated theme.
    $response = $idsapi->search($type, $dataset, $api_key, 'short', 0, 0, array('archived' => 'false', 'parent_object_id' => $id));
  }
  else {
    // We retrieve all level 1 themes.
    $response = $idsapi->search($type, $dataset, $api_key, 'short', 0, 0, array('archived' => 'false', 'level' => 1));
  }
  
  $results = array();
  if (!$response->isError()) {
    foreach ($response->results as $item) {
      $results[] = "{ 'label': '$item->title', 'value': '$item->object_id', 'items': [{ 'value': '$url&id=$item->object_id', 'label': 'Loading...' }] }";
    }
  }
  $output = implode(",\n", $results);
  $output = $callback . "(\n" . "[\n" . $output . "\n])";

  header('Content-Type: application/json');
  echo $output;
}  
  
