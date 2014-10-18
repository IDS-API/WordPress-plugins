<?php

define('IDS_DEFAULT_LANGUAGE', 'en');

// Check if the permalinks configuration has changed and we need to recreate the rules.
function ids_check_permalinks_changed($plugin) {
  $permalinks = idsapi_variable_get($plugin, 'permalink_structure', '');
  $current_permalinks = get_option('permalink_structure');
  if ($permalinks !== $current_permalinks) {
    idsapi_variable_set($plugin, 'permalink_structure', $current_permalinks);
    return TRUE;
  }
  else {
    return FALSE;
  }
}

// Return available languages.
function ids_languages($all = FALSE) {
  $languages = array(
                'ar' => __('Arabic'),
                'en' => __('English'),
                'nl' => __('Dutch'),
                'fr' => __('French'),
                'it' => __('Italian'),
                'pt' => __('Portuguese'),
                'es' => __('Spanish'),
              );
  if ($all) {
    $languages = array( 'all' => __('All languages')) + $languages;    
  }
  return $languages;
}

function ids_exclude_asset($plugin, $asset) {
  $exclude = FALSE;
  $exclude_uris = idsapi_variable_get($plugin, 'exclude_uris', '');
  if ($exclude_uris && isset($asset->urls)) {
    $uris = explode(',', $exclude_uris);
    foreach($uris as $uri) {
      $uri = preg_quote(trim($uri), '/');
      $matches = preg_grep("/$uri/i", $asset->urls);
      if ($exclude = !empty($matches)) {
        break;
      }
    }
  }
  return $exclude;
}

function ids_excerpt_post($plugin, $content) {
  $ids_excerpt_length = idsapi_variable_get($plugin, 'excerpt_length', 0);
  if(strlen($content) > $ids_excerpt_length) {
    $excerpt = substr($content, 0, $ids_excerpt_length-3);
    $space = strrpos($excerpt, ' ');
    $excerpt = substr($excerpt, 0, $space);
    $excerpt .= '...';
  } else {
    $excerpt = $content;
  }
  return $excerpt;
}

function ids_get_translation($asset, $field, $lang_code, $get_default = TRUE) {
  $translation = '';
  if (isset($asset->language_array[$lang_code])) {
    if (isset($asset->language_array[$lang_code][$field])) {
      $translation = $asset->language_array[$lang_code][$field];
    }
  }
  if (!$translation && $get_default) {
    if (isset($asset->{$field})) {
      $translation = $asset->{$field};
    }
  }
  return $translation;
}
