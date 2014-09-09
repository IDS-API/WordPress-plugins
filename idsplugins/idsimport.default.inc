<?php
// $Id;

/**
 * @file
 *   IDS Wordpress Import Plugin default parameters.
 */

// Environment in which we are using the API wrapper. Possible values: 'generic', 'wordpress', 'drupal'. Default is 'generic'.
if (!defined('IDS_API_LIBRARY_PATH')) define('IDS_API_LIBRARY_PATH', 'idsapi/');
if (!defined('IDS_API_ENVIRONMENT')) define('IDS_API_ENVIRONMENT', 'wordpress');

define('IDS_IMPORT_DEFAULT_DATASET_PUBLIC', 'all');
define('IDS_IMPORT_DEFAULT_DATASET_ADMIN', 'all');
define('IDS_IMPORT_IMPORT_USER', 'IDS');
define('IDS_IMPORT_CATEGORY_LEVELS', 2);
define('IDS_IMPORT_IMPORT_RECURRENCE_ASSETS', 'daily');
define('IDS_IMPORT_IMPORT_RECURRENCE_CATEGORIES', 'monthly');
define('IDS_IMPORT_USER_EMAIL','api@ids.ac.uk');
define('IDS_IMPORT_MENU_TITLE', 'IDS Import');
// This might be defined in IDS Expose, too.
if (!defined('IDS_IMPORT_TAXONOMY')) define('IDS_IMPORT_TAXONOMY', 'idsimport_taxonomy');

define('IDS_IMPORT_DEFAULT_LANGUAGE', 'en');
define('IDS_IMPORT_UPDATED_ASSETS', 'keep');

define('IDS_IMPORT_NEW_CATEGORIES', TRUE);
define('IDS_IMPORT_MAP_CATEGORIES', FALSE);
define('IDS_IMPORT_VERBOSE_IMPORT', TRUE);
define('IDS_IMPORT_INCLUDE_IMPORTED_DOCUMENTS', TRUE);
define('IDS_IMPORT_INCLUDE_IMPORTED_ORGANISATIONS', TRUE);
define('IDS_IMPORT_TAGS', TRUE);



