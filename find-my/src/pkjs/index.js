var SERVER_URL = "http://127.0.0.1:8000";
var DEVICE_ID = null;

Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage', function(event) {
  var msg = event.payload;
  // console.log(msg);
  console.log('Message received from watch:', event.data);
  for (m in msg) {
    console.log("m: " + m);
  }
  if ('Find' in msg) {
    console.log("find command found");
    var body = {
			deviceId: DEVICE_ID
		};
    xhrRequest("POST", "find", body, function (data) {
      console.log("It worked!");
      Pebble.sendAppMessage({'status': 200})
    });
  }

});

const xhrRequest = function(method, endpoint, body, callback) {
  // if (!SERVER_URL.startsWith("http")) { SERVER_URL = 'http://' + SERVER_URL; }
  // if (endpoint.startsWith("/")) { endpoint = endpoint.slice(1); }
  var url = SERVER_URL + "/" + endpoint;

  var xhr = new XMLHttpRequest();
  xhr.addEventListener("readystatechange", function() {
    if (xhr.readyState === 4) {
      console.debug(method, url, xhr.status);
      if (xhr.status >= 200 && xhr.status < 300) {
        var data;
        if (xhr.responseText) {
          data = JSON.parse(xhr.responseText);
        }
        else {
          data = "OK";
        }
        callback(data);
      }
      else {
        console.error(xhr.status);
      }
    }
  });
  xhr.open(method, url);
  xhr.setRequestHeader("Accept", "application/json");
  // if (BEARER_TOKEN) {
  //   xhr.setRequestHeader("Authorization", BEARER_TOKEN);
  // }
  if (body) {
    xhr.setRequestHeader("Content-Type", "application/json");
    xhr.send(JSON.stringify(body));
  }
  else { xhr.send(); } 
};