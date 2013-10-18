YWeatherFace
============


pebble watch app that displays weather info relative to user geo location.
it has an accompanying android app that handles location tracking and internet connectivity

it uses YQL to translate long/lat to WOEID using geo.placefinder
and retrievs the weather forecast from weather.yahooapis.com/forecastrss

YQL
http://query.yahooapis.com/v1/public/yql?q=use%20'http%3A%2F%2Fdl.dropbox.com%2Fu%2F2662001%2Fweather.woeid.xml'%20as%20weather%3B%20%0Aselect%20*%20from%20weather%20where%20w%20in%20(select%20woeid%20from%20geo.placefinder%20where%20text%3D%2232.08%2C%2034.8%22%20and%20gflags%3D%22R%22)%20and%20u%20%3D%20'c'&format=json&callback=

Android App uses async connectivity and a custom location tracker to retrieve forecast
and push to pebble
