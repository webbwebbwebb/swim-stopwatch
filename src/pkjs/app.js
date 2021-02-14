Pebble.addEventListener('ready', function() {
  // PebbleKit JS is ready!
  console.log('PebbleKit JS ready!');
});

Pebble.addEventListener('showConfiguration', function() {
  // navigate to url of settings page
  var url;
  console.log(Pebble.getActiveWatchInfo().platform);
  if(Pebble.getActiveWatchInfo().platform === "basalt"){
    url = 'https://raw.githack.com/webbwebbwebb/swim-stopwatch/master/config/?color'; // settings with color
  }else{
    url = 'https://raw.githack.com/webbwebbwebb/swim-stopwatch/master/config/?bw';
  }
  console.log(url);
  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  // Decode the user's preferences from the return URL
  var configData = JSON.parse(decodeURIComponent(e.response));
  console.log('Configuration page returned: ' + JSON.stringify(configData));
  
  // Assemble data in dictionary
  var dict = {
    'AppKeyDistUnit': parseInt(configData.AppKeyDistUnit),
    'AppKeyLapDist': parseInt(configData.AppKeyLapDist),
    'AppKeyDispMode': parseInt(configData.AppKeyDispMode),
    'AppKeyColorMain' : parseInt(configData.AppKeyColorMain,16),
    'AppKeyColorAccent' : parseInt(configData.AppKeyColorAccent,16),
    'AppKeyBackButtonLongPress' : parseInt(configData.AppKeyBackButtonLongPress),
  };
  
  // Send dictionary to the watchapp
  Pebble.sendAppMessage(dict, function() {
    console.log('Config data sent successfully!');
  }, function(e) {
    console.log('Error sending config data!');
  });
});