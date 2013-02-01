<?php

define('IDS_PLUGINS_SCRIPTS_PATH', 'scripts/');

//--------------------------------- Custom template tags --------------------------

// IDS date_updated tag
function ids_date_updated($before = '', $after = '') {
  global $post;
  $date = (isset($post->date_updated) ? $post->date_updated : get_post_meta($post->ID, 'date_updated', TRUE));
  if ($date) {
    echo $before . $date . $after;
  }
}

// IDS acronym tag
function ids_acronym($before = '', $after = '') {
  global $post;
  $acronym = (isset($post->acronym) ? $post->acronym : get_post_meta($post->ID, 'acronym', TRUE));
  if ($acronym) {
    echo $before . $acronym . $after;
  }
}

// IDS location_country tag
function ids_location_country($before = '', $after = '') {
  global $post;
  $location_country = (isset($post->location_country) ? $post->location_country : get_post_meta($post->ID, 'location_country', TRUE));
  if ($location_country) {
    echo $before . $location_country . $after;
  }
}

// IDS location_country tag
function ids_organisation_type($before = '', $after = '') {
  global $post;
  $organisation_type = (isset($post->organisation_type) ? $post->organisation_type : get_post_meta($post->ID, 'organisation_type', TRUE));
  if ($organisation_type) {
    echo $before . $organisation_type . $after;
  }
}

// IDS organisation_url tag
function ids_organisation_url($before = '', $after = '') {
  global $post;
  $organisation_url = (isset($post->organisation_url) ? $post->organisation_url : get_post_meta($post->ID, 'organisation_url', TRUE));
  if ($organisation_url) {
    echo $before . ids_new_html_link($organisation_url, $organisation_url) . $after;
  }
}

// IDS authors tag
function ids_authors($before = '', $sep = ', ', $after = '') {
  global $post;
  $authors = (isset($post->authors) ? $post->authors : get_post_meta($post->ID, 'authors', FALSE));
  if ($authors) {
    $authors = implode($sep, $authors);
    if ($authors) {
      echo $before . $authors . $after;
    }
  }
}

// IDS urls tag
function ids_external_urls($before = '', $sep = ', ', $after = '') {
  global $post;
  $urls = (isset($post->urls) ? $post->urls : get_post_meta($post->ID, 'urls', FALSE));
  if ($urls) {
    $urls = implode($sep, array_map('ids_new_html_link', $urls, $urls));
    if ($urls) {
      echo $before . $urls . $after;
    }
  }
}

// IDS countries tag
// TODO. Include links passing the countries as parameters to idsview_documents / idsview_organisations.
function ids_countries($before = '', $sep = ', ', $after = '') {
  global $post;
  $countries = (isset($post->ids_countries) ? $before . implode($sep, array_map('ids_link_category', $post->ids_countries)) . $after : ids_get_list_categories($post->ID, 'countries', $before, $sep, $after ));
  if (($countries) && (!is_wp_error($countries))) {
    echo $countries;
  }
}

// IDS regions tag
function ids_regions($before = '', $sep = ', ', $after = '') {
  global $post;
  $regions = (isset($post->ids_regions) ? $before . implode($sep, array_map('ids_link_category', $post->ids_regions)) . $after : ids_get_list_categories($post->ID, 'regions', $before, $sep, $after ));
  if (($regions) && (!is_wp_error($regions))) {
    echo $regions;
  }
}

// IDS themes tag
function ids_themes($before = '', $sep = ', ', $after = '') {
  global $post;
  $themes = (isset($post->ids_themes) ? $before . implode($sep, array_map('ids_link_category', $post->ids_themes)) . $after : ids_get_list_categories($post->ID, 'themes', $before, $sep, $after ));
  if (($themes) && (!is_wp_error($themes))) {
    echo $themes;
  }
}

// Returns IDS categories attached to post (the post could have been assigned both Eldis and Bridge categories).
function ids_get_list_categories($post_id, $category, $before, $sep, $after) {
  global $ids_datasets;
  $terms = '';
  foreach ($ids_datasets as $dataset) {
    $taxonomy_name = $dataset . '_' . $category;
    if (taxonomy_exists($taxonomy_name)) {
      if ($list_terms = get_the_term_list($post_id, $taxonomy_name, $before, $sep, $after)) {
        $terms .= $list_terms;
      }
    }
  }
  return $terms;
}

//-------------------- Generation of HTML and javascript code -------------------

// Shows a message to the user at the top of an admin page.
function ids_show_message($message, $visibility = 'block') {
?>
  <div class="updated" style="display:<?php echo $visibility; ?>>;">
    <p>
    <?php
      echo $message;
    ?>
    </p>
  </div>
<?php
}

// Generate select boxes.
function  ids_select_box($select_name, $id, $items, $selected_value = '', $multiple = FALSE, $class = FALSE) {
	$html = "<select id='$id' name='$select_name'";
  if ($multiple) {
    $html .= ' multiple="multiple"';
  }
  if ($class) {
    $html .= ' class="' . $class . '"';
  }
  $html .= '>';
  $keys = array_keys($items);
	foreach($keys as $key) {
    $value = $items[$key];
    $html .= "<option value='$key' ";
    if ($key == $selected_value) {
      $html .= 'selected';
    }
    $html .= '>';
    $html .= $value;
    $html .= '</option>';
	}
	$html .= '</select>';
  return $html;
}

// Generate input field.
function ids_input_field($name, $size, $value, $id, $label, $description, $class_field, $required) {
	$html  = "<input name='$name' id='$id' class='$class_field' type='text' value='$value' size='$size' aria-required='$required' />";
	$html .= "<p class='description'>$description</p>";
  return $html;
}

// Generate <tr> field wrapper.
function ids_tr_field_wrapper($class, $field_name, $label, $contents) {
	$html  = "<tr class='$class'>";
	$html .= "	<th scope='row' valign='top'>";
	$html .= "		<label for='$field_name'>$label</label>";
	$html .= "	</th>";
	$html .= "	<td>";
  $html .= "    $contents";
	$html .= "	</td>";
	$html .= "</tr>";
  return $html;
}

// Generate <div> field wrapper.
function ids_div_field_wrapper($class, $field_name, $label, $contents) {
	$html  = "<div class='$class'>";
  if ($field_name && $label) {
    $html .= "	<label for='$field_name'>$label</label>";
  }
  $html .= "    $contents";
	$html .= "</div>";
  return $html;
}

// Given a category object, returns a link to it. Used in idsview_documents/idsview_organisations.
function ids_link_category($category) {
  $baseurl = get_page_link();
  if (isset($category->name) && isset ($category->object_type) && isset($category->object_id)) {
    $object_type = strtolower($category->object_type);
    $category_name = rtrim($category->name, " ($category->object_id)");
    $par = ((preg_match('[\?]', $baseurl)) ? '&' : '?');
    $url = '<a href="'. $baseurl . $par . $object_type . "=$category->object_id" . '">' . $category_name . '</a>';
    return $url;
  }
}

// Create and return link
function ids_new_html_link($link, $text) {
  return '<a href="' . esc_url( $link ) . '" rel="tag">' . $text . '</a>';
}

// Generate array of categories that correspond to the indicated dataset.
function ids_get_category_array($api_key, $dataset, $category) {
  $idsapi = new IdsApiWrapper;
  switch ($category) {
    case 'countries':
      $array_category = $idsapi->search('countries', $dataset, $api_key)->getArrayTitles();
      break;
    case 'regions': 
      $array_category = $idsapi->search('regions', $dataset, $api_key)->getArrayTitles();
      break;
    case 'themes': 
      $array_category = $idsapi->search('themes', $dataset, $api_key)->getArrayTitles('--');
      break;
  }
  return $array_category;
}

// Creates javascript arrays from category arrays retrieved by the API.
function ids_js_array($array_category) {
  $js_code = array();
  foreach ($array_category as $key => $value) {
    $value = addslashes($value);
    $js_code[] = "'$key':'$value'";
  }
  $js_array = "{\n" . implode(",\n", $js_code) . '};';
  return $js_array;
}

// Creates javascript arrays to populate trees from category arrays retrieved by the API.
function ids_js_tree($array_category) {
  if (!empty($array_category)) {
    $current_level = 0;
    $js_tree = '';
    foreach ($array_category as $cat_id => $label) {
      $level = substr_count($label, '--') + 1;
      $tabs = '';
      for ($i = 1; $i <= $level; $i++) {
        $tabs .= '  ';
      }
      if ($current_level == 0) {
        $current_level = 1;
        $js_tree .= '[' . "\n" . $tabs . '{' ;
      }
      elseif ($level < $current_level) {
        $step = $current_level - $level;
        $current_level = $level;
        $js_tree .= '}';
        for ($i = 1; $i <= $step; $i++) {
          $js_tree .= ' ]}';
        }
        $js_tree .=  ',' . "\n" . $tabs . ' {';
      }
      elseif ($level > $current_level) {
        if ($level - $current_level > 1) { 
          $level = $current_level+1;
        }
        $current_level = $level;
        $js_tree .= ',' . "\n" . $tabs . ' items:['. "\n" . $tabs . ' {';
      }
      else {
        $js_tree .= '},' . "\n" . $tabs . ' {';
      }
      $js_tree .= 'label:"' . addslashes(trim($label, '-')) . '", value:"' . $cat_id . '"';
    }
    $js_tree .= '}' . "\n";
    for ($i = 1; $i < $current_level; $i++) {
      $js_tree .= ' ]}';
    }
    $js_tree .= "\n" . ' ];' . "\n";
  }
  else {
    $js_tree = '{}';
  }
  return $js_tree;
}

// Converts a PHP array into a javascript one.
function ids_php2js($php_array = array()) {
  if (empty($php_array)) {
    $js_array = '{}';
  }
  else {
    $js_array = "['" . implode("','", $php_array) . "']";
  }
  return $js_array;
}

// Initialize category arrays for each dataset in a javascript variable, default dataset and keep track of selected categories.
function ids_init_javascript($api_key, $api_key_validated, $default_dataset, $categories, $categories_mappings = array(), $default_user = FALSE) {
  global $ids_datasets;
  ?>
  <script type="text/javascript">
  var $jqorig = jQuery.noConflict();
  </script>
  <script type='text/javascript' src='<?php echo plugins_url(IDS_PLUGINS_SCRIPTS_PATH . 'jqwidgets/scripts/jquery-1.8.2.min.js', __FILE__); ?>'></script>
  <script type="text/javascript">
  var $jqtree = jQuery.noConflict();
  </script>
  <script type='text/javascript' src='<?php echo plugins_url(IDS_PLUGINS_SCRIPTS_PATH . 'chosen/chosen.jquery.min.js', __FILE__); ?>'></script>
  <script type="text/javascript">
  var $jqchosen = jQuery.noConflict();
  </script>
  <script type="text/javascript">

  ids_array_categories = {'eldis':{'countries':{}, 'regions':{}, 'themes':{}}, 'bridge':{'countries':{}, 'regions':{}, 'themes':{}}};
  ids_array_categories_mappings = {'eldis':{'countries':{}, 'regions':{}, 'themes':{}}, 'bridge':{'countries':{}, 'regions':{}, 'themes':{}}};

  ids_array_trees_themes = {'eldis':{}, 'bridge':{}};

  selected_categories = {'eldis':{}, 'bridge':{}};
  selected_categories_mappings = {'eldis':{}, 'bridge':{}};

  default_dataset = '<?php echo $default_dataset; ?>';

  /* Arrays with previously selected categories and mappings */
  <?php foreach ($ids_datasets as $dataset) { ?>
  selected_categories['<?php echo $dataset; ?>'] = {
    'countries':<?php echo ids_php2js($categories['countries'][$dataset]); ?>,
    'regions':<?php echo ids_php2js($categories['regions'][$dataset]); ?>,
    'themes':<?php echo ids_php2js($categories['themes'][$dataset]); ?>
  };
  <?php if (!empty($categories_mappings)) { ?>
  selected_categories_mappings['<?php echo $dataset; ?>'] = {
    'countries':<?php echo ids_php2js($categories_mappings['countries'][$dataset]); ?>,
    'regions':<?php echo ids_php2js($categories_mappings['regions'][$dataset]); ?>,
    'themes':<?php echo ids_php2js($categories_mappings['themes'][$dataset]); ?>
  };
  <?php } ?>
  <?php } ?>

  function initCategoriesArrays() {
  <?php
  $ids_array_categories = array();
  if ($api_key_validated) {
    $types_category = array_keys($categories);
    foreach ($ids_datasets as $dataset) {
      $ids_array_categories[$dataset] = array('countries', 'regions', 'themes');
      foreach ($types_category as $category) {
        $ids_array_categories[$dataset][$category] = ids_get_category_array($api_key, $dataset, $category);
        // Populate the sources of the select boxes for categories in filters and mappings.
        echo "\nids_array_categories['$dataset']['$category'] = " . ids_js_array($ids_array_categories[$dataset][$category]);
      }
      // Populate sources of the tree widgets.
      // In the future this and ids_array_categories will be unified so as not to have duplicated data. */
      echo "\nids_array_trees_themes['$dataset'] = " . ids_js_tree($ids_array_categories[$dataset]['themes']);
    } 
  }
  ?>
  }

  <?php if ($default_user) { ?>
  default_user = '<?php echo $default_user; ?>';
  <?php } ?>
  </script>
  <?php
}






