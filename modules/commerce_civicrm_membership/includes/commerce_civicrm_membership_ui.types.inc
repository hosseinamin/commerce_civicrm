<?php

function commerce_civicrm_membership_ui_types($product_type) {
  /*
   * Pre-requisite fields for granting membership at new purchase
   * Reference (Entity) to Membership type [required if membership is enabled]
   * String from any field type with rules for start date, [optional]
   *  An example would be. Creating a field called start_at as
   *      select from a list field, possible values can be
   *          integer[unit] (1m as One month)
   *  This is a simplified ruleset to set start date relative from now.
   */
  $mmetadata = commerce_civicrm_membership_ptype_metadata_load($product_type['type']);
  return drupal_get_form('commerce_civicrm_membership_ui_types_form', array(
    'ptype' => $product_type,
    'mmetadata' => $mmetadata
  ));
}

function commerce_civicrm_membership_ui_types_form($form, &$form_state, $info) {
  $mmetadata = $info['mmetadata'];
  $excluded_fields = array( 'commerce_price' );
  $fields = field_info_instances('commerce_product', $info['ptype']['type']);
  $fields_option_list = array( "" => "Select a field" );
  foreach($fields as $key => $data) {
    if(!in_array($key, $excluded_fields)) {
      $fields_option_list[$key] = $data['label'].' ('.$key.')';
    }
  }
  if(empty($mmetadata)) {
    $mmetadata = array(
      'active' => 0,
      'membership_type_field' => '',
      'start_date_field' => ''
    );
  }
  // store initial state
  $form_state['info'] = $info;
  $form_state['fields'] = $fields;

  $form['settings'] = array( '#tree' => TRUE );
  $settings =& $form['settings'];

  $settings['active'] = array(
    '#type' => 'checkbox',
    '#title' => t('Active'),
    '#default_value' => $mmetadata['active'],
    '#description' => t('Indicates if civicrm commerce membership feature should get effected on this product type'),
  );

  $settings['membership_type_field'] = array(
    '#type' => 'select',
    '#title' => t('Field of Membership Type'),
    '#options' => $fields_option_list,
    '#default_value' => $mmetadata['membership_type_field'],
    '#description' => t('A field of type <em>Entity Reference</em> pointing at CiviCRM <strong>Membership type</strong>')
  );

  $settings['start_date_field'] = array(
    '#type' => 'select',
    '#title' => t('Field of Start date of membership'),
    '#options' => $fields_option_list,
    '#default_value' => $mmetadata['start_date_field'],
    '#description' => t('A field of any type with single value. [List (text)] is recommended. Value of the field will indicate relative date from now. Examples, `0` today, `1m` one month from now'),
  );

  $form['actions'] = array(
    '#type' => 'actions',
    '#weight' => 40,
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save')
  );
  $form['#validate'] = array(
    '_commerce_civicrm_membership_ui_types_post_validate'
  );
  return $form;
}

function _commerce_civicrm_membership_ui_types_post_validate($form, &$form_state) {
  if($form_state["values"]['settings']['active'] &&
    !$form_state["values"]['settings']['membership_type_field']) {
    form_set_error('settings][membership_type_field', t('Field of Membership Type field is required.'));
  }
}

function commerce_civicrm_membership_ui_types_form_submit($form, &$form_state) {
  $product_type = $form_state['info']['ptype'];
  $fields = array(
    'active' => intval($form_state["values"]['settings']['active']),
    'membership_type_field' => $form_state["values"]['settings']['membership_type_field'],
    'start_date_field' => $form_state["values"]['settings']['start_date_field']
  );
  if(db_query("SELECT COUNT(*) FROM commerce_civicrm_membership_ptype_metadata WHERE product_type_id = :id", array(':id' => $product_type['type']))->fetchField() > 0) {
    db_update('commerce_civicrm_membership_ptype_metadata')
      ->fields($fields)
      ->condition('product_type_id', $product_type['type'])
      ->execute();
  } else {
    $fields['product_type_id'] = $product_type['type'];
    db_insert('commerce_civicrm_membership_ptype_metadata')
      ->fields($fields)
      ->execute();
  }
  drupal_set_message(t('Settings saved.'));
  $form_state['redirect'] = 'admin/commerce/products/types/' . strtr($product_type['type'], '_', '-') . '/civicrm-membership';
}
