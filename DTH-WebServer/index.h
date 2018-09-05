const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
    body {
      text-align: center;
      font-family: "Trebuchet MS", Arial;
    }
    table {
      border-collapse: collapse;
      width:35%;
      margin-left:auto;
      margin-right:auto;
    }
    th {
      padding: 12px;
      background-color: #3372d8;
      color: white;
    }
    tr {
      border: 1px solid #ddd;
      padding: 12px;
    }
    tr:hover {
      background-color: #bcbcbc;
    }
    td {
      border: none; padding: 12px;
    }
    .sensor {
      color:rgb(0, 0, 0);
      font-weight: bold;
      background-color: #fdfdfd;
      padding: 1px;
    }
  </style>
</head>
<body>
  <h1>ESP32 with DHT</h1>
  <table>
    <tr>
      <th>MEASUREMENT</th>
      <th>VALUE</th>
    </tr>
    <tr>
      <td>Temp. Celsius</td>
      <td><span class="sensor"><span id="Celsius">0</span> *C</span></td></tr>
    <tr>
      <td>Temp. Fahrenheit</td>
      <td><span class="sensor"><span id="Fahrenheit">0</span> *F</span></td>
    </tr>
    <tr>
      <td>Humidity</td>
      <td><span class="sensor"><span id="Humidity">0</span> %</span></td>
    </tr>
    <tr>
      <td>Soil moisture</td>
      <td><span class="sensor"><span id="Soil">0</span> %</span></td>
    </tr>
    <tr>
      <td>IP Address</td>
      <td><span class="sensor"><span id="IP">0</span> </span></td>
    </tr>

    <script>
            setInterval(function() {
                // Call a function repetatively with 2.5 Second interval
                getData();
            }, 2500); //2500mSeconds update rate
            
            function getData(){
                var xhttp = new XMLHttpRequest();
                xhttp.onreadystatechange = function() {
                if (this.readyState == 4 && this.status == 200) {
                    console.log(this.responseText);
                    obj = JSON.parse(this.responseText);
                    document.getElementById("Celsius").innerHTML = obj.Celsius;
                    document.getElementById("Fahrenheit").innerHTML = obj.Fahrenheit;
                    document.getElementById("Humidity").innerHTML = obj.Humidity;
                    document.getElementById("Soil").innerHTML = obj.Soil;
                    document.getElementById("IP").innerHTML = obj.IP;
                    console.log(obj.Celsius);
                    console.log(obj.Fahrenheit);
                    console.log(obj.Humidity);
                    console.log(obj.Soil);
                    console.log(obj.IP);
                }
              };
              xhttp.open("GET", "readData", true);
              xhttp.send();
            }
            </script>
</body>
</html>
)=====";