<?php

// Load Importer API
require_once ABSPATH . 'wp-admin/includes/import.php';

if ( !class_exists( 'WP_Importer' ) ) {
	$class_wp_importer = ABSPATH . 'wp-admin/includes/class-wp-importer.php';
	if ( file_exists( $class_wp_importer ) ) {
		require_once $class_wp_importer;
  }
}

//------------------------ IDS_Importer class definition -------------------------

if ( class_exists( 'WP_Importer' ) ) {
class IDS_Importer extends WP_Importer {

// Posts and taxonomy terms created/updated
  var $terms = array('eldis' => array(), 'bridge' => array());

//--------------------------- User interface methods -----------------------------

  // Determine what to display to the user, depending on the step.
	function dispatch() {
		if (empty ($_GET['step'])) {
			$step = 0;
    }
		else  {
			$step = (int) $_GET['step'];
    }
		$this->header();
		switch ($step) {
			case 0:
				$this->greet();
				break;
			case 1:
        check_admin_referer('idsimport_import', 'idsimport_import_check_referrer');
        $importer_url = admin_url() . 'admin.php?import=idsimport_importer';
        printf(__('<br/>Go back to the <a href="%s">IDS Importer page</a>.'), $importer_url);
        $this->import();
        $this->display_results();
        idsapi_report_errors();
				break;
		}
		$this->footer();
	}

  // Display header.
	function header() {
		echo '<div class="wrap">';
		screen_icon();
		echo '<h2>'.__('IDS Importer', 'idsimport-importer').'</h2>';
	}

  // Display footer.
	function footer() {
		echo '</div>';
	}

  // Display main import page. Selection of types of items to import.
	function greet() {
		$idsimport_setup_done = idsapi_variable_get('idsimport', 'setup_done', FALSE);
		?>
		<div class="narrow">
		<?php 
		if (!$idsimport_setup_done) {
      echo '<p>'.sprintf(__('Before starting to import IDS content, you need you need to set a number of parameters, including the IDS API key, at the <a href="%s">IDS API administrative page</a>.', 'idsimport_import'), 'options-general.php?page=idsimport').'</p>'; 
		}
    else {
      idsimport_importer_form();
    }
  }

  function display_results() {
    $total = 0;
    foreach ($this->terms as $dataset => $results_dataset) {
      $exclude = FALSE;
      foreach ($results_dataset as $type => $results) {
        $exclude = idsapi_exclude($dataset, $type);
        if (!$exclude) {
          if ($results['new']) {
            $total += $results['new'];
            $output = __('Created ') . $results['new'] . ' ' . ucfirst($dataset) . ' ' . __($type);
            echo ids_div_field_wrapper('ids-results-import', '', '', $output);
          }
          if ($results['updated']) {
            $total += $results['updated'];
            $output = __('Updated ') . $results['updated'] . ' ' . ucfirst($dataset) . ' ' . __($type);
            echo ids_div_field_wrapper('ids-results-import', '', '', $output);
          }
        }
        if (!$total && !$exclude) {
          $output = __('Nothing was imported.');
          echo ids_div_field_wrapper('ids-results-import', '', '', $output);
        }
      }
    }
  }
//-------------------------------- Import methods ------------------------------

  // Check which types of content to import and call to import methods. If selected, categories are imported first.
  // Keeps track of latest dates and results for each type, in order to display this information on the admin page (periodic imports do the same).
  function import() {
    global $ids_categories;
    global $ids_assets;
    global $ids_exclude;
    $date = date(DATE_RFC822);
    $idsimport_new_categories = idsapi_variable_get('idsimport', 'import_new_categories', IDS_IMPORT_NEW_CATEGORIES);
    $idsimport_import_dates = idsapi_variable_get('idsimport', 'import_dates', array('eldis' => array(), 'bridge' => array()));
    $idsimport_import_results = idsapi_variable_get('idsimport', 'import_results', array('eldis' => array(), 'bridge' => array()));
    $datasets = idsimport_get_datasets();
    foreach ($datasets as $dataset) {
      // Import selected categories (if the option is enabled).
      if ($idsimport_new_categories) {
        foreach ($ids_categories as $category) {
          $parameter_name = 'idsimport_import_' . $category;
          if (isset($_POST[$parameter_name]) && ($_POST[$parameter_name])) {
            $this->import_categories($dataset, $category);
            $idsimport_import_dates[$dataset][$category] = $date;
            $idsimport_import_results[$dataset][$category] = $this->terms[$dataset][$category];
          }
        }
      }
      // Import selected assets.
      foreach ($ids_assets as $asset) {
        $parameter_name = 'idsimport_import_' . $asset;
        if (isset($_POST[$parameter_name]) && ($_POST[$parameter_name])) {
          $this->import_assets($dataset, $asset);
          $idsimport_import_dates[$dataset][$asset] = $date;
          $idsimport_import_results[$dataset][$asset] = $this->terms[$dataset][$asset];
        }
      }
    }
    idsapi_variable_set('idsimport', 'import_dates', $idsimport_import_dates);
    idsapi_variable_set('idsimport', 'import_results', $idsimport_import_results);
  }

  // Import assets.
  function import_assets($dataset, $type) {
    $this->terms[$dataset][$type] = array('new' => 0, 'updated' => 0);
    if (!idsapi_exclude($dataset, $type)) {
      if ($response = $this->retrieve_content($dataset, $type)) {
        $assets = $response->results;
        if (!empty($assets)) {
          $post_type = 'ids_' . $type;
          $object_id_field = $dataset . '_object_id';
          if (post_type_exists($post_type)) {
            if (is_admin() && IDS_IMPORT_VERBOSE_IMPORT) {
              $output = __('Importing') . ' ' . ucfirst($dataset) . ' ' . __($type) . '...';
              echo ids_div_field_wrapper('ids-title-output-importer', '', '', $output);
            }
            foreach ($assets as $asset) {
              if ($post_id = idsimport_get_id_by_meta('post', $object_id_field, $asset->object_id)) { // The post already exists in the dataset. Update it.
                $post_id = idsimport_update_asset($post_id, $dataset, $post_type, $asset);
                if ($post_id) {
                  $this->terms[$dataset][$type]['updated']++;
                }
                else {
                  idsapi_register_error('idsimport', __('The post could not be updated for ') . $asset->object_id, 'import_assets', 'warning');
                }
              }
              else { // The asset does not exist in the dataset. Create it.
                $post_id = idsimport_insert_asset($dataset, $post_type, $asset);
                if ($post_id) {
                  $this->terms[$dataset][$type]['new']++ ;
                }
                else {
                  idsapi_register_error('idsimport', __('A new post could not be created for ') . $asset->object_id, 'IDS_Importer::import_assets', 'warning');
                }
              }
            }
          }
        }
        else {
          idsapi_register_error('idsimport', __('Sorry, we cannot find any content that matches your criteria. Please remove some filters or search more generally and try again.'), 'IDS_Importer::import_assets', 'notification', FALSE);
        }
      }
    }
  }

  // Import categories.
  function import_categories($dataset, $type) {
    if (!idsapi_exclude($dataset, $type)) {
      $this->terms[$dataset][$type] = array('new' => 0, 'updated' => 0);
      $update_children = array();
      $levels = idsapi_variable_get('idsimport', 'level_categories', IDS_IMPORT_CATEGORY_LEVELS);
      if ($response = $this->retrieve_content($dataset, $type, $levels)) {
        $categories = $response->results;
        if (!empty($categories)) {
          $taxonomy_name = $dataset . '_' . $type;
          if (taxonomy_exists($taxonomy_name)) {
            $object_id_field = $dataset . '_object_id';
            if (is_admin() && IDS_IMPORT_VERBOSE_IMPORT) {
              $output = __('Importing') . ' ' . ucfirst($dataset) . ' ' . __($type) . '...';
              echo ids_div_field_wrapper('ids-title-output-importer', '', '', $output);
            }
            foreach ($categories as $category) {
              $update_parent = FALSE;
              $category_name_display = idsimport_get_category_name($category);
              $category_name = $category_name_display . " ($category->object_id)";
              $term_args = array('name' => $category_name);
              $term = FALSE;
              if ((isset($category->level)) && ($category->level > 1) && (isset($category->parent_object_id))) {
                if ($term_id_parent = idsimport_get_id_by_meta(IDS_IMPORT_TAXONOMY, $object_id_field, $category->parent_object_id)) { // The parent is a WP term.
                  $term_args['parent'] = $term_id_parent;
                  // $term = term_exists($category_name, $taxonomy_name, $term_id_parent);
                }
                else { // It has a parent, but the parent term has not been created in the dataset yet.
                  // $term = term_exists($category_name, $taxonomy_name);
                  $update_parent = TRUE;
                }
              }
              /*
              else {
                $term = term_exists($category_name, $taxonomy_name);
              }
              */
              $term = term_exists($category_name, $taxonomy_name);
              if ($term) {
                if (is_array($term)) {
                  $term_id = $term['term_id'];
                }
                else {
                  $term_id = $term;
                }
                if ($update_parent) {
                  if (isset($update_children[$category->parent_object_id])) {
                  $update_children[$category->parent_object_id][] = $term_id;
                  }
                  else {
                    $update_children[$category->parent_object_id] = array($term_id);
                  }
                }
                // Update the term corresponding to the category.
                if (is_admin() && IDS_IMPORT_VERBOSE_IMPORT) {
                  $output = $category_name_display . ' (' . __('updated') . ') ';
                  echo ids_div_field_wrapper('ids-item-output-importer', '', '', $output);
                }
                if (idsimport_update_category($term_id, $dataset, $taxonomy_name, $category, $term_args)) {
                  $this->terms[$dataset][$type]['updated']++;
                }
                else {
                  idsapi_register_error('idsimport', __('The term could not be updated for category ') . $category->object_id, 'import_categories', 'warning');
                }
              }
              else {
                // Create a new term for the category.
                if (is_admin() && IDS_IMPORT_VERBOSE_IMPORT) {
                  $output = $category_name_display . ' (' . __('new') . ') ';
                  echo ids_div_field_wrapper('ids-item-output-importer', '', '', $output);
                }
                $term_id = idsimport_insert_category($dataset, $taxonomy_name, $category, $term_args);
                if ($term_id) {
                  $this->terms[$dataset][$type]['new']++ ;
                }
                else {
                  idsapi_register_error('idsimport', __('A new term could not be created for category ') . $category->object_id, 'import_categories', 'warning');
                }
              }
            }
            idsimport_update_children($dataset, $update_children, $taxonomy_name);
          }
        }
        else {
          idsapi_register_error('idsimport', __('No categories were retrieved by the API call.'), 'import_categories', 'warning');
        }
      }
    }
  }

  // Call the API to retrieve content of the indicated type, using the the plugin settings.
  function retrieve_content($dataset, $type, $levels = 0) {
    $idsapi = new IdsApiWrapper;
    $api_key = idsapi_variable_get('idsimport', 'api_key', '');
    $format = 'full';
    $filter_params = array();
    $is_asset = ($type == 'documents' || $type == 'organisations');
    if ($is_asset) {
      $num_requested = idsapi_variable_get('idsimport', 'num_items', IDS_API_NUM_ITEMS);
      $age_results = idsapi_variable_get('idsimport', 'age_new_assets', IDS_API_AGE_NEW_ASSETS);
      $idsimport_import_query = idsapi_variable_get('idsimport', 'import_query', '');
      $idsimport_import_countries_assets = idsapi_variable_get('idsimport', $dataset . '_countries_assets', array());
      $ids_import_regions_assets = idsapi_variable_get('idsimport', $dataset . '_regions_assets', array());
      $idsimport_import_themes_assets = idsapi_variable_get('idsimport', $dataset . '_themes_assets', array());
      if ($idsimport_import_query) {
        $search_params = explode(',', $idsimport_import_query);
        foreach ($search_params as $search_param) {
          $param = explode('=',trim($search_param));
          $key = $param[0];
          $value = $param[1];
          $filter_params[$key] = $value;
        }
      }
      if (!empty($idsimport_import_countries_assets)) {
        $filter_params['country'] = implode('|', $idsimport_import_countries_assets);
      }
      if (!empty($ids_import_regions_assets)) {
        $filter_params['region'] = implode('|', $ids_import_regions_assets);
      }
      if (!empty($idsimport_import_themes_assets)) {
        $filter_params['theme'] = implode('|', $idsimport_import_themes_assets);
      }
      if ($type == 'documents') { // Document-specific filters
        $idsimport_import_authors_assets = idsapi_variable_get('idsimport', 'import_authors_assets', '');
        $idsimport_import_publishers_assets = idsapi_variable_get('idsimport', 'import_publishers_assets', '');
        if ($idsimport_import_authors_assets) {
          $authors = explode(',', $idsimport_import_authors_assets);
          $list_authors = implode('|', array_map('trim', $authors));
          $filter_params['author'] = $list_authors;
        }
        if ($idsimport_import_publishers_assets) {
          $publishers = explode(',', $idsimport_import_publishers_assets);
          $list_publishers = implode('|', array_map('trim', $publishers));
          $filter_params['publisher_name'] = $list_publishers;
        }
      }
    } else { // is country, region or theme
      if ($type == 'themes') {
        if ($levels > 0) {
          $include_levels = array();
          for ($i = 1; $i <= $levels; $i++) {
            $include_levels[] = $i;
          }
          $filter_params['level'] = implode('|', $include_levels);
        }
      }
      $num_requested = 0;
      $age_results = 0;
    }
    $response = $idsapi->search($type, $dataset, $api_key, $format, $num_requested, $age_results, $filter_params);
    if ($response->isError()) {
      $error_message = __('No content retrieved by the API call. ') . $response->errorMessage();
      idsapi_register_error('idsimport', $error_message, 'IDS_Importer::retrieve_content', 'warning');
      $response = FALSE;
    }
    return $response;
  }

} // Class IDS_Import

} // If WP_Importer exists

//----------------------- End of IDS_Importer class definition -------------------------

//------------------------ Functions to manage imported content ------------------------

// Create new post with imported asset.
function idsimport_insert_asset($dataset, $post_type, $asset) {
  $import_keywords = idsapi_variable_get('idsimport', 'import_tags', IDS_IMPORT_TAGS);
  $idsimport_import_user_id = idsapi_variable_get('idsimport', 'import_user_id', '0');
  $ids_api_key = idsapi_variable_get('idsimport', 'api_key', '');
  $idsimport_pub_date = idsapi_variable_get('idsimport', 'pub_date', 'default');
  $display_sites = idsimport_display_datasets('public');
  $countries_taxonomy = $dataset . '_countries';
  $regions_taxonomy = $dataset . '_regions';
  $themes_taxonomy = $dataset . '_themes';
  $object_id_field = $dataset . '_object_id';
  $post = array();
  $post['post_title'] = $asset->title;
  $post['post_name'] = $dataset . '-' . sanitize_title($asset->title);
  $post['post_content'] = $asset->description;
  $post['post_type'] = $post_type;
  if (in_array($dataset, $display_sites)) {
    $post['post_status'] = 'publish';
  }
  else {
    $post['post_status'] = 'pending';
  }
  if ($idsimport_import_user_id) {
    $post['post_author'] = $idsimport_import_user_id;
  }
  if (isset($asset->date_created) && ($idsimport_pub_date == 'date_created')) { 
    $post['post_date'] = date_i18n('Y-m-d H:i:s', $asset->date_created);
  }
  if (isset($asset->date_updated) && ($idsimport_pub_date == 'date_updated')) { 
    $post['post_date'] = date_i18n('Y-m-d H:i:s', $asset->date_updated);
  }
  $post_id  = wp_insert_post($post);
  if (is_admin() && IDS_IMPORT_VERBOSE_IMPORT) {
    $output = $asset->title . ' (' . __('new') . ') ';
    echo ids_div_field_wrapper('ids-item-output-importer', '', '', $output);
  }
  if ($post_id) {
    add_post_meta($post_id, $object_id_field, $asset->object_id, TRUE);
    // Extra fields that could not be present in 'short' responses.
    if (isset($asset->site)) { add_post_meta($post_id, 'site', $asset->site, TRUE); }
    if (isset($asset->timestamp)) { add_post_meta($post_id, 'timestamp', $asset->timestamp, TRUE); }
    if (isset($asset->website_url)) { add_post_meta($post_id, 'website_url', $asset->website_url, TRUE); }
    if (isset($asset->asset_id)) { add_post_meta($post_id, 'asset_id', $asset->asset_id, TRUE); }
    if (isset($asset->date_created)) { add_post_meta($post_id, 'date_created', date_i18n(get_option('date_format'), $asset->date_created), TRUE); }
    if (isset($asset->date_updated)) { add_post_meta($post_id, 'date_updated', date_i18n(get_option('date_format'), $asset->date_updated), TRUE); }
    if (isset($asset->country_focus_array)) { idsimport_update_categories_post($post_id, $dataset, $countries_taxonomy, $asset->country_focus_array); }
    if (isset($asset->category_region_array)) { idsimport_update_categories_post($post_id, $dataset, $regions_taxonomy, $asset->category_region_array); }
    if (isset($asset->category_theme_array)) { idsimport_update_categories_post($post_id, $dataset, $themes_taxonomy, $asset->category_theme_array); }
    if (isset($asset->metadata_url)) {
      $metadata_url = $asset->metadata_url;
      if ($ids_api_key) {
        $metadata_url .= '?' . IDS_API_KEY_PAR . '=' . $ids_api_key;
      }
      add_post_meta($post_id, 'metadata_url', $metadata_url, TRUE);
    }
    if (($import_keywords) && (isset($asset->keywords))) {
      wp_set_post_tags($post_id, $asset->keywords, TRUE);
    }
    if ($post_type == 'ids_documents') {
    // Document-specific fields.
      if (isset($asset->authors)) { idsimport_add_post_array_meta($post_id, 'authors', $asset->authors, TRUE); }
      if (isset($asset->language_name)) { add_post_meta($post_id, 'language_name', $asset->language_name, TRUE); }
      if (isset($asset->publication_date)) { add_post_meta($post_id, 'publication_date', $asset->publication_date, TRUE); }
      if (isset($asset->licence_type)) { add_post_meta($post_id, 'licence_type', $asset->licence_type, TRUE); }
      if (isset($asset->publisher_array)) { idsimport_add_post_array_meta($post_id, 'publisher_array', $asset->publisher_array, TRUE); }
      if (isset($asset->urls)) { idsimport_add_post_array_meta($post_id, 'urls', $asset->urls, TRUE); }
    }
    elseif ($post_type == 'ids_organisations') {
    // Organisation-specific fields.
      if (isset($asset->acronym)) { add_post_meta($post_id, 'acronym', $asset->acronym, TRUE); }
      if (isset($asset->alternative_acronym)) { add_post_meta($post_id, 'alternative_acronym', $asset->alternative_acronym, TRUE); }
      if (isset($asset->organisation_type)) { add_post_meta($post_id, 'organisation_type', $asset->organisation_type, TRUE); }
      if (isset($asset->organisation_url)) { add_post_meta($post_id, 'organisation_url', $asset->organisation_url, TRUE); }
      if (isset($asset->location_country)) { add_post_meta($post_id, 'location_country', $asset->location_country, TRUE); }
    }
  }
  else {
    idsapi_register_error('idsimport',  __('Error inserting asset ') . $asset->title, 'idsimport_insert_asset', 'warning');
  }
  return array($post_id);
}

// Update post with imported asset.
function idsimport_update_asset($post_id, $dataset, $post_type, $asset) {
  $import_keywords = idsapi_variable_get('idsimport', 'import_tags', '0');
  $idsimport_import_user_id = idsapi_variable_get('idsimport', 'import_user_id', '0');
  $ids_api_key = idsapi_variable_get('idsimport', 'api_key', '');
  $idsimport_pub_date = idsapi_variable_get('idsimport', 'pub_date', 'default');
  $countries_taxonomy = $dataset . '_countries';
  $regions_taxonomy = $dataset . '_regions';
  $themes_taxonomy = $dataset . '_themes';
  $object_id_field = $dataset . '_object_id';
  $updated_post = array();
  $updated_post['ID'] = $post_id;
  $updated_post['post_title'] = $asset->title;
  $updated_post['post_content'] = $asset->description;
  if ($idsimport_import_user_id) {
    $updated_post['post_author'] = $idsimport_import_user_id;
  }
  if (isset($asset->date_created) && ($idsimport_pub_date == 'date_created')) { 
    $updated_post['post_date'] = date_i18n('Y-m-d H:i:s', $asset->date_created);
  }
  if (isset($asset->date_updated) && ($idsimport_pub_date == 'date_updated')) { 
    $updated_post['post_date'] = date_i18n('Y-m-d H:i:s', $asset->date_updated);
  }
  $new_post_id  = wp_update_post($updated_post);
  if (is_admin() && IDS_IMPORT_VERBOSE_IMPORT) {
    $output = $asset->title . ' (' . __('updated') . ') ';
    echo ids_div_field_wrapper('ids-item-output-importer', '', '', $output);
  }
  if ($new_post_id) {
    update_post_meta($post_id, $object_id_field, $asset->object_id, TRUE);
    // Extra fields that could not be present in 'short' responses.
    if (isset($asset->site)) { update_post_meta($post_id, 'site', $asset->site, TRUE); }
    if (isset($asset->timestamp)) { update_post_meta($post_id, 'timestamp', $asset->timestamp, TRUE); }
    if (isset($asset->website_url)) { update_post_meta($post_id, 'website_url', $asset->website_url, TRUE); }
    if (isset($asset->asset_id)) { update_post_meta($post_id, 'asset_id', $asset->asset_id, TRUE); }
    if (isset($asset->date_created)) { update_post_meta($post_id, 'date_created', date_i18n(get_option('date_format'), $asset->date_created), TRUE); }
    if (isset($asset->date_updated)) { update_post_meta($post_id, 'date_updated', date_i18n(get_option('date_format'), $asset->date_updated), TRUE); }
    if (isset($asset->country_focus_array)) { idsimport_update_categories_post($post_id, $dataset, $countries_taxonomy, $asset->country_focus_array); }
    if (isset($asset->category_region_array)) { idsimport_update_categories_post($post_id, $dataset, $regions_taxonomy, $asset->category_region_array); }
    if (isset($asset->category_theme_array)) { idsimport_update_categories_post($post_id, $dataset, $themes_taxonomy, $asset->category_theme_array); }
    if (isset($asset->metadata_url)) {
      $metadata_url = $asset->metadata_url;
      if ($ids_api_key) {
        $metadata_url .= '?' . IDS_API_KEY_PAR . '=' . $ids_api_key;
      }
      update_post_meta($post_id, 'metadata_url', $metadata_url, TRUE);
    }
    if (($import_keywords) && (isset($asset->keywords))) {
      wp_set_post_tags($post_id, $asset->keywords, FALSE);
    }
    if ($post_type == 'ids_documents') {
    // Document-specific fields.
      if (isset($asset->authors)) { idsimport_update_post_array_meta($post_id, 'authors', $asset->authors, TRUE); }
      if (isset($asset->language_name)) { update_post_meta($post_id, 'language_name', $asset->language_name, TRUE); }
      if (isset($asset->publication_date)) { update_post_meta($post_id, 'publication_date', $asset->publication_date, TRUE); }
      if (isset($asset->licence_type)) { update_post_meta($post_id, 'licence_type', $asset->licence_type, TRUE); }
      if (isset($asset->publisher)) { update_post_meta($post_id, 'publisher', $asset->publisher, TRUE); }
      if (isset($asset->publisher_country)) { update_post_meta($post_id, 'publisher_country', $asset->licence_type, TRUE); }
      if (isset($asset->urls)) { idsimport_update_post_array_meta($post_id, 'urls', $asset->urls, TRUE); }
    }
    elseif ($post_type == 'ids_organisations') {
    // Organisation-specific fields.
      if (isset($asset->acronym)) { update_post_meta($post_id, 'acronym', $asset->acronym, TRUE); }
      if (isset($asset->alternative_acronym)) { update_post_meta($post_id, 'alternative_acronym', $asset->alternative_acronym, TRUE); }
      if (isset($asset->organisation_type)) { update_post_meta($post_id, 'organisation_type', $asset->organisation_type, TRUE); }
      if (isset($asset->organisation_url)) { update_post_meta($post_id, 'organisation_url', $asset->organisation_url, TRUE); }
      if (isset($asset->location_country)) { update_post_meta($post_id, 'location_country', $asset->location_country, TRUE); }
    }
  }
  else {
    idsapi_register_error('idsimport',  __('Error updating asset ') . $asset->title, 'idsimport_update_asset', 'warning');
  }
  return array($new_post_id);
}

// Gets the category name, looking into the retrieved fields.
function idsimport_get_category_name($category) {
  $category_name = '';
  if (isset($category->title)) {
    $category_name = $category->title;
  } elseif (isset($category->object_name)) {
    $category_name = $category->object_name;
  } elseif (isset($category->name)) {
    $category_name = $category->name;
  }
  return trim($category_name);
}

// This is necessary if parents don't come before their children, which is not the case now with the IDS API, but better to make sure.
function idsimport_update_children($dataset, $update_children, $taxonomy_name) {
  $object_id_field = $dataset . '_object_id';
  foreach ($update_children as $parent_object_id => $array_children_term_ids) {
    if ($term_id_parent = idsimport_get_id_by_meta(IDS_IMPORT_TAXONOMY, $object_id_field, $parent_object_id)) {
      foreach ($array_children_term_ids as $term_id) {
        wp_update_term($term_id, $taxonomy_name, array('parent' => $term_id_parent));
      }
    }
  }
}

// Create new term with imported category.
// Returns the term id if everything goes ok, NULL if it could not be inserted.
// TODO - insert only categories not archived?
function idsimport_insert_category($dataset, $taxonomy, $category, $term_args) {
  $ids_api_key = idsapi_variable_get('idsimport', 'api_key', '');
  $countries_taxonomy = $dataset . '_countries';
  $regions_taxonomy = $dataset . '_regions';
  $object_id_field = $dataset . '_object_id';
  $term_id = NULL;
  $term_slug = $dataset . '-' . sanitize_title($term_args['name']);
  $term_args['slug'] = $term_slug;
  $new_term = wp_insert_term($term_args['name'], $taxonomy, $term_args);
  if (!is_wp_error($new_term)) {
    $term_id = $new_term['term_id'];
    idsimport_add_term_meta($term_id, $object_id_field, $category->object_id, TRUE);
    // There might be some fields not available when inserting categories (in the process of inserting an asset or with 'short' responses).
    if (isset($category->site)) { idsimport_add_term_meta($term_id, 'site', $category->site, TRUE); }
    if (isset($category->timestamp)) { idsimport_add_term_meta($term_id, 'timestamp', $category->timestamp, TRUE); }
    if (isset($category->website_url)) { idsimport_add_term_meta($term_id, 'website_url', $category->website_url, TRUE); }
    if ($taxonomy == $countries_taxonomy) {
      if (isset($category->alternative_name)) { idsimport_add_term_meta($term_id, 'alternative_name', $category->alternative_name, TRUE); }
      if (isset($category->asset_id)) { idsimport_add_term_meta($term_id, 'asset_id', $category->asset_id, TRUE); }
      if (isset($category->category_region_array)) { idsimport_update_categories_term($term_id, $dataset, $regions_taxonomy, $category->category_region_array); }
      if (isset($category->iso_number)) { idsimport_add_term_meta($term_id, 'iso_number', $category->iso_number, TRUE); }
      if (isset($category->iso_three_letter_code)) { idsimport_add_term_meta($term_id, 'iso_three_letter_code', $category->iso_three_letter_code, TRUE); }
      if (isset($category->iso_two_letter_code)) { idsimport_add_term_meta($term_id, 'iso_two_letter_code', $category->iso_two_letter_code, TRUE); }
    } else {
      if (isset($category->archived)) { idsimport_add_term_meta($term_id, 'archived', $category->archived, TRUE); }
      if (isset($category->level)) { idsimport_add_term_meta($term_id, 'level', $category->level, TRUE); }
      if (isset($category->cat_parent)) { idsimport_add_term_meta($term_id, 'cat_parent', $category->cat_parent, TRUE); }
      if (isset($category->cat_superparent)) { idsimport_add_term_meta($term_id, 'cat_superparent', $category->cat_superparent, TRUE); }
      if (isset($category->category_id)) { idsimport_add_term_meta($term_id, 'category_id', $category->category_id, TRUE); }
    }
    if (isset($category->metadata_url)) {
      $metadata_url = $category->metadata_url;
      if ($ids_api_key) {
        $metadata_url .= '?' . IDS_API_KEY_PAR . '=' . $ids_api_key;
      }
      idsimport_add_term_meta($term_id, 'metadata_url', $metadata_url, TRUE);
    }
  }
  else {
    idsapi_register_error('idsimport',  __('Error inserting category ') . $term_args['name'] . ': ' . $new_term->get_error_message(), 'idsimport_insert_category', 'warning');
  }
  return $term_id;
}


// Update term with imported category.
// Returns NULL if there was an error when updating.
// TODO - process categories that are now archived. Delete them?
function idsimport_update_category($term_id, $dataset, $taxonomy, $category, $term_args) {
  $ids_api_key = idsapi_variable_get('idsimport', 'api_key', '');
  $countries_taxonomy = $dataset . '_countries';
  $regions_taxonomy = $dataset . '_regions';
  $object_id_field = $dataset . '_object_id';
  $update_parent = FALSE;
  $new_term_id = NULL;
  $new_term  = wp_update_term($term_id, $taxonomy, $term_args);
  if (!is_wp_error($new_term)) {
    $new_term_id = $new_term['term_id'];
    idsimport_update_term_meta($term_id, $object_id_field, $category->object_id, TRUE);
    if (isset($category->site)) { idsimport_update_term_meta($term_id, 'site', $category->site, TRUE); }
    if (isset($category->timestamp)) { idsimport_update_term_meta($term_id, 'timestamp', $category->timestamp, TRUE); }
    if (isset($category->website_url)) { idsimport_update_term_meta($term_id, 'website_url', $category->website_url, TRUE); }
    if ($taxonomy == $countries_taxonomy) {
      if (isset($category->alternative_name)) { idsimport_add_term_meta($term_id, 'alternative_name', $category->alternative_name, TRUE); }
      if (isset($category->asset_id)) { idsimport_add_term_meta($term_id, 'asset_id', $category->asset_id, TRUE); }
      if (isset($category->category_region_array)) { idsimport_update_categories_term($term_id, $dataset, $regions_taxonomy, $category->category_region_array); }
      if (isset($category->iso_number)) { idsimport_add_term_meta($term_id, 'iso_number', $category->iso_number, TRUE); }
      if (isset($category->iso_three_letter_code)) { idsimport_add_term_meta($term_id, 'iso_three_letter_code', $category->iso_three_letter_code, TRUE); }
      if (isset($category->iso_two_letter_code)) { idsimport_add_term_meta($term_id, 'iso_two_letter_code', $category->iso_two_letter_code, TRUE); }
    } else {
      if (isset($category->archived)) { idsimport_update_term_meta($term_id, 'archived', $category->archived, TRUE); }
      if (isset($category->level)) { idsimport_update_term_meta($term_id, 'level', $category->level, TRUE); }
      if (isset($category->cat_parent)) { idsimport_update_term_meta($term_id, 'cat_parent', $category->cat_parent, TRUE); }
      if (isset($category->cat_superparent)) { idsimport_update_term_meta($term_id, 'cat_superparent', $category->cat_superparent, TRUE); }
      if (isset($category->category_id)) { idsimport_update_term_meta($term_id, 'category_id', $category->category_id, TRUE); }
    }
    if (isset($category->metadata_url)) {
      $metadata_url = $category->metadata_url;
      if ($ids_api_key) {
        $metadata_url .= '?' . IDS_API_KEY_PAR . '=' . $ids_api_key;
      }
      idsimport_update_term_meta($term_id, 'metadata_url', $metadata_url, TRUE);
    }
  }
  else {
    idsapi_register_error('idsimport',  __('Error updating category ') . $term_args['name'] . ': ' . $new_term->get_error_message(), 'idsimport_update_category', 'warning');
  }  
  return $new_term_id;
}

// Relate a post with its IDS categories. If the categories do not exist as terms, they are created.
// TODO. Get the arrays outside of this function. "Clean" previously assigned categories in the 'category' taxonomy in case they changed.
function idsimport_update_categories_post($post_id, $dataset, $taxonomy, $categories) {
  $idsimport_new_categories = idsapi_variable_get('idsimport', 'import_new_categories', IDS_IMPORT_NEW_CATEGORIES);
  $idsimport_map_categories = idsapi_variable_get('idsimport', 'import_map_categories', IDS_IMPORT_MAP_CATEGORIES);
  if ($idsimport_new_categories) {
    // Assign assets to new taxonomy terms corresponding to imported IDS categories.
    $terms_ids = idsimport_get_terms_ids($dataset, $taxonomy, $categories);
    wp_set_post_terms($post_id, $terms_ids, $taxonomy, FALSE);
  }
  if ($idsimport_map_categories) {
    // Assign assets to existing Wordpress categories (based on mappings).
    $mappings = idsimport_get_mappings($taxonomy);
    $mappings_ids = array();
    foreach ($categories as $category) {
      if (isset($mappings[$category->object_id])) {
        $mappings_ids[] = $mappings[$category->object_id];
      }
    }
    if (!empty($mappings_ids)) {
      wp_set_post_terms($post_id, $mappings_ids, 'category', TRUE);
    }
  }
}

function idsimport_get_mappings($taxonomy_name) {
  $idscats_wpterms = array();
  $selected_mappings = $taxonomy_name . '_mappings';
  $mappings = idsapi_variable_get('idsimport', $selected_mappings, array());
  foreach ($mappings as $mapping) {
    $relation = explode(',', $mapping);
    if (isset($relation[0]) && isset($relation[1])) {
      $idscats_wpterms[$relation[0]] = $relation[1];
    }
  }
  return $idscats_wpterms;
}

// Relate a term with its related IDS categories. This currently only applies to countries (that are linked to regions).
function idsimport_update_categories_term($term_id, $dataset, $taxonomy, $categories) {
  $idsimport_new_categories = idsapi_variable_get('idsimport', 'import_new_categories', IDS_IMPORT_NEW_CATEGORIES);
  if ($idsimport_new_categories) {
    $terms_ids = idsimport_get_terms_ids($dataset, $taxonomy, $categories);
    wp_set_post_terms($term_id, $terms_ids, $taxonomy, FALSE);
  }
}

// Given an array of categories objects, return an array of their corresponding term ids. New terms are created if they don't exist.
function idsimport_get_terms_ids($dataset, $taxonomy, $categories) {
  $terms_ids = array();
  foreach ($categories as $category) {
    $category_name = idsimport_get_category_name($category);
    $category_name .= " ($category->object_id)";
    if ($term = term_exists($category_name, $taxonomy)) {
      if (is_array($term)) {
        $term_id = $term['term_id'];
      }
      else {
        $term_id = $term;
      }
    }
    else { // The term does not exist, insert it.
      $term_id = idsimport_insert_category($dataset, $taxonomy, $category, array('name' => $category_name));
    }
    if ($term_id) {
      $terms_ids[] = $term_id;
    }
    else {
      idsapi_register_error('idsimport',  __('Category not inserted.'), 'idsimport_get_terms_ids', 'warning');
    }
  }
  return $terms_ids;
}

//------------------------ Functions to manage periodic imports ------------------------

// Define new cron intervals
function idsimport_cron_intervals($schedules) {
	$schedules['twiceweekly'] = array(
		'interval' => 302400,
		'display' => __('Twice Weekly')
	);
	$schedules['weekly'] = array(
		'interval' => 604800,
		'display' => __('Once Weekly')
	);
	$schedules['monthly'] = array(
		'interval' => 2635200,
		'display' => __('Once Monthly')
	);
	return $schedules;
}

// Schedule periodic imports
function idsimport_schedule_imports() {
  $idsimport_new_categories = idsapi_variable_get('idsimport', 'import_new_categories', IDS_IMPORT_NEW_CATEGORIES);
  $import_assets['documents'] = idsapi_variable_get('idsimport', 'import_documents', 0);
  $import_assets['organisations'] = idsapi_variable_get('idsimport', 'import_organisations', 0);
  $import_categories['countries'] = idsapi_variable_get('idsimport', 'import_countries', 0);
  $import_categories['regions'] = idsapi_variable_get('idsimport', 'import_regions', 0);
  $import_categories['themes'] = idsapi_variable_get('idsimport', 'import_themes', 0);
  $frequency_assets = idsapi_variable_get('idsimport', 'import_period_assets', IDS_IMPORT_IMPORT_RECURRENCE_ASSETS);
  $frequency_categories = idsapi_variable_get('idsimport', 'import_period_categories', IDS_IMPORT_IMPORT_RECURRENCE_CATEGORIES);
  idsimport_unschedule_imports();
  if ($frequency_assets) {
    foreach ($import_assets as $assets_type => $enabled) {
      if ($enabled) {
        $schedule_result = wp_schedule_event(time(), $frequency_assets, 'idsimport_scheduled_events', array($assets_type));
      }
    }
  }
  if ($frequency_categories && $idsimport_new_categories) {
    foreach ($import_categories as $categories_type => $enabled) {
      if ($enabled) {
        $schedule_result = wp_schedule_event(time(), $frequency_categories, 'idsimport_scheduled_events', array($categories_type));
      }
    }
  }
}

// Call to periodic imports
function idsimport_run_periodic_imports($type) {
  $date = date(DATE_RFC822);
  $idsimport_import_dates = idsapi_variable_get('idsimport', 'import_dates', array('eldis' => array(), 'bridge' => array()));
  $idsimport_import_results = idsapi_variable_get('idsimport', 'import_results', array('eldis' => array(), 'bridge' => array()));
  $idsimport_new_categories = idsapi_variable_get('idsimport', 'import_new_categories', IDS_IMPORT_NEW_CATEGORIES);
  $datasets = idsimport_get_datasets();
  $importer = new IDS_Importer();
  switch ($type) {
    case 'documents':
      foreach ($datasets as $dataset) {
        $importer->terms[$dataset][$type] = array('new' => 0, 'updated' => 0);
        $importer->import_assets($dataset, 'documents');
        $idsimport_import_dates[$dataset]['documents'] = $date;
        $idsimport_import_results[$dataset]['documents'] = $importer->terms[$dataset]['documents'];
      }
      break;
    case 'organisations':
      foreach ($datasets as $dataset) {
        $importer->terms[$dataset][$type] = array('new' => 0, 'updated' => 0);
        $importer->import_assets($dataset, 'organisations');
        $idsimport_import_dates[$dataset]['organisations'] = $date;
        $idsimport_import_results[$dataset]['organisations'] = $importer->terms[$dataset]['organisations'];
      }
      break;
    case 'countries':
      if ($idsimport_new_categories) {
        foreach ($datasets as $dataset) {
          $importer->terms[$dataset][$type] = array('new' => 0, 'updated' => 0);
          $importer->import_categories($dataset, 'countries');
          $idsimport_import_dates[$dataset]['countries'] = $date;
          $idsimport_import_results[$dataset]['countries'] = $importer->terms[$dataset]['countries'];
        }
      }
      break;
    case 'regions': 
      if ($idsimport_new_categories) {
        foreach ($datasets as $dataset) {
          $importer->terms[$dataset][$type] = array('new' => 0, 'updated' => 0);
          $importer->import_categories($dataset, 'regions');
          $idsimport_import_dates[$dataset]['regions'] = $date;
          $idsimport_import_results[$dataset]['regions'] = $importer->terms[$dataset]['regions'];
        }
      }
      break;
    case 'themes':
      if ($idsimport_new_categories) {
        foreach ($datasets as $dataset) {
          $importer->terms[$dataset][$type] = array('new' => 0, 'updated' => 0);
          $importer->import_categories($dataset, 'themes');
          $idsimport_import_dates[$dataset]['themes'] = $date;
          $idsimport_import_results[$dataset]['themes'] = $importer->terms[$dataset]['themes'];
        }
      }
      break;
  }  
  idsapi_variable_set('idsimport', 'import_dates', $idsimport_import_dates);
  idsapi_variable_set('idsimport', 'import_results', $idsimport_import_results);
}

// Unschedule all the import events
function idsimport_unschedule_imports() {
  wp_clear_scheduled_hook('idsimport_scheduled_events', array('documents'));
  wp_clear_scheduled_hook('idsimport_scheduled_events', array('organisations'));
  wp_clear_scheduled_hook('idsimport_scheduled_events', array('countries'));
  wp_clear_scheduled_hook('idsimport_scheduled_events', array('regions'));
  wp_clear_scheduled_hook('idsimport_scheduled_events', array('themes'));
}





