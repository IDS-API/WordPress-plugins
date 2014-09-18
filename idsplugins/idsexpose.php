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
require_once('idsexpose.filters.inc');

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
  //register_deactivation_hook( __FILE__, 'idsexpose_deactivate' );
  //register_uninstall_hook(__FILE__, 'idsexpose_uninstall');
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
  $array_post_types = array();//array('post' => 'Default Wordpress posts');
  $post_types = get_post_types(array('public' => true), 'objects') ;
  foreach ($post_types as $post_type) {
    $array_post_types[$post_type->name] = $post_type->labels->menu_name;
  }
  return $array_post_types;
}

function idsexpose_get_taxonomies() {
  $array_taxonomies = array();
  $taxonomies = get_taxonomies(array('public' => true), 'objects');
  foreach ($taxonomies as $taxonomy) {
    $array_taxonomies[$taxonomy->name] = $taxonomy->labels->menu_name;
  }
  return $array_taxonomies;
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
