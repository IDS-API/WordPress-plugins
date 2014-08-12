<?php

// Checks if it has to display the admin settings or if it has to initialize the plugin's data, etc.
function idsview_admin_main() {
  if (!current_user_can( 'manage_options')) {
    wp_die(__('You do not have sufficient permissions to manage the plugin settings.'));
  }
  idsview_render_form();
}

// Display registered error messages
function idsview_admin_notices() {
  settings_errors('idsview_options', FALSE, TRUE);
  idsapi_report_errors();
}

// Returns an array with the selected datasets.
function idsview_get_datasets() {
  global $ids_datasets;
  $default_dataset = idsapi_variable_get('idsview', 'default_dataset', IDS_API_DEFAULT_DATASET);
  if ($default_dataset == 'all') {
    $datasets = $ids_datasets;
  }
  else {
    $datasets = array($default_dataset);
  }
  return $datasets;
}

// Check if the permalinks configuration was changed and we need to flush the rules.
function idsview_check_permalinks_changed() {
  global $wp_rewrite;
  $idsview_permalinks = idsapi_variable_get('idsview', 'permalink_structure', '');
  $current_permalinks = get_option('permalink_structure');
  if ($idsview_permalinks !== $current_permalinks) {
    idsapi_variable_set('idsview', 'permalink_structure', $current_permalinks);
    $wp_rewrite->flush_rules();
  }
}

// When a dataset is disabled, the status of the page that shows its content is changed to 'pending'.
function idsview_update_pages_status() {
  global $ids_datasets;
  global $ids_assets;
  $current_datasets = idsview_get_datasets();
  foreach ($ids_datasets as $dataset) {
    foreach ($ids_assets as $type) {
      $exclude = idsapi_exclude($dataset, $type);
      if (!$exclude) {
        $page = get_page_by_path($dataset . 'view_' . $type);
        if (isset($page->ID)) {
          if (in_array($dataset, $current_datasets)) {
            $page->post_status = 'publish';
          }
          else {
            $page->post_status = 'pending';
          }
          wp_update_post((array) $page);
        }
      }
    }
  }
}

// Render the plugin's main admin form.
function idsview_render_form() {
  global $ids_datasets;
  global $ids_categories;

  $idsview_api_key_validated = idsapi_variable_get('idsview', 'api_key_validated', FALSE);
	?>
	<div class="wrap">
		<!-- Display Plugin Icon, Header, and Description -->
		<div class="icon32" id="icon-options-general"></div>
		<h2><?php _e('IDS View Plugin Settings'); ?></h2>
		<p>
      <?php printf(__('This plugin provides access to the Eldis and BRIDGE datasets via the IDS Knowledge Services API (<a href="%s" target="_new">%s</a>).'), IDS_API_HOME_URL, IDS_API_HOME_URL); ?>
      <br />
      <br />
      <?php _e('BRIDGE is a research and information service supporting gender advocacy and mainstreaming efforts by bridging the gaps between theory, policy and practice.'); ?>
      <br />
      <?php _e('Eldis is an online information service covering development research on a wide change of thematic areas including agriculture, climate change, conflict, gender, governance, and health.'); ?>
      <br />
      <br />
      <i>
      <?php _e('Please look through all the tabs on this page to set your configuration options. Changes will only be effective after submitting them by clicking on the "Save changes" button.'); ?></i>
    </p>

    <!-- Beginning of the Plugin Options Form -->
		<form method="post" id="ids_view_form" action="options.php">
      <?php
          settings_fields('idsview');
      ?>

    <div class="ui-tabs">
      <ul class="ui-tabs-nav">
        <li><a href="#general"><?php _e('General settings'); ?></a></li>
        <li><a href="#filters"><?php _e('Filters'); ?></a></li>
      </ul>

      <!---------------------------------- GENERAL SETTINGS ------------------------------->

      <h3><?php _e('General settings'); ?></h3>
			<table class="form-table ui-tabs-panel" id="general">
				<!-- IDS API Key -->
				<tr>
					<th scope="row"><?php _e('IDS API Token-GUID (key)'); ?></th>
					<td>
						<input type="text" size="57" name="idsview_options[api_key]" value="<?php echo idsapi_variable_get('idsview', 'api_key', ''); ?>" /><br />
						<p class="description">
              <?php printf(__('If you do not have an IDS API key, please request one at <a href="%s" target="_new">%s</a>'), IDS_API_KEY_URL, IDS_API_KEY_URL); ?>
            </p>
					</td>
				</tr>

      <?php if ($idsview_api_key_validated) { ?>

        <?php idsview_update_pages_status(); ?>

        <!-- Dataset -->
        <tr valign="top">
          <th scope="row"><?php _e('Display content from:'); ?></th>
          <td id="radio_dataset">
            <label><input name="idsview_options[default_dataset]" type="radio" value="eldis" <?php checked('eldis', idsapi_variable_get('idsview', 'default_dataset', IDS_API_DEFAULT_DATASET)); ?> onchange="changeDataset();" /> Eldis only</label> <br />
            <label><input name="idsview_options[default_dataset]" type="radio" value="bridge" <?php checked('bridge', idsapi_variable_get('idsview', 'default_dataset', IDS_API_DEFAULT_DATASET)); ?> onchange="changeDataset();" /> Bridge only</label><br />
            <label><input name="idsview_options[default_dataset]" type="radio" value="all" <?php checked('all', idsapi_variable_get('idsview', 'default_dataset', IDS_API_DEFAULT_DATASET)); ?> onchange="changeDataset();" /> All sources</label><br />
          </td>
        </tr>

        <!-- Languages -->
        <tr>
          <th scope="row"><?php _e('Default language'); ?></th>
          <td>
            <?php echo ids_select_box('idsview_options[language]', 'idsview_language', ids_languages(), array(idsapi_variable_get('idsview', 'language', IDS_VIEW_DEFAULT_LANGUAGE))); ?>
            <p class="description">
            <?php _e('Translatable metadata (title, description) will be displayed in this language, if available.'); ?>
            </p>
          </td>
        </tr>

        <!-- Number of assets to retrieve -->
        <tr>
          <th scope="row"><?php _e('Excerpts length'); ?></th>
          <td>
            <input type="text" size="10" name="idsview_options[excerpt_length]" value="<?php echo idsapi_variable_get('idsview', 'excerpt_length', 0); ?>" />
            <p class="description">
            <?php _e('Maximum length of excerpts to display.'); ?>
            <br/>
            <?php _e('If not set (or if the value is 0) the full description will be used.'); ?>
            </p>
          </td>
        </tr>

      </table>

      <h3><?php _e('Filters'); ?></h3>
      <table class="form-table ui-tabs-panel" id="filters">
        <tr>
          <td colspan="2">
            <p class="description"><?php _e('Documents and/or organisations to be displayed can be limited to those matching the following filters.'); ?></p>
          </td>
        </tr>

        <!-- IDS Categories (countries, regions, themes) -->
        <?php foreach ($ids_datasets as $dataset) { ?>
        <tr class="ids_categories_<?php echo $dataset; ?>">
          <td colspan="2">
            <p id="title_categories" class="ids-settings-separator ids-categories-<?php echo $dataset; ?>"><?php echo ucfirst($dataset); ?> categories</p>
          </td>
        </tr>
        <?php foreach (array('countries', 'regions') as $category) { 
        $field_id = $dataset . '_' . $category . '_assets';
        $option_name = $dataset . '_' . $category . '_assets';
        $category_name = ucfirst($category);
        ?>
        <tr class="ids-categories-<?php echo $dataset; ?>">
         <th scope="row"><?php _e($category_name); ?></th>
          <td>
            <select data-placeholder="Select <?php echo $category; ?>..." class="chzn-select" style="width:350px;" id='<?php echo $field_id; ?>' name='idsview_options[<?php echo $option_name; ?>][]' multiple='multiple' size='10'></select>
            <p class="description"><?php _e("Display only documents/organisations with the selected focus $category."); ?>
            <a href="javascript:deselectAll('<?php echo $field_id; ?>');"><?php _e('Deselect all'); ?></a>
            </p>
          </td>
        </tr>
        <?php } // countries, regions ?>

        <!-- Themes trees -->
        <?php $themes_dropdown_id = 'dropDownButton_' . $dataset; ?>
        <?php $themes_tree_id = 'jqxTree_' . $dataset; ?>
        <tr class="ids-categories-<?php echo $dataset; ?>">
         <th scope="row"><?php _e('Themes'); ?></th>
          <!-- Themes select boxes -->
          <td valign="top">
          <?php
          $field_id = $dataset . '_themes_assets';
          $option_name = $dataset . '_themes_assets';
          ?>
          <div class="ids-categories-themes">
            <select data-placeholder="No themes selected" class="chzn-select" style="width:350px;" id='<?php echo $field_id; ?>' name='idsview_options[<?php echo $option_name; ?>][]' multiple='multiple' size='10'></select><br>
          </div>

          <div id='<?php echo $themes_dropdown_id; ?>' style="float:left; margin-right:40px;">
            <div id='<?php echo $themes_tree_id; ?>'>
            </div>
          </div>

          <div style="clear:both;">
          <p class="description">
            <br>
            Import only documents/organisations with the selected focus themes.
            <a href="javascript:removeAll('<?php echo $field_id; ?>');"><?php _e('Deselect all'); ?></a>
          </p>
          </div>

          </td>
        </tr>



        <?php } // foreach dataset ?>

        <tr>
          <td colspan="2">
            <p class="ids-settings-separator">Additional filters</p>
          </td>
        </tr>

        <!-- Age of assets to view -->
        <tr>
         <th scope="row"><?php _e('Age of documents and/or organisations'); ?></th>
          <td>
            <?php echo ids_select_box('idsview_options[age_new_assets]', 'idsview_age_new_assets', array('0' => __('Do not filter by age'), '7' => __('One week'), '30' => __('One month'), '180' => __('Six months'), '365' => __('One year')), array(idsapi_variable_get('idsview', 'age_new_assets', IDS_API_AGE_NEW_ASSETS))); ?>
            <p class="description"><?php _e('Only documents/organisations published after the indicated time will be shown.'); ?></p>
          </td>
        </tr>

        <!-- Resources' languages -->
        <tr>
          <th scope="row"><?php _e('Resources\' languages'); ?></th>
          <td>
            <?php echo ids_select_box('idsview_options[language_name_codes][]', 'idsview_language_name_codes', ids_languages(), idsapi_variable_get('idsview', 'language_name_codes', array()), TRUE); ?>
            <a href="javascript:deselectAll('idsview_language_name_codes');"><?php _e('Deselect all'); ?></a>
            <p class="description">
            <?php _e('Only display resources that are available in the selected languages. If none is selected, resources in all languages will be displayed.'); ?>
            </p>
          </td>
        </tr>

        <!-- Exclude URIs -->
        <tr>
          <th scope="row"><?php _e('Exclude URIs'); ?></th>
          <td>
            <input type="text" size="57" name="idsview_options[exclude_uris]" value="<?php echo idsapi_variable_get('idsview', 'exclude_uris', get_bloginfo('url')); ?>" /><br />
            <p class="description">
            <?php _e('Exclude resources that match certain URIs. Partial matching will be applied. Use commas to indicate several URIs.'); ?><br />
            <?php _e('This filter can be used, for instance, to avoid displaying resources that are originated from this site.'); ?><br />
            <?php _e('Example: <i>http://www.worldbank.org/en/news, undp.org</i> would exclude WB\'s press releases and all resources from servers with undp.org as domain name.'); ?></p>
          </td>
        </tr>

        <!-- Authors -->
        <tr>
          <th scope="row"><?php _e('Documents\' author(s)'); ?></th>
          <td>
            <input type="text" size="57" name="idsview_options[view_authors_assets]" value="<?php echo idsapi_variable_get('idsview', 'view_authors_assets', ''); ?>" /><br />
            <p class="description">
            <?php _e('A list of comma-separated authors\' names.'); ?><br />
            <?php _e('Example: <i>Chomsky, Foucault</i> will retrieve documents with either <i>Chomsky</i> or <i>Foucault</i> in the "author" field.'); ?></p>
          </td>
        </tr>

        <!-- Publishers -->
        <tr>
          <th scope="row"><?php _e('Documents\' publisher(s)'); ?></th>
          <td>
            <input type="text" size="57" name="idsview_options[view_publishers_assets]" value="<?php echo idsapi_variable_get('idsview', 'view_publishers_assets', ''); ?>" /><br />
            <p class="description">
            <?php _e('A list of comma-separated publishers\' names.'); ?><br />
            <?php _e('Example: <i>World Bank, UNDP</i> will retrieve documents with either <i>World Bank</i> or <i>UNDP</i> in the "publisher" field.'); ?></p>
          </td>
        </tr>

        <!-- Additional query terms -->
        <tr>
          <th scope="row"><?php _e('Query terms'); ?></th>
          <td>
            <input type="text" size="57" name="idsview_options[view_query]" value="<?php echo idsapi_variable_get('idsview', 'view_query', ''); ?>" /><br />
            <p class="description">
            <?php _e('Query string to be appended to the API URL.'); ?><br />
            <?php _e('Use comma-separated pairs of unencoded query fields and values (example: <i>country=Argentina, keyword=climate change</i>).'); ?><br />
            <?php printf(__('Please check the <a href="%s" target="_new">online documentation</a> for a list of available query fields.'), IDS_API_SEARCH_FIELDS_URL); ?>
           </p>
          </td>
        </tr>
      </table>

    </div>	<!-- UI tabs -->	

      <p class="submit">
        <input name="idsview_options[submit_save]" type="submit" class="button-primary" value="<?php _e('Save changes') ?>" />
      </p>

      <?php } else { // If API key is not validated ?>

      </table>
      <p class="submit ids-view-settings-message">
        <?php _e('Please validate your IDS API key in order to continue.'); ?>
        <input name="idsview_options[submit_validate]" type="submit" class="button-primary" value="<?php _e('Validate') ?>" />
      </p>
      <?php } ?>
		</form>
  <?php
}

// Validate input and the API key.
function idsview_validate_options($input) {

  //print_r($input);

  $idsapi = new IdsApiWrapper;
  $error = FALSE;
  $error_message = '';
  $idsview_api_key_validated = idsapi_variable_get('idsview', 'api_key_validated', FALSE);
  $idsview_default_dataset = idsapi_variable_get('idsview', 'default_dataset', IDS_API_DEFAULT_DATASET);
  $reg_api_key = '/^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$/';
  $reg_api_url = '/^http:\/\/[\/0-9a-z\.]+$/';
  //$reg_api_menu_title = '/^[0-9a-zA-Z_\ ]+$/';
  $allowed_html = array();

  $current_permalinks = get_option('permalink_structure');
  $input['permalink_structure'] = idsapi_variable_get('idsview', 'permalink_structure', $current_permalinks);
  $input['eldisview_documents'] = idsapi_variable_get('idsview', 'eldisview_documents', 0);
  $input['eldisview_organisations'] = idsapi_variable_get('idsview', 'eldisview_organisations', 0);
  $input['bridgeview_documents'] = idsapi_variable_get('idsview', 'bridgeview_documents', 0);

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
  if ($idsview_api_key_validated) { //If the API key was not validated, the other fields have not been displayed, so there's nothing else to check.
    if (isset($input['excerpt_length'])) {
      if (!is_numeric($input['excerpt_length'])) {
        $error = TRUE;
        $error_message .= ' ' . $input['excerpt_length'] . __(' is not a valid length. Please enter a numeric value.');
        $input['excerpt_length'] = 0;
      }
    }
    if (isset($input['exclude_uris'])) {
      $input['exclude_uris'] = wp_kses($input['exclude_uris'], $allowed_html);
    }
    if (isset($input['view_authors_assets'])) {
      $input['view_authors_assets'] = wp_kses($input['view_authors_assets'], $allowed_html);
    }
    if (isset($input['view_publishers_assets'])) {
      $input['view_publishers_assets'] = wp_kses($input['view_publishers_assets'], $allowed_html);
    }
    if (isset($input['view_query'])) {
      $input['view_query'] = wp_kses($input['view_query'], $allowed_html);
    }
  }
  if ($error) {
    $input['setup_done'] = FALSE;
    add_settings_error('idsview_options', 'idsview_errors', $error_message);
  }
  else {
    $input['setup_done'] = TRUE;
  }
  return $input;
}
