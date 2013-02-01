<?php

//------------------------- Generate admin settings page ------------------------

// Checks if it has to display the admin settings or if it has to initialize the plugin's data, etc.
function idsimport_admin_main() {
  if (!current_user_can( 'manage_options')) {
    wp_die(__('You do not have sufficient permissions to manage the plugin settings.'));
  }
  $idsimport_setup_done = idsapi_variable_get('idsimport', 'setup_done', FALSE);
  if ($idsimport_setup_done) {
    $importer_url = 'import.php?import=idsimport_importer';
    $message = __("You can change the plugin configuration options below or proceed to the <input type=\"button\" class=\"button-primary\" value=\"manual import page\" onclick=\"window.location='$importer_url';\">");
    ids_show_message($message);
    idsimport_after_config();
  }
  idsimport_render_form();
}

// Display registered error messages
function idsimport_admin_notices() {
  settings_errors('idsimport_options', FALSE, TRUE);
  idsapi_report_errors();
}

// Run plugin initialization tasks after configuration.
function idsimport_after_config() {
  // Schedule periodic imports
  idsimport_schedule_imports();
  // Create/update author of imported posts.
  $import_user = idsapi_variable_get('idsimport', 'import_user', IDS_IMPORT_IMPORT_USER);
  $user_id = idsapi_variable_get('idsimport', 'import_user_id', 0);
  $ids_user = get_user_by('email', IDS_IMPORT_USER_EMAIL);
  if ((!$ids_user) && (!empty($import_user))) {
    $user_id = idsimport_new_user($import_user);
    idsapi_variable_set('idsimport', 'import_user_id', $user_id);
  }
  if ($user_info = get_userdata($user_id)) {
    idsapi_variable_set('idsimport', 'import_user', $user_info->user_login);
  }
  idsapi_report_errors();
}

// Create new user (author of imported content)
function idsimport_new_user($username) {
  $password = wp_generate_password( $length=12, $include_standard_special_chars=false );
  $user_data = array (
    'user_login' => $username,
    'user_pass' => $password,
    'user_email' => IDS_IMPORT_USER_EMAIL,
    'role' => 'editor',
    'description' => 'Default author for the IDS imported content.',
    'user_url' => IDS_API_URL
  );
  $user_id = wp_insert_user( $user_data );
  if (is_wp_error($user_id)) {
    idsapi_variable_set('idsimport', 'import_user', IDS_IMPORT_IMPORT_USER);
    idsapi_register_error('idsimport', __('The user could not be created. Error message: ') . $user_id->get_error_message(), 'idsimport_new_user', 'warning');
    $user_id = 0;
  }
  return $user_id;
}

function idsimport_get_datasets() {
  global $ids_datasets;
  $default_dataset = idsapi_variable_get('idsimport', 'default_dataset', IDS_IMPORT_DEFAULT_DATASET_ADMIN);
  if ($default_dataset == 'both') {
    $datasets = $ids_datasets;
  }
  else {
    $datasets = array($default_dataset);
  }
  return $datasets;
}

function idsimport_display_datasets($area) {
  global $ids_datasets;
  $display_dataset = array();
  $datasets = $ids_datasets;
  $display_dataset['admin'] = idsapi_variable_get('idsimport', 'display_dataset_admin', IDS_IMPORT_DEFAULT_DATASET_ADMIN);
  $display_dataset['public'] = idsapi_variable_get('idsimport', 'display_dataset_public', IDS_IMPORT_DEFAULT_DATASET_PUBLIC);
  if (isset($display_dataset[$area]) && ($display_dataset[$area] != 'both')) {
    $datasets = array($display_dataset[$area]);
  }
  return $datasets;
}

function idsimport_check_permalinks_changed() {
  global $wp_rewrite;
  $idsimport_permalinks = idsapi_variable_get('idsimport', 'permalink_structure', '');
  $current_permalinks = get_option('permalink_structure');
  if ($idsimport_permalinks !== $current_permalinks) {
    idsapi_variable_set('idsimport', 'permalink_structure', $current_permalinks);
    $wp_rewrite->flush_rules();
  }
}

//------------------------------ Options validation -----------------------------

// Validate input and the API key.
function idsimport_validate_options($input) {
  $idsapi = new IdsApiWrapper;
  $error = FALSE;
  $error_message = '';
  $idsimport_api_key_validated = idsapi_variable_get('idsimport', 'api_key_validated', FALSE);
  $idsimport_default_dataset = idsapi_variable_get('idsimport', 'default_dataset', IDS_IMPORT_DEFAULT_DATASET_ADMIN);
  $reg_api_key = '/^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$/';
  $reg_api_url = '/^http:\/\/[\/0-9a-z\.]+$/';
  $allowed_html = array();

  $current_permalinks = get_option('permalink_structure');
  $input['permalink_structure'] = idsapi_variable_get('idsimport', 'permalink_structure', $current_permalinks);

  if ((isset($input['api_key'])) && preg_match($reg_api_key, $input['api_key'])) {
    if ($idsapi->validateKey($input['api_key'])) {
        $input['api_key_validated'] = TRUE;
    }
    else {
      $input['api_key'] = '';
      $input['api_key_validated'] = FALSE;
      $error = TRUE;
      $error_message = __(' The IDS API call did not retrieve results. Please check that you have entered a valid API key.');
    }
  }
  else {
    $input['api_key'] = '';
    $input['api_key_validated'] = FALSE;
    $error = TRUE;
    $error_message = __(' The IDS API key format is not valid. Please enter a valid API key.');
  }
  if ($idsimport_api_key_validated) {
    $input['include_imported_documents'] = (isset($input['include_imported_documents']) && $input['include_imported_documents']);
    $input['include_imported_organisations'] = (isset($input['include_imported_organisations']) && $input['include_imported_organisations']);
    $input['import_new_categories'] = (isset($input['import_new_categories']) && $input['import_new_categories']);
    $input['import_map_categories'] = (isset($input['import_map_categories']) && $input['import_map_categories']);
    $input['import_tags'] = (isset($input['import_tags']) && $input['import_tags']);

    //If the API key had not been validated, the other fields had not been displayed, so they keep the default values.
    if (isset($input['num_items'])) {
      if (!is_numeric($input['num_items'])) {
        $error = TRUE;
        $error_message .= ' ' . $input['num_items'] . __(' is not a valid number of items to import. Please enter a numeric value.');
        $input['num_items'] = IDS_API_NUM_ITEMS;
      }
    }
    if (isset($input['import_authors_assets'])) {
      $input['import_authors_assets'] = wp_kses($input['import_authors_assets'], $allowed_html);
    }
    if (isset($input['import_publishers_assets'])) {
      $input['import_publishers_assets'] = wp_kses($input['import_publishers_assets'], $allowed_html);
    }
    if (isset($input['import_query'])) {
      $input['import_query'] = wp_kses($input['import_query'], $allowed_html);
    }
    if (isset($input['import_user'])) {
      $input['import_user'] =  sanitize_user($input['import_user'], TRUE);
    }
  }
  else {
    $input['include_imported_documents'] = idsapi_variable_get('idsimport', 'include_imported_documents', IDS_IMPORT_INCLUDE_IMPORTED_DOCUMENTS);
    $input['include_imported_organisations'] = idsapi_variable_get('idsimport', 'include_imported_organisations', IDS_IMPORT_INCLUDE_IMPORTED_ORGANISATIONS);
    $input['import_new_categories'] = idsapi_variable_get('idsimport', 'import_new_categories', IDS_IMPORT_NEW_CATEGORIES);
    $input['import_map_categories'] = idsapi_variable_get('idsimport', 'import_map_categories', IDS_IMPORT_MAP_CATEGORIES);
    $input['import_tags'] = idsapi_variable_get('idsimport', 'import_tags', IDS_IMPORT_TAGS);
  }
  if ($error) {
    $input['setup_done'] = FALSE;
    add_settings_error('idsimport_options', 'idsimport_errors', $error_message);
  }
  else {
    $input['setup_done'] = TRUE;
  }

  return $input;
}



