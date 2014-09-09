<?php
/**
 * Custom XML feed generator template.
 * Adapted from Javier Corre's Custom XML feed plugin (http://javiercorre.com/)
 */
global $wpdb;
$taxonomy = (get_query_var('taxonomy')) ? get_query_var('taxonomy') : 'category';
$num_items = (isset($_GET['num_items'])) ? $_GET['num_items'] : get_option('posts_per_rss');
$current_link = add_query_arg('taxonomy', $taxonomy, site_url() . '?feed=ids_categories');
$current_link = add_query_arg('num_items', $num_items, $current_link);
$offset = (isset($_GET['offset'])) ? $_GET['offset'] : 0;
$table_metadata = $wpdb->prefix . IDS_IMPORT_TAXONOMY . 'meta';
$metadata_exists = ($wpdb->get_var("SHOW TABLES LIKE '$table_metadata'") == $table_metadata);
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
echo '<?xml version="1.0" encoding="'.get_option('blog_charset').'"?'.'>';
?>
<channel>
<response>
	<response_date><?php $time= current_time( 'mysql' ); echo $time ;?></response_date>
	<last_build_date></last_build_date>
	<language><?php bloginfo_rss( 'language' ); ?></language>
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
  <?php
    if (function_exists('idsimport_filter_the_category')) { // Generalize with callback functions for all the fields.
      $term_name = idsimport_filter_the_category($term->name);
    }
    else {
      $term_name = $term->name;
    }
  ?>
	<item>
  <title><?php echo $term_name; ?></title>
	<guid><?php echo get_term_link($term); ?></guid>
  <?php $description = isset($term->description) ? $term->description : ''; ?>
  <description><?php echo $description; ?></description>
  <?php
    if ($metadata_exists) {
      if ($term_meta = get_metadata(IDS_IMPORT_TAXONOMY, $term->term_id)) {
        foreach ($term_meta as $key => $values) {
          if (is_array ($values)) {
            foreach ($values as $value) {
              echo "<$key>" . htmlspecialchars($value, ENT_QUOTES, 'UTF-8') . "</$key>";
            }
          }
          elseif (is_scalar($values)) {
            echo "<$key>" . htmlspecialchars($values, ENT_QUOTES, 'UTF-8') . "</$key>";
          }
        }
      }
    }
  ?>
	</item>
<?php } // foreach ?>
</results>
</channel>