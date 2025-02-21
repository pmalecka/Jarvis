# ESPHome Jarvis component

This project is a ESPHome component for [fully's Jarvis standing desk](https://www.fully.com/standing-desks/jarvis.html). 

It enables Home Assistant to control the desk and get data out of it. It has all the features from the official handset and more. 
This connects to a RJ-11 port on the controlbox marked **"F"**

### Backstory
Originally I wanted to be able to control the desk from a Home Assistant automation so I started looking if there are any projects that has done this already. 
Luckily, I came across [Phil Hord's IoT project](https://github.com/phord/Jarvis), which I strongly recommend to check out. He has done the reverse engineering of the communication and the wiring that I extensively used to create this project.

I looked at the controller and googled the part number. Found out that there is another connector there marked as **"F"**.
I poked around Jiecang's website and read through some user manuals. It turns out, that the connector can be used to add Bluetooth capability to the desk.

https://www.jiecang.com/JCP35N-BLT-pd44755680.html

https://fccid.io/2ANKDJCP35NBLT/External-Photos/External-Photos-3727738

https://fccid.io/2ANKDJCP35NBLT/Internal-Photos/Internal-Photos-3727739

https://fccid.io/2ANKDJCP35NBLT/User-Manual/Users-Manual-3727745

https://play.google.com/store/apps/details?id=com.jiecang.app.android.aidesks&hl=en&gl=US
So I started reverse engineer it even further. I discovered, through fuzzing about 2 dozen of message types that were not used by neither the controller nor the handset in the stock configuration and managed to exploit some bugs as well that I found during the journey. With all that knowledge I decided to implement a man-in-the-middle device that captures all traffic and is able to inject fake messages into both of the data streams.

There were 3 things I wanted to have: (1) The ability to turn the screen on anytime for notification (this I think is impossible). (2) Always-on display to show numbers without activating the screen. I really didn't like touching it twice to move it. (3) Custom numbers on the display to have something more useful than the current height, which is almost completely useless for me.


## Features

* Send messages to the controlbox, obtain current status of the desk
* Handset still works, as if nothing is even there
* ESPHome configuration

## Technical notes

Check out [Phil's repository](https://github.com/phord/Jarvis) for details on the wiring and the communication protocol.
Check out [Dániel's repository](https://github.com/maraid/Jarvis) for inspiration, and details about the protocol

Important findings:
* The "handset control lines" are unnecessary. Everything can be done via UART messages.
* __Display behavior__:
	* When 0x01 (height report) is received by the HS (Handset), normally it gets stored in memory (0x1B) and then displayed.
	* If 0x01's payload is outside the display's range (1-1800) it won't be written to memory, but the content of 0x1B will be shown. (i.e. the last value sent)
	* 0x1B can be set by hand without updating the display. __Can be set to 0__, so the next out-of-range 0x01 will show 0. It cannot be set above 1800.
	* The screen turns off if the received 0x01 is the same as the number in 0x1B for ~9 seconds. This means that if an out-of-range number is being sent periodically (0 or anything above 1800) the screen won't turn off, since 0x1B won't be updated.
	* Some errors can turn it on in off mode without touching, but I found no way to keep it active, it always goes to sleep mode after the error disappears.
	* I found no way of changing brightness or locking the handset since these are self contained.
* __Controlbox__:
	* Kill mode: disable anti-collision. It's not possible through the handset for some weird reason.
	* You can get the current settings with 0x07. Check the sheet for details.
	* No motor frequency change or anything too crazy unfortunately.
	* I managed to crash it a couple of times, rendering it unresponsive, but I cannot reproduce it. A simple power cycle fixes it though.
* (exploitable) __Bugs__:
    * Empty screen, active buttons: On powerup the hanset turns on and starts spamming 0x29 until answered. Goes to sleep in about 10 seconds after showing "fully" logo. Next time it's pressed it wakes up the controller with signallines. However, if no answer is received then the buttons stay lit and active and the screen is blank.
    * Dark mode: Sending 0x23 to the sleeping handset will put it in an errornous state if 1 or 2 send in P0. The lights will not turn on until reboot. Everything else behaves the same way. No way found to reset it without power cycle.
(Same applies to error satetes 0x01 - 0x0D and 0x10)

## Installation

#### Requirements:

* [Home Assistant](https://www.home-assistant.io/)
* Wemos D1 Mini Pro clone has a borked HW serial, so I ended up using Lolin S3 (a huge overkill)
* A bit of soldering.

In progress...

