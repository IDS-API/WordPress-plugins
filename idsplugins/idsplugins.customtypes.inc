<?php

//----------------------------------- Custom post types ---------------------------

// Creates new post types for IDS assets.
function ids_post_types_init() {
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

//----------------------------------- Retrieve custom types / taxonomies ---------------------------


// This is only for IDS taxonomies, as there is no standard way of implementing custom fields in Wordpress taxonomies.
function ids_get_taxonomy_custom_fields($taxonomy) {
  $fields = array();
  if (preg_match('/(eldis|bridge)_/', $taxonomy)) {
    list($site, $type) = explode('_', $taxonomy);
    $object['site'] = $site;
    $fields = array_keys(get_class_vars(get_class(IdsApiObject::factory($object, $type))));
  }
  return $fields;
}

// This is general, for any custom type.
function ids_get_type_custom_fields($post_type) {
  $custom_fields = array();
  $keys = get_meta_keys();
  if ($keys) {
    natcasesort($keys);
    foreach ($keys as $key) {
      if (is_protected_meta($key, $post_type)) continue;
      $custom_fields[$key] = $key;
    }
  }
  return $custom_fields;
}

// This is for Wordpress default posts.
function ids_get_default_fields($post_type) {
  $default_fields = array();
  $post = get_default_post_to_edit($post_type);
  foreach ($post as $field => $value) {
    $default_fields[$field] = $field;
  }
  return $default_fields;
}

// Taxonomies default fields. Added 'slug', which is not really a field.
function ids_get_term_default_fields() {
  $fields = array (
              'term_id',
              'name',
              'slug',
              'term_group',
              'term_taxonomy_id',
              'taxonomy',
              'description',
              'parent',
              'count',
            );
  return array_combine($fields, $fields);
}




