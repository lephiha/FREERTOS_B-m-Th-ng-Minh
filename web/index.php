<?php
    include_once('esp-database.php');

    // Initialize variables
    $start_time = '';
    $duration = '';

// Handle form submission for start time and duration
if ($_SERVER['REQUEST_METHOD'] == 'POST') {
    if (isset($_POST['start-time']) && isset($_POST['duration'])) {
        $start_time = $_POST['start-time'];
        $duration = $_POST['duration'];

        // Validate and sanitize input
        $start_time = htmlspecialchars(stripslashes(trim($start_time)));
        $duration = htmlspecialchars(stripslashes(trim($duration)));

        insertTime($start_time, $duration);
    }

    // Handle form submission for pump status
    if (isset($_POST['pump-status'])) {
        $pump_status = $_POST['pump-status'];

        // Validate and sanitize input
        $pump_status = htmlspecialchars(stripslashes(trim($pump_status)));

        updatePumpStatus($pump_status); // Hàm cập nhật trạng thái bơm
    }
}

    {
      $readings_count = 40;
    }

    $last_reading = getLastReadings();
    $last_reading_temp = $last_reading["value1"];
    $last_reading_humi = $last_reading["value2"];
    $last_reading_time = $last_reading["reading_time"];

    // Uncomment to set timezone to - 1 hour (you can change 1 to any number)
    //$last_reading_time = date("Y-m-d H:i:s", strtotime("$last_reading_time - 1 hours"));
    // Uncomment to set timezone to + 7 hours (you can change 7 to any number)
    //$last_reading_time = date("Y-m-d H:i:s", strtotime("$last_reading_time + 7 hours"));

    $min_temp = minReading($readings_count, 'value1');
    $max_temp = maxReading($readings_count, 'value1');
    $avg_temp = avgReading($readings_count, 'value1');

    $min_humi = minReading($readings_count, 'value2');
    $max_humi = maxReading($readings_count, 'value2');
    $avg_humi = avgReading($readings_count, 'value2');

    $s_data = getAllGraph($readings_count);

?>

<!DOCTYPE html>
<html>
  <head>
    <title>Bơm thông minh</title>
    <meta http-equiv="refresh" content="60">
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
    <meta http-equiv="Cache-Control" content="no-cache, no-store, must-revalidate" />
    <meta http-equiv="Pragma" content="no-cache" />
    <meta http-equiv="Expires" content="0" />

    <link rel="stylesheet" type="text/css" href="esp-style.css">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.4.1/jquery.min.js"></script>
    <script src="https://code.highcharts.com/highcharts.js"></script>
  </head>
<body>
	<h2>Bơm thông minh</h2>
    <p>Last reading: <?php echo $last_reading_time; ?></p>
    <section class="content">
	    <div class="box gauge--1">
	    <h3>HUMIDITY</h3>
              <div class="mask">
			  <div class="semi-circle"></div>
			  <div class="semi-circle--mask"></div>
			</div>
		    <p style="font-size: 30px;" id="temp">--</p>
		    <table cellspacing="5" cellpadding="5">
		        <tr>
		            <th colspan="3">HUMIDITY <?php echo $readings_count; ?> readings</th>
	            </tr>
		        <tr>
		            <td>Min</td>
                    <td>Max</td>
                    <td>Average</td>
                </tr>
                <tr>
                    <td><?php echo $min_temp['min_amount']; ?> &deg;C</td>
                    <td><?php echo $max_temp['max_amount']; ?> &deg;C</td>
                    <td><?php echo round($avg_temp['avg_amount'], 2); ?> &deg;C</td>
                </tr>
            </table>
        </div>
        <div class="box gauge--2">
            <h3>TEMPERATURE</h3>
            <div class="mask">
                <div class="semi-circle"></div>
                <div class="semi-circle--mask"></div>
            </div>
            <p style="font-size: 30px;" id="humi">--</p>
            <table cellspacing="5" cellpadding="5">
                <tr>
                    <th colspan="3">Temperature <?php echo $readings_count; ?> readings</th>
                </tr>
                <tr>
                    <td>Min</td>
                    <td>Max</td>
                    <td>Average</td>
                </tr>
                <tr>
                    <td><?php echo $min_humi['min_amount']; ?> %</td>
                    <td><?php echo $max_humi['max_amount']; ?> %</td>
                    <td><?php echo round($avg_humi['avg_amount'], 2); ?> %</td>
                </tr>
            </table>
        </div>
    </section>
    <div class="container">
        <h2>Bật tắt từ xa</h2>
        <form id="turn-form"  method="post">
               <table>
             
			 <tr>
            <td><label for="pump-status">Trạng thái bơm:</label></td>
            <td>
                <input type="radio" id="pump-on" name="pump-status" value="on" <?php echo (getPumpStatus() == 'on') ? 'checked' : ''; ?>> Bật
                <input type="radio" id="pump-off" name="pump-status" value="off" <?php echo (getPumpStatus() == 'off') ? 'checked' : ''; ?>> Tắt
            </td>
        </tr>
            </table>
            <div class="form-group">
                <button type="submit">Lưu</button>
            </div>
        </form>
    </div>
	<div class="container">
        <h2>Hẹn giờ tưới nước</h2>
        <form id="watering-form"  method="post">
               <table>
             <tr>
                    <td>Thời gian hẹn giờ hiện tại:</td>
                    <td><?php echo getStartTime(); ?> </td>
                </tr>
                <tr>
                    <td>Khoảng thời gian bơm (phút) hiện tại:</td>
                    <td><?php echo getDuration(); ?></td>
                </tr>
             <tr>
                <td><label for="start-time">Thời gian bắt đầu:</label></td>
                <td><input type="time" id="start-time" name="start-time" required></td>
             </tr>
             <tr>
                <td><label for="duration">Thời gian tưới (phút):</label></td>
                <td><input type="number" id="duration" name="duration" min="1" required></td>
            </tr>
			 <tr>
           
        </tr>
            </table>
            <div class="form-group">
                <button type="submit">Lưu</button>
            </div>
        </form>
    </div>
    
    <h2>Lịch sử dữ liệu</h2>

<?php

while ($g_data = $s_data->fetch_assoc()){

    $sensor_data[] = $g_data;
}

$readings_time = array_column($sensor_data, 'reading_time');

// ******* Uncomment to convert readings time array to your timezone ********
$i = 0;
foreach ($readings_time as $reading){
    // Uncomment to set timezone to - 1 hour (you can change 1 to any number)
    //$readings_time[$i] = date("Y-m-d H:i:s", strtotime("$reading - 1 hours"));
    // Uncomment to set timezone to + 4 hours (you can change 4 to any number)
    //$readings_time[$i] = date("Y-m-d H:i:s", strtotime("$reading + 7 hours"));
    $i += 1;
}


$value1 = json_encode(array_reverse(array_column($sensor_data, 'value1')), JSON_NUMERIC_CHECK);
$value2 = json_encode(array_reverse(array_column($sensor_data, 'value2')), JSON_NUMERIC_CHECK);
$value3 = json_encode(array_reverse(array_column($sensor_data, 'value3')), JSON_NUMERIC_CHECK);
$value4 = json_encode(array_reverse(array_column($sensor_data, 'value4')), JSON_NUMERIC_CHECK);
$value5 = json_encode(array_reverse(array_column($sensor_data, 'value5')), JSON_NUMERIC_CHECK);
$reading_time = json_encode(array_reverse($readings_time), JSON_NUMERIC_CHECK);

?>

<script>
    var value1 = <?php echo $last_reading_temp; ?>;
    var value2 = <?php echo $last_reading_humi; ?>;
    setTemperature(value1);
    setHumidity(value2);

    function setTemperature(curVal){
    	//set range for Temperature in Celsius -5 Celsius to 38 Celsius
    	var minTemp = -5.0;
    	var maxTemp = 100.0;
        //set range for Temperature in Fahrenheit 23 Fahrenheit to 100 Fahrenheit
    	//var minTemp = 23;
    	//var maxTemp = 100;

    	var newVal = scaleValue(curVal, [minTemp, maxTemp], [0, 180]);
    	$('.gauge--1 .semi-circle--mask').attr({
    		style: '-webkit-transform: rotate(' + newVal + 'deg);' +
    		'-moz-transform: rotate(' + newVal + 'deg);' +
    		'transform: rotate(' + newVal + 'deg);'
    	});
    	$("#temp").text(curVal + ' %');
    }

    function setHumidity(curVal){
    	//set range for Humidity percentage 0 % to 100 %
    	var minHumi = 0;
    	var maxHumi = 35;

    	var newVal = scaleValue(curVal, [minHumi, maxHumi], [0, 180]);
    	$('.gauge--2 .semi-circle--mask').attr({
    		style: '-webkit-transform: rotate(' + newVal + 'deg);' +
    		'-moz-transform: rotate(' + newVal + 'deg);' +
    		'transform: rotate(' + newVal + 'deg);'
    	});
    	$("#humi").text(curVal + ' ºC');
    }

    function scaleValue(value, from, to) {
        var scale = (to[1] - to[0]) / (from[1] - from[0]);
        var capped = Math.min(from[1], Math.max(from[0], value)) - from[0];
        return ~~(capped * scale + to[0]);
    }
</script>


<!-- Chart -->
<div id="chart-temperature" class="container"></div>
<div id="chart-humidity" class="container"></div>
<div id="chart-pressure" class="container"></div>
<div id="chart-signal" class="container"></div>
<div id="chart-volt" class="container"></div>
<script>


var value1 = <?php echo $value1; ?>;
var value2 = <?php echo $value2; ?>;
var value3 = <?php echo $value3; ?>;
var value4 = <?php echo $value4; ?>;
var reading_time = <?php echo $reading_time; ?>;

var chartT = new Highcharts.Chart({
  chart:{ renderTo : 'chart-temperature' },
  title: { text: 'Temperature' },
  series: [{
    showInLegend: false,
    data: value1
  }],
  plotOptions: {
    line: { animation: false,
      dataLabels: { enabled: true }
    },
    series: { color: '#8ecae6' }
  },
  xAxis: { 
    type: 'datetime',
    categories: reading_time
  },
  yAxis: {
    title: { text: 'Humidity (%)' }
    //title: { text: 'Temperature (Fahrenheit)' }
  },
  credits: { enabled: false }
});

var chartH = new Highcharts.Chart({
  chart:{ renderTo:'chart-humidity' },
  title: { text: 'Humidity' },
  series: [{
    showInLegend: false,
    data: value2
  }],
  plotOptions: {
    line: { animation: false,
      dataLabels: { enabled: true }
    },
    series: { color: '#219ebc' }
  },
  xAxis: {
    type: 'datetime',
    //dateTimeLabelFormats: { second: '%H:%M:%S' },
    categories: reading_time
  },
  yAxis: {
    title: { text: 'Temperature (Celsius)' }
  },
  credits: { enabled: false }
});

var chartS = new Highcharts.Chart({
  chart:{ renderTo:'chart-pressure' },
  title: { text: 'PUMP' },
  series: [{
    showInLegend: false,
    data: value3
  }],
  plotOptions: {
    line: { animation: false,
      dataLabels: { enabled: true }
    },
    series: { color: '#023047' }
  },
  xAxis: {
    type: 'datetime',
    categories: reading_time
  },
  yAxis: {
    title: { text: 'Relay' }
  },
  credits: { enabled: false }
});


var chartV = new Highcharts.Chart({
  chart:{ renderTo:'chart-volt' },
  title: { text: 'soilMoisture' },
  series: [{
    showInLegend: false,
    data: value4
  }],
  plotOptions: {
    line: { animation: false,
      dataLabels: { enabled: true }
    },
    series: { color: '#fb8500' }
  },
  xAxis: {
    type: 'datetime',
    //dateTimeLabelFormats: { second: '%H:%M:%S' },
    categories: reading_time
  },
  yAxis: {
    title: { text: 'AnalogRead()' }
  },
  credits: { enabled: false }
});

</script>
</body>
</html>