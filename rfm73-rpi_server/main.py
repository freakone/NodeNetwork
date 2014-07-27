import rfm73
import time

def rfm_interrupt(gpio_id, val):
	status = rfm73.register_read(0x07) #read status reg
	
	if status & (1 << 6): #rx data
		print "int:", rfm73.receive()
	if status & (1 << 4): #max_rt
		print "packet not sent"
	rfm73.register_write(0x07, status) #clear int flags
	
	
rfm73.init(rfm_interrupt)
rfm73.init_banks()
rfm73.receive_mode()

while 1:
	time.sleep(0.1)