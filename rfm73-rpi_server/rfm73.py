import spidev
import time
import RPIO
from rfm73_defs import *

CSN = 24 #GPIO pinu CSN
CE = 7 #GPIO pinu CE
spi = spidev.SpiDev() #inicjalizacja SPI

RX0_Address = [0x34, 0x43, 0x10, 0x10, 0x01] #adres urzadzenia

bank0_vals=[ #ustawienia modulu
   [  0, 0x0F ], # receive, enabled, CRC 2, enable interupts
   [  1, 0x3F ], # auto-ack on all pipes enabled
   [  2, 0x03 ], # Enable pipes 0 and 1
   [  3, 0x03 ], # 5 bytes addresses
   [  4, 0xff ], # auto retransmission delay 4000 ms, 15 times
   [  5, 0x0A ], # channel 10
   [  6 ,0x07 ], # data rate 1Mbit, power 5dbm, LNA gain high
   [  7, 0x07 ], # nie ruszac
   [  8, 0x00 ], # clear Tx packet counters
   [ 23, 0x00 ], # fifo status
]

def init_banks():
	bank(0)
	for val in bank0_vals:
		register_write(val[0], val[1])

	register_write(RFM73_REG_RX_ADDR_P0, RX0_Address)
	register_write(RFM73_REG_RX_ADDR_P1, RX0_Address)
	register_write(RFM73_REG_TX_ADDR, RX0_Address)

	i = register_read(29)
	if i == 0:
		register_write( RFM73_CMD_ACTIVATE, 0x73 )
		
	#select dynamic payload
	register_write(28, 0x3F)
	register_write(29, 0x07)  
	val = register_read(RFM73_REG_DYNPD )
	val = val | 0x03
	register_write( RFM73_REG_DYNPD, val )
	register_write( RFM73_REG_RX_PW_P0, 0)
	register_write( RFM73_REG_RX_PW_P0 + 1,0)
	
	
	bank(1) #mind blowing bank
	
	for i in range(0,9):
		w = []
		for j in range(0,4):
			w.append(int((bank1_vals[i]>>(8*j))&0xff))
		register_write(i, w)
		
	for i in range(9,14):
		w = []
		for j in range(0,4):
			w.append(int((bank1_vals[i]>>(8*(3-j)))&0xff))
		register_write(i, w)
	
	register_write(14, bank1_reg14)
	
	w = []
	for j in range(0,4):
		w.append((bank1_vals[4]>>(8*j))&0xff)
	w[0] = w[0] | 0x06
	register_write(4, w)
	w[0] = w[0] & 0xf9
	register_write(4, w)
	
	bank(0)
	time.sleep(0.05)
   
		
def init():
	RPIO.setup(25, RPIO.IN) #IRQ
	RPIO.setup(CSN, RPIO.OUT) #CSN
	RPIO.setup(CE, RPIO.OUT) #CE
	RPIO.output(CSN, True)
	RPIO.output(CE, False)
	spi.open(0,0) # wybieramy wolny CE na RPi, nie bedziemy z niego korzystac
	
	
def register_read(reg):
	RPIO.output(CSN, False)
	spi.xfer2([reg])
	val = spi.xfer2([0x00])
	RPIO.output(CSN, True)
	return val[0]
	
def buffer_read(reg, len):
	RPIO.output(CSN, False)
	spi.xfer2([reg])
	val = []
	for i in range(0, len):
		val.append(spi.xfer2([0x00]))
	RPIO.output(CSN, True)
	return val
	
def register_write(reg, val):
	RPIO.output(CSN, False)
	if reg < 0x20:
		reg |= 0x20
	spi.xfer2([reg])
	if type(val) in (tuple, list):
		for entry in val:
			spi.xfer2([entry])
	elif type(val) == str:
		for entry in val:
			spi.xfer2([ord(entry)])
	else:
		spi.xfer2([val])
	RPIO.output(CSN, True)
	
def bank(b):
	st = 0x80 & register_read(RFM73_REG_STATUS)
	if(( st and ( b == 0 )) or (( st == 0 ) and b )):
		register_write(RFM73_CMD_ACTIVATE, 0x53)
		
def is_alive():
	st1 = register_read( RFM73_REG_STATUS)
	register_write( RFM73_CMD_ACTIVATE, 0x53 )
	st2 = register_read( RFM73_REG_STATUS )
	register_write( RFM73_CMD_ACTIVATE, 0x53 )
	print st1, st2
	return ( st1 ^ st2 ) == 0x80
	
def receive_mode():
	RPIO.output(CE, False)
	register_write( RFM73_CMD_FLUSH_RX, 0 )
	value = register_read( RFM73_REG_STATUS )
	register_write( RFM73_REG_STATUS ,value ) 

	value = register_read( RFM73_REG_CONFIG )
	value = value | 0x03
	register_write( RFM73_REG_CONFIG, value)
	RPIO.output(CE, True)
	
def transmit_mode():
	register_write( RFM73_CMD_FLUSH_TX, 0)
	value = register_read(RFM73_REG_STATUS)
	register_write( RFM73_REG_STATUS,value)
   
	RPIO.output(CE, False)
	value = register_read( RFM73_REG_CONFIG )
	value = value & 0xFE
	value = value | 0x02
	register_write( RFM73_REG_CONFIG, value )
	RPIO.output(CE, True)
	
def transmit(data):
	
	if type(data) in (list, tuple, str) and len(data) > 32:
		print "err"
		return
		
	register_write(RFM73_CMD_W_TX_PAYLOAD, data)
	
def next_pipe():
	status = register_read(RFM73_REG_STATUS)
	return (status >> 1) & 0x07
		
def receive():
	p = next_pipe()
	if p == 0x07:
		return False
	len = register_read(RFM73_CMD_R_RX_PL_WID)
	return buffer_read(RFM73_CMD_R_RX_PAYLOAD, len)