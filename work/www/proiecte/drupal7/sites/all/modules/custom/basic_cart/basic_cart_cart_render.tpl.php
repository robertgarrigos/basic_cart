<?php if( empty($cart) ): ?>
  <p><?php print t('Your shopping cart is empty.'); ?></p>
<?php else: ?>
  <div class="basic-cart-cart basic-cart-grid">
    <?php if(is_array($cart) && count($cart) >= 1): ?>
      <?php foreach($cart as $nid => $node): ?>
        <div class="basic-cart-cart-contents row">
            <div class="basic-cart-cart-quantity cell"><?php print $node->basic_cart_quantity; ?> x </div>
            <div class="basic-cart-cart-node-title cell">
              <?php print l($node->title, 'node/'.$node->nid); ?><br />
              <span class="basic-cart-cart-node-summary"><?php print drupal_substr($node->body['und'][0]['value'], 0, 50); ?> ... </span>
            </div>
            <?php if(!$is_checkout): ?>
              <div class="basic-cart-delete-image cell">
                <?php print l('<img src="/' . drupal_get_path('module', 'basic_cart') . '/images/delete.gif" border="0" />', 'cart/remove/' . $nid, array('html' => TRUE)); ?>
              </div>
            <?php endif; ?>
        </div>
      <?php endforeach; ?>
      <?php if(!$is_checkout): ?>
        <div class="basic-cart-cart-checkout-button row">
          <?php print l(t('Checkout'), 'checkout', array('attributes' => array('class' => array('button')))); ?>
        </div>
      <?php endif; ?>
    <?php endif; ?>
  </div>
<?php endif; ?>