/*
 * comm_relay.h
 *
 *  Created on: Mar 19, 2019
 *      Author: root
 */

#ifndef COMM_RELAY_H_
#define COMM_RELAY_H_

#define RELAY1_PIN 4
#define RELAY2_PIN 5

#define RELAYSIZE 3

typedef struct {
	uint8 relay[RELAYSIZE];
}CommRelay;


#endif /* COMM_RELAY_H_ */
