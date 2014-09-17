<?php
/**
 * Custom XML feed generator template.
 */
global $post;
global $default_standard_fields;
global $default_taxonomy_fields;
global $ids_taxonomy_fields;

$post_type = (get_query_var('post_type')) ? get_query_var('post_type') : 'post';
$exposed_custom_fields = idsapi_variable_get('idsexpose', 'custom_fields_'.$post_type, array());
$exposed_standard_fields = idsapi_variable_get('idsexpose', 'standard_fields_'.$post_type, $default_standard_fields);
$paged = (get_query_var('paged')) ? get_query_var('paged') : 1;
$num_items = (isset($_GET['num_items'])) ? $_GET['num_items'] : get_option('posts_per_rss');
$table_metadata = $wpdb->prefix . IDS_IMPORT_TAXONOMY . 'meta';
if (defined('IDS_IMPORT_TAXONOMY')) {
  $table_metadata = $wpdb->prefix . IDS_IMPORT_TAXONOMY . 'meta';
  $metadata_exists = ($wpdb->get_var("SHOW TABLES LIKE '$table_metadata'") == $table_metadata);
}
else {
  $metadata_exists = FALSE;
}
if (is_string($post_type)) {
  $post_types = array($post_type);
}
elseif (is_array($post_type)) {
  $post_types = $post_type;
}
$args = array( 
  'post_type' => $post_types, 
  'posts_per_page' => $num_items,
  'paged' => $paged,
);
if (isset($_GET['cats'])) {
  $tax_operator = (isset($_GET['cats_op'])) ? $_GET['cats_op'] : 'OR';
  foreach($_GET['cats'] as $taxonomy => $terms) {
    $array_terms = explode('|', $terms);
    $tax_query[] = array('taxonomy' => $taxonomy, 'field' => 'id', 'terms' => $array_terms);
  }
  $tax_query['relation'] = $tax_operator;
  $args['tax_query'] = $tax_query;
}
$taxonomies = get_object_taxonomies($post_type);
$loop = new WP_Query($args); 
$next_page = ($paged < $loop->max_num_pages) ? $paged + 1 : 0;
$prev_page = ($paged > 1) ? $paged - 1 : 0;
header('Content-Type: ' . feed_content_type('rss-http') . '; charset=' . get_option('blog_charset'), true);
echo '<?xml version="1.0" encoding="'.get_option('blog_charset').'"?'.'>';
?>
<channel>
<response>
	<response_date><?php echo current_time('mysql'); ?></response_date>
	<last_build_date></last_build_date>
	<language><?php bloginfo_rss('language'); ?></language>
</response>
<metadata>
	<identifier><?php echo site_url(); ?></identifier>
  <total_results><?php echo $loop->found_posts; ?></total_results>
  <?php if ($next_page) { ?>
    <next_page><?php echo get_next_posts_page_link($loop->max_num_pages); ?></next_page>
  <?php } ?>
  <?php if ($prev_page) { ?>
    <prev_page><?php echo get_previous_posts_page_link(); ?></prev_page>
  <?php } ?>
</metadata>
<results>
<?php while ($loop->have_posts()) : $loop->the_post(); ?>
	<item>
    <?php
      // Type of asset (for IDS documents/organisations)
      // Generalize rewriting of values.
      if ($post_type == 'ids_documents') {
        echo '<object_type>Document</object_type>';
      }
      elseif ($post_type == 'ids_organisations') {
        echo '<object_type>Organisation</object_type>';
      }
    ?>
    <?php 
      // Standard (Wordpress) fields
      // TODO: Split tag and attributes when enabled in admin.
      foreach ($exposed_standard_fields as $field) {
        if (isset($post->{$field})) {
          $tag = apply_filters('exposed_tag', $field, $post_type);
          if ($value = apply_filters('exposed_post_value', $post->{$field}, $field, $post_type)) {
            idsexpose_print_xml($tag, $value);
          }
        }
      }
    ?>
    <?php
      // Custom (meta) fields
      $post_meta = get_post_meta($post->ID);
      if ($post_meta) {
        foreach ($post_meta as $field => $value) {
          if (in_array($field, $exposed_custom_fields)) {
            $tag = apply_filters('exposed_tag', $field, $post_type);
            if ($value = apply_filters('exposed_post_value', $post->{$field}, $field, $post_type)) {
              idsexpose_print_xml($tag, $value);
            }
          }
        }
      }
    ?>
    <?php
      // Category terms
      foreach($taxonomies as $tax_name) { 
        $exposed_standard_taxonomy_fields = idsapi_variable_get('idsexpose', 'standard_fields_'.$tax_name, $default_taxonomy_fields);
        $exposed_custom_taxonomy_fields = idsapi_variable_get('idsexpose', 'custom_fields_'.$tax_name, array());
        if ($terms = get_the_terms($post->ID, $tax_name)) {
          $tag_tax_name = apply_filters('exposed_tag', $tax_name, $post_type);
          echo "<$tag_tax_name-list>";
          foreach ($terms as $term) {
            echo "<$tag_tax_name>";
            // Standard taxonomy fields
            foreach ($exposed_standard_taxonomy_fields as $field) {
              if (isset($term->{$field})) {
                $tag = apply_filters('exposed_tag', $field, $tax_name);
                if ($value = apply_filters('exposed_term_value', $term->{$field}, $field, $tax_name)) {
                  idsexpose_print_xml($tag, $value);
                }
              }
            }
            // Custom (meta) fields
            if ($metadata_exists && !empty($exposed_custom_taxonomy_fields)) {
              if ($term_meta = get_metadata(IDS_IMPORT_TAXONOMY, $term->term_id)) {
                foreach ($term_meta as $field => $value) {
                  if (in_array($field, $exposed_custom_taxonomy_fields)) {
                    $tag = apply_filters('exposed_tag', $field, $tax_name);
                    if ($value = apply_filters('exposed_term_value', $value, $field, $tax_name)) {
                      idsexpose_print_xml($tag, $value);
                    }
                  }
                }
              }
            }
            echo "</$tag_tax_name>";
          }
          echo "</$tag_tax_name-list>";
        }
      }
    ?>
    <?php rss_enclosure(); ?>
    <?php do_action('rss2_item'); ?>
	</item>
<?php endwhile; ?>
</results>
</channel>