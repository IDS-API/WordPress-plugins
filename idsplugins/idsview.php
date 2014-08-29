<?php
/*
Plugin Name: IDS View
Plugin URI: http://api.ids.ac.uk/category/plugins/
Description: Displays documents from the IDS collection via the IDS Knowledge Services API.
Version: 1.1
Author: Pablo Accuosto for the Institute of Development Studies (IDS)
Author URI: http://api.ids.ac.uk/
License: GPLv3

    Copyright 2012  Institute of Development Studies (IDS)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

if (!defined('IDS_API_LIBRARY_PATH')) define('IDS_API_LIBRARY_PATH', dirname(__FILE__) . '/idswrapper/');
if (!defined('IDS_API_ENVIRONMENT')) define('IDS_API_ENVIRONMENT', 'wordpress');

require_once('idsview.default.inc');
require_once(IDS_API_LIBRARY_PATH . 'idswrapper.wrapper.inc');

require_once('idsplugins.customtypes.inc');
require_once('idsplugins.functions.inc');
require_once('idsplugins.html.inc');
require_once('idsview.admin.inc');
require_once('idsview.widget.inc');

//-------------------------------- Set-up hooks ---------------------------------

register_activation_hook(__FILE__, 'idsview_activate');
add_action('init', 'idsview_init');
add_action('widgets_init', 'idsview_register_widget');
add_action('admin_init', 'idsview_admin_init');
add_action('admin_menu', 'idsview_add_options_page');
add_action('admin_menu', 'idsview_add_menu', 9);
add_action('admin_notices', 'idsview_admin_notices');
add_filter('plugin_action_links', 'idsview_plugin_action_links', 10, 2);
add_action('wp_enqueue_scripts', 'idsview_add_stylesheet');
add_action('admin_enqueue_scripts', 'idsview_add_admin_stylesheet');
add_action('admin_enqueue_scripts', 'idsview_add_javascript');
add_filter('query_vars', 'idsview_query_vars' );
add_filter('get_pagenum_link', 'idsview_append_offset');
add_filter('the_permalink', 'idsview_asset_permalink');
add_filter('the_content', 'idsview_asset_content');

//--------------------------- Set-up / init functions ----------------------------

// Create new pages to display IDS assets
function idsview_activate() {
  global $ids_assets;
  global $ids_datasets;
  foreach ($ids_datasets as $dataset) {
    foreach ($ids_assets as $asset_type) {
      $exclude = idsapi_exclude($dataset, $asset_type);
      if (!$exclude) {
        $page_id = 0;
        $page_name = $dataset . 'view_' . $asset_type;
        $page_exists = get_page_by_path($page_name);
        if (!$page_exists) {
          $page_title = ucfirst($dataset) . ' ' . ucfirst($asset_type);
          $page_template = 'idsview_' . $dataset . '_' . $asset_type . '.php';
          $page_content = sprintf(__('The %s displayed on this page are retrieved from the IDS collections by the IDS View plugin.'), $asset_type) . '<br/>';
          $page_content .= __('You can edit this page as any other regular WordPress page, but please make sure to keep it associated with a template that includes a call to the function idsview_assets() to make sure that the retrieved documents are included in the page.') . '<br/>';
          $page_content .= sprintf(__('For an example, please take a look at the template %s, included with the plugin.'), 'idsview_' . $dataset . '_' . $asset_type . '.php');
          $page_content = '<em><small>' . $page_content . '</small></em>';
          $page = array(
            'post_title'    => $page_title,
            'post_content' => $page_content,
            'post_status'   => 'pending',
            'comment_status' => 'closed',
            'ping_status' => 'closed',
            'post_name' => $page_name,
            'post_type' => 'page',
          );
          $page_id = wp_insert_post($page);
          if (!is_wp_error($page_id)) {
            update_post_meta($page_id, "_wp_page_template", $page_template);
          }
        }
        elseif (isset($page_exists->ID)) {
          $page_id = $page_exists->ID;
        }
        idsapi_variable_set('idsview', $page_name, $page_id);
      }
    }
  }
}


// Initialize plugin. Create custom post types (ids_documents/ids_organisations) if they do not exist.
function idsview_init() {
  ids_post_types_init();
  ids_check_permalinks_changed('idsview');
}

// Clean up on deactivation.
function idsview_deactivate() {
  global $wp_rewrite;  
  idsview_delete_assets_pages();
  idsview_delete_plugin_options();
  idsapi_cache_flush();
  $wp_rewrite->flush_rules();
}

// Clean up on deactivation.
function idsview_uninstall() {
}

// Delete idsview_documents / idsview_organisations pages.
function idsview_delete_assets_pages() {
  global $ids_assets;
  global $ids_datasets;
  foreach ($ids_datasets as $dataset) {
    foreach ($ids_assets as $asset_type) {
      $exclude = idsapi_exclude($dataset, $asset_type);
      if (!$exclude) {
        $page_name = $dataset . 'view_' . $asset_type;
        $page = get_page_by_path($page_name);
        if ($page) {
          wp_delete_post($page->ID, TRUE);
        }
      }
    }
  }
}

// Initialize the plugin's admin options
function idsview_admin_init(){
  register_setting('idsview', 'idsview_options', 'idsview_validate_options');
  $options = get_option('idsview_options');
  if(!is_array($options)) { // The options are corrupted.
    idsview_delete_plugin_options();
  }
  register_deactivation_hook( __FILE__, 'idsview_deactivate' );
  register_uninstall_hook(__FILE__, 'idsview_uninstall');
}

// Add settings link
function idsview_add_options_page() {
  add_options_page('IDS View Settings Page', 'IDS View', 'manage_options', 'idsview', 'idsview_admin_main');
}

// Display a 'Settings' link on the main Plugins page
function idsview_plugin_action_links($links, $file) {
	if ($file == plugin_basename(__FILE__)) {
		$idsapi_links = '<a href="' . get_admin_url() . 'options-general.php?page=idsview">' . __('Settings') . '</a>';
		array_unshift($links, $idsapi_links);
	}
	return $links;
}

// Delete options entries
function idsview_delete_plugin_options() {
	delete_option('idsview_options');
}

// Register IDS View Widget
function idsview_register_widget() {
  register_widget('IDS_View_Widget');
}

//---------------------- Admin interface (links, menus, etc -----------------------

// Add menu
function idsview_add_menu() {
  $idsview_menu_title = idsapi_variable_get('idsview', 'menu_title', IDS_VIEW_MENU_TITLE);
  if (idsapi_variable_get('idsview', 'api_key_validated', FALSE)) {
    add_menu_page('IDS View', $idsview_menu_title, 'manage_options', 'idsview_menu', 'idsview_general_page', plugins_url('images/ids.png', __FILE__));
    add_submenu_page( 'idsview_menu', 'IDS View', 'IDS View', 'manage_options', 'idsview_menu');
    add_submenu_page( 'idsview_menu', 'Settings', 'Settings', 'manage_options', 'options-general.php?page=idsview');
  }
  else {
    add_menu_page('IDS View', $idsview_menu_title, 'manage_options', 'idsview_menu', 'idsview_admin_main', plugins_url('images/ids.png', __FILE__));
  }
  add_submenu_page( 'idsview_menu', 'Help', 'Help', 'manage_options', 'idsview_help', 'idsview_help_page');
}

function idsview_general_page() {
  global $ids_assets;
  $datasets = idsview_get_datasets();
  ?>
  <div class="wrap">
  <div id="icon-edit-pages" class="icon32"><br /></div>
  <div id="ids-logo"><a href="http://api.ids.ac.uk" target="_blank"><img src="http://api.ids.ac.uk/docs/wp-content/uploads/2012/01/KS_powered.gif"></a></div>
  <h2>IDS View Plugin </h2>
  <br /><br />
  <u class="ids-category-list">
    <li class="cat-item"> <a href="<?php echo get_admin_url() . 'options-general.php?page=idsview' ?>"><?php _e('Settings'); ?></a></li>
    <li class="cat-item"> <a href="<?php echo get_admin_url() . 'admin.php?page=idsview_help' ?>"><?php _e('Help'); ?></a></li>
  <?php
  foreach ($datasets as $dataset) {
    foreach ($ids_assets as $type) {
      $exclude = idsapi_exclude($dataset, $type);
      if (!$exclude) {
        $page = get_page_by_path($dataset . 'view_' . $type);
        if (isset($page->ID) && ($page->post_status == 'publish')) { 
          $type_page = ucfirst($dataset ) . ' ' . ucfirst($type);
        ?>
          <li class="cat-item"> <a href="<?php echo home_url("?page_id=$page->ID") ?>"><?php echo __('View ') . $type_page; ?></a></li>
        <?php } 
      }
    }
  }
  ?>
  </u>
  <br /><br />
  <?php printf(__('Please go to the <a href="%s">Appereance > Widgets</a> settings in order to configure the IDS View Widget.'), get_admin_url() . 'widgets.php'); ?>
  </div>
<?php
}

// TODO. Complete.
function idsview_help_page() {
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
  <b><?php _e('IDS View plugin'); ?></b>
  <p>
  <?php _e('This plugin enables the display of a user defined subset of the IDS collection of documents or organisations using the IDS View widget, available in the Wordpress Widget panel.'); ?>
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

// Enqueue stylesheet. We keep separate functions as in the future we might want to use different stylesheets for each plugin.
function idsview_add_stylesheet() {
  wp_register_style('idsview_style', plugins_url(IDS_PLUGINS_SCRIPTS_PATH . 'idsplugins.css', __FILE__));
  wp_enqueue_style('idsview_style');
}

// Enqueue admin stylesheet
function idsview_add_admin_stylesheet() {
  idsview_add_stylesheet();
  wp_register_style('idsview_chosen_style', plugins_url(IDS_PLUGINS_SCRIPTS_PATH . 'chosen/chosen.css', __FILE__));
  wp_enqueue_style('idsview_chosen_style');
  wp_register_style('idsview_jqwidgets_style', plugins_url(IDS_PLUGINS_SCRIPTS_PATH . 'jqwidgets/styles/jqx.base.css', __FILE__));
  wp_enqueue_style('idsview_jqwidgets_style');
}

// Enqueue javascript
function idsview_add_javascript($hook) {
  global $ids_datasets;
  if ($hook == 'settings_page_idsview') { // Only in the admin page.
    wp_print_scripts( 'jquery' );
    wp_print_scripts( 'jquery-ui-tabs' );
    wp_register_script('idsview_chosen_javascript', plugins_url(IDS_PLUGINS_SCRIPTS_PATH . 'chosen/chosen.jquery.js', __FILE__));
    wp_enqueue_script('idsview_chosen_javascript');
    wp_register_script('idsview_jqwidgets_jqxcore_javascript', plugins_url(IDS_PLUGINS_SCRIPTS_PATH . 'jqwidgets/jqwidgets/jqxcore.js', __FILE__));
    wp_enqueue_script('idsview_jqwidgets_jqxcore_javascript');
    wp_register_script('idsview_jqwidgets_jqxbuttons_javascript', plugins_url(IDS_PLUGINS_SCRIPTS_PATH . 'jqwidgets/jqwidgets/jqxbuttons.js', __FILE__));
    wp_enqueue_script('idsview_jqwidgets_jqxbuttons_javascript');
    wp_register_script('idsview_jqwidgets_jqxdropdownbutton_javascript', plugins_url(IDS_PLUGINS_SCRIPTS_PATH . 'jqwidgets/jqwidgets/jqxdropdownbutton.js', __FILE__));
    wp_enqueue_script('idsview_jqwidgets_jqxdropdownbutton_javascript');
    wp_register_script('idsview_jqwidgets_jqxscrollbar_javascript', plugins_url(IDS_PLUGINS_SCRIPTS_PATH . 'jqwidgets/jqwidgets/jqxscrollbar.js', __FILE__));
    wp_enqueue_script('idsview_jqwidgets_jqxscrollbar_javascript');
    wp_register_script('idsview_jqwidgets_jqxpanel_javascript', plugins_url(IDS_PLUGINS_SCRIPTS_PATH . 'jqwidgets/jqwidgets/jqxpanel.js', __FILE__));
    wp_enqueue_script('idsview_jqwidgets_jqxpanel_javascript');
    wp_register_script('idsview_jqwidgets_jqxtree_javascript', plugins_url(IDS_PLUGINS_SCRIPTS_PATH . 'jqwidgets/jqwidgets/jqxtree.js', __FILE__));
    wp_enqueue_script('idsview_jqwidgets_jqxtree_javascript');
    wp_register_script('idsview_jqwidgets_jqxcheckbox_javascript', plugins_url(IDS_PLUGINS_SCRIPTS_PATH . 'jqwidgets/jqwidgets/jqxcheckbox.js', __FILE__));
    wp_enqueue_script('idsview_jqwidgets_jqxcheckbox_javascript');
    wp_register_script('idsview_javascript', plugins_url(IDS_PLUGINS_SCRIPTS_PATH . 'idsplugins.js', __FILE__));
    wp_enqueue_script('idsview_javascript');
    $api_key = idsapi_variable_get('idsview', 'api_key', '');
    $api_key_validated = idsapi_variable_get('idsview', 'api_key_validated', FALSE);
    $default_dataset = idsapi_variable_get('idsview', 'default_dataset', IDS_IMPORT_DEFAULT_DATASET_ADMIN);
    foreach ($ids_datasets as $dataset) {
      $countries[$dataset] = idsapi_variable_get('idsview', $dataset . '_countries_assets', array());
      $regions[$dataset] = idsapi_variable_get('idsview', $dataset . '_regions_assets', array());
      $themes[$dataset] = idsapi_variable_get('idsview', $dataset . '_themes_assets', array());
    }
    $categories = array('countries' => $countries, 'regions' => $regions, 'themes' => $themes);
    ids_init_javascript('idsview', $api_key, $api_key_validated, $default_dataset, $categories);
  }
}

//-------------------------- Retrieve and show assets ----------------------------

// Define new URL parameters.
function idsview_query_vars($query_vars) {
  $query_vars[] = 'object_id';
  $query_vars[] = 'country';
  $query_vars[] = 'region';
  $query_vars[] = 'theme';
  $query_vars[] = 'ids_offset';
  return $query_vars;
}

function idsview_append_offset($link) {
  global $wp_query;
  if (isset($wp_query->query_vars['ids_offset'])) { 
    $ids_offset = (get_query_var('ids_offset')) ? get_query_var('ids_offset') : 0;
    $current_page_num = (get_query_var('paged')) ? get_query_var('paged') : 1;
    $prev_offset = (get_query_var('prev_offset')) ? get_query_var('prev_offset') : 0;
    $num_excluded = (get_query_var('num_excluded')) ? get_query_var('num_excluded') : 0;
    $posts_per_page = (get_query_var('posts_per_page')) ? get_query_var('posts_per_page') : 1;
    parse_str($link, $query_array);
    if (isset($query_array['paged'])) {
      $linked_page_num = $query_array['paged'];
    }
    else {
      $linked_page_num = 0;
    }
    if ($linked_page_num == $current_page_num + 1) {
      $offset = $ids_offset;
    }
    elseif ($linked_page_num == $current_page_num - 1) {
      $offset = ($prev_offset > $posts_per_page+$num_excluded) ? $prev_offset-($posts_per_page+$num_excluded) : 0;
    }
    else {
      $offset = $posts_per_page*$linked_page_num;
    }
    $link = add_query_arg('ids_offset', $offset, $link);
  }
  return $link;
}

function idsview_asset_permalink($link) {
  global $post;
  if (isset($post->object_id)) {
    $link = add_query_arg('object_id', $post->object_id, $link);
  }
  return $link;
}

function idsview_asset_content($content) {
  global $post;
  global $wp_query;
  if (isset($wp_query->query_vars['object_id'])) { // It's a request for a single asset.
    if ($post->post_type == 'page') { // The content of the general page (idsview_documents / idsview_organisations)
      $content = '';
    }
  }
  return $content;
}

// Populate the loop with content retrieved by calling the IDS API.
function idsview_assets($dataset, $type) {
  global $wp_query;
  $posts = array();
  if (isset($wp_query->query_vars['object_id'])) { // It's a request for a single asset.
    $object_id = $wp_query->query_vars['object_id'];
    $response = idsview_get_single($dataset, $type, $object_id);
    if ($response) {
      $assets = $response->getResults();
      if (!empty($assets)) {
        $posts[] = idsview_create_post($dataset, $assets[0], $type);
        $wp_query->post = $posts[0];
        $wp_query->posts = $posts;
      }
    }
    $wp_query->is_single = 1;
    $wp_query->is_singular = 1;
    $wp_query->found_posts = 0;
    $wp_query->post_count = 1;
  }
  else { // It's a general request.
    $filters = array();
    $posts_per_page = $wp_query->query_vars['posts_per_page'];
    if (isset($wp_query->query_vars['ids_offset'])) {
      $offset = $wp_query->query_vars['ids_offset'];
    }
    else {
      $offset = 0;
    }
    if (isset($wp_query->query_vars['country'])) {
      $filters['country'] = $wp_query->query_vars['country'];
    }
    if (isset($wp_query->query_vars['region'])) {
      $filters['region'] = $wp_query->query_vars['region'];
    }
    if (isset($wp_query->query_vars['theme'])) {
      $filters['theme'] = $wp_query->query_vars['theme'];
    }
    $num_retrieved = 0;
    $num_excluded = 0;
    $total_results = 0;
    $more_results = TRUE;
    $prev_offset = $offset;
    while (($num_retrieved < $posts_per_page) && $more_results) {
      $response = idsview_get_assets($dataset, $type, $posts_per_page, $offset, $filters);
      if ($response) {
        $assets = $response->getResults();
        if (!empty($assets)) {
          foreach ($assets as $asset) {
            if (ids_exclude_asset('idsview', $asset)) {
              $num_excluded++;
            }
            else {
              $posts[] = idsview_create_post($dataset, $asset, $type);
              $num_retrieved++;
            }
            $offset++; 
            if ($num_retrieved == $posts_per_page) {
              break;
            }
          }
          $total_results = $response->getTotalResults();
          $more_results = ($total_results > $offset);
        }
        else {
          $more_results = FALSE;
        }
      }
    }
    if (!empty($posts)) {
      $wp_query->posts = $posts;
      $wp_query->found_posts = $total_results;
      $wp_query->post_count = $num_retrieved;
      $wp_query->post = $posts[0];
      $wp_query->is_page = FALSE;
      $wp_query->is_archive = TRUE;
      $wp_query->queried_object = NULL;
      $wp_query->queried_object_id = NULL;
      $wp_query->max_num_pages = round($total_results/$posts_per_page, 0);
      $wp_query->set('ids_offset', $offset);
      $wp_query->set('prev_offset', $prev_offset);
      $wp_query->set('num_excluded', $num_excluded);
    }
  }
  if (IDS_VIEW_DISPLAY_ERRORS) {
    idsapi_report_errors();
  }
}

// Posts that hold the retrieved content.
function idsview_create_post($dataset, $asset, $type) {
  $page_name = $dataset . 'view_' . $type;
  if ($assets_page = get_page_by_path($page_name)) {
    $id = $assets_page->ID;
  }
  else {
    $id = 0;
  }
  $url = $dataset . 'view_' . $type;
  $post_name = sanitize_title($asset->title);
  $default_language = idsapi_variable_get('idsview', 'language', IDS_VIEW_DEFAULT_LANGUAGE);
  $title = ids_get_translation($asset, 'title', $default_language);
  $description = ids_get_translation($asset, 'description', $default_language);
  $post = array (
            'ID' => $id,
            'post_author' => 1,
            'post_date' => date_i18n('Y-m-d H:i:s', $asset->date_created),
            'post_date_gmt' => get_gmt_from_date(date_i18n('Y-m-d H:i:s', $asset->date_created)),
            'post_content' => $description,
            'post_title' => $title,
            'post_excerpt' => ids_excerpt_post('idsview', $description),
            'post_status' => 'publish',
            'comment_status' => 'closed',
            'ping_status' => 'closed',
            'post_name' => $post_name,
            'post_modified' => date_i18n('Y-m-d H:i:s', $asset->date_updated),
            'post_modified_gmt' => get_gmt_from_date(date_i18n('Y-m-d H:i:s', $asset->date_updated)),
            'post_parent' => 0,
            'guid' => home_url($url) . '/' . $post_name,
            'menu_order' => 0,
            'post_type' => 'ids_' . $type,
            'comment_count' => 0,
        );
  $post['object_id'] = $asset->object_id;
  if (isset($asset->site)) { $post['site'] = $asset->site; }
  if (isset($asset->timestamp)) { $post['timestamp'] = $asset->timestamp; }
  if (isset($asset->website_url)) { $post['website_url'] = $asset->website_url; }
  if (isset($asset->country_focus_array)) { $post['ids_countries'] = $asset->country_focus_array; }
  if (isset($asset->category_region_array)) { $post['ids_regions'] = $asset->category_region_array; }
  if (isset($asset->category_theme_array)) { $post['ids_themes'] = $asset->category_theme_array; }
  if (isset($asset->metadata_url)) { $post['metadata_url'] = $asset->metadata_url; }
  if (isset($asset->keywords)) { $post['post_tag'] = $asset->keywords; }
  if ($type == 'documents') {
    // Document-specific fields.
    if (isset($asset->authors)) { $post['authors'] = $asset->authors; }
    if (isset($asset->language_name)) { $post['language_name'] = $asset->language_name; }
    if (isset($asset->publication_date)) { $post['publication_date'] = $asset->publication_date; }
    if (isset($asset->licence_type)) { $post['licence_type'] = $asset->licence_type; }
    if (isset($asset->publisher_array)) { $post['publisher_array'] = $asset->publisher_array; }
    if (isset($asset->urls)) { $post['urls'] = $asset->urls; }
  }
  elseif ($type == 'organisations') {
    // Organisation-specific fields.
    if (isset($asset->acronym)) { $post['acronym'] = $asset->acronym; }
    if (isset($asset->alternative_acronym)) { $post['alternative_acronym'] = $asset->alternative_acronym; }
    if (isset($asset->organisation_type)) { $post['organisation_type'] = $asset->organisation_type; }
    if (isset($asset->organisation_url)) { $post['organisation_url'] = $asset->organisation_url; }
    if (isset($asset->location_country)) { $post['location_country'] = $asset->location_country; }
  }
  $post = (object) $post;
  return $post;
}

// Call the IDS API to retrieve a single asset.
function idsview_get_single($dataset, $type, $object_id) {
  $idsapi = new IdsApiWrapper;
  $response = FALSE;
  $api_key_validated = idsapi_variable_get('idsview', 'api_key_validated', FALSE);
  if ($api_key_validated) {
    $api_key = idsapi_variable_get('idsview', 'api_key', FALSE);
    $response = $idsapi->get($type, $dataset, $api_key, 'full', $object_id);
    if ($response->isError()) {
      $error_message = __('No content retrieved by the API call. ') . $response->getErrorMessage();
      idsapi_register_error('idsview', $error_message, 'idsview_get_single', 'warning');
    }
  }
  else {
    echo 'Please enter a valid API key in the IDS View Plugin admin area.';
  }
  return $response;
}

// Call the IDS API to retrieve a set of assets.
function idsview_get_assets($dataset, $assets_type, $num_items, $offset, $filters = array()) {
  $idsapi = new IdsApiWrapper;
  $response = FALSE;
  $api_key_validated = idsapi_variable_get('idsview', 'api_key_validated', FALSE);
  if ($api_key_validated) {
    $filter_params = array();
    $api_key = idsapi_variable_get('idsview', 'api_key', FALSE);
    $age_results = idsapi_variable_get('idsview', 'age_new_assets', IDS_API_AGE_NEW_ASSETS);
    $view_query = idsapi_variable_get('idsview', 'view_query', '');
    if (isset($filters['country'])) {
      $countries_assets = array($filters['country']);
    }
    else {
      $countries_assets = idsapi_variable_get('idsview', $dataset . '_countries_assets', array());
    }
    if (isset($filters['region'])) {
      $regions_assets = array($filters['region']);
    }
    else {
      $regions_assets = idsapi_variable_get('idsview', $dataset . '_regions_assets', array());
    }
    if (isset($filters['theme'])) {
      $themes_assets = array($filters['theme']);
    }
    else {
      $themes_assets = idsapi_variable_get('idsview', $dataset . '_themes_assets', array());
    }
    if ($view_query) {
      $search_params = explode(',', $view_query);
      foreach ($search_params as $search_param) {
        $param = explode('=', trim($search_param));
        $key = $param[0];
        $value = $param[1];
        $filter_params[$key] = $value;
      }
    }
    if (!empty($countries_assets)) {
      $filter_params['country'] = implode('|', $countries_assets);
    }
    if (!empty($regions_assets)) {
      $filter_params['region'] = implode('|', $regions_assets);
    }
    if (!empty($themes_assets)) {
      $filter_params['theme'] = implode('|', $themes_assets);
    }
    if ($offset) {
      $filter_params['start_offset'] = $offset;
    }
    if ($assets_type == 'documents') { // Document-specific filters
      $authors_assets = idsapi_variable_get('idsview', 'view_authors_assets', '');
      $publishers_assets = idsapi_variable_get('idsview', 'view_publishers_assets', '');
      $language_name_codes = idsapi_variable_get('idsview', 'language_name_codes', array());
      if ($authors_assets) {
        $authors = explode(',', $authors_assets);
        $list_authors = implode('|', array_map('trim', $authors));
        $filter_params['author'] = $list_authors;
      }
      if ($publishers_assets) {
        $publishers = explode(',', $publishers_assets);
        $list_publishers = implode('|', array_map('trim', $publishers));
        $filter_params['publisher_name'] = $list_publishers;
      }
      if ($language_name_codes) {
        $ids_languages = ids_languages();
        $list_language_names = array();
        foreach ($language_name_codes as $language_code) {
          if (isset($ids_languages[$language_code])) {
            $list_language_names[] = $ids_languages[$language_code]; 
          }
          if ($list_language_names) {
            $language_names = implode('|', $list_language_names);
            $filter_params['language_name'] = $language_names;
          }
        }
      }
    }
    $response = $idsapi->search($assets_type, $dataset, $api_key, 'full', $num_items, $age_results, $filter_params);
    if ($response->isError()) {
      $error_message = __('No content retrieved by the API call. ') . $response->getErrorMessage();
      idsapi_register_error('idsview', $error_message, 'idsview_get_assets', 'warning');
    }
  }
  else {
    echo 'Please enter a valid API key in the IDS View Plugin admin area.';
  }
  return $response;
}

// Display category title for archive-like listings.
function idsview_category_title($dataset=IDS_API_DEFAULT_DATASET) {
  $title = '';
  $cat_id = 0;
  $api_key = idsapi_variable_get('idsview', 'api_key', FALSE);
  if ($api_key) {
    if ($cat_id = get_query_var('country')) {
      $cat_name = 'countries';
    }
    elseif ($cat_id =  get_query_var('region')) {
      $cat_name = 'regions';
    }
    elseif ($cat_id = get_query_var('theme')) {
      $cat_name = 'themes';
    }
    if ($cat_id) {
      $idsapi = new IdsApiWrapper;
      $response = $idsapi->get($cat_name, $dataset, $api_key, 'short', $cat_id);
      if ((!$response->isError()) && (!empty($response->results))) {
        $title = $response->results[0]->title;
      }
    }
  }
  return $title;
}

// A single object is being displayed.
function idsview_is_single() {
  return get_query_var('object_id');
}







