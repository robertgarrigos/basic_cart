<?php

/**
 * @file
 * Basic cart shopping cart implementation functions
 */

/**
 * Callback function for cart listing
 */
function basic_cart_cart() {
  return theme('basic_cart_cart_render', array('cart' => basic_cart_get_cart(), 'is_checkout' => FALSE));
}

/**
 * Callback function for cart/add/
 *
 * @param int $nid node id
 *    We are using the node id to store the node in the shopping cart
 */
function basic_cart_add_to_cart($nid=NULL) {
  $nid = (int)$nid;
  if( $nid > 0 ) {   
    // If a node is added multiple times, we just update the quantity
    $cart = basic_cart_get_cart();
    if( !empty($cart) && in_array($nid, array_keys($cart)) ) {
      $quantity = (int)basic_cart_get_cart($nid)->basic_cart_quantity;
      $_SESSION['basic_cart']['cart'][$nid]->basic_cart_quantity++;
    }
    else {
      // Uncomment for faster method

      // $query = db_select('node', 'n')->fields('n', array('nid', 'vid', 'type', 'title'))->fields('revision', array('title'));
      // $query->join('node_revision', 'revision', 'n.vid = revision.vid');
      // $query->condition('n.nid', $nid);
      // $node = $query->execute()->fetchObject();
      
      // Slower, but easyer to implement
      $node = node_load($nid);
      $node->basic_cart_quantity = 1;
      $_SESSION['basic_cart']['cart'][$nid] = $node;
    }

  }
  drupal_goto('cart');
}

/**
 * Callback function for cart/remove/
 *
 * @param int $nid node id
 */
function basic_cart_remove_from_cart($nid=NULL) {
  $nid = (int)$nid;
  if( $nid > 0 ) {
    unset($_SESSION['basic_cart']['cart'][$nid]);
  }
  drupal_goto('cart');
}

/**
 * Function for shopping cart retrieval
 *
 * @param int $nid
 * @return mixed
 */
function basic_cart_get_cart($nid=NULL) {
  if( isset($nid) ) return $_SESSION['basic_cart']['cart'][$nid];
  if( isset($_SESSION['basic_cart']['cart']) ) return $_SESSION['basic_cart']['cart'];
  // empty cart
  return array();
}

/**
 * Shopping cart reset
 */
function basic_cart_empty_cart() {
  unset($_SESSION['basic_cart']['cart']);
}

/**
 * Checkout
 */

/**
 * Checkout form implementation
 */
function basic_cart_checkout() {
  $cart = theme('basic_cart_cart_render', array('cart' => basic_cart_get_cart(), 'is_checkout' => TRUE));
  $form = drupal_render(drupal_get_form('basic_cart_checkout_form'));
  // if the cart is empty, we don't want to show the checkout form;
  $shopping_cart = basic_cart_get_cart();
  if( empty($shopping_cart) ) return $cart;
  return $cart . $form;
}

/**
 * Checkout form
 *
 * @return mixed
 */
function basic_cart_checkout_form() {
  $form['basic_cart_checkout_name'] = array(
    '#title' => t('Name'),
    '#type' => 'textfield',
    '#required' => TRUE,
    '#description' => t('Please enter your name.'),
  );

  $form['basic_cart_checkout_email'] = array(
    '#title' => t('Email'),
    '#type' => 'textfield',
    '#required' => TRUE,
    '#description' => t('Please enter your email.'),
  );

  $form['basic_cart_checkout_phone'] = array(
    '#title' => t('Phone'),
    '#type' => 'textfield',
    '#description' => t('Please enter your phone.'),
  );

  $form['basic_cart_checkout_address'] = array(
    '#title' => t('Address'),
    '#type' => 'textfield',
    '#description' => t('Please enter your address.'),
  );

  $form['basic_cart_checkout_message'] = array(
    '#title' => t('Message'),
    '#type' => 'textarea',
    '#description' => t('If you have something to tell us, please fill the message area.'),
  );

  $form['basic_cart_checkout_submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit order'),
  );

  return $form;
}

/**
 * Checkout form validation
 */
function basic_cart_checkout_form_validate($form, &$form_state) {
  if( !valid_email_address($form_state['values']['basic_cart_checkout_email']) ) {
    form_set_error('basic_cart_checkout_email', t('Please enter a valid email address.'));
  }
}

/**
 * Checkout form submit proccess
 * Sending the 2 mails
 */
function basic_cart_checkout_form_submit($form, &$form_state) {
  // %ORDER_DETAILS% placeholder
  $order_details = '';
  $cart = basic_cart_get_cart();
  foreach($cart as $nid => $node) {
    $order_details .= $node->basic_cart_quantity . ' x ' . $node->title . "\n";
  }
  $order_details .= "\n";
  
  // Pleaceholder replacement
  $search = array('%CUSTOMER_NAME', '%CUSTOMER_EMAIL', '%CUSTOMER_PHONE', '%CUSTOMER_ADDRESS', '%CUSTOMER_MESSAGE', '%ORDER_DETAILS');
  $replace = array(
    $form_state['values']['basic_cart_checkout_name'], $form_state['values']['basic_cart_checkout_email'],
    $form_state['values']['basic_cart_checkout_phone'], $form_state['values']['basic_cart_checkout_address'],
    $form_state['values']['basic_cart_checkout_message'], $order_details,
  );

  // Admin mail
  $message_html = variable_get('basic_cart_admin_message');
  $message_html = str_replace($search, $replace, $message_html);

  //$a = $message_html;

  // Sending mail
  $my_module = 'basic_cart';
  $my_mail_token = 'checkout';
  $from = variable_get('site_mail');

  $message = array(
    'id' => $my_module . '_' . $my_mail_token,
    'to' => $from,
    'subject' => variable_get('basic_cart_admin_subject'),
    'body' => $message_html,
    'headers' => array(
      'From' => $from,
      'Sender' => $from,
      'Return-Path' => $from,
    ),
  );
  $system = drupal_mail_system($my_module, $my_mail_token);
  $ok = $system->mail($message) ? TRUE : FALSE;

  // User email
  $send_user_mail = variable_get('basic_cart_send_user_message');
  if( $send_user_mail ) {
    $message_html = variable_get('basic_cart_user_message');
    $message_html = str_replace($search, $replace, $message_html);
    
    //$b = $message_html;

    $message = array(
      'id' => $my_module . '_' . $my_mail_token,
      'to' => $form_state['values']['basic_cart_checkout_email'],
      'subject' => variable_get('basic_cart_user_subject'),
      'body' => $message_html,
      'headers' => array(
        'From' => $from,
        'Sender' => $from,
        'Return-Path' => $from,
      ),
    );
    $system = drupal_mail_system($my_module, $my_mail_token);
    $ok = $system->mail($message) ? TRUE : FALSE;
  }

  if( $ok ) {
    basic_cart_empty_cart();
    //drupal_set_message(t('The order was successfully placed.'));

    //drupal_set_message($a);
    //drupal_set_message($b);
    drupal_goto('checkout/thank-you');
  }
  else {
    drupal_set_message(t('There was a problem in submitting your order. Please try again later.'));
  }
}

/**
 * Callback for thank you page
 */
function basic_cart_checkout_thank_you() {
  $message = variable_get('basic_cart_thank_you_message');
  return nl2br($message);
}