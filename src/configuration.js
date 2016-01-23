var CALLBACK_ID_KEY = 32767;
var CBID_CONFIG = 0;

var KEY_IS_MINUTELY = 1;

Pebble.addEventListener('showConfiguration', function(e) {
  Pebble.openURL('http://bobbygalaxy.com/precipitation/');
});

Pebble.addEventListener('webviewclosed', function(e) {
  // Decode and parse config data as JSON
  var config_data = JSON.parse(decodeURIComponent(e.response));
  //console.log('Config window returned: ' + JSON.stringify(config_data));

  // Prepare AppMessage payload
  var dict = {};
  
  dict[CALLBACK_ID_KEY] = CBID_CONFIG;
  dict[KEY_IS_MINUTELY] = config_data["is_minutely"];
  
  // Send settings to Pebble watchapp
  Pebble.sendAppMessage(dict, function(){
    console.log('Sent config data to Pebble');  
  }, function() {
    console.log('Failed to send config data!');
  });
});