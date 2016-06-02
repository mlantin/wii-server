#include "wiiuse.h"                     /* for wiimote_t, classic_ctrl_t, etc */
#include "protobuf/VRCom.pb.h"
#include "easywsclient.hpp"
#include <iostream>
#include <cassert>
#include <sstream>
#ifndef WIIUSE_WIN32
#include <unistd.h>                     /* for usleep */
#endif

#define MAX_WIIMOTES				4



/**
 *	@brief Callback that handles a disconnection event.
 *
 *	@param wm				Pointer to a wiimote_t structure.
 *
 *	This can happen if the POWER button is pressed, or
 *	if the connection is interrupted.
 */
void handle_disconnect(wiimote* wm) {
	printf("\n\n--- DISCONNECTED [wiimote id %i] ---\n", wm->unid);
}


short any_wiimote_connected(wiimote** wm, int wiimotes) {
	int i;
	if (!wm) {
		return 0;
	}

	for (i = 0; i < wiimotes; i++) {
		if (wm[i] && WIIMOTE_IS_CONNECTED(wm[i])) {
			return 1;
		}
	}

	return 0;
}


/**
 *	@brief main()
 *
 *	Connect to up to two wiimotes and print any events
 *	that occur on either device.
 */
int main(int argc, char** argv) {

	using easywsclient::WebSocket;
	WebSocket::pointer ws = NULL;


	wiimote** wiimotes;
	int found, connected;

	/*
	 *	Initialize an array of wiimote objects.
	 *
	 *	The parameter is the number of wiimotes I want to create.
	 */
	wiimotes =  wiiuse_init(MAX_WIIMOTES);

	/*
	 *	Find wiimote devices
	 *
	 *	Now we need to find some wiimotes.
	 *	Give the function the wiimote array we created, and tell it there
	 *	are MAX_WIIMOTES wiimotes we are interested in.
	 *
	 *	Set the timeout to be 10 seconds.
	 *
	 *	This will return the number of actual wiimotes that are in discovery mode.
	 */
	found = wiiuse_find(wiimotes, MAX_WIIMOTES, 10);
	if (!found) {
		printf("No wiimotes found.\n");
		return 0;
	}

	/*
	 *	Connect to the wiimotes
	 *
	 *	Now that we found some wiimotes, connect to them.
	 *	Give the function the wiimote array and the number
	 *	of wiimote devices we found.
	 *
	 *	This will return the number of established connections to the found wiimotes.
	 */
	connected = wiiuse_connect(wiimotes, MAX_WIIMOTES);
	if (connected) {
		printf("Connected to %i wiimotes (of %i found).\n", connected, found);
	} else {
		printf("Failed to connect to any wiimote.\n");
		return 0;
	}


	/*
	 *	Now set the LEDs and rumble for a second so it's easy
	 *	to tell which wiimotes are connected (just like the wii does).
	 */
	for (int i = 0; i < connected; i++) {
		wiiuse_set_leds(wiimotes[i], WIIMOTE_LED_1);
	
		wiiuse_rumble(wiimotes[i], 1);
	}

#ifndef WIIUSE_WIN32
	usleep(200000);
#else
	Sleep(200);
#endif

	for (int i = 0; i < connected; i++) {	
		wiiuse_rumble(wiimotes[i], 0);
	}
	

	// open the websocket server connection
    #ifdef _WIN32
    INT rc;
    WSADATA wsaData;

    rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (rc) {
      printf("WSAStartup Failed.\n");
      return 1;
    }
    #endif

    std::string websocketAddr = "ws://192.168.11.34:4567";

    if (argc > 1) {
    	websocketAddr = argv[1];
    }

    ws = WebSocket::from_url(websocketAddr);
    assert(ws);

    // Prepare the VRCom data structure
    std::ostringstream msgbuf;
	VRCom::Update* updateMsg = new VRCom::Update();
	VRCom::Wiimote* msg = new VRCom::Wiimote();
	updateMsg->set_allocated_wiimote(msg);

	/*
	 *	This is the main loop
	 *
	 *	wiiuse_poll() needs to be called with the wiimote array
	 *	and the number of wiimote structures in that array
	 *	(it doesn't matter if some of those wiimotes are not used
	 *	or are not connected).
	 *
	 *	This function will set the event flag for each wiimote
	 *	when the wiimote has things to report.
	 */
	while (any_wiimote_connected(wiimotes, MAX_WIIMOTES)) {
		if (wiiuse_poll(wiimotes, MAX_WIIMOTES)) {
			/*
			 *	This happens if something happened on any wiimote.
			 *	So go through each one and check if anything happened.
			 */
			int i = 0;
			for (; i < MAX_WIIMOTES; ++i) {
				switch (wiimotes[i]->event) {
					case WIIUSE_EVENT:
						/* a generic event occurred */
						if (wiimotes[i]->btns || wiimotes[i]->btns_released) {
							msg->set_id(i+1);
							msg->clear_buttons_pressed();
							msg->set_buttons_pressed(wiimotes[i]->btns);
							msg->clear_buttons_released();
							msg->set_buttons_released(wiimotes[i]->btns_released);
							std::cerr << msg->DebugString() << std::endl;
							updateMsg->SerializeToOstream(&msgbuf);
							ws->sendBinary(msgbuf.str());
							ws->poll();

							std::ostringstream().swap(msgbuf);
    	      				msgbuf.clear();
          				}	
						break;

					case WIIUSE_DISCONNECT:
					case WIIUSE_UNEXPECTED_DISCONNECT:
						/* the wiimote disconnected */
						handle_disconnect(wiimotes[i]);
						break;

					default:
						break;
				}
			}
		}
	}

	/*
	 *	Disconnect the wiimotes
	 */
	wiiuse_cleanup(wiimotes, MAX_WIIMOTES);

	return 0;
}
