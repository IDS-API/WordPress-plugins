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
	?>
	<div class="wrap">
		<!-- Display Plugin Icon, Header, and Description -->
		<div class="icon32" id="icon-options-general"></div>
		<h2><?php _e('IDS View Plugin Settings'); ?></h2>
		<p>
      <?php printf(__('This plugin exposes content ...'), IDS_API_HOME_URL, IDS_API_HOME_URL); ?>
    </p>

    <!-- Beginning of the Plugin Options Form -->
		<form method="post" id="ids_expose_form" action="options.php">
      <?php
          settings_fields('idsexpose');
      ?>

    <div class="ui-tabs">
      <ul class="ui-tabs-nav">
        <li><a href="#filters"><?php _e('Filter content to expose'); ?></a></li>
        <li><a href="#mappings"><?php _e('Mappings'); ?></a></li>
      </ul>
      <h3><?php _e('Filter content to expose'); ?></h3>
			<table class="form-table ui-tabs-panel" id="filters">
				<tr>
          <td>
          Here you can filter the content that you want to expose...
          <!-- There would be two types of filter: based on fields and content. -->
          </td>
        </tr>
      </table>

      <h3><?php _e('Mappings'); ?></h3>
      <table class="form-table ui-tabs-panel" id="mappings">
          <td>
          Here you can map your current fields to the IDS schema ...
          </td>
        </tr>
      </table>
    </div>	<!-- UI tabs -->	

    <p class="submit">
      <input name="idsexpose_options[submit_save]" type="submit" class="button-primary" value="<?php _e('Save changes') ?>" />
    </p>
		</form>
  <?php
}

// Validate input and the API key.
function idsexpose_validate_options($input) {
  // Here the admin options are validated.

  // The parameters will be available through get_option('name-of-parameter');

  // For instance, let's assume that two arrays are saved here as options, they can be retrieved by:
  // $ids_exposer_filters = get_option('ids_exposer_filters');
  // $ids_exposer_mappings = get_option('ids_exposer_mappings');

  }
  return $input;
}
