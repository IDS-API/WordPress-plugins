<?php
/**
 * Custom XML feed generator template.
 */
global $post;
$default_standard_fields = array('post_title', 'guid', 'post_content');
//TODO: Generalize taxonomies and add to admin.
$default_taxonomy_fields = array('name', 'term_id');
$ids_taxonomy_fields = array_merge($default_taxonomy_fields, array('object_id'));
$post_type = (get_query_var('post_type')) ? get_query_var('post_type') : 'post';
$exposed_custom_fields = idsapi_variable_get('idsexpose', 'custom_fields_'.$post_type, array());
$exposed_standard_fields = array_merge($default_standard_fields, idsapi_variable_get('idsexpose', 'standard_fields_'.$post_type, array()));
$paged = (get_query_var('paged')) ? get_query_var('paged') : 1;
$num_items = (isset($_GET['num_items'])) ? $_GET['num_items'] : get_option('posts_per_rss');
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
          $tag = idsexpose_get_tag($field, $post_type);
          if ($value = idsexpose_get_value($field, $post->{$field}, $post_type)) {
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
          $tag = idsexpose_get_tag($field, $post_type);
          if ($value = idsexpose_get_value($field, $value, $post_type)) {
            idsexpose_print_xml($tag, $value);
          }
        }
      }
    ?>
    <?php
      // Category terms
      foreach($taxonomies as $tax_name) { 
        echo "<$tax_name>";
        if (preg_match('/(eldis|bridge)_/', $tax_name)) {
          $taxonomy_fields = $ids_taxonomy_fields;
        }
        else {
          $taxonomy_fields = $default_taxonomy_fields;
        }
        if ($terms = get_the_terms($post->ID, $tax_name)) {
          foreach ($terms as $term) {
            echo "<list-item>";
            foreach ($taxonomy_fields as $field) {
              if (isset($term->{$field})) {
                idsexpose_print_xml($field, $term->{$field});
              }
            }
            echo "</list-item>";
          }
        }
        echo "</$tax_name>";
      }
    ?>
    <?php rss_enclosure(); ?>
    <?php do_action('rss2_item'); ?>
	</item>
<?php endwhile; ?>
</results>
</channel>