#! /usr/bin/env python3
# ECE 631 Spring 2024
# Author: Erik Van Schijndel 
# Modified UID_GET file to support stored and authorized users for
# simulated NFC door lock.
#
"""
This example shows connecting to the PN532 with I2C (requires clock
stretching support), SPI, or UART. SPI is best, it uses the most pins but
is the most reliable and universally supported.
After initialization, try waving various 13.56MHz RFID cards over it!
"""

import time
import datetime
import binascii
import RPi.GPIO as GPIO
import pn532
import MQTTClients
import json

# Erik and Jacob's UID codes
UID = ["a257283e", "5d11273e"]
count = 0
correct = False

if __name__ == '__main__':
	try:
		MQTTPub = MQTTClients.MQTTPusher('127.0.0.1',1883)
	except ConnectionRefusedError:
		print("Connection Refused\nRetrying...")
		time.sleep(5)
	try:
		MQTTPub = MQTTClients.MQTTPusher('127.0.0.1',1883)
		PN532 = pn532.PN532_SPI(debug=False, reset=22, cs=8)

		ic, ver, rev, support = PN532.get_firmware_version()
		print('Found PN532 with firmware version: {0}.{1}'.format(ver, rev))

		# Configure PN532 to communicate with MiFare cards
		PN532.SAM_configuration()

		last_heartbeat = time.time()
		last_uid_pub = 0

		print('Waiting for RFID/NFC card...')
		while True: #Main Loop
			try:
				# Check if a card is available to read with 100ms timeout
				uid = PN532.read_passive_target(timeout=0.1)
				if time.time() - last_heartbeat  >= 5:
					ragPayload = {"RAG":"1"}
					ragState = {"RAGState":"Flash"}
					TimeStampStr = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')
					MQTTPub.PushData("ece631/FinalProject/NFC/Heartbeat",'{"Heartbeat":"' + TimeStampStr + '"}')
					MQTTPub.PushData("ece631/FinalProject/RAG",json.dumps(ragPayload))
					MQTTPub.PushData("ece631/FinalProject/RAG/RAGState",json.dumps(ragState))
					last_heartbeat = time.time()
				# If card is not present
				if uid is None:
					continue
				if time.time() - last_uid_pub >= 1:
					uid = (str)(binascii.hexlify(uid).decode('utf-8'))
					for x in UID:
						# count+=1
						if x == uid:
							payload = {"NFC":"Authorized"}
							ragPayload = {"RAG":"0"}
							ragState = {"RAGState":"Normal"}
							MQTTPub.PushData("ece631/FinalProject/NFC",json.dumps(payload))
							MQTTPub.PushData("ece631/FinalProject/NFC/UID",'{"UID":"' + uid + '"}')
							MQTTPub.PushData("ece631/FinalProject/RAG",json.dumps(ragPayload)) 
							MQTTPub.PushData("ece631/FinalProject/RAG/RAGState",json.dumps(ragState))
							last_uid_pub = time.time()
							print('Found card with UID: %s'%uid)
							correct = True;
							break
						if correct == False:
							payload = {"NFC":"Denied"}
							ragPayload = {"RAG":"2"}
							ragState = {"RAGState":"Normal"}
							MQTTPub.PushData("ece631/FinalProject/NFC",json.dumps(payload))
							MQTTPub.PushData("ece631/FinalProject/NFC/UID",'{"UID":"' + uid + '"}')
							MQTTPub.PushData("ece631/FinalProject/RAG",json.dumps(ragPayload))
							MQTTPub.PushData("ece631/FinalProject/RAG/RAGState",json.dumps(ragState))
							last_uid_pub = time.time()
							print('Access Denied with UID: %s'%uid)
							#count = 0
						correct = False;
			except Exception as e:
				print("Loop Exception: %s"%e)
	except Exception as e:
		print("Main Exception: %s"%e)
	finally:
		GPIO.cleanup()
		del MQTTPub
