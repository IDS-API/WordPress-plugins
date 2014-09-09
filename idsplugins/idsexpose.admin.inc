<?php

// Checks if it has to display the admin settings or if it has to initialize the plugin's data, etc.
function idsexpose_admin_main() {
  if (!current_user_can( 'manage_options')) {
    wp_die(__('You do not have sufficient permissions to manage the plugin settings.'));
  }
  idsexpose_render_form();
}

// Display registered error messages
function idsexpose_admin_notices() {
  settings_errors('idsexpose_options', FALSE, TRUE);
  idsapi_report_errors();
}

// Render the plugin's main admin form.
function idsexpose_render_form() {
  $select_post_types = idsexpose_get_post_types();
  $select_taxonomies = idsexpose_get_taxonomies();
  $wp_fields_mappings = array('post_title' => 'title', 'post_content' => 'description');
  $ids_fields_mappings = $wp_fields_mappings + array('bridge_object_id' => 'object_id', 'eldis_object_id' => 'object_id');
  $default_mappings = array('post' => $wp_fields_mappings, 'ids_documents' => $ids_fields_mappings, 'ids_organisations' => $ids_fields_mappings);
  $current_mappings = idsapi_variable_get('idsexpose', 'field_mappings', $default_mappings);
  //$current_mappings = idsapi_variable_get('idsexpose', 'field_mappings', array());
	?>
  <div class="wrap">
	<div class="ids-wrap">
		<!-- Display Plugin Icon, Header, and Description -->
		<div class="icon32" id="icon-options-general"></div>
		<h2><?php _e('IDS Expose Plugin Settings'); ?></h2>
		<p>
      <?php printf(__('This plugin exposes content to be contributed to the IDS KS Hub.')); ?></br>
      <?php $admin_feeds_url = get_admin_url() . 'admin.php?page=idsexpose_create_feeds' ?>
      <?php printf(__('Multiple exposing URLs can be generated <a href="%s">in this page</a>, filtering content by several criteria.'), $admin_feeds_url); ?>
    </p>

    <!-- Beginning of the Plugin Options Form -->
		<form method="post" id="ids_expose_form" action="options.php" onSubmit="selectAllClass('idsexpose-mappings-select')">
      <?php
          settings_fields('idsexpose');
      ?>

    <div class="ui-tabs">
      <ul class="ui-tabs-nav">
        <li><a href="#content_types"><?php _e('Types of content'); ?></a></li>
        <li><a href="#mappings"><?php _e('Mappings'); ?></a></li>
      </ul>
      <h3><?php _e('Types of content'); ?></h3>
			<table class="form-table ui-tabs-panel" id="content_types">
        <tr>
          <td colspan="2">
            <p class="description">
            <?php _e('Please indicate the content that you want to make available to the exposer.'); ?><br>
            <?php _e('The fields post_title, post_content and guid will always be exposed, even if not selected.'); ?><br>
            </p>
          </td>
        </tr>

        <!-- Content types -->
        <tr>
          <th scope="row"><?php _e('Content types'); ?></th>
          <td>
            <p class="description">
            <?php _e('Please indicate, for each selected type of content, the fields that you would like to expose in addition to the standard Wordpress fields. Only fields with valid values will be exposed.'); ?> </br></br>
            </p>
            <?php $selected_types = idsapi_variable_get('idsexpose', 'content_types', array()); ?>
            <?php foreach ($select_post_types as $post_type => $label) { ?>
              <label><input id="<?php echo $post_type; ?>" class="idsexpose-content-types" name="idsexpose_options[content_types][<?php echo $post_type; ?>]" type="checkbox" 
              <?php echo (isset($selected_types[$post_type]) ? 'checked="yes"' : ''); ?> onchange="changeFields();" /> <strong><?php _e($label); ?></strong></label></br>
              <div class="<?php echo 'fields-' . $post_type; ?>" style="border:1px solid grey;width:400px;">
                <table>
                <tr>
                <td>
                <i>Standard Wordpress fields</i></br>
                <?php
                // Standard fields. Some fields are always included (title, guid).
                $standard_fields = ids_get_default_fields($post_type);
                $var_name = 'standard_fields_' . $post_type;
                $select_id = 'idsexpose_' . $var_name;
                $select_name = "idsexpose_options[$var_name][]";
                echo ids_select_box($select_name, $select_id, $standard_fields, idsapi_variable_get('idsexpose', $var_name, array()), array('multiple' => 'multiple'));
                ?>
                </br>
                <a href="javascript:selectAll('<?php echo $select_id; ?>');"><?php _e('Select all'); ?></a> / 
                <a href="javascript:deselectAll('<?php echo $select_id; ?>');"><?php _e('Deselect all'); ?></a>                
                </td>
                <td>
                <i>Custom fields</i></br>
                <?php
                // Custom fields
                $custom_fields = ids_get_type_custom_fields($post_type);
                $var_name = 'custom_fields_' . $post_type;
                $select_id = 'idsexpose_' . $var_name;
                $select_name = "idsexpose_options[$var_name][]";
                echo ids_select_box($select_name, $select_id, $custom_fields, idsapi_variable_get('idsexpose', $var_name, array()), array('multiple' => 'multiple'));
                ?>
                </br>
                <a href="javascript:selectAll('<?php echo $select_id; ?>');"><?php _e('Select all'); ?></a> / 
                <a href="javascript:deselectAll('<?php echo $select_id; ?>');"><?php _e('Deselect all'); ?></a>                
                </td>
                </tr>
                </table>
                <!--hr style="width:600px;margin-left:0;text-align:left;"-->
              </div>
              </br>
            <?php } ?>
          </td>
        </tr>
        <!-- Taxonomies -->
        <tr>
          <th scope="row"><?php _e('Taxonomies'); ?></th>
          <td>
            <p class="description">
            <?php _e('Select the taxonomies that you would like to expose.'); ?></br>
            <?php _e('Please indicate, for each taxonomy, the fields that you would like to expose. Only fields with valid values will be exposed.'); ?> </br></br>
            </p>
            <?php $selected_taxonomies = idsapi_variable_get('idsexpose', 'taxonomies', array()); ?>
            <?php foreach ($select_taxonomies as $taxonomy => $label) { ?>
              <label><input id="<?php echo $taxonomy; ?>" class="idsexpose-content-types" name="idsexpose_options[taxonomies][<?php echo $taxonomy; ?>]" type="checkbox" 
              <?php echo (isset($selected_taxonomies[$taxonomy]) ? 'checked="yes"' : ''); ?> onchange="changeFields();" /> <strong><?php _e($label); ?></strong></label></br>
              <!-- In the case of taxonomies, we only give the option to select fields for IDS taxonomies, as there is not a default way of implementing custom fields for taxonomies in Wordpress -->
              <?php // TODO: Generalise.
              if ($taxonomy == 'eldis_countries' || $taxonomy == 'eldis_regions' || $taxonomy == 'eldis_themes' || $taxonomy == 'bridge_countries' || $taxonomy == 'bridge_regions' || $taxonomy == 'bridge_themes') { ?>
              <div class="<?php echo 'fields-' . $taxonomy; ?>" style="border:1px solid grey;width:400px;">
                <table>
                <tr>
                <td>
                <i>Custom fields</i></br>
                <?php
                // Custom fields
                $custom_fields = ids_get_taxonomy_custom_fields($taxonomy);
                if (!empty($custom_fields)) {
                  $custom_fields = array_combine($custom_fields, $custom_fields);
                  natcasesort($custom_fields);
                }
                $var_name = 'custom_fields_' . $taxonomy;
                $select_id = 'idsexpose_' . $var_name;
                $select_name = "idsexpose_options[$var_name][]";
                echo ids_select_box($select_name, $select_id, $custom_fields, idsapi_variable_get('idsexpose', $var_name, array()), array('multiple' => 'multiple'));
                ?>
                </br>
                <a href="javascript:selectAll('<?php echo $select_id; ?>');"><?php _e('Select all'); ?></a> / 
                <a href="javascript:deselectAll('<?php echo $select_id; ?>');"><?php _e('Deselect all'); ?></a>                
                </td>
                </tr>
                </table>
                <!--hr style="width:600px;margin-left:0;text-align:left;"-->
              </div>
              <?php } ?>
              </br>
            <?php } ?>
          </td>
        </tr>
      </table>

      <!-- TODO: Generalize. Allow nested mappings -->
      <h3><?php _e('Mappings'); ?></h3>
      <table class="form-table ui-tabs-panel" id="mappings">
        <tr>
          <td colspan="2">
            <p class="description">
              <?php _e('Please indicate, for each field in the exposed types of content, if you would like to map it to a new XML element.'); ?>
            </p>
          </td>
        </tr>
        <?php foreach ($select_post_types as $post_type => $label) { ?>
        <tr class="<?php echo 'fields-' . $post_type; ?>">
          <th scope="row"><?php echo $post_type; ?></th>
          <td>
              <table id="<?php echo 'field-mappings-' . $post_type; ?>" style="border:1px solid grey">
                <tr>
                <td>
                <table style="border: 1px solid lightgrey";>
                <tr>
                  <td>
                  <p class="description"><?php _e("Standard Wordpress fields"); ?></p>
                  <?php
                  $select_mappings_id = 'select_field_mappings_'.$post_type;
                  $select_mappings_name = "idsexpose_options[select_field_mappings][$post_type][]";
                  // Standard fields
                  $standard_fields = ids_get_default_fields($post_type);
                  $select_standard_fields = 'standard_fields_mappings_' . $post_type;
                  $input_new_field = 'new_standard_field_mapping_'.$post_type;
                  echo ids_select_box($select_standard_fields, $select_standard_fields, $standard_fields, array());
                  ?>
                  <p class="description">&nbsp;</p>              
                  </td>
                  <td>
                    <p class="description"><?php _e("Map to:"); ?></p>
                    <input type="text" size="16" id="<?php echo $input_new_field; ?>" value="" />
                    <p class="description">
                      <a href="javascript:addFieldMapping('<?php echo $select_standard_fields?>', '<?php echo $input_new_field?>', '<?php echo $select_mappings_id; ?>');"><?php _e('Add mapping'); ?> &rarr;</a>
                    </p>
                  </td>
                </tr>
                <tr>
                  <td>
                  <p class="description"><?php _e("Custom fields"); ?></p>
                  <?php
                  // Custom fields
                  $custom_fields = ids_get_type_custom_fields($post_type);
                  $select_custom_fields = 'custom_fields_mappings_' . $post_type;
                  $input_new_field = 'new_custom_field_mapping_'.$post_type;
                  echo ids_select_box($select_custom_fields, $select_custom_fields, $custom_fields, array());
                  ?>
                  <p class="description">&nbsp;</p>              
                  </td>
                  <td>
                    <p class="description"><?php _e("Map to:"); ?></p>
                    <input type="text" size="16" id="<?php echo $input_new_field?>" value="" />
                    <p class="description">
                      <a href="javascript:addFieldMapping('<?php echo $select_custom_fields?>', '<?php echo $input_new_field?>', '<?php echo $select_mappings_id; ?>');"><?php _e('Add mapping'); ?> &rarr;</a>
                    </p>
                  </td>
                </tr>
                </table>
                </td>
                <td>
                <?php 
                /*
                echo ids_select_box($select_mappings_name, $select_mappings_id, $display_mappings, array(), array('size' => 8, 'class' => 'idsexpose-mappings-select', 'multiple' => 'multiple')); 
                */  
                ?>   
                <select name="<?php echo $select_mappings_name?>" id="<?php echo $select_mappings_id?>" class="idsexpose-mappings-select" size=8 multiple="multiple">
                <?php
                  if (isset($current_mappings[$post_type])) {
                    foreach ($current_mappings[$post_type] as $source => $destination) {
                      echo "<option name='$source' value='$source,$destination' >$source --> $destination</option>";
                    }
                  }
                ?>
                </select>            
                <p class="description">
                  <a href="javascript:removeMappings('<?php echo $select_mappings_id; ?>');">&larr; <?php _e('Remove selected'); ?></a>
                  </br>
                  <a href="javascript:selectAll('<?php echo $select_mappings_id; ?>');"><?php _e('Select all'); ?></a> /
                  <a href="javascript:deselectAll('<?php echo $select_mappings_id; ?>');"><?php _e('Deselect all'); ?></a>
                </p>
                </td>
                </tr>
                </table>
              </div>
            </td>

          </tr>
        <?php } ?>
      </table>
    </div>	<!-- UI tabs -->	

    <p class="submit">
      <input name="idsexpose_options[submit_save]" type="submit" class="button-primary" value="<?php _e('Save changes') ?>" />
    </p>
		</form>
  </div>
  </div>
  <?php
}

// Validate input.
function idsexpose_validate_options($input) {
  $field_mappings = array();
  $error = FALSE;
  $error_message = '';
  if (isset($input['select_field_mappings'])) {
    foreach($input['select_field_mappings'] as $type => $mappings) {
      $field_mappings[$type] = array();
      foreach ($mappings as $mapping) {
        list($source, $destination) = explode(',', $mapping);
        if (idsexpose_valid_xml_name($destination)) {
          $field_mappings[$type][$source] = $destination;
        }
        else {
          $error = TRUE;
          $error_message .= sprintf(__(' %s is not a valid XML tag.'), $destination);
        }
      }
    }  
  }
  if ($field_mappings) {
    $input['field_mappings'] = $field_mappings;
  }
  if ($error) {
    $input['setup_done'] = FALSE;
    add_settings_error('idsimport_options', 'idsimport_errors', $error_message);
  }
  else {
    $input['setup_done'] = TRUE;
  }
  return $input;
}

// Validate XML mapped tags.
// TODO: Change to allow attributes.
function idsexpose_valid_xml_name($qname) {
  $prefixedName = (!strpos($qname, ':') ? 'prefix:' : '') . $qname;
  try {
    new DOMElement($prefixedName, NULL, 'uri:ns');
    return TRUE;
  } catch (DOMException $e) {
    return FALSE;
  }
}


