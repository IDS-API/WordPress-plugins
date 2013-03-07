<?php

/**
* Class IdsApiWrapper.
*
* Objects of this class are used to retrieve content in the IDS collections (Eldis, BRIDGE) through the IDS API.
* 
*/

require_once('idsapi.default.inc'); 
require_once('idsapi.object.inc'); 
require_once('idsapi.request.inc'); 
require_once('idsapi.response.inc');

switch (IDS_API_ENVIRONMENT) {
  case 'drupal':
    require_once('cms/idsapi.drupal.inc');
    break;
  case 'wordpress':
    // In this case we also use the generic functions.
    require_once('cms/idsapi.wordpress.inc');
  default:
    require_once('idsapi.generic.inc');
}

class IdsApiWrapper {

  // Current request object.
  private $request;

  /**
  * Method used to retrieve a set of items in the IDS collection (by a 'search' request to the API).
  *
  * @param string $type - type of object(s) requested (documents, organisations, themes, regions, countries).
  * @param string $set - dataset from where to retrieve the information (eldis, bridge).
  * @param string $api_key - IDS API token guid.
  * @param string $format - format of the responses (short, full) (default 'full').
  * @param int $num_requested - the number of results requested (default 0, means all available results).
  * @param int $age_results - only results published in this last number of days are retrieved (default 0, means no limit).
  * @param array $params - unencoded filters to be added to the request (eg. array('publisher' => 'UNDP', 'country' => 'Costa Rica|Nicaragua')).
  * @param array $extra_fields - extra fields to be returned (in short responses). (eg. array('headline', 'category_theme_array')).
  * @return IdsApiResponse - response object holding the data retrieved by the API call.
  */
  public function search($type, $set, $api_key, $format = 'full', $num_requested = 0, $age_results = 0, $params = array(), $extra_fields = array()) {
    $this->request = new IdsApiRequest('search', $type, $format, $set, $api_key);
    $this->request->setNumRequested($num_requested);
    if ($age_results) {
      $this->request->setAgeResults($age_results);
    }
    foreach ($params as $key => $value) {
      $this->request->setParam($key, $value);
    }
    foreach ($extra_fields as $field) {
      $this->request->setExtraField($field);
    }
    $response = $this->makeRequest();
    return $response;
  }

  /**
  * Method used to retrieve one item in the IDS collection (by a 'get' request to the API).
  * @param string $type - type of object(s) requested (assets, documents, organisations, themes, regions, countries).
  * @param string $set - dataset from where to retrieve the information (eldis, bridge).
  * @param string $api_key - IDS API token guid.
  * @param string $format - format of the responses (short, full) (default 'full').
  * @param string $object_id - object_id value of the object to be retrieved.
  * @return IdsApiResponse - response object holding the data retrieved by the API call.
  */
  public function get($type, $set, $api_key, $format = 'full', $object_id) {
    $this->request = new IdsApiRequest('get', $type, $format, $set, $api_key);
    $this->request->setObjectId($object_id);
    $response = $this->makeRequest();
    return $response;
  }

  /**
  * Method used to retrieve all items of a specific type in the IDS collection (by a 'get_all' request to the API).
  * @param string $type - type of object(s) requested (assets, documents, organisations, themes, regions, countries).
  * @param string $set - dataset from where to retrieve the information (eldis, bridge).
  * @param string $api_key - IDS API token guid.
  * @param string $format - format of the responses (short, full) (default 'full').
  * @return IdsApiResponse - response object holding the data retrieved by the API call.
  */
  public function get_all($type, $set, $api_key, $format = 'full') {
    $this->request = new IdsApiRequest('get_all', $type, $format, $set, $api_key);
    $response = $this->makeRequest();
    return $response;
  }

  /**
  * Make a call to the API in order to validate the key.
  * @param string $api_key - IDS API token guid.
  */
  public function validateKey($api_key) {
    return (!$this->search('regions', IDS_API_DEFAULT_DATASET, $api_key, 'short')->isEmpty());
  }

  /**
  * Builds the URL, checks if the request is in the cache, and if not makes the call and saves it in the cache.
  * @return IdsApiResponse - response object holding the data retrieved by the API call.
  */
  protected function makeRequest() {
    $cached = FALSE;
    // Build the URL.
    $this->request->setUrl();
    // Check if it's in the cache.  If it's not, make the call and save it in the cache.
    if ($data = $this->cacheGet()) {
      $cached = TRUE;
      list($results, $total_results) = $data;
    }
    else {
      list($results, $total_results) = $this->getDecodedResults($this->request->getUrl(), 0);
      $this->cacheSet(array($results, $total_results));
    }
    $response = new IdsApiResponse($results, $this->request->getFormat(), $this->request->getObjectType(), $total_results, $this->request->getSite(), $cached);
    return $response;
  }

  /**
  * Make the actual API calls, implementing pagination and returning the decoded data.
  * @param string $url - API URL to get the data from.
  * @param int $num_retrieved - number of items retrieved so far (initially, 0).
  * @return array - array containing two elements: an array with the 'results' part of the response and 
  *                 the total number of results available in the dataset (as indicated in the 'metadata' part of the response).
  */
  protected function getDecodedResults($url, $num_retrieved) {
    $data = $this->getResponseData($url); 
    if (is_array($data) && !empty($data)) {
      $results = $this->getResponseResults($data);
      $metadata = $this->getResponseMetadata($data);
      $total_results = $metadata['total_results'];
      $num_results = count($results);
      $num_retrieved += $num_results;
      if (($total_results > $num_retrieved) && (($num_retrieved < $this->request->getNumRequested()) || ($this->request->getNumRequested() == 0))) {
        if (isset($metadata['next_page'])) {
          $next_page_url = $metadata['next_page'];
          list($new_results) = $this->getDecodedResults($next_page_url, $num_retrieved);
          if (is_array($new_results)) {
            $results = array_merge_recursive($results, $new_results);
          }
        }
      }
    }
    else {
      $results = $data;
      $total_results = 0;
    }
    return array($results, $total_results);
  }

  /**
  * Retrieve the URL.
  * We check first if there is a function that overrides this functionality (as in the case of Wordpress or Drupal).
  * @param string $url - API URL to get the data from.
  * @return array $data - array containing the decoded JSON object retrieved by the API call.
  */
  protected function getResponseData($url) {
    // In other environments (like Drupal or Wordpress) different implementations of this function are provided.
    // Commented until further analysed, as there seems to be a problem when using wp_remote_get now.
    /* if (function_exists('idsapi_get_data')) {
      $data = idsapi_get_data($url);
    } */
    // If cURL is available, we use it.
    if (function_exists('curl_init')) {
      $ch = curl_init($url);
      curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
      $content = curl_exec($ch);
      curl_close($ch);
      $data = json_decode($content, true);
      if (is_array($data) && !isset($data['results']) && isset($data['detail'])) {
        $data = $data['detail'];
      }
    }
    // Otherwise, we use file_get_contents.
    else {
      $content = @file_get_contents($url);
      if (!$content) {
        $error = error_get_last();
        $data = preg_replace('/(.*) (HTTP.*)/', '$2', $error['message']);
      }
      else {
        $data = json_decode($content, true);
      }
    }
    return $data;
  }

  /**
  * Get the results part of the response.
  * @param array $results - array containing the 'results' part of the response.
  * @return array $data - array containing the decoded JSON object retrieved by the API call.
  */
  protected function getResponseResults($data) {
    $results = array();
    if (isset($data['results'])) {
      $results = $data['results'];
      if (isset($results['object_id'])) { //When there's only one response
        $results = array($results);
      }
    }
    return $results;
  }

  /**
  * Get the metadata part of the response.
  * @param array $results - array containing the 'results' part of the response.
  * @return array $metadata - array containing the decoded JSON object retrieved by the API call.
  */
  protected function getResponseMetadata($data) {
    $metadata = array();
    if (isset($data['metadata'])) {
      $metadata = $data['metadata'];
    }
    else {
      $metadata['total_results'] = 1;
    }
    return $metadata;
  }
  
  /**
  * Retrieve the cache.
  * @return string - text of the cached response corresponding to the current request.
  */
  protected function cacheGet() {
    static $items = array();
    $cid = $this->cacheId();
    if (!isset($items[$cid])) {
      $items[$cid] = idsapi_cache_get($cid, IDS_API_CACHE_REQUESTS);
    }
    return $items[$cid];
  }

  /**
  * Populate the cache.
  * @param array $data - text of the response corresponding to the current request to be cached.
  */
  protected function cacheSet($data) {
    if ($data === FALSE) {
      // If we don't get a valid response we set a temporary cache to avoid making repeated failed requests in a short period of time.
      idsapi_cache_set($this->cacheId(), FALSE, IDS_API_CACHE_REQUESTS, 300);
    }
    else {
      $expire = IDS_API_DEFAULT_CACHE_TIME;
      idsapi_cache_set($this->cacheId(), $data, IDS_API_CACHE_REQUESTS, $expire);
    }
  }

  /**
  * Generate a cache item id based on the hash of the url of the current request.
  * @return string - id by which to identify the cached response of the current request.
  */
  protected function cacheId() {
    return get_class($this) .':'. md5($this->request->getUrl());
  }

  /**
  * Delete the cache entry for the current request.
  */
  public function cacheClear() {
    idsapi_cache_clear($this->cacheId());
  }

  /**
  * Delete all the cache entries.
  */
  public function cacheFlush() {
    idsapi_cache_flush();
  }

} // Class IdsApiWrapper
