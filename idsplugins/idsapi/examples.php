<?php

/*
* STEP 1: The IdsApiWrapper class definition is included.
*/
require_once('idsapi.wrapper.inc');

/*
* STEP 2: A new IdsApiWrapper object is created.
*/
$idsapi = new IdsApiWrapper;

$valid_api_key = '9cff1f93-8bb7-4106-9933-224e4ea60c17';
$wrong_api_key = '9cff1f93-8bb7-4106-99999999999999999';

header('Content-Type: text/html');

echo "<b>A new wrapper object is created</b>\n";
echo '<pre>';
echo '$idsapi = new IdsApiWrapper;';
echo '</pre>';
echo '<hr>';

/*
* STEP 3: We call to IdsApiWrapper::search, IdsApiWrapper::get and IdsApiWrapper::get_all to retrieve data from the IDS collections.
*/

/*
* Example 1: Get titles, with filters.
*/
echo "<b>Example 1:</b> Get an array, indexed by object_id, with the titles of Eldis documents published by the London School of Economics between 2009 and 2012.\n\n";
echo '<pre>';
echo '$response = $idsapi->search(\'documents\', \'eldis\', $valid_api_key, \'short\', 0, 0, array(\'document_published_before\' => \'2012-12-31\', \'document_published_after\' => \'2009-01-01\', \'publisher_name\' => \'London School of Economics\'))' . "\n";
$response = $idsapi->search('documents', 'eldis', $valid_api_key, 'short', 0, 0, array('document_published_before' => '2012-12-31', 'document_published_after' => '2009-01-01', 'publisher_name' => 'London School of Economics'));
echo '$response->getArrayTitles(): ';
print_r($response->getArrayTitles());
echo '</pre>';
echo '<hr>';

/*
* Example 2: Get links, with filters.
*/
echo "<b>Example 2:</b> Get an array with links to the 15 most recent Eldis documents related to climate change in India.\n\n";
echo '<pre>';
echo '$response = $idsapi->search(\'documents\', \'eldis\', $valid_api_key, \'full\', 15, 0, array(\'country\' => \'India\', \'theme\' => \'climate change\'))' . "\n";
$response = $idsapi->search('documents', 'eldis', $valid_api_key, 'full', 15, 0, array('country' => 'India', 'theme' => 'climate change'));
echo '$response->getArrayLinks(): ';
print_r($response->getArrayLinks());
echo '</pre>';
echo '<hr>';

/*
* Example 3: "OR" in filters and extra fields.
*/
echo "<b>Example 3:</b> Search the three most recent Eldis organisations with keywords beginning with 'climate' or 'community' , retrieving information available in the 'short' format, plus the organisation's acronym and keywords.\n\n";
echo '<pre>';
echo '$response = $idsapi->search(\'organisations\', \'eldis\', $valid_api_key, \'short\', 3, 0, array(\'keyword\' => \'climate*|community*\'), array(\'acronym\', \'keyword\'))' . "\n";
$response = $idsapi->search('organisations', 'eldis', $valid_api_key, 'short', 3, 0, array('keyword' => 'climate*|community*'), array('acronym', 'keyword'));
echo '$response: ';
print_r($response);
echo '</pre>';
echo '<hr>';

/*
* Example 4: get.
*/
echo "<b>Example 4:</b> Get 'full' record for Eldis theme with object_id=C54.\n\n";
echo '<pre>';
echo '$response = $idsapi->get(\'themes\', \'eldis\', $valid_api_key, \'full\', \'C54\')' . "\n";
$response = $idsapi->get('themes', 'eldis', $valid_api_key, 'full', 'C54');
echo '$response: ';
print_r($response);
echo '</pre>';
echo '<hr>';

/*
* Example 5: get_all.
*/
echo "<b>Example 5:</b> Get all 'short' records for Bridge regions.\n\n";
echo '<pre>';
echo '$response = $idsapi->get_all(\'regions\', \'bridge\', $valid_api_key, \'short\')' . "\n";
$response = $idsapi->get_all('regions', 'bridge', $valid_api_key, 'short');
echo '$response: ';
print_r($response);
echo '</pre>';
echo '<hr>';

/*
* Example 6: Empty response.
*/
echo "<b>Example 6:</b> Bridge's organisations (empty set).\n\n";
echo '<pre>';
echo '$response = $idsapi->search(\'organisations\', \'bridge\', $valid_api_key)' . "\n";
$response = $idsapi->search('organisations', 'bridge', $valid_api_key);
echo '$response->isEmpty(): ' . $response->isEmpty() . "\n";
echo '$response->isError(): ' . $response->isError() . "\n";
echo '</pre>';
echo '<hr>';

/*
* Example 7: Wrong API key.
*/
echo "<b>Example 7:</b> Using a wrong API key.\n\n";
echo '<pre>';
echo '$response = $idsapi->search(\'documents\', \'bridge\', $wrong_api_key)' . "\n";
$response = $idsapi->search('documents', 'bridge', $wrong_api_key);
echo '$response->isEmpty(): ' . $response->isEmpty() . "\n";
echo '$response->isError(): ' . $response->isError() . "\n";
echo '$response->getErrorMessage(): ' . $response->getErrorMessage() . "\n";
echo '</pre>';
echo '<hr>';

/*
* Example 8: Wrong dataset.
*/
echo "<b>Example 8:</b> Using a wrong dataset.\n\n";
echo '<pre>';
echo '$response = $idsapi->search(\'documents\', \'wrong\', $valid_api_key)' . "\n";
$response = $idsapi->search('documents', 'wrong', $valid_api_key);
echo '$response->isEmpty(): ' . $response->isEmpty() . "\n";
echo '$response->isError(): ' . $response->isError() . "\n";
echo '$response->getErrorMessage(): ' . $response->getErrorMessage() . "\n";
echo '</pre>';

/*
* We flush all the cached requests as we are testing it and don't want to get the responses from the cache the next time we run it.
*/
$idsapi->cacheFlush();

