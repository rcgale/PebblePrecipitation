var JS_KEY_API_KEY = 98;
var JS_KEY_IS_MINUTELY = 99;
var CALLBACK_ID_KEY = 32767;
var CBID_FORECAST = 1;

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

function getIntensityPercent(intensity) {
  return parseInt(Math.min(intensity / 0.3, 1.0) * 100);
}

function locationSuccess(pos) {
  // Construct URL
  var url = 'https://api.forecast.io/forecast/' + apiKey + '/' +
      //"45.5634004,-122.6829831";
      pos.coords.latitude + ',' +  pos.coords.longitude;
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
          payload[clockIndex] = getIntensityPercent(dataPoint.precipIntensity / 0.3, 1.0);
        }
      }
      else {
        for (var i = 0; i < 12; i++) {
          var MAX_INTENSITY = 0.6;
          var dataPoint = json.hourly.data[i];
          var date = new Date(dataPoint.time * 1000);
          var clockIndex = Math.round((date.getMinutes() / 770.0 + date.getHours() / 12.0) * 60) % 60.0;
          var intensity = getIntensityPercent(dataPoint.precipIntensity / MAX_INTENSITY, 1.0);
          var nextIntensity = (i + 1 in json.hourly.data)
            ? getIntensityPercent(json.hourly.data[i + 1].precipIntensity / MAX_INTENSITY, 1.0)
            : intensity;
          var interpolateIncrement = (nextIntensity - intensity) / 5.0;
          payload[clockIndex] = intensity;
          payload[clockIndex + 1] = Math.round(intensity + interpolateIncrement * 1);
          payload[clockIndex + 2] = Math.round(intensity + interpolateIncrement * 2);
          payload[clockIndex + 3] = Math.round(intensity + interpolateIncrement * 3);
          payload[clockIndex + 4] = Math.round(intensity + interpolateIncrement * 4);
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