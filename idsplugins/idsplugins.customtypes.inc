<?php

//----------------------------------- Custom post types ---------------------------

// Creates new post types for IDS assets.
function ids_post_types_init() {
  global $wpdb;
  $ids_post_types = array('documents' => 'Document', 'organisations' => 'Organisations');
  foreach ($ids_post_types as $post_type => $singular_name) {
    $post_type_name = 'ids_' . $post_type;
    $post_type_label = 'IDS ' . ucfirst($post_type);
    ids_new_post_type($post_type_name, $post_type_label, $singular_name, TRUE);
  }
}

// Register a new post type.
function ids_new_post_type($post_type_name, $post_type_label, $singular_name) {
  global $wp_rewrite;
  global $ids_datasets;
  global $ids_categories;
  if (!post_type_exists($post_type_name)) {
    $labels = array(
      'name' => _x( $post_type_label, 'post type general name' ),
      'singular_name' => _x( $singular_name, 'post type singular name' ),
      'add_new' => __( 'Add New ' ) . __( $singular_name ),
      'all_items' => __( $post_type_label ),
      'add_new_item' => __( 'Add New ' ) . __( $singular_name ),
      'edit_item' => __( 'Edit ' ) . __( $singular_name ),
      'new_item' => __( 'New ' ) . __( $singular_name ),
      'view_item' => __( 'View ' ) . __( $singular_name ),
      'search_items' => __( 'Search ' ) . __( $post_type_label ),
      'not_found' =>  __( 'No ' ) . __( $post_type_label  ) . __( ' found' ),
      'not_found_in_trash' => __( 'No ' ) . __( $post_type_label ) . __( ' found in trash' ),
      'parent_item_colon' => __( 'Parent ' ) . __( $singular_name  ) . ':',
      'menu_name' => __( $post_type_label )
    );
    $args = array(
      'labels' => $labels,
      'description' => __( 'Assets retrieved via the IDS API'),
      'public' => true,
      'exclude_from_search' => false,
      'publicly_queryable' => true,
      'show_ui' => true, 
      'show_in_nav_menus' => true, 
      'show_in_menu' => false,
      'menu_icon' => '../images/icons32.png',
      'capability_type' => 'post',
      'hierarchical' => false,
      'supports' => array('title','editor','author','excerpt','custom-fields'),
      'has_archive' => true,
      'query_var' => true,
      'can_export' => true,
      'rewrite' => array('slug' => $post_type_name)
    );
    $args['taxonomies'] = array( 'post_tag', 'category' );
    foreach ($ids_datasets as $dataset) {
      foreach ($ids_categories as $category) {
        $ids_taxonomy_name = $dataset . '_' . $category;
        if (taxonomy_exists($ids_taxonomy_name)) {
          $args['taxonomies'][] = $ids_taxonomy_name;
        }
      }
    }
    register_post_type($post_type_name, $args);
    $wp_rewrite->flush_rules();
  }
}
