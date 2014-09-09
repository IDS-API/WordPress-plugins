<?php
/*
Plugin Name: IDS Expose
Plugin URI: http://api.ids.ac.uk/category/plugins/
Description: Exposes content to be incorporated to the IDS Knowledge Services Hub.
Version: 1.0
Author: Pablo Accuosto for the Institute of Development Studies (IDS)
Author URI: http://api.ids.ac.uk/
License: GPLv3

    Copyright 2014  Institute of Development Studies (IDS)

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

require_once('idsexpose.default.inc');
require_once(IDS_API_LIBRARY_PATH . 'idswrapper.wrapper.inc');

require_once('idsplugins.customtypes.inc');
require_once('idsplugins.functions.inc');
require_once('idsplugins.html.inc');
require_once('idsexpose.admin.inc');

//-------------------------------- Set-up hooks ---------------------------------

//register_activation_hook(__FILE__, 'idsexpose_activate');
add_action('init', 'idsexpose_init');
add_action('admin_init', 'idsexpose_admin_init');
add_action('admin_menu', 'idsexpose_add_options_page');
add_action('admin_menu', 'idsexpose_add_menu', 9);
add_action('admin_notices', 'idsexpose_admin_notices');
add_filter('plugin_action_links', 'idsexpose_plugin_action_links', 10, 2);
add_action('wp_enqueue_scripts', 'idsexpose_add_stylesheet');
add_action('admin_enqueue_scripts', 'idsexpose_add_admin_stylesheet');
add_action('admin_enqueue_scripts', 'idsexpose_add_javascript');
add_action('do_feed_ids_assets', 'idsexpose_feed_ids_assets', 10, 1);
add_action('do_feed_ids_categories', 'idsexpose_feed_ids_categories', 10, 1);
//add_filter('query_vars', 'idsexpose_query_vars' );
add_filter('wp_dropdown_cats', 'idsexpose_dropdown_cats' );

//--------------------------- Set-up / init functions ----------------------------


// Initialize plugin.
function idsexpose_init() {
  ids_check_permalinks_changed('idsexpose');
}

// Initialize the plugin's admin options
function idsexpose_admin_init(){
  register_setting('idsexpose', 'idsexpose_options', 'idsexpose_validate_options');
  $options = get_option('idsexpose_options');
  if(!is_array($options)) { // The options are corrupted.
    idsexpose_delete_plugin_options();
  }
  register_deactivation_hook( __FILE__, 'idsexpose_deactivate' );
  register_uninstall_hook(__FILE__, 'idsexpose_uninstall');
}

// Delete options entries
function idsexpose_delete_plugin_options() {
	delete_option('idsexpose_options');
}

// Enqueue stylesheet. We keep separate functions as in the future we might want to use different stylesheets for each plugin.
function idsexpose_add_stylesheet() {
    wp_register_style('idsexpose_style', plugins_url(IDS_PLUGINS_SCRIPTS_PATH . 'idsplugins.css', __FILE__));
    wp_enqueue_style('idsexpose_style');
}

// Enqueue admin stylesheet
function idsexpose_add_admin_stylesheet() {
  idsexpose_add_stylesheet();
  wp_register_style('idsexpose_chosen_style', plugins_url(IDS_PLUGINS_SCRIPTS_PATH . 'chosen/chosen.css', __FILE__));
  wp_enqueue_style('idsexpose_chosen_style');
  wp_register_style('idsexpose_jqwidgets_style', plugins_url(IDS_PLUGINS_SCRIPTS_PATH . 'jqwidgets/styles/jqx.base.css', __FILE__));
  wp_enqueue_style('idsexpose_jqwidgets_style');
}

// Enqueue javascript
function idsexpose_add_javascript($hook) {
  if (($hook == 'settings_page_idsexpose') || ($hook == 'ids-expose_page_idsexpose_create_feeds')) { // Only in the admin pages.
    wp_print_scripts( 'jquery' );
    wp_print_scripts( 'jquery-ui-tabs' );
    wp_register_script('idsexpose_javascript', plugins_url(IDS_PLUGINS_SCRIPTS_PATH . 'idsplugins.js', __FILE__));
    wp_enqueue_script('idsexpose_javascript');
    ids_init_javascript('idsexpose');
  }
}

// Display a 'Settings' link on the main Plugins page
function idsexpose_plugin_action_links($links, $file) {
	if ($file == plugin_basename(__FILE__)) {
		$idsapi_links = '<a href="' . get_admin_url() . 'options-general.php?page=idsexpose">' . __('Settings') . '</a>';
		array_unshift($links, $idsapi_links);
	}
	return $links;
}

// Make categories selects multiple.
function idsexpose_dropdown_cats($output) {
  if (preg_match('/<select name=\'idsexpose/', $output)) {
    $output = preg_replace('/<select /', '<select multiple="multiple" ', $output);
  }
  return $output;
}

// Add settings link
function idsexpose_add_options_page() {
  add_options_page('IDS Expose Settings Page', 'IDS Expose', 'manage_options', 'idsexpose', 'idsexpose_admin_main');
}

// Add menu
function idsexpose_add_menu() {
  add_menu_page('IDS Expose', 'IDS Expose', 'manage_options', 'idsexpose_menu', 'idsexpose_general_page', plugins_url('images/ids.png', __FILE__));
  add_submenu_page('idsexpose_menu', 'Settings', 'Settings', 'manage_options', 'options-general.php?page=idsexpose');
  add_submenu_page('idsexpose_menu', 'Feeds', 'Feeds', 'manage_options', 'idsexpose_create_feeds', 'idsexpose_create_feeds_page');
  add_submenu_page('idsexpose_menu', 'Help', 'Help', 'manage_options', 'idsexpose_help', 'idsexpose_help_page');
}

// Admin page.
function idsexpose_general_page() {
?>
  <div class="wrap">
  <div id="icon-edit-pages" class="icon32"><br /></div>
  <div id="ids-logo"><a href="http://api.ids.ac.uk" target="_blank"><img src="http://api.ids.ac.uk/docs/wp-content/uploads/2012/01/KS_powered.gif"></a></div>
  <h2>IDS Expose Plugin </h2>
  <br /><br />
  <u class="ids-category-list">
    <li class="cat-item"> <a href="<?php echo get_admin_url() . 'options-general.php?page=idsexpose' ?>"><?php _e('Settings'); ?></a></li>
    <li class="cat-item"> <a href="<?php echo get_admin_url() . 'admin.php?page=idsexpose_create_feeds' ?>"><?php _e('Feeds'); ?></a></li>
    <li class="cat-item"> <a href="<?php echo get_admin_url() . 'admin.php?page=idsexpose_help' ?>"><?php _e('Help'); ?></a></li>
  </u>
  </div>
<?php
}

// Page to generate new feeds based on the content types and taxonomies selected in the settings page.
function idsexpose_create_feeds_page() {
  $array_post_types = idsexpose_get_post_types();
  $array_taxonomies = idsexpose_get_taxonomies();
  $selected_types = idsapi_variable_get('idsexpose', 'content_types', array());
  $selected_taxonomies = idsapi_variable_get('idsexpose', 'taxonomies', array());
  foreach ($array_post_types as $type => $type_label) {
    if (isset($selected_types[$type])) {
      $select_types[$type] = $type_label;
    }
  }
  foreach ($array_taxonomies as $taxonomy => $taxonomy_label) {
    if (isset($selected_taxonomies[$taxonomy])) {
      $select_taxonomies[$taxonomy] = $taxonomy_label;
    }
  }
  $feed_assets_url = site_url() . '?feed=ids_assets'; 
  $feed_categories_url = site_url() . '?feed=ids_categories'; 
?>
  <div class="wrap">
  <div id="icon-tools" class="icon32"><br /></div>
  <h2><?php _e('IDS Expose feeds'); ?></h2>
  <p>
  <b><?php _e('Create new feeds'); ?></b>
  <form id="idsexpose_create_feed">
	<table class="form-table" id="content_types">
    <tr>
      <th scope="row"><b><?php _e('Feed URL'); ?></b></th>
      <td>
      <p class="description">
      <?php _e('Feed URL generated by the selections made below. Change the selections in order to generate a new URL.'); ?> </br>
      </p>
			<input type="hidden" id="idsexpose_original_posts_url" value="<?php echo $feed_assets_url; ?>" />
			<input type="hidden" id="idsexpose_original_categories_url" value="<?php echo $feed_categories_url; ?>" />
      <textarea id="idsexpose_feed_url" rows="3" cols="90"><?php echo $feed_assets_url; ?></textarea></br>
      <button type="button" onclick="generateFeedUrl()">Generate new feed URL</button> 
      <button type="button" onclick="gotoFeed()">Go to feed</button></br>
      </td>
    </tr>
    <tr>
      <th scope="row"><?php _e('Number of elements in feed'); ?></th>
      <td>
			<input type="text" size="4" id="idsexpose_num_items" value="<?php echo get_option('posts_per_rss'); ?>" />
      </td>
    </tr>
  </table>

	<table class="form-table" id="content_types">
    <tr>
      <th scope="row"><?php _e('Type of feed'); ?></th>
      <td>
      <?php echo ids_select_box('idsexpose_type_feed', 'idsexpose_type_feed', array('posts' => 'Posts', 'categories' => 'Categories'), array(), array('onchange' => 'changeFeedType()')); ?>
      </td>
    </tr>
  </table>

  <!---------------------------------------------- Posts ---------------------------------------------->
  <table class="form-table" id="idsexpose_select_posts">
    <tr>
      <th scope="row"><?php _e('Content type'); ?></th>
      <td>
      <?php echo ids_select_box('idsexpose_posts', 'idsexpose_posts', $select_types, array(), array('onchange' => 'changeFeedFilters()')); ?>
      </td>
    </tr>
    <?php foreach (array_keys($select_types) as $content_type) { 
      $objects_taxonomies = get_object_taxonomies($content_type, 'objects');
      foreach($objects_taxonomies as $tax_name => $tax_object) { 
        if ($tax_object->show_ui) { ?>
        <tr class="idsexpose_filters idsexpose_filters_<?php echo $content_type;?>">
          <th scope="row"><?php printf(__('Filter by %s'), __($tax_object->label)); ?></th>
          <td>
            <?php wp_dropdown_categories(array('name' => 'idsexpose_' . $tax_name . '[]', 'class' => 'idsexpose_' . $content_type . '_taxonomy_select', 'id' => $content_type . '-' .$tax_name, 'taxonomy' => $tax_name, 'hierarchical' => $tax_object->hierarchical, 'hide_empty' => FALSE, 'orderby' => 'name')); ?><br/>
            <a href="javascript:deselectAll('<?php echo $content_type . '-' .$tax_name; ?>');"><?php _e('Deselect all'); ?></a>                
          </td>
        </tr>
      <?php }
        }
    } ?>
    <?php if (!empty($objects_taxonomies)) { ?>
    <tr>
      <th scope="row"><?php _e('Filters operator'); ?></th>
      <td>
      <?php echo ids_select_box('idsexpose_posts_cats_op', 'idsexpose_posts_cats_op', array('OR' => 'Posts matching any of the filters (OR)', 'AND' => 'Posts matching all the filters (AND)'), array('OR')); ?>
      <p class="description">
      <?php _e('Please note that this selection indicates how to join different filters.'); ?></br>
      <?php _e('Options within the same filter are considered as alternatives and posts matching at least one of them will be retrieved.'); ?> </br>
      </p>
      </td>
    </tr>
    <?php } ?>
  </table>

  <!---------------------------------------------- Categories ---------------------------------------------->
  <table class="form-table" id="idsexpose_select_categories">
    <tr>
      <th scope="row"><?php _e('Taxonomy'); ?></th>
      <td>
      <?php echo ids_select_box('idsexpose_categories', 'idsexpose_categories', $select_taxonomies, array(), array('onchange' => 'changeFeedFilters()')); ?>
      </td>
    </tr>
  </table>

  </form>
  </p>
  </div>
<?php
}

function idsexpose_help_page() {
?>
  <div class="wrap">
  <div id="icon-edit-comments" class="icon32 icon32-posts-page"><br /></div>
  <h2><?php _e('Help'); ?></h2>
  <p>
  <?php _e('The IDS KS API plugin allows access to IDS Knowledge Services content of thematically organised and hand selected academic research on poverty reduction in the developing world that is freely available to access online.'); ?>
  </p>
  <b><?php _e('IDS Expose plugin'); ?></b>
  <p>
  <?php _e('This plugin allow to expose contributed content for the IDS KS Hub.'); ?>
  </p>
<?php
}

function idsexpose_feed_ids_assets() {
	load_template(plugin_dir_path( __FILE__ ) . 'templates/idsexpose_assets_template.php');
}

function idsexpose_feed_ids_categories() {
	load_template(plugin_dir_path( __FILE__ ) . 'templates/idsexpose_categories_template.php');
}

/*
function idsexpose_query_vars($query_vars) {
  $query_vars[] = 'num';
  return $query_vars;
}
*/

function idsexpose_get_post_types() {
  $array_post_types = array('post' => 'Default Wordpress posts');
  $post_types = get_post_types(array('public' => true, '_builtin' => false), 'objects') ;
  foreach ($post_types as $post_type) {
    $array_post_types[$post_type->name] = $post_type->labels->menu_name;
  }
  return $array_post_types;
}

function idsexpose_get_taxonomies() {
  $array_taxonomies = array('category' => 'Default Wordpress categories');
  $taxonomies = get_taxonomies(array('public' => true, '_builtin' => false), 'objects');
  foreach ($taxonomies as $taxonomy) {
    $array_taxonomies[$taxonomy->name] = $taxonomy->labels->menu_name;
  }
  return $array_taxonomies;
}

function idsexpose_get_tag($field, $type) {
  $field_mappings = idsapi_variable_get('idsexpose', 'field_mappings', array());
  $tag = (isset($field_mappings[$type]) && isset($field_mappings[$type][$field])) ? $field_mappings[$type][$field] : $field;
  return $tag;
}

// TODO: Generalize.
function idsexpose_get_value($field, $value, $type) {
  switch ($field) {
		case 'post_title':
      $value = get_the_title_rss();
      break;
		case 'post_author':
      $value = get_the_author();
      break;
		case 'guid':
      $value = get_permalink();
      break;
		case 'post_content':
      $value = get_the_content_feed('rss2');
      break;
		case 'post_date':
      $value = mysql2date('Y-m-d', get_post_time('Y-m-d H:i', true), true);
      break;
		case 'post_modified':
      $value = the_modified_date('c');
      break;
		case '_edit_lock':
		case '_edit_last':
      $value = '';
      break;
  }
  return $value;
}

function idsexpose_get_xml($tag, $value) {
  $ret = '';
  if ($unserialized = @unserialize($value)) {
    $value = $unserialized;
  }
  if (is_array ($value)) {
    $ret = "<$tag>";
    foreach ($value as $val) {
      $ret .= idsexpose_get_xml('list-item', $val);
    }
    $ret .= "</$tag>";
  }
  elseif (is_object($value)) {
    $ret = "<$tag>";
    foreach ($value as $key => $val) {
      $ret .= idsexpose_get_xml($key, $val);
    }
    $ret .= "</$tag>";
  }
  elseif (is_scalar($value)) {
    $ret = "<$tag>" . htmlspecialchars($value, ENT_QUOTES, 'UTF-8') . "</$tag>";
  }
  return $ret;
}

function idsexpose_print_xml($tag, $value) {
  if ($value) {
    if (is_array($value)) {
      foreach ($value as $val) {
        echo idsexpose_get_xml($tag, $val);
      }
    }
    else {
      echo idsexpose_get_xml($tag, $value);
    }
  }
}
