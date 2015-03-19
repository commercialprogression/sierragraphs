<?php

/**
 * Form for Sierra API auth.
 */
function sierragraphs_auth_admin_settings() {
  // Get Sierra API settings.
  $form = array(
    'sierragraphs' => array(
      '#title' => 'API Settings',
      '#type' => 'fieldset',
      '#weight' => 20,
      'sierragraphs_auth_client_id' => array(
        '#title' => t('API Key'),
        '#type' => 'textfield',
        '#required' => TRUE,
        '#default_value' => variable_get('sierragraphs_auth_client_id', ''),
      ),
      'sierragraphs_auth_client_secret' => array(
        '#title' => t('Client secret'),
        '#type' => 'textfield',
        '#required' => TRUE,
        '#default_value' => variable_get('sierragraphs_auth_client_secret', ''),
      ),
      'sierragraphs_auth_host' => array(
        '#title' => t('Host'),
        '#description' => 'e.g. lib.example.edu',
        '#type' => 'textfield',
        '#required' => TRUE,
        '#default_value' => variable_get('sierragraphs_auth_host', ''),
      ),
    ),
  );

  $form['#validate'][] = 'sierragraphs_settings_form_validate';

  return system_settings_form($form);
}

function sierragraphs_settings_form_validate($form, &$form_state) {
  $valid = TRUE;
  $host = $form_state['values']['sierragraphs_auth_host'];
  if(strpos($host, '://') !== FALSE) {
    form_set_error('sierragraphs_auth_host', t('Please do not include the protocol,
      i.e. do not include the http:// at the beginning of the URL.'));
    $valid = FALSE;
  }
  if (strpos($host, ' ') !== FALSE) {
    form_set_error('sierragraphs_auth_host', t('The host can not contain space characters.'));
    $valid = FALSE;
  }
  if (strpos($host, '/') === 0) {
    form_set_error('sierragraphs_auth_host', t('The host should not contain leading slashes.'));
    $valid = FALSE;
  }
  if (!empty($host) && substr($host, -1) == '/') {
    form_set_error('sierragraphs_auth_host', t('The host should not contain trailing slashes.'));
    $valid = FALSE;
  }
  if($valid) {
    sierragraphs_get_auth_token($form_state['values']['sierragraphs_auth_client_id'],
      $form_state['values']['sierragraphs_auth_client_secret'], $form_state['values']['sierragraphs_auth_host']);
  }

}

function sierragraphs_get_auth_token($client_id, $client_secret, $host = '') {
  $url = 'https://' . (empty($host) ? variable_get('sierragraphs_auth_host', '') : $host) . '/iii/sierra-api/v1/token';

  if (!empty($client_id) && !empty($client_secret)) {
    $credentials = base64_encode($client_id . ':' . $client_secret);

    $headers = array(
      'Authorization' => 'Basic ' . $credentials,
      'Content-Type' => 'application/x-www-form-urlencoded',
    );
    $data = 'grant_type=client_credentials';

    $options = array('headers' => $headers, 'method' => 'POST', 'data' => $data);

    $response = drupal_http_request($url, $options);

    if($response->code == "200") {
      $json_response = json_decode($response->data);
      cache_set($credentials, $json_response->access_token, 'cache', REQUEST_TIME + $json_response->expires_in);
      if($_POST['form_id'] == 'sierragraphs_auth_admin_settings') {
        drupal_set_message(t('You have been successfully authenticated in Sierra'));
      }
    }
    else {
      drupal_set_message(t('Something went wrong - double check your Sierra client key, client secret, and host.'), "error");
    }
  }
  else {
    drupal_set_message(t('You must enter a client key and a client secret.'), "error");
  }
}

/**
 * Helper function (not module-specific) for getting the access token.
 */
function sierragraphs_auth_token($reset = FALSE) {
  $token = &drupal_static(__FUNCTION__);
  if (!isset($token) || !$reset) {
    $client_id = variable_get('sierragraphs_auth_client_id', '');
    $client_secret = variable_get('sierragraphs_auth_client_secret', '');
    $credentials = base64_encode($client_id . ':' . $client_secret);
    $auth_var = cache_get($credentials, 'cache');
    if (isset($auth_var->data) && time() < $auth_var->expire) {
      $token = $auth_var->data;
    }
    else { // Refresh cache table
      sierragraphs_get_auth_token($client_id, $client_secret);
      $auth_var = cache_get($credentials, 'cache');
      $token = $auth_var->data;
    }
  }
  return $token;
}