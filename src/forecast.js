var precipColors = [
  //0xFFFFFF, // GColorWhite
  0xAAFF55, // GColorInchworm
  0x00FF00, // GColorGreen
  0x00AA00, // GColorIslamicGreen 
  0x005500  // GColorDarkGreen
];
var tempColors = [
  0x0000FF, // 030 GColorBlue
  0x0055FF, // 035 GColorBlueMoon
  0x00AAFF, // 040 GColorVividCerulean
  0x00FFFF, // 045 GColorCyan
  0x55FFFF, // 050 GColorElectricBlue
  0xAAFFFF, // 055 GColorCeleste
  0xFFFFAA, // 060 GColorPastelYellow 
  0xFFFF55, // 065 GColorIcterine 
  0xFFFF00, // 070 GColorYellow
  0xFFAA55, // 075 GColorRajah 
  0xFFAA00, // 080 GColorChromeYellow
  0xFF5500, // 085 GColorOrange
  0xFF0000, // 090 GColorRed 
  0xFF0055, // 095 GColorFolly 
  0xAA0000  // 100 GColorDarkCandyAppleRed   
];

var JS_KEY_API_KEY = 98;
var JS_KEY_IS_MINUTELY = 99;
var CALLBACK_ID_KEY = 32767;
var CBID_FORECAST = 1;
var MIN_PROBABILITY = 0.2;
var MIN_TEMP = 30.0;
var MAX_TEMP = 100.0;

var apiKey;
var isMinutely = false;

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function getColorInterpolated(firstPoint, secondPoint, n) {
  var firstValue = getPrecipValue(firstPoint);
  var secondValue = getPrecipValue(secondPoint);
  var colorArray = precipColors;
  if (firstValue == 0 && secondValue == 0) {
    firstValue = getTempValue(firstPoint);
    secondValue = getTempValue(secondPoint);
    colorArray = tempColors;
  }
  var interpolateIncrement = (secondValue - firstValue) / n;
  var colors = [];
  for (var i = 0; i < n; i++) {
    var interpolateValue = firstValue + interpolateIncrement * n;
    colors[i] = getColor(interpolateValue, colorArray);    
  }
  return colors;
}

function getColor(percent, colorArray) {
  var rounded = Math.floor(percent * colorArray.length);
  return colorArray[rounded];
}

function getPrecipValue(dataPoint) {
  var MAX_INTENSITY = 0.6;
  return getIntensityPercent(dataPoint.precipIntensity / MAX_INTENSITY, dataPoint.precipProbability);
}

function getIntensityPercent(intensity, probability) {
  if (probability < MIN_PROBABILITY) {
    return 0;
  }
  return Math.min(intensity / 0.3, 1.0);    
}

function getTempValue(dataPoint) {
  var temp = dataPoint.temperature;
  if (temp < MIN_TEMP) {
    return 0.0;
  }
  if (temp > MAX_TEMP) {
    return 1.0;
  }
  return 1.0 * (temp - MIN_TEMP) / (MAX_TEMP - MIN_TEMP);
}

function locationSuccess(pos) {
  // Construct URL
  var url = 'https://api.forecast.io/forecast/' + apiKey + '/' +
      "19.4300,-99.1300";
      //pos.coords.latitude + ',' +  pos.coords.longitude;
  console.log(url);
  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);
      var payload = {};
      if (isMinutely) {
        for (var i in json.minutely.data) {
          var dataPoint = json.minutely.data[i];
          var date = new Date(dataPoint.time * 1000);
          var clockIndex = date.getMinutes();
          var colors = getColorInterpolated(dataPoint, dataPoint, 1);
          payload[clockIndex] = colors[0];
        }
      }
      else {
        for (var i = 0; i < 12; i++) {
          var dataPoint = json.hourly.data[i];
          var date = new Date(dataPoint.time * 1000);
          var clockIndex = Math.round((date.getMinutes() / 770.0 + date.getHours() / 12.0) * 60) % 60.0;
          var colors = getColorInterpolated(dataPoint, json.hourly.data[i + 1], 5);

          payload[clockIndex] = colors[0];
          payload[clockIndex + 1] = colors[1];
          payload[clockIndex + 2] = colors[2];
          payload[clockIndex + 3] = colors[3];
          payload[clockIndex + 4] = colors[4];
        }
      }
      payload[CALLBACK_ID_KEY] = CBID_FORECAST;
      
      // Send to Pebble
      Pebble.sendAppMessage(payload,
        function(e) {
          console.log('Weather info sent to Pebble successfully!');
        },
        function(e) {
          console.log('Error sending weather info to Pebble!');
        }
      );
    }      
  );
}

function locationError(err) {
  console.log('Error requesting location!');
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('Received message: ' + JSON.stringify(e.payload));
    apiKey = e.payload[JS_KEY_API_KEY];
    isMinutely = e.payload[JS_KEY_IS_MINUTELY] ? true : false;
    getWeather();
  }                     
);