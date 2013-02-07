<?php

require_once('idsplugins.html.inc');

// Plugin's page from where to access all its services
function idsimport_general_page() {
  global $ids_categories;
  global $ids_assets;
  $datasets = idsimport_display_datasets('admin');
  $idsapi = new IdsApiWrapper;
  ?>
  <div class="wrap">
  <div id="icon-edit-pages" class="icon32"><br /></div>
  <div id="ids-logo"><a href="http://api.ids.ac.uk" target="_blank"><img src="http://api.ids.ac.uk/docs/wp-content/uploads/2012/01/KS_powered.gif"></a></div>
  <h2><?php _e('IDS Import Plugin'); ?></h2>
  <br /><br />
  <ul class="ids-category-list">
    <li class="cat-item"> <p><a href="<?php echo get_admin_url() . 'options-general.php?page=idsimport'; ?>"><?php _e('Settings'); ?></a></p>
    <li class="cat-item"> <p><a href="<?php echo get_admin_url() . 'admin.php?page=idsimport_help'; ?>"><?php _e('Help'); ?></a></p>
  </ul>

  <table>
    <tr valign="top">
    <td class="idsimport-admin-col1">

    <b><?php _e('Import content'); ?></b>
    <ul class="ids-category-list">
    <li class="cat-item"> <p><a href="<?php echo get_admin_url() . 'admin.php?import=idsimport_importer'; ?>"><?php _e('IDS Importer'); ?></a></p>
    </ul>

    <br/>
    <br/>

    <b><?php _e('View the imported items on your website'); ?></b>
    <ul class="ids-category-list">
    <?php if (in_array('eldis', $datasets) && in_array('bridge', $datasets)) { ?>
    <li class="cat-item"> <p><a href="<?php echo get_post_type_archive_link('ids_documents'); ?>"><?php _e('View all imported documents'); ?></a></p>
    <li class="cat-item"> <p><a href="<?php echo get_post_type_archive_link('ids_organisations'); ?>"><?php _e('View all imported organisations'); ?></a></p>
    <?php } ?>

    <?php
    foreach ($datasets as $dataset) {
      foreach ($ids_assets as $asset) {
        if (!idsapi_exclude($dataset, $asset)) {
          $name = ucfirst($dataset) . ' ' . ucfirst($asset); 
          $link = get_post_type_archive_link('ids_' . $asset);
          $par = ((preg_match('[\?]', $link)) ? '&' : '?'); ?>
          <li class="cat-item"> <p><a href="<?php echo $link . $par . 'ids_site=' . $dataset; ?>"><?php _e('View'); ?> <?php echo $name; ?></a></p>
      <?php }
      }
    } ?>
      </ul>
    </td>

    <td class="idsimport-admin-col2">
    <b><?php _e('Edit imported assets'); ?></b>
    <ul class="ids-category-list">
    <?php
    foreach ($datasets as $dataset) {
      foreach ($ids_assets as $asset) {
        if (!idsapi_exclude($dataset, $asset)) {
          $name = ucfirst($dataset) . ' ' . ucfirst($asset); ?>
          <li class="cat-item"> <p><a href="<?php echo get_admin_url() . 'edit.php?post_type=ids_' . $asset; ?>&ids_site=<?php echo $dataset; ?>"><?php _e('Edit'); ?> <?php echo $name; ?></a></p>
      <?php }
      }
    } ?>
    </ul>

    <br/>
    <br/>

    <?php
    $idsimport_new_categories = idsapi_variable_get('idsimport', 'import_new_categories', IDS_IMPORT_NEW_CATEGORIES);
    if ($idsimport_new_categories) { ?>
      <b><?php _e('Manage imported categories'); ?></b>
      <ul class="ids-category-list">
      <?php
      foreach ($datasets as $dataset) {
        foreach ($ids_categories as $category) {
          $cat_admin_page = 'admin.php?page=' . $dataset . '_' . $category;
          $cat_name = ucfirst($dataset) . ' ' . ucfirst($category); ?>
          <li class="cat-item"> <p><a href="<?php echo get_admin_url() . $cat_admin_page; ?>"><?php _e('View / Edit'); ?> <?php echo $cat_name; ?></a></p>
      <?php } ?>
      <?php } ?>
    <?php } ?>
      </ul>
    </td>
    
    <td class="idsimport-admin-col3">
    <b><?php _e('Administrative tasks'); ?></b>
    <br/>
    <br/>
    <i><?php _e('Cache management'); ?></i>
    <ul class="ids-category-list">
    <?php
    $task = '';
    if(isset($_GET['admin_task'])) {
      $task = $_GET['admin_task'];
    }
    if ($task == 'clear_cache') {
      $idsapi->cacheFlush();
      echo '<li class="cat-item">' . '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;' . __('All cache entries have been deleted.');
    }
    else { ?>
      <li class="cat-item"> <a href="<?php echo get_admin_url() . 'admin.php?page=idsimport_menu&admin_task=clear_cache'; ?>"><?php _e('Clear all cache data'); ?></a>
    <?php } ?>
    </ul>

    <?php
    $site = '';
    $category_type = '';
    $asset_type = '';
    ?>
    <br/>
    <i><?php _e('Bulk processing'); ?></i>
    <br/>
    <br/>

    <?php _e('Assets'); ?>
    <ul class="ids-category-list">
    <?php if ($task == 'unpublish_assets') {
      if (isset($_GET['site']) && isset($_GET['asset_type'])) {
        $site = $_GET['site'];
        $asset_type = $_GET['asset_type'];
        if (idsimport_unpublish_assets('ids_' . $asset_type, $site)) {
          echo '<li class="cat-item">' . '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp' . __('All ') . ucfirst($site) . ' ' . $asset_type . __(' have been marked as "pending".');
        }
      }
    }
    foreach ($datasets as $dataset) { 
      foreach ($ids_assets as $asset) {
        if (!idsapi_exclude($dataset, $asset)) {
          if (($task != 'unpublish_assets') || ($dataset != $site) || ($asset != $asset_type)) { ?>
            <li class="cat-item"> <a href="<?php echo get_admin_url() . 'admin.php?page=idsimport_menu&admin_task=unpublish_assets&site=' . $dataset . '&asset_type=' . $asset; ?>"><?php echo __('Unpublish ') . ucfirst($dataset) . ' ' . $asset; ?></a>  <i>(<?php _e('Change the status to "pending".'); ?>)</i>
          <?php }
        } 
      }
    } ?>
    </ul>

    <?php _e('Categories'); ?>
    <ul class="ids-category-list">
    <?php if ($task == 'delete_terms') {
      if (isset($_GET['site']) && isset($_GET['category_type'])) {
        $site = $_GET['site'];
        $category_type = $_GET['category_type'];
        if (idsimport_delete_taxonomy_terms($site . '_' . $category_type)) {
          echo '<li class="cat-item">' . '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp' . __('All ') . ucfirst($site) . ' ' . $category_type . __(' have been deleted.');
        }
      }
    }
    foreach ($datasets as $dataset) { 
      foreach ($ids_categories as $category) {
        if (($task != 'delete_terms') || ($dataset != $site) || ($category != $category_type)) { ?>
          <li class="cat-item"> <a href="<?php echo get_admin_url() . 'admin.php?page=idsimport_menu&admin_task=delete_terms&site=' . $dataset . '&category_type=' . $category; ?>"><?php echo __('Delete ') . ucfirst($dataset) . ' ' . $category; ?></a> <i>(<?php _e('This action cannot be undone.'); ?>)</i>
        <?php } ?>
      <?php }
    } ?>
    </ul>
    </td>
    </tr>
  </table>
  </div>
<?php
}

// Importer main page
function idsimport_importer_form() {
  $idsapi = new IdsApiWrapper;
  global $ids_datasets;
  global $ids_categories;

  $idsimport_new_categories = idsapi_variable_get('idsimport', 'import_new_categories', IDS_IMPORT_NEW_CATEGORIES);
  $idsimport_map_categories = idsapi_variable_get('idsimport', 'import_map_categories', IDS_IMPORT_MAP_CATEGORIES);
  $default_dataset = idsapi_variable_get('idsimport', 'default_dataset', IDS_IMPORT_DEFAULT_DATASET_ADMIN);
  $api_key = idsapi_variable_get('idsimport', 'api_key', '');

  /* Display latest imported content from each dataset */
  $latest_import_dates = idsapi_variable_get('idsimport', 'import_dates', array('eldis' => array(), 'bridge' => array()));
  if (!empty($latest_import_dates)) {
    $message = __('Latest imports: ') . '<br />';
    $latest_import_results = idsapi_variable_get('idsimport', 'import_results', array('eldis' => array(), 'bridge' => array()));
    foreach ($latest_import_dates as $set => $results_dataset) {
      foreach ($results_dataset as $type => $date) {
        if (!idsapi_exclude($set, $type)) {
          $new = (isset($latest_import_results[$set][$type]) ? $latest_import_results[$set][$type]['new'] : 0);
          $updated = (isset($latest_import_results[$set][$type]) ? $latest_import_results[$set][$type]['updated'] : 0);
          $message .= ucfirst($set) . ' ' . ucfirst(__($type)) . ' imported on ' . $date . ': ' . $new . ' new, ' . $updated . ' updated' . '<br />';
        }
      }
    }
    ids_show_message($message);
  }
  $message = __('Please note that it can take some time to import a large volume of content. Do not stop your browser.');
  ids_show_message($message);
  ?>
  <form method="post" action="admin.php?import=idsimport_importer&step=1">
    <table class="form-table ids-importer-types">
      <!-- Types of content to import -->
      <tr valign="top">
        <th scope="row"><?php _e('Select the types of content to import'); ?></th>
        <td>
          <label><strong><?php _e('Assets'); ?></strong></label><br/>
          <label><input name="idsimport_import_documents" type="checkbox" /> <?php _e('Documents'); ?></label> &nbsp;&nbsp;&nbsp;
          <?php //TODO. Generalize
          if ($default_dataset !== 'bridge')  { ?>
            <label><input name="idsimport_import_organisations" type="checkbox" /> <?php _e('Organisations'); ?></label>
          <?php } ?>
          <br/>

          <?php if ($idsimport_new_categories) { ?>
          <br/>
          <label><strong><?php _e('Categories'); ?></strong></label><br/>
          <label><input name="idsimport_import_countries" type="checkbox" /> <?php _e('Countries'); ?></label> &nbsp;&nbsp;&nbsp;
          <label><input name="idsimport_import_regions" type="checkbox" /> <?php _e('Regions'); ?></label> &nbsp;&nbsp;&nbsp;
          <label><input name="idsimport_import_themes" type="checkbox" /> <?php _e('Themes'); ?></label>
          <?php } ?>
        </td>
      </tr>
    </table>
    <?php wp_nonce_field('idsimport_import', 'idsimport_import_check_referrer'); ?>
    <p class="submit">
      <input name="submit_import" type="submit" class="button-primary" value="<?php _e('Import now') ?>" />
    </p>
  </form>

  <table class="form-table">
    <tr valign="top">
      <td colspan="2"> 
        <strong><?php _e('Active settings and filters'); ?></strong><br />
        (<?php printf(__('Please go to the <a href="%s">IDS API administrative page</a> in order to view the details or change these settings.'), 'options-general.php?page=idsimport'); ?>)
      </td>
    </tr>
    <tr valign="top">
      <th scope="row"><?php _e('General settings'); ?></th>
      <td>
        <ul>
          <li> <?php echo '<i>' . __('Import from: ') . '</i>' . '<strong>' . (($default_dataset == 'both') ? __('Both Eldis and Bridge') : ucfirst($default_dataset)) . '</strong>'; ?>
          <li> <?php echo '<i>' . __('Number of documents/organisations: ') . '</i>' . '<strong>' . idsapi_variable_get('idsimport', 'num_items', IDS_API_NUM_ITEMS) . '</strong>'; ?>
          <li> <?php echo '<i>' . __('Username assigned to imported content: ') . '</i>' . '<strong>' . idsapi_variable_get('idsimport', 'import_user', IDS_IMPORT_IMPORT_USER) . '</strong>'; ?>
          <li> <?php echo '<i>' . __('Levels of categories to import: ') . '</i>' . '<strong>' . (($idsimport_level_categories = idsapi_variable_get('idsimport', 'level_categories', IDS_IMPORT_CATEGORY_LEVELS)) ? $idsimport_level_categories : 'All levels') . '</strong>' ?>
        </ul>
      </td>
    </tr>
    <tr valign="top">
      <th scope="row"><?php _e('Categories'); ?></th>
      <td>
        <ul>
        <?php
        if ($idsimport_new_categories) {
          echo '<li><i>' . __('IDS categories will be imported and assigned to documents and/or organisations.') . '</i>';
        }
        else {
          echo '<li><i>' . __('Importing of IDS categories <u>is disabled</u>.') . '</i>';
        }
        if ($idsimport_map_categories) {
          echo '<li><i>' . __('IDS categories will be mapped to existing Wordpress categories.') . '</i>';
        }
        else {
          echo '<li><i>' . __('Mapping of IDS categories to Wordpress categories <u>is disabled</u>.') . '</i>';
        }
        if (!($idsimport_new_categories || $idsimport_map_categories)) {
          echo '<li><i>' . __('<u>Imported documents and organisations will be filed as "Uncategorized"</u>.') . '</i>';
        }
        ?>
        </ul>
      </td>
    </tr>

    <tr valign="top">
      <th scope="row"><?php _e('Active filters for documents and/or organisations'); ?></th>
      <td>
        <?php
        foreach ($ids_datasets as $dataset) {
          if (($default_dataset == $dataset) || ($default_dataset == 'both')) {
            $array_countries = $idsapi->search('countries', $dataset, $api_key)->getArrayTitles();
            $array_regions = $idsapi->search('regions', $dataset, $api_key)->getArrayTitles();
            $array_themes = $idsapi->search('themes', $dataset, $api_key)->getArrayTitles();
            echo '<b><u>' . ucfirst($dataset) . ' categories</u></b>';
        ?>
        <ul>
          <li> <?php echo '<i>' . __('Countries: ') . '</i>' . '<strong>' . (($idsimport_import_countries_assets = implode(', ', idsimport_get_names(idsapi_variable_get('idsimport', $dataset . '_countries_assets', array()), $array_countries))) ? $idsimport_import_countries_assets : '<i>none selected</i>') . '</strong>' ?>
          <li> <?php echo '<i>' . __('Regions: ') . '</i>' . '<strong>' . (($idsimport_import_regions_assets = implode(', ', idsimport_get_names(idsapi_variable_get('idsimport', $dataset . '_regions_assets', array()), $array_regions))) ? $idsimport_import_regions_assets : '<i>none selected</i>') . '</strong>' ?>
          <li> <?php echo '<i>' . __('Themes: ') . '</i>' . '<strong>' . (($idsimport_import_themes_assets = implode(', ', idsimport_get_names(idsapi_variable_get('idsimport', $dataset . '_themes_assets', array()), $array_themes))) ? $idsimport_import_themes_assets : '<i>none selected</i>') . '</strong>' ?>
        </ul>
          <?php } ?>
        <?php } ?>
        <b><u><?php _e('Additional filters'); ?></u></b>
        <ul>
          <li> <?php echo '<i>' . __('Age of documents and/or organisations: ') . '</i>' . '<strong>' . (($idsimport_age_new_assets = idsapi_variable_get('idsimport', 'age_new_assets', IDS_API_AGE_NEW_ASSETS)) ? $idsimport_age_new_assets . ' days' : '<i>Do not filter by age</i>') . '</strong>' ?>
          <li> <?php echo '<i>' . __('Documents\' author(s): ') . '</i>' . '<strong>' . (($idsimport_import_authors_assets = idsapi_variable_get('idsimport', 'import_authors_assets', '')) ? $idsimport_import_authors_assets : '<i>none selected</i>') . '</strong>' ?>
          <li> <?php echo '<i>' . __('Documents\' publisher(s): ') . '</i>' . '<strong>' . (($idsimport_import_publishers_assets = idsapi_variable_get('idsimport', 'import_publishers_assets', '')) ? $idsimport_import_publishers_assets : '<i>none selected</i>') . '</strong>' ?>
          <li> <?php echo '<i>' . __('Additional query terms: ') . '</i>' . '<strong>' . (($idsimport_import_query = idsapi_variable_get('idsimport', 'import_query', '')) ? $idsimport_import_query : '<i>none selected</i>') . '</strong>' ?>
        </ul>
      </td>
    </tr>
  </table>
<?php
}

// Render the plugin's main admin form.
function idsimport_render_form() {
  global $ids_datasets;
  global $ids_categories;

  $idsimport_api_key_validated = idsapi_variable_get('idsimport', 'api_key_validated', FALSE);
	?>
	<div class="wrap">
		<!-- Display Plugin Icon, Header, and Description -->
		<div class="icon32" id="icon-options-general"></div>
		<h2><?php _e('IDS Import Plugin Settings'); ?></h2>
		<p>
      <?php printf(__('This plugin provides access to the Eldis and BRIDGE datasets via the IDS Knowledge Services API (<a href="%s" target="_new">%s</a>).'), IDS_API_HOME_URL, IDS_API_HOME_URL); ?>
      <br />
      <br />
      <?php _e('BRIDGE is a research and information service supporting gender advocacy and mainstreaming efforts by bridging the gaps between theory, policy and practice.'); ?>
      <br />
      <?php _e('Eldis is an online information service covering development research on a wide change of thematic areas including agriculture, climate change, conflict, gender, governance, and health.'); ?>
      <br />
      <br />
      <i><?php _e('Please look through all the tabs on this page to set your configuration options. Changes will only be effective after submitting them by clicking on the "Save changes" button.</i>'); ?>
    </p>

    <!-- Beginning of the Plugin Options Form -->
		<form method="post" id="ids_import_form" action="options.php" onSubmit="selectAllMappings()">
      <?php
          settings_fields('idsimport');
      ?>

    <div class="ui-tabs">
      <ul class="ui-tabs-nav">
        <li><a href="#general"><?php _e('General settings'); ?></a></li>
        <li><a href="#import"><?php _e('Import settings'); ?></a></li>
        <li><a href="#filters"><?php _e('Import filters'); ?></a></li>
        <li><a href="#display"><?php _e('Display settings'); ?></a></li>
        <li><a href="#categories"><?php _e('Category settings'); ?></a></li>
        <li><a href="#periodic_import"><?php _e('Periodic imports'); ?></a></li>
      </ul>

      <!---------------------------------- GENERAL SETTINGS ------------------------------->

      <h3><?php _e('General settings'); ?></h3>

			<table class="form-table ui-tabs-panel" id="general">
				<!-- IDS API Key -->
				<tr>
					<th scope="row"><?php _e('IDS API Token-GUID (key)'); ?></th>
					<td>
						<input type="text" size="57" name="idsimport_options[api_key]" value="<?php echo idsapi_variable_get('idsimport', 'api_key', ''); ?>" /><br />
						<p class="description">
              <?php printf(__('If you do not have an IDS API key, please request one at <a href="%s" target="_new">%s</a>'), IDS_API_KEY_URL, IDS_API_KEY_URL); ?>
            </p>
					</td>
				</tr>

        <!-- IDS API URL --
        <tr>
          <th scope="row"><?php _e('IDS API URL'); ?></th>
          <td>
            <input type="text" size="57" name="idsimport_options[api_url]" value="<?php echo idsapi_variable_get('idsimport', 'api_url', IDS_API_URL); ?>" /><br />
            <p class="description"><?php _e('In general, you would not need to change this.'); ?></p>
          </td>
        </tr>
        -->

      <?php if ($idsimport_api_key_validated) { ?>
      </table>

      <!---------------------------- GENERAL IMPORT SETTINGS ------------------------------>

      <h3><?php _e('General import settings'); ?></h3>
      <table class="form-table ui-tabs-panel" id="import">

        <!-- Dataset -->
        <tr valign="top">
          <th scope="row"><?php _e('Datasets to import from'); ?></th>
          <td>
            <label><input name="idsimport_options[default_dataset]" type="radio" value="eldis" <?php checked('eldis', idsapi_variable_get('idsimport', 'default_dataset', IDS_IMPORT_DEFAULT_DATASET_ADMIN)); ?> onchange="changeDataset('eldis');" /> Eldis only</label> <br />
            <label><input name="idsimport_options[default_dataset]" type="radio" value="bridge" <?php checked('bridge', idsapi_variable_get('idsimport', 'default_dataset', IDS_IMPORT_DEFAULT_DATASET_ADMIN)); ?> onchange="changeDataset('bridge');" /> Bridge only</label><br />
            <label><input name="idsimport_options[default_dataset]" type="radio" value="both" <?php checked('both', idsapi_variable_get('idsimport', 'default_dataset', IDS_IMPORT_DEFAULT_DATASET_ADMIN)); ?> onchange="changeDataset('both');" /> Both Eldis and Bridge</label><br />
          </td>
        </tr>

        <!-- Number of assets to import -->
        <tr>
          <th scope="row"><?php _e('Number of documents/organisations'); ?></th>
          <td>
            <input type="text" size="10" name="idsimport_options[num_items]" value="<?php echo idsapi_variable_get('idsimport', 'num_items', IDS_API_NUM_ITEMS); ?>" />
            <p class="description">
            <?php _e('Number of documents and/or organisations to retrieve <u>from each selected dataset</u> in each call to the IDS API.'); ?>
            <br/>
            <?php _e('If not set (or if the value is 0) all available items will be imported.'); ?>
            </p>
          </td>
        </tr>

        <!-- Publication date -->
        <tr>
          <th scope="row"><?php _e('Publication date'); ?></th>
          <td>
            <?php echo ids_select_box('idsimport_options[pub_date]', 'idsimport_pub_date', array('default' => __('Import date'), 'date_created' => __('Date created (IDS API)'), 'date_updated' => __('Date updated (IDS API)')), idsapi_variable_get('idsimport', 'pub_date', 'default')); ?>
            <p class="description">
            <?php _e('Date of publication of imported documents and/or organisations.'); ?>
            <br />
            <?php _e('If selected, the date_created or date_updated fields retrieved by the API will be used, instead of the date in which they were imported into Wordpress.'); ?>
            </p>
          </td>
        </tr>

        <!-- Username (author of posts) -->
        <?php
        // We only allow to create a new user once. Then, it has to be selected from existing users.
        if ($user = get_user_by('email', IDS_IMPORT_USER_EMAIL)) {
          $new_user_allowed = FALSE;
          $hide_field = 'style="display: none"';
        }
        else {
          $new_user_allowed = TRUE;
          $hide_field = '';
        }
        ?>
          <tr>
            <th scope="row"><?php _e('Username'); ?></th>
            <td>
              <input id="ids_import_user" type="text" size="20" name="idsimport_options[import_user]" value="<?php echo idsapi_variable_get('idsimport', 'import_user', IDS_IMPORT_IMPORT_USER); ?>" <?php echo $hide_field; ?>/>
              <?php if ($new_user_allowed) { ?>
                <p class="description">
                <?php _e('Enter a new username or select an existing one: '); ?>
              <?php } ?>
              <?php wp_dropdown_users(array('id' => 'ids_import_user_select', 'name' => 'idsimport_options[import_user_id]', 'selected' => idsapi_variable_get('idsimport', 'import_user_id', -1), 'show_option_none' => '-- Username --'));
              ?>
              <br/>
              <p class="description">
              <?php _e('Imported IDS content will be assigned to this user.'); ?>
              </p>
              </p>
            </td>
          </tr>

        <!-- Import tags (IDS keywords) -->
        <tr valign="top">
          <th scope="row"><?php _e('Import IDS keywords'); ?></th>
          <td>
            <?php $import_tags = idsapi_variable_get('idsimport', 'import_tags', IDS_IMPORT_TAGS); ?>
            <label><input name="idsimport_options[import_tags]" type="checkbox" <?php  echo ($import_tags ? 'checked="yes"' : ''); ?> /> <?php _e('Include keywords as tags'); ?></label>
            <p class="description"><?php _e('Please indicate if you would like to include IDS keywords as tags of the imported documents and/or organisations.'); ?></p>
          </td>
        </tr>
      </table>

      <!---------------------------------------- FILTERS ---------------------------------->

      <h3><?php _e('Import filters'); ?></h3>
      <table class="form-table ui-tabs-panel" id="filters">
        <tr>
          <td colspan="2">
            <p class="description"><?php _e('Documents and/or organisations to be imported can be limited to those matching the following filters.'); ?></p>
          </td>
        </tr>

        <!-- IDS Categories (countries, regions, themes) -->
        <?php foreach ($ids_datasets as $dataset) { ?>
        <tr class="ids-categories-<?php echo $dataset; ?>">
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
            <select data-placeholder="Select <?php echo $category; ?>..." class="chzn-select" style="width:350px;" id='<?php echo $field_id; ?>' name='idsimport_options[<?php echo $option_name; ?>][]' multiple='multiple' size='10'></select>
            <p class="description"><?php _e("Import only documents/organisations with the selected focus $category."); ?>
            <a href="javascript:selectAll('<?php echo $field_id; ?>');"><?php _e('Select all'); ?></a> / 
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
            <select data-placeholder="No themes selected" class="chzn-select" style="width:350px;" id='<?php echo $field_id; ?>' name='idsimport_options[<?php echo $option_name; ?>][]' multiple='multiple' size='10'></select><br>
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
        <tr>
        <?php } // foreach dataset ?>

          <td colspan="2">
            <p class="ids-settings-separator">Additional filters</p>
          </td>
        </tr>

        <!-- Age of assets to import -->
        <tr>
         <th scope="row"><?php _e('Age of documents and/or organisations'); ?></th>
          <td>
            <?php echo ids_select_box('idsimport_options[age_new_assets]', 'idsimport_age_new_assets', array('0' => __('Do not filter by age'), '7' => __('One week'), '30' => __('One month'), '180' => __('Six months'), '365' => __('One year')), idsapi_variable_get('idsimport', 'age_new_assets', IDS_API_AGE_NEW_ASSETS)); ?>
            <p class="description"><?php _e('Only documents/organisations published after the indicated time will be imported.'); ?></p>
          </td>
        </tr>

        <!-- Authors -->
        <tr>
          <th scope="row"><?php _e('Documents\' author(s)'); ?></th>
          <td>
            <input type="text" size="57" name="idsimport_options[import_authors_assets]" value="<?php echo idsapi_variable_get('idsimport', 'import_authors_assets', ''); ?>" /><br />
            <p class="description">
            <?php _e('A list of comma-separated authors\' names.'); ?><br />
            <?php _e('Example: <i>Chomsky, Foucault</i> will retrieve documents with either <i>Chomsky</i> or <i>Foucault</i> in the "author" field.'); ?></p>
          </td>
        </tr>

        <!-- Publishers -->
        <tr>
          <th scope="row"><?php _e('Documents\' publisher(s)'); ?></th>
          <td>
            <input type="text" size="57" name="idsimport_options[import_publishers_assets]" value="<?php echo idsapi_variable_get('idsimport', 'import_publishers_assets', ''); ?>" /><br />
            <p class="description">
            <?php _e('A list of comma-separated publishers\' names.'); ?><br />
            <?php _e('Example: <i>World Bank, UNDP</i> will retrieve documents with either <i>World Bank</i> or <i>UNDP</i> in the "publisher" field.'); ?></p>
          </td>
        </tr>

        <!-- Additional query terms -->
        <tr>
          <th scope="row"><?php _e('Query terms'); ?></th>
          <td>
            <input type="text" size="57" name="idsimport_options[import_query]" value="<?php echo idsapi_variable_get('idsimport', 'import_query', ''); ?>" /><br />
            <p class="description">
            <?php _e('Query string to be appended to the API URL.'); ?><br />
            <?php _e('Use comma-separated pairs of unencoded query fields and values (example: <i>country=Argentina, keyword=climate change, language_name=Spanish</i>).'); ?><br />
            <?php printf(__('Please check the <a href="%s" target="_new">online documentation</a> for a list of available query fields.'), IDS_API_SEARCH_FIELDS_URL); ?>
           </p>
          </td>
        </tr>
      </table>

      <!---------------------------- DISPLAY SETTINGS ------------------------------>

      <h3><?php _e('General import settings'); ?></h3>
      <table class="form-table ui-tabs-panel" id="display">

        <!-- Include imported content in the loop -->
        <tr valign="top">
          <th scope="row"><?php _e('Display imported content with regular posts'); ?></th>
          <td>
            <?php $include_imported_documents = idsapi_variable_get('idsimport', 'include_imported_documents', IDS_IMPORT_INCLUDE_IMPORTED_DOCUMENTS); ?>
            <label><input name="idsimport_options[include_imported_documents]" type="checkbox" <?php  echo ($include_imported_documents ? 'checked="yes"' : ''); ?>" /> <?php _e('Include imported documents'); ?></label>
            <br/>
            <?php $include_imported_organisations = idsapi_variable_get('idsimport', 'include_imported_organisations', IDS_IMPORT_INCLUDE_IMPORTED_ORGANISATIONS); ?>
            <label><input name="idsimport_options[include_imported_organisations]" type="checkbox" <?php  echo ($include_imported_organisations ? 'checked="yes"' : ''); ?>" /> <?php _e('Include imported organisations'); ?></label>

            <p class="description">
            <?php _e('Please indicate if you would like to include the imported IDS documents / organisations when displaying regular posts.'); ?><br>
            <?php _e('If these options are not checked, IDS documents / organisations will not be displayed on your site by default, but they will still be accesible through specific URLs, available on:'); ?><br>
            <a href="<?php echo get_admin_url(); ?>/admin.php?page=idsimport_menu"><?php echo get_admin_url(); ?>/admin.php?page=idsimport_menu</a>
            </p>
          </td>
        </tr>

        <!-- Dataset in public areas -->
        <tr valign="top">
          <th scope="row"><?php _e('Datasets in public areas'); ?></th>
          <td>
            <label><input name="idsimport_options[display_dataset_public]" type="radio" value="eldis" <?php checked('eldis', idsapi_variable_get('idsimport', 'display_dataset_public', IDS_IMPORT_DEFAULT_DATASET_PUBLIC)); ?> /> Eldis only</label> <br />
            <label><input name="idsimport_options[display_dataset_public]" type="radio" value="bridge" <?php checked('bridge', idsapi_variable_get('idsimport', 'display_dataset_public', IDS_IMPORT_DEFAULT_DATASET_PUBLIC)); ?> /> Bridge only</label><br />
            <label><input name="idsimport_options[display_dataset_public]" type="radio" value="both" <?php checked('both', idsapi_variable_get('idsimport', 'display_dataset_public', IDS_IMPORT_DEFAULT_DATASET_PUBLIC)); ?> /> Both Eldis and Bridge</label><br />
            <p class="description">
            <?php _e('Content from the indicated datasets will be marked as "published" when imported. If a dataset is not selected, its content will be imported with "pending" status.'); ?><br>
            <?php _e('Please note that this applies to new content. The status of content that has already been imported can be changed in the plugin\'s administrative area.'); ?>
            </p>
          </td>
        </tr>

        <!-- Dataset in admin areas -->
        <tr valign="top">
          <th scope="row"><?php _e('Datasets in administrative areas'); ?></th>
          <td>
            <label><input name="idsimport_options[display_dataset_admin]" type="radio" value="eldis" <?php checked('eldis', idsapi_variable_get('idsimport', 'display_dataset_admin', IDS_IMPORT_DEFAULT_DATASET_ADMIN)); ?> /> Eldis only</label> <br />
            <label><input name="idsimport_options[display_dataset_admin]" type="radio" value="bridge" <?php checked('bridge', idsapi_variable_get('idsimport', 'display_dataset_admin', IDS_IMPORT_DEFAULT_DATASET_ADMIN)); ?> /> Bridge only</label><br />
            <label><input name="idsimport_options[display_dataset_admin]" type="radio" value="both" <?php checked('both', idsapi_variable_get('idsimport', 'display_dataset_admin', IDS_IMPORT_DEFAULT_DATASET_ADMIN)); ?> /> Both Eldis and Bridge</label><br />
            <p class="description"><?php _e('Please indicate the datasets to display in the administrative areas of your site.'); ?></p>
          </td>
        </tr>
      </table>

      <!------------------------------- CATEGORIES SETTINGS ------------------------>

      <h3><?php _e('Categories settings'); ?></h3>

      <table class="form-table ui-tabs-panel" id="categories">
        <tr>
          <td colspan="4">
            <p class="description">
            <?php _e('Please indicate if you want to import and use IDS categories (countries, regions, themes) and if IDS categories associated to documents/organisations should be mapped to existing WordPress categories.'); ?><br>
            </p>
        <?php $idsimport_new_categories = idsapi_variable_get('idsimport', 'import_new_categories', IDS_IMPORT_NEW_CATEGORIES); ?>
        <table class="form-table">
        <tr valign="top">
          <th scope="row"><?php _e('Categories settings'); ?></th>
          <td>
            <label><input id="ids_new_categories" name="idsimport_options[import_new_categories]" type="checkbox" <?php echo ($idsimport_new_categories ? 'checked="yes"' : '');  ?> onchange="changeNewCategories();" /> Enable IDS categories </label>
            <p class="description">
            <?php _e('If checked, IDS countries, themes and regions will be imported and be made available on Wordpress as terms.'); ?><br>
            <?php _e('They will also be automatically assigned to imported documents and organisations.'); ?><br>
            <?php _e('If this option is unchecked, IDS countries, themes and regions will not be imported and <u>will not be available on Wordpress</u>.'); ?>
            </p>
          </td>
        </tr>

        <!-- Category levels to import -->
        <tr valign="top" class="ids-categories-new">
          <th scope="row"><?php _e('Level of categories'); ?></th>
          <td>
          <label><?php echo ids_select_box('idsimport_options[level_categories]', 'idsimport_level_categories', array(1 => __('One level'), 2 => __('Two levels'), 0 => __('All levels')), idsapi_variable_get('idsimport', 'level_categories', IDS_IMPORT_CATEGORY_LEVELS)); ?></label>
          <p class="description">
          <?php _e('The level shows the position in the thematic hierarchy, starting at 1, the top level. Select the category levels to import.'); ?>
          </p>
          </td>
        </tr>

         <?php $idsimport_map_categories = idsapi_variable_get('idsimport', 'import_map_categories', IDS_IMPORT_MAP_CATEGORIES); ?>
        <tr valign="top">
          <th scope="row"><?php _e('Category mappings'); ?></th>
          <td>
            <label><input id="ids_map_categories" name="idsimport_options[import_map_categories]" type="checkbox" <?php echo ($idsimport_map_categories ? 'checked="yes"' : ''); ?> onchange="changeMappingsSettings();" /> Map to existing categories </label><br />
            <p class="description">
              <?php _e('Map IDS countries, regions and themes to categories to terms in the default Wordpress \'category\' taxonomy when importing documents and/or organisations.'); ?><br/>
              <?php _e('Please note that this option is independent of the category importing settings: an IDS category can be imported as a new term and, in addition, mapped to an existing category.'); ?></p>
          </td>
        </tr>
        </table>

        <!-- Wordpress default taxonomy -->
        <?php
          $wp_categories_select = array();
          $all_wp_categories = get_categories( array('taxonomy' => 'category', 'hide_empty' => FALSE) );
          foreach ($all_wp_categories as $wp_category) {
            $wp_categories_select[$wp_category->term_id] = $wp_category->name;
          }
        ?>

        <table class="form-table">
        <?php if (!empty($wp_categories_select)) { ?>
          <!-- IDS Categories mappings (countries, regions, themes) -->
          <?php foreach ($ids_datasets as $dataset) { ?>
            <tr valign="top" class="ids-categories-mapping ids-mappings-<?php echo $dataset; ?>">
              <td colspan="4">
                <p id="title_categories" class="ids-settings-separator"><?php echo ucfirst($dataset) . ' mappings'; ?></p>
              </td>
            </tr>
            <?php foreach ($ids_categories as $category) { 
              $field_id_source = $dataset . '_' . $category . '_source';
              $field_id_target = $dataset . '_' . $category . '_target';
              $field_id_mappings = $dataset . '_' . $category . '_mappings';
              $option_name_mappings = $dataset . '_' . $category . '_mappings';
              $category_name = ucfirst($category);
              ?>
              <tr valign="top" class="ids-categories-mapping ids-mappings-section ids-mappings-<?php echo $dataset; ?>">
              <th scope="row"><?php _e($category_name); ?></th>
                <td class="ids-categories-container">
                  <select class="ids-mapping-select" id='<?php echo $field_id_source; ?>' multiple='multiple' size='5'></select>
                    <p class="description"><a href="javascript:selectAll('<?php echo $field_id_source; ?>');"><?php _e('Select all'); ?></a> / <a href="javascript:deselectAll('<?php echo $field_id_source; ?>');"><?php _e('Deselect all'); ?></a></p>
                </td>
                <td class="ids-categories-container">
                  <p class="description"><?php _e("Map selected $category to:"); ?></p>
                  <?php echo ids_select_box($field_id_target, $field_id_target, $wp_categories_select, '', FALSE, 'ids-mapping-select'); ?>
                  <br /><br />
                  <p class="description"><a href="javascript:addMappings('<?php echo $field_id_source; ?>', '<?php echo $field_id_target; ?>', '<?php echo $field_id_mappings; ?>');"><?php _e('Add mapping'); ?> &rarr;</a></p>
                </td>
                <td class="ids-categories-container">
                  <select class="ids-mapping-select" name='idsimport_options[<?php echo $option_name_mappings; ?>][]' id='<?php echo $field_id_mappings; ?>' multiple='multiple' size='5'></select>
                  <p class="description"> <a href="javascript:removeMappings('<?php echo $field_id_mappings; ?>');">&larr; <?php _e('Remove selected'); ?></a> / <a href="javascript:selectAll('<?php echo $field_id_mappings; ?>');"><?php _e('Select all'); ?></a> / <a href="javascript:deselectAll('<?php echo $field_id_mappings; ?>');"><?php _e('Deselect all'); ?></a></p>
                </td>
              </tr>
              <?php } // foreach category ?>
          <?php } // foreach dataset ?>
        <?php } // there are no wordpress categories to map to
        else { ?>
            <tr valign="top" class="ids-categories-mapping ids-mappings-<?php echo $dataset; ?>">
              <td colspan="4">
                <p id="title_categories" class="ids-settings-separator"><?php _e('Please, create the Wordpress categories to map to first.'); ?></p>
              </td>
            </tr>          
        <?php } ?>
        </table>

          </td>
        </tr>
      </table>

      <!------------------------------------ PERIODIC IMPORTS ----------------------------->

      <h3><?php _e('Periodic import'); ?></h3>
      <table class="form-table ui-tabs-panel" id="periodic_import">
        <!-- Types of content to import -->
        <tr valign="top">
          <th scope="row"><?php _e('Select the types of content to import periodically'); ?></th>
          <td>
            <label><strong><?php _e('Assets'); ?></strong></label><br/>
            <label><input id="ids_import_documents_checkbox" name="idsimport_options[import_documents]" type="checkbox" <?php echo (idsapi_variable_get('idsimport', 'import_documents', '0') ? 'checked="yes"' : ''); ?> onchange="changeImportSettings();" /> <?php _e('Documents'); ?></label> &nbsp;&nbsp;&nbsp;
            <label><input id="ids_import_organisations_checkbox" name="idsimport_options[import_organisations]" type="checkbox" <?php  echo (idsapi_variable_get('idsimport', 'import_organisations', '0') ? 'checked="yes"' : ''); ?> onchange="changeImportSettings();" /> <?php _e('Organisations'); ?></label>
            <p class="description"><?php _e('Assets (documents and organisations) are updated frequently.'); ?></p>
          </td>
        </tr>
        <tr class = "ids-categories-new">
          <th scope="row"></th>
          <td>
            <label><strong><?php _e('Categories'); ?></strong></label><br/>
            <label><input id="ids_import_countries_checkbox" name="idsimport_options[import_countries]" type="checkbox" <?php echo (idsapi_variable_get('idsimport', 'import_countries', '0') ? 'checked="yes"' : ''); ?> onchange="changeImportSettings();" /> <?php _e('Countries'); ?></label> &nbsp;&nbsp;&nbsp;
            <label><input id="ids_import_regions_checkbox" name="idsimport_options[import_regions]" type="checkbox" <?php echo (idsapi_variable_get('idsimport', 'import_regions', '0') ? 'checked="yes"' : ''); ?> onchange="changeImportSettings();" /> <?php _e('Regions'); ?></label> &nbsp;&nbsp;&nbsp;
            <label><input id="ids_import_themes_checkbox" name="idsimport_options[import_themes]" type="checkbox" <?php echo (idsapi_variable_get('idsimport', 'import_themes', '0') ? 'checked="yes"' : ''); ?> onchange="changeImportSettings();" /> <?php _e('Themes'); ?></label>
            <p class="description"><?php _e('Categories are updated less frequently and it could be a good alternative to import them manually.'); ?></p>
          </td>
        </tr>

        <!-- Frequency to import -->
        <?php $idsimport_schedules = idsimport_get_schedules(); ?>
        <tr class="ids-assets-import-field">
          <th scope="row"><?php _e('Frequency to import/update documents/organisations'); ?></th>
          <td>
            <?php echo ids_select_box('idsimport_options[import_period_assets]', 'idsimport_import_period_assets', $idsimport_schedules, idsapi_variable_get('idsimport', 'import_period_assets', IDS_IMPORT_IMPORT_RECURRENCE_ASSETS)); ?>
            <p class="description"><?php _e('Please indicate the frequency with which you would like to import or update documents and/or organisations.'); ?></p>
          </td>
        </tr>

        <tr class="ids-categories-import-field ids-categories-new">
          <th scope="row"><?php _e('Frequency to import/update categories'); ?></th>
          <td>
            <?php echo ids_select_box('idsimport_options[import_period_categories]', 'idsimport_import_period_categories', $idsimport_schedules, idsapi_variable_get('idsimport', 'import_period_categories', IDS_IMPORT_IMPORT_RECURRENCE_CATEGORIES)); ?>
            <p class="description"><?php _e('Please indicate the frequency with which you would like to import or update categories (countries, regions and themes).'); ?></p>
          </td>
        </tr>
      </table>

    </div>	<!-- UI tabs -->	

      <p class="submit">
        <input name="idsimport_options[submit_save]" type="submit" class="button-primary" value="<?php _e('Save changes') ?>"/>
      </p>

    <?php } else { // If API key is not validated ?>

      </table>
    </div>	<!-- UI tabs -->	

      <p class="submit ids-import-settings-message">
        <?php _e('Please validate your IDS API key in order to continue setting the plugin options.'); ?>
        <input name="idsimport_options[submit_validate]" type="submit" class="button-primary" value="<?php _e('Validate') ?>" />
      </p>
    <?php } ?>

		</form>
	</div> <!-- wrap -->
	<?php	
}

// Add columns to the posts edit page
function idsimport_posts_columns($defaults) {
  global $ids_categories;
  global $ids_datasets;
  $idsimport_new_categories = idsapi_variable_get('idsimport', 'import_new_categories', IDS_IMPORT_NEW_CATEGORIES);
  $site = '';
  $datasets = $ids_datasets;
  if (isset($_GET['post_type'])) {
    $post_type = $_GET['post_type'];
    if (($post_type == 'ids_documents') || ($post_type == 'ids_organisations')) {
      if(isset($_GET['ids_site'])) {
        $site = $_GET['ids_site'];
      }
      elseif (isset($_GET['eldis_countries']) || isset($_GET['eldis_regions']) || isset($_GET['eldis_themes'])) {
        $site = 'eldis';
      }
      elseif (isset($_GET['bridge_countries']) || isset($_GET['bridge_regions']) || isset($_GET['bridge_themes'])) {
        $site = 'bridge';
      }
      if (($site == 'eldis') || ($site == 'bridge')) {
        $datasets = array($site);
      }
      foreach ($datasets as $dataset) {
        foreach ($ids_categories as $category) {
          $taxonomy_name = $dataset . '_' . $category;
          if (taxonomy_exists($taxonomy_name) && $idsimport_new_categories) {
            $category_name = ucfirst($dataset) . ' ' . ucfirst($category);
            $defaults[$taxonomy_name] = $category_name;
          }
        }
      }
    }
  }
  return $defaults;
}

// Removes the IDS category widgets if they are not enabled in settings.
function idsimport_edit_post_form() {
  global $ids_datasets;
  global $ids_categories;
  $idsimport_new_categories = idsapi_variable_get('idsimport', 'import_new_categories', IDS_IMPORT_NEW_CATEGORIES);
  if (!$idsimport_new_categories) {
    $types = array('post', 'ids_documents', 'ids_organisations');
    foreach ($ids_datasets as $dataset) {
      foreach ($ids_categories as $category) {
        foreach ($types as $type) {
          $custom_taxonomy_slug = $dataset . '_' . $category;
          remove_meta_box($custom_taxonomy_slug.'div', $type, 'side');
        }
      }
    }
  }
}

// Add columns contents in the posts edit page
function idsimport_populate_posts_columns($column){
  global $post;
  $is_ids_category = preg_match('/[eldis][bridge]/', $column);
  if ($is_ids_category) {
    $str = explode('_', $column);
	  $terms = get_the_term_list($post->ID, $column,'',', ','');
    if ($terms) {
      $output = $terms;
    }
    else {
      $output = 'No ' . ucfirst($str[1]);
    }
    echo $output;
  }
}

// Functions to list IDS imported categories.

function idsimport_eldis_countries_page() {
  idsimport_categories_page('eldis', 'countries');
}

function idsimport_eldis_regions_page() {
  idsimport_categories_page('eldis', 'regions');
}

function idsimport_eldis_themes_page() {
  idsimport_categories_page('eldis', 'themes');
}

function idsimport_bridge_countries_page() {
  idsimport_categories_page('bridge', 'countries');
}

function idsimport_bridge_regions_page() {
  idsimport_categories_page('bridge', 'regions');
}

function idsimport_bridge_themes_page() {
  idsimport_categories_page('bridge', 'themes');
}

function idsimport_categories_page($dataset, $category) {
?>
  <div class="wrap">
  <div id="icon-edit-pages" class="icon32 icon32-posts-page"><br /></div>
  <h2><?php echo ucfirst($dataset) . ' ' . ucfirst($category); ?> <a href="edit-tags.php?taxonomy=<?php echo $dataset . '_' . $category; ?>" class="add-new-h2">Add/Edit/Delete <?php echo ucfirst($dataset) . ' ' . ucfirst($category); ?></a> </h2>
  <br/><br/>
  <ul class="ids-category-list">
  <?php
  $taxonomy_name = $dataset . '_' . $category;
  $args = array(
    'hide_empty'         => FALSE,
    'orderby'            => 'name',
    'order'              => 'ASC',
    'hierarchical'       => true,
    'title_li'           => '',
    'show_option_none'   => __("No $category found"),
    'taxonomy'           => $taxonomy_name,
  );
  wp_list_categories($args);
  ?>
  </ul>
<?php
}

// Displays the help page.
function idsimport_help_page() {
?>
  <div class="wrap">
  <div id="icon-edit-comments" class="icon32 icon32-posts-page"><br /></div>
  <h2><?php _e('Help'); ?></h2>

  <p>
  <?php _e('The IDS KS API plugin allows access to IDS Knowledge Services content of thematically organised and hand selected academic research on poverty reduction in the developing world that is freely available to access online.'); ?>
  </p>
  
  <p>
  <?php _e('IDS Knowledge Services offer two collections that can be accessed via this plugin. They are:'); ?>
  </p>
  
  <p>
  <?php _e('BRIDGE is a research and information service supporting gender advocacy and mainstreaming efforts by bridging the gaps between theory, policy and practice. BRIDGE acts as a catalyst by facilitating the generation and exchange of relevant, accessible and diverse gender information in print, online and through other innovative forms of communication. BRIDGE hosts a global resources library on its website, which includes gender-focused information materials in a number of languages, including Arabic, Chinese, English, French, Portuguese and Spanish.'); ?>
  </p>
  
  <p>
  <?php _e('Eldis is an online information service covering development research on a wide change of thematic areas including agriculture, climate change, conflict, gender, governance, and health. Eldis includes over 32,000 summaries and links to free full-text research and policy documents from over 8,000 publishers.  Each document is editorially selected by members of our team.'); ?>
  </p>
  
  <b><?php _e('What content can I get?'); ?></b>
  
  <p>
  <b><?php _e('Documents'); ?></b> <?php _e('are a wide range of online resources, primarily academic research, on development issues that are freely available online.'); ?>
  <br>
  <?php _e('They are editorially selected, organised thematically and are summarised, in clear, non-technical language for easy consumption.'); ?>
  </p>
  
  <p>
  <b><?php _e('Organisations'); ?></b> <?php _e('are a wide range of organisations engaged in reducing poverty. Most of the organisations in our datasets have published documents available in the data.'); ?>
  <br>
  <?php _e('Please note: currently there are no organisations in the BRIDGE dataset. Organisational websites are recorded as "documents".'); ?>
  </p>
  
  <p>
  <b><?php _e('Countries'); ?></b> <?php _e('are used to identify either:'); ?>
  <ul class="ids-category-list">
  <li class="cat-item"> <?php _e('which geographic area the research document covers'); ?>
  <li class="cat-item"> <?php _e('where the research was produced, based on the publisher country'); ?>
  <li class="cat-item"> <?php _e('the countries in which an organisation works'); ?>
  </ul>
  </p>
  
  <p>
  <b><?php _e('Regions'); ?></b> <?php _e('are broad geographic groupings, which enable users to explore our documents and organisations by more than one country.'); ?>
  </p>

  <p>
  <b><?php _e('Themes'); ?></b> <?php _e('are development topics which reflect the key themes in development.'); ?>
  </p>
  
  <b><?php _e('How do I use the plugin?'); ?></b>
  
  <p>
  <?php _e('To use the plugin, you must obtain a unique Token-GUID or key for the API. Please register for your API key <a href="http://api.ids.ac.uk/accounts/register/" target="_new">here</a>. Once obtained, enter this key into the IDS API Token-GUID (key) section of the IDS Plugin Settings.'); ?>
  </p>
  
  <p>
  <?php _e('The IDS API package actually has two plugins on offer, both allow the administrator to select content from'); ?> <a href="http://www.eldis.org/" target="_new">Eldis</a>, <a href="http://www.bridge.ids.ac.uk/" target="_new">BRIDGE</a> <?php _e('or both collections, and bring relevant content easily into the site.'); ?>
  </p>
  
  <b><?php _e('IDS Import plugin'); ?></b>
  
  <p>
  <?php _e('This plugin enables the one time or periodic importing of content into the Wordpress database. IDS documents and organisations are imported as custom Wordpress content types, while IDS regions, countries and themes are imported as Wordpress custom taxonomies. Content and terms can also be imported manually.'); ?>
  </p>
  
  <p>
  <?php _e('You can select the number of items to import and assign to a specific Wordpress user. You can filter the content you import by country, region or theme. It is also possible the filter by publisher or author name.  You can integrate content seamlessly into your existing content structures by mapping Eldis or BRIDGE taxonomies to existing Wordpress taxonomies.'); ?>
  </p>
  
  <p>
  <?php _e('So, if you were the administrator of a website that highlighted recent research on ICTs and Gender in Kenya, you would be able to select specific criteria to suit your content. You would select the themes ICT and Gender, as well as the country Kenya.'); ?>
  </p>
  
  <b><?php _e('Tips for searching'); ?></b>
  
  <p>
  <?php _e('Although the Eldis and BRIDGE datasets are very wide ranging, very specific search terms may not return the number of results that you expect or want. Therefore, if you are not getting the results you expect, it might be worth trying more generic search terms or even simply searching at the Theme level.'); ?>
  </p>
  
  <p>
  <?php _e('Please note: You can add multiple selections for the Theme and Country fields. The search then looks for results that contain either Theme or Country.'); ?>
  </p>
  
  <p>
  <?php _e('e.g. A multiple selection of "India" and "Bangladesh" would show any results for:'); ?>
  </p>
  
  <p>
  <?php _e('either "India" OR "Bangladesh"'); ?>
  </p>
  
  <p>
  <?php _e('All other fields are combined in the search, so only results that contain ALL fields are returned.'); ?>
  </p>
  
  <p>
  <?php _e('e.g. A selection of "World Bank" as the publisher, "2010" as the year of publication, and a multiple selection of "India" and "Bangladesh" would show any results for:'); ?>
  </p>
  
  <p>
  <?php _e('World Bank AND 2010 AND (India OR Bangladesh)'); ?>
  </p>
  
  <p>
  <?php _e('For terms and condition for using API see'); ?> <a href="http://api.ids.ac.uk/about/terms/" target="_new"><?php _e('here'); ?></a>.
  </p>

<?php
}

// Helper function to display the names of selected categories in the IDS Importer page from their object_ids.
function idsimport_get_names($selected_object_ids, $names) {
  $ret = array();
  foreach ($selected_object_ids as $object_id) {
    if (isset($names[$object_id])) {
      $ret[] = $names[$object_id];
    }
  }
  return $ret;
}

// Format the array of available schedules to be displayed on the admin page.
function idsimport_get_schedules() {
  $available_schedules = wp_get_schedules();
  uasort($available_schedules, 'idsimport_sched_sort');
  foreach ($available_schedules as $recurrence => $sched) {
    $idsimport_schedules[$recurrence] = $sched['display'];
  }
  return $idsimport_schedules;
}

// Sorts the schedules according to the time period
function idsimport_sched_sort($sched1, $sched2) {
  if ($sched1['interval'] == $sched2['interval']) {
    return 0;
  }
  return ($sched1['interval'] < $sched2['interval']) ? -1 : 1;
}


