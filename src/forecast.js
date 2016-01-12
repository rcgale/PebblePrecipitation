var JS_KEY_API_KEY = 98;
var JS_KEY_IS_MINUTELY = 99;

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

function locationSuccess(pos) {
  // Construct URL
  var url = 'https://api.forecast.io/forecast/' + apiKey + '/' +
      //"40.7142,-74.0064";
      pos.coords.latitude + ',' +  pos.coords.longitude;
  console.log(url);
  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);
      var probability = {};
      var data = isMinutely ? json.minutely.data : json.hourly.data;
      for (var i in data) {
        var dataPoint = data[i];
        //probability[i] = parseInt(Math.tanh(2 * dataPoint.precipIntensity) * 100);
        probability[i] = parseInt(dataPoint.precipProbability * 100);
        //console.log(dataPoint.precipIntensity + "\t" + probability[i] + "\t" + parseInt(dataPoint.precipProbability *100));
      }
      
      // Send to Pebble
      Pebble.sendAppMessage(probability,
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