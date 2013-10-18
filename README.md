YWeatherFace
============


pebble watch app that displays weather info relative to user geo location.
it has an accompanying android app that handles location tracking and internet connectivity

it uses YQL to translate long/lat to WOEID using geo.placefinder
and retrievs the weather forecast from weather.yahooapis.com/forecastrss

YQL
use 'http://dl.dropbox.com/u/2662001/weather.woeid.xml' as weather; 
select * from weather where w in (select woeid from geo.placefinder where text="32.08, 34.8" and gflags="R") and u = 'c'

