var temp_units;
var location_status;
var zip_code;

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function(){
    callback(this.responseText);
  };
  xhr.open(type,url);
  xhr.send();
};

function locationSuccess(pos) {
  // Construct URL

  var url; 

// Test location
 //url = "http://api.openweathermap.org/data/2.5/weather?q=albany,usa";

 //real location 

 if (location_status == 0 || zip_code === 'undefined' || zip_code == 0)
 {
   url = "http://api.openweathermap.org/data/2.5/weather?lat=" +
   pos.coords.latitude + "&lon=" + pos.coords.longitude;
   console.log("Latitude: " + pos.coords.latitude);
   console.log("Longitude: " + pos.coords.longitude); 
 }
 else
 {
  url = "http://api.openweathermap.org/data/2.5/weather?q=" + zip_code;
}  

console.log("URL:" + url);

  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);

      var temperature;
      
      console.log("Temp_units " + temp_units);

      if (temp_units == 0)
      {
        temperature = Math.round(json.main.temp - 273.15);
      }
      else
      {
        temperature = Math.round((json.main.temp - 273.15) * 1.8 + 32);
      }
      
      console.log("Temperature is " + temperature);

      // Conditions
      var conditions = json.weather[0].main;      
      console.log("Conditions are " + conditions);
      
      // Location
      var location = json.name;      
      console.log("Location is " + location);

      
      // Assemble dictionary using our keys
      var dictionary = {
        "KEY_TEMPERATURE": temperature,
        "KEY_CONDITIONS": conditions,
        "KEY_LOCATION": location
      };

      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("Weather info sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending weather info to Pebble!");
        }
        );
    }      
    );
}


function locationError(err){
  console.log("Error reuesting location.");


  var url; 

  url = "http://api.openweathermap.org/data/2.5/weather?q=" + zip_code;

  console.log("URL:" + url);

  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);

      var temperature;
      
      console.log("Temp_units " + temp_units);

      if (temp_units == 0)
      {
        temperature = Math.round(json.main.temp - 273.15);
      }
      else
      {
        temperature = Math.round((json.main.temp - 273.15) * 1.8 + 32);
      }
      
      console.log("Temperature is " + temperature);

      // Conditions
      var conditions = json.weather[0].main;      
      console.log("Conditions are " + conditions);
      
      // Location
      var location = json.name;      
      console.log("Location is " + location);

      
      // Assemble dictionary using our keys
      var dictionary = {
        "KEY_TEMPERATURE": temperature,
        "KEY_CONDITIONS": conditions,
        "KEY_LOCATION": location
      };

      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("Weather info sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending weather info to Pebble!");
        }
        );
    }      
    );
}

function getWeather(){
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
    );
}

Pebble.addEventListener("showConfiguration", function(e) {
  Pebble.openURL('http://cobweb.seas.gwu.edu/~poolec/block_time/block_time_settings_page_w_color.html');
});

Pebble.addEventListener("webviewclosed", function(e){
  var values;
  console.log("Configuration closed");
  console.log("Response = " + e.response.length + "   " + e.response);
  if (e.response !== undefined && e.response !== '' && e.response !== 'CANCELLED') {
    console.log("User hit save");
    values = JSON.parse(decodeURIComponent(e.response));
    console.log("stringified options: " + JSON.stringify((values)));
    
    var json = JSON.parse(e.response);

    temp_units = json.unit;
    location_status = json.location_option;
    zip_code = json.zip_code;

    console.log("Temperature units = " + temp_units);
    
    getWeather();

    Pebble.sendAppMessage(values,
      function(e) {
        console.log("Config sent to Pebble successfully!");
      },
      function(e) {
        console.log("Error sending config data to Pebble!");
      }
      );
  }
});

//start to listen for the watchface to be opened
Pebble.addEventListener('ready',
  function(e) {
    console.log("PebbleKit JS ready!");
  }
  );

//listen for when a message is received from the app
Pebble.addEventListener('appmessage',
  function(e) {
    getWeather();
    
    console.log("AppMessage received!");

    console.log('Received appmessage: ' + JSON.stringify(e.payload));
    
    temp_units = JSON.stringify(e.payload["unit"]);
    location_status = JSON.stringify(e.payload["location"]);
    zip_code = JSON.stringify(e.payload["zip_code"]);
  }
  );