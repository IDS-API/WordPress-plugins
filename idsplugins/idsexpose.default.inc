<?php
// $Id;

/**
 * @file
 *   IDS Expose Plugin default parameters.
 *
 */

if (!defined('IDS_API_LIBRARY_PATH')) define('IDS_API_LIBRARY_PATH', 'idsapi/');
if (!defined('IDS_API_ENVIRONMENT')) define('IDS_API_ENVIRONMENT', 'wordpress');

// In order to use the idsimport_taxonomy metadata table even if the IDS Import plugin has been de-activated.
if (!defined('IDS_IMPORT_TAXONOMY')) define('IDS_IMPORT_TAXONOMY', 'idsimport_taxonomy');