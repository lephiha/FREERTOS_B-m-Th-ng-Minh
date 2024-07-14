<?php
include 'esp-database.php';

echo (getPumpStatus() == 'on') ? '1' : '0';
?>
