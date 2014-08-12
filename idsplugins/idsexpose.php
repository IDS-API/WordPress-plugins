<?php
/*
Plugin Name: IDS Expose
Plugin URI: http://api.ids.ac.uk/category/plugins/
Description: Exposes content to be incorporated to the IDS Knowledge Services Hub.
Version: 1.0

*/

// The definition of the IDS fields and how they are displayed can be stored in a separate file.
require_once('idsexpose.fields.inc');

// This function would add boxes to the post create/edit page by calling add_meta_box() for each IDS field.
function idsexpose_meta_boxes($post) {
  //See http://codex.wordpress.org/Function_Reference/add_meta_box
  $idsexpose_ids_fields = idsexpose_fields_definitions();
  foreach ($idsexpose_fields as $field) {
    add_meta_box($field['id'], $field['title'], $field['callback'], $field['post_type'], $field['context'], $field['priority'], $field['callback_args']);
  }
}
add_action('add_meta_boxes_post', 'idsexpose_meta_boxes', 10, 2);

/*
 If the importer plugin is installed and the IDS taxonomies exist, the user could choose to use those taxonomies in their posts.
 In that case, register_taxonomy_for_object_type should be used.
 Example:.
 $ids_category = 'eldis_regions'; // for instance.
 if ((taxonomy_exists($ids_category)) && ($use_ids_categories[$ids_category])) {
      register_taxonomy_for_object_type($ids_category, 'posts');
    }
 }
---> This could also be an option of the importer plugin, actually.
*/

/* Instead, if we want to give the option to the user of creating IDS taxonomies (regions, countries, etc) but not necessarily import IDS catefories:
   The same code used to create taxonomies that is used in the idsimport plugin could be used:
   - idsimport_new_taxonomy()
*/

// Copied from the custom-xml-feed plugin, this could be done here.
function idsexpose_feed_myxml() {
	load_template(plugin_dir_path( __FILE__ ) . 'idsexpose_xml_template.php');
}
add_action( 'do_feed_myxml', 'idsexpose_feed_myxml', 10, 1 );

// Add settings link
function idsexpose_add_options_page() {
  add_options_page('IDS Expose Settings Page', 'IDS Expose', 'manage_options', 'idsexpose', 'idsexpose_admin_main');
}

// Add menu
function idsexpose_add_menu() {
  $idsexpose_menu_title = idsapi_variable_get('idsexpose', 'menu_title', 'IDS Expose');
  add_menu_page('IDS API', $idsexpose_menu_title, 'manage_options', 'idsexpose_menu', 'idsexpose_general_page', plugins_url('images/ids.png', __FILE__));
  add_submenu_page('idsexpose_menu', 'Settings', 'Settings', 'manage_options', 'options-general.php?page=idsexpose');
  add_submenu_page('idsexpose_menu', 'Help', 'Help', 'manage_options', 'idsexpose_help', 'idsexpose_help_page');
}


