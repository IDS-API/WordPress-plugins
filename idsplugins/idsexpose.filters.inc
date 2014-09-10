<?php
/*
  This file shows how tag names and values can be modified by filters.
  The following filters are called by the plugin:

  - exposed_tag($field, $type); - Alters field names to generate tags for posts and terms ($type can be a post type or taxonomy name).
  - exposed_post_value($value, $field, $post_type); - Alters values for field name $tag in posts.
  - exposed_term_value($value, $field, $taxonomy); - Alters values for field name $tag in taxonomy terms.

  If a mapping is defined for a post type or taxonomy's field, it will be applied before calling to the
  exposed_post_tag_{$tag} or exposed_term_tag_{$tag} filters.

  These filters are called *within the loop*, when generating the feeds.

*/

add_filter('exposed_post_value', 'idsexpose_value_post', 10, 3);
add_filter('exposed_term_value', 'idsexpose_value_term', 10, 3);
add_filter('exposed_tag', 'idsexpose_tag', 10, 2);

// Applies the mappings defined in the IDS Expose plugin's settings to generate the feed's tags.
function idsexpose_tag($field, $type) {
  $field_mappings = idsapi_variable_get('idsexpose', 'field_mappings', array());
  $tag = (isset($field_mappings[$type]) && isset($field_mappings[$type][$field])) ? $field_mappings[$type][$field] : $field;
  return $tag;
}

// Alters the values of posts' fields when generating the feeds.
function idsexpose_value_post($value, $field, $post_type) {
  switch ($field) {
    case 'metadata_url':
      $value = preg_replace('/\?_token_guid=[\w\-]+/', '', $value);
      break;
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

// Alters the values of terms' fields when generating the feeds.
function idsexpose_value_term($value, $field, $taxonomy) {
  switch ($field) {
    case 'metadata_url':
      $value = preg_replace('/\?_token_guid=[\w\-]+/', '', $value);
      break;
		case 'name':
      $value = (function_exists('idsimport_filter_the_category') && (preg_match('/(eldis|bridge)_/', $taxonomy))) ? idsimport_filter_the_category($value) : $value;
      break;
		case 'slug':
      $value = get_term_link($value, $taxonomy);
      break;
  }
  return $value;
}


