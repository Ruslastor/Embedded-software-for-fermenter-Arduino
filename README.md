<h1>The Arduino Fermenter with SMS notification.</h1>
<p>The aim of this project was to build a fermenter. The machine, that keeps the temperature inside of it for ome period of time.</p>

<p>The fermenter functionality consits of:</p>
<ul>
  <li>4 independent cells with separate temperature controll.</li>
  <li>Fermenter has nice user interface</li>
  <li>SIM800L module to send messages, when one of the cell is ready</li>
  <li>Thermal isolation</li>
</ul>

<h2>project electronics:</h2>
<ul>
  <li>Heating - 220V/6W heating bulbs</li>
  <li>Temperature measurements - thermistors</li>
  <li>Bulb control - 4 relay modules</li>
  <li>User Interface - Arduino LCD + buttons shield.</li>
  <li>Control Unit - Arduino UNO</li>
  <li>SMS sending - SIM800l module</li>
  <li>The 5V 3A power supply</li>
</ul>
<p>The fermenter itself could be way much more compact. However, the main target was to use the modules, that I already had.</p>
<p>The Arduino UNO could have been replaced by faster, smaller and with less power consumption microcontroller (Like Raspberry PI Pico), but as soon as I had a nice UI shield, I decided to stick to Arduino. Also, the second relay module uses inverted relays, so in the code it is implemented, that some bulbs require inverted logic to be controlled (However, I could just placed the wire in relay on another port, but I enjoy when both modules look symmetrical :) ). The temperature is measured in the way, that the each NTC thermistors are connected to 10k resistor, to obtain a devide circuit, and the signal is being measured from their middle point. It could be easier to connect the thermocouple, but there was no high temperature.</p>
<img src="images_ferm/cabels.jpg" width="300"/>
<p>Fermenter front view:</p>
<img src="images_ferm/fermenter.gif" width="300" alt="Nice blinking video"/>
<p>Fermenter first connection testing:</p>
<img src="images_ferm/blinking.gif" width="300" alt="Nice blinking video"/>
<p>Fermenter user interface:</p>
<img src="images_ferm/ui.gif" alt="Nice ui" width="300"/>
<p>Fermenter in use, making jogurt:</p>
<img src="images_ferm/fermenting.gif" alt="Nice ui" width="300"/>
