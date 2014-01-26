//define requirements
var http = require("http");
var mysql = require("mysql");
var spr = require("sprintf");
var stringify = require("json-stringify-safe");
var fs = require("fs");
var raw = require("raw-socket");
var net2 = require('net');

var connection = mysql.createConnection({
	host     : 'localhost',
	port	   : '3306',
	user     : 'root',
	password : 'bradley'
});

var server = net2.createServer(function(c) { //'connection' listener

  console.log('server connected');
 // console.log(c);
 

 var completeData = "";
 
 c.on('data', function(data) {
	console.log(data.toString("utf8", 0, data.length));
	//console.log(data.length);
	
	var databuf = data.toString("utf8", 0, data.length);
	
	for (var i = 0; i < databuf.length; i++)
	{
		
		if(completeData.length == 27)
		{
			postCo(completeData);
			completeData = "";
		}
		else
		{
			if(completeData.length == 0 && databuf.charAt(i) == 'P')
				completeData += databuf.charAt(i);
			else if (completeData.length != 0)
				completeData += databuf.charAt(i);
		}
	
	}
	
 });
  
  
  c.on('end', function() {
    console.log('server disconnected');
  });
  //c.pipe(c);
});
server.listen(27016, function() { //'listening' listener
  console.log('server bound');
});

http.createServer(function(request, response) {

		//construct webpage
		
		//get journey coordinates
		connection.query('USE project;');
		connection.query('SELECT CONCAT(lon, "#", lat) FROM data;', function(err, results)
		{
			//read in html
			html1 = fs.readFileSync("./page3", "utf8");	
			html4 = fs.readFileSync("./page4", "utf8");
		
			var html = html1;
					
		
			for( var i = 0; i < results.length; i++)
			{
		
				var coo = str(JSON.stringify(results[i]));
				
				
				html2 = spr.sprintf('\nnew google.maps.LatLng(%s, %s),\n', coo[0], coo[1]);

				html += html2;
				
			}
				
			html += html4;
				
			//serve webpage
			response.writeHead(200, {'Content-Type': 'text/html'});
			response.end(html);
				
		})	

	
}).listen(27015);



function str(string){

var coords = new Array();
coords[0] = "";
coords[1] = "";
var j = 0;
var i = 0;
var k = 0;

if(string.charAt(2) == 'C')
	k = 28;
else
	k = 0;

	for (i = k; i < string.length; i++)
	{
		if(string.charAt(i) == '0' || string.charAt(i) == '1' || string.charAt(i) == '2' || string.charAt(i) == '3' || string.charAt(i) == '4' || string.charAt(i) == '5' || string.charAt(i) == '6' || string.charAt(i) == '7' || string.charAt(i) == '8' || string.charAt(i) == '9' || string.charAt(i) == '-' || string.charAt(i) == '.')
			{
				coords[j] += string.charAt(i);
			}
		if(string.charAt(i) == '#')
			j++;
	}

return coords;
}

function conTest(string)
{

	var test = new Array();
	test[0] = 0;
	test[1] = 0;

	for( var i = 0; i < string.length; i++)
	{
		if (string.charAt(i) == 'P' && string.charAt(i+1) == 'O' && string.charAt(i+2) == 'S' && string.charAt(i+3) == 'T')
		{
			test[0] = 2;
			
			if (string.charAt(i+4) == 'y')
				test[1] = 1;
		}
	}

	
	return test;

}

function postCo(string)
{

//string processing - GET/POST, reset

console.log('posted');

var conType = [0, 0];
conType = conTest(stringify(string, null, 2));


//POST - check for reset & insert into table

	if (conType[1] == 1)
	{ //reset journey if necessary
		connection.query('USE project;');
		connection.query('TRUNCATE TABLE data;');
	}
	
	if (conType[0] == 2)
	{
		//add coordinates to table
		var coords = str(JSON.stringify(string, null, 2));
		
		connection.query('USE project;');
		var string = spr.sprintf('INSERT INTO data VALUES(%s, %s);', coords[0], coords[1]);
		connection.query(string);
	}


}