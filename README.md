# Drishtikon (CPG80)
## EdgeAI based human detection system for automated lighting control, also aimimg for HVAC applications in future! <br>
 
+ The beginnings: ***Drishtikon*** is a framework consisting of two modules, the mainboard and the controller modules that can be powered directly from the 220VAC mains in dorms, libraries and canteens. The two boards communicate with each other using a BLE protocol and the mainboard is connected to the internet via Wi-Fi, it connects to the local network using hard coding.<br> <br>
  <div>
  - <img src="https://github.com/Debanx3/Drishtikon/blob/main/Documents/cap_logo.png" alt="Drishtikon" width="250" height="250"> 
  - <img src="https://github.com/Debanx3/Drishtikon/blob/main/Documents/picx.jpg" alt="Mainboard PCB" width="250" height="250">
  - <img src="https://github.com/Debanx3/Drishtikon/blob/main/Documents/picy.jpg" alt="Mainboard PCB" width="250" height="250">
  - <img src="https://github.com/Debanx3/Drishtikon/blob/main/Documents/WhatsApp%20Image%202025-10-20%20at%2014.41.27_4e0a38e9.jpg" alt="Mainboard PCB" width="250" height="250">
  - <img src=" " alt="Mainboard PCB" width="250" height="250">
  - <img src=" " alt="Mainboard PCB" width="250" height="250">
  </div>
+ Mainboard: Runs an algorithm which is responsible for capturing the heat map of the FOV and considers the LUX values, to decide which load to powerup and sends the message via BLE to the controller.<br> <br>
  <div>
  - <img src="https://github.com/Debanx3/Drishtikon/blob/main/Documents/Mainboard_3D-removebg-preview.png" alt="Drishtikon" width="250" height="250">
  - <img src="https://github.com/Debanx3/Drishtikon/blob/main/Documents/pic9.png" alt="Drishtikon" width="250" height="250">
  </div>
+ Controller:  Runs an algorithm which is responsible for fetching the message via BLE from the mainboard and in turn powers up the connected AC loads.<br> <br>
  <div>
  - <img src="https://github.com/Debanx3/Drishtikon/blob/main/Documents/Controller_3D-removebg-preview.png" width="250" height="250">
  - <img src="https://github.com/Debanx3/Drishtikon/blob/main/Documents/pic8.png" width="250" height="250">
  </div>
+ Power-supply: AC-DC power supply for powering both the modules via mains 230VAC. Both mainboard and controller have each power supply of their own in the casing.<br> <br>
  <div>
  - <img src="https://github.com/Debanx3/Drishtikon/blob/main/Documents/power-removebg-preview.png" width="250" height="250">
  - <img src="https://github.com/Debanx3/Drishtikon/blob/main/Documents/pic12.png" width="250" height="250">
  </div>

