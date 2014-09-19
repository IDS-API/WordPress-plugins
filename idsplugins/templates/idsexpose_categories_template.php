<?php
/**
 * Custom XML feed generator template.
 */
global $wpdb;
global $default_taxonomy_fields;
$taxonomy = (get_query_var('taxonomy')) ? get_query_var('taxonomy') : 'category';
$exposed_standard_fields = idsapi_variable_get('idsexpose', 'standard_fields_'.$taxonomy, $default_taxonomy_fields);
$exposed_custom_fields = idsapi_variable_get('idsexpose', 'custom_fields_'.$taxonomy, array());
$num_items = (isset($_GET['num_items'])) ? $_GET['num_items'] : get_option('posts_per_rss');
$current_link = add_query_arg('taxonomy', $taxonomy, site_url() . '?feed=ids_categories');
$current_link = add_query_arg('num_items', $num_items, $current_link);
$offset = (isset($_GET['offset'])) ? $_GET['offset'] : 0;
if (defined('IDS_IMPORT_TAXONOMY')) {
  $table_metadata = $wpdb->prefix . IDS_IMPORT_TAXONOMY . 'meta';
  $metadata_exists = ($wpdb->get_var("SHOW TABLES LIKE '$table_metadata'") == $table_metadata);
}
else {
  $metadata_exists = FALSE;
}
if (is_string($taxonomy)) {
  $taxonomies = array($taxonomy);
}
elseif (is_array($taxonomy)) {
  $taxonomies = $taxonomy;
}
$total_terms = get_terms($taxonomies, array('fields' => 'count', 'hide_empty' => FALSE));
if ($total_terms) {
  $args = array( 
    'number' => $num_items,
    'hide_empty' => FALSE,
    'offset' => $offset,
  );
  $terms = get_terms($taxonomies, $args);
  $next_page_offset =  ($offset + count($terms) < $total_terms) ? $offset + $num_items : 0;
  $prev_page_offset = ($offset) ? $offset - $num_items : 0;
}
header('Content-Type: ' . feed_content_type('rss-http') . '; charset=' . get_option('blog_charset'), true);
echo '<?xml version="1.0" encoding="'.get_option('blog_charset').'"?>';
?>
<channel lang="<?php bloginfo_rss('language'); ?>">
<response>
	<response_date><?php $time= current_time( 'mysql' ); echo $time ;?></response_date>
</response>
<metadata>
  <num_items><?php echo $num_items; ?></num_items>
	<identifier><?php echo site_url(); ?></identifier>
  <total_results><?php echo $total_terms; ?></total_results>
  <?php if ($next_page_offset) { ?>
  <next_page><?php echo htmlspecialchars(add_query_arg('offset', $next_page_offset, $current_link), ENT_QUOTES, 'UTF-8'); ?></next_page>
  <?php } ?>
  <?php if ($offset) { ?>
  <prev_page><?php echo htmlspecialchars(add_query_arg('offset', $prev_page_offset, $current_link), ENT_QUOTES, 'UTF-8'); ?></prev_page>
  <?php } ?>
</metadata>
<results>
<?php foreach($terms as $term) { ?>
	<item>  
    <?php 
      // Standard fields
      // TODO: Split tag and attributes when enabled in admin.
      foreach ($exposed_standard_fields as $field) {
        if (isset($term->{$field})) {
          $tag = apply_filters('exposed_tag', $field, $taxonomy);
          if ($value = apply_filters('exposed_term_value', $term->{$field}, $field, $taxonomy)) {
            idsexpose_print_xml($tag, $value);
          }
        }
      }
      // Custom (meta) fields
      if ($metadata_exists && !empty($exposed_custom_fields)) {
        if ($term_meta = get_metadata(IDS_IMPORT_TAXONOMY, $term->term_id)) {
          foreach ($term_meta as $field => $value) {
            if (in_array($field, $exposed_custom_fields)) {
              $tag = apply_filters('exposed_tag', $field, $taxonomy);
              if ($value = apply_filters('exposed_term_value', $value, $field, $taxonomy)) {
                idsexpose_print_xml($tag, $value);
              }
            }
          }
        }
      }
    ?>
	</item>
<?php } // foreach ?>
</results>
</channel>